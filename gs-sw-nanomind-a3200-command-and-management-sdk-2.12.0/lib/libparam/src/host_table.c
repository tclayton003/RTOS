/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "host_table_local.h"
#include "host_log.h"
#include <gs/util/string.h>
#include <gs/util/check.h>
#include <stdlib.h>

/* Table instances */
static gs_param_table_instance_t g_table_instances[GS_PARAM_MAX_TABLES];

GS_LOG_GROUP(gs_param_log, "param", GS_LOG_CAT_PARAM, LOG_DEFAULT_MASK);

gs_error_t gs_param_host_init(void)
{
    gs_log_group_register(LOG_DEFAULT); // may return GS_OK or GS_ERROR_EXIST depending on libutil

    return GS_OK;
}

static gs_error_t gs_param_table_lock_init(gs_param_table_instance_t * tinst)
{
#if (GS_PARAM_DISABLE_LOCK)
    tinst->lock = NULL;
    return GS_OK;
#else
    gs_error_t error = gs_mutex_create(&tinst->lock);
    if (error) {
        tinst->lock = NULL;
    }
    return error;
#endif
}

unsigned int gs_param_table_max_instances(void)
{
    return GS_PARAM_MAX_TABLES;
}

gs_param_table_instance_t * gs_param_table_raw(gs_param_table_id_t table_id)
{
    if (table_id < GS_PARAM_MAX_TABLES) {
        return &g_table_instances[table_id];
    }
    return NULL;
}

gs_param_table_instance_t * gs_param_table_by_id(gs_param_table_id_t table_id)
{
    // check if table is on the exact index
    if ((table_id < GS_PARAM_MAX_TABLES) && (g_table_instances[table_id].id == table_id) && (g_table_instances[table_id].name)) {
        return &g_table_instances[table_id];
    }
    // loop through tables and check for it
    for (unsigned int i = (GS_PARAM_MAX_TABLES - 1); i < GS_PARAM_MAX_TABLES; --i) {
        if ((g_table_instances[i].id == table_id) && (g_table_instances[i].name)) {
            return &g_table_instances[i];
        }
    }
    return NULL;
}

gs_error_t gs_param_table_iterate(const char * name_pattern, void * ctx, bool(*iter)(void *ctx, gs_param_table_instance_t * tinst))
{
    bool found = false;
    if (name_pattern && name_pattern[0]) {
        for (unsigned int i = 0; i < GS_PARAM_MAX_TABLES; ++i) {
            if (gs_string_match(name_pattern, g_table_instances[i].name)) {
                found = true;
                if ((iter)(ctx, &g_table_instances[i]) == false) {
                    return GS_OK;
                }
            }
        }
    }
    return found ? GS_OK : GS_ERROR_NOT_FOUND;
}

gs_error_t gs_param_table_by_name2(const char * name, gs_param_table_instance_t ** return_tinst)
{
    if (name && name[0]) {
        const size_t name_len = strlen(name);
        unsigned int pcount = 0;
        for (unsigned int i = 0; i < GS_PARAM_MAX_TABLES; ++i) {
            if (g_table_instances[i].name) {
                if (strcasecmp(g_table_instances[i].name, name) == 0) {
                    *return_tinst = &g_table_instances[i];
                    return GS_OK;
                }
                if ((strncasecmp(g_table_instances[i].name, name, name_len) == 0) ||
                    gs_string_match(name, g_table_instances[i].name)) {
                    *return_tinst = &g_table_instances[i];
                    ++pcount;
                }
            }
        }
        if (pcount == 1) {
            return GS_OK;
        }
        if (pcount > 1) {
            return GS_ERROR_AMBIGUOUS;
        }
        uint8_t id;
        gs_error_t error = gs_string_to_uint8(name, &id);
        if (error == GS_OK) {
            *return_tinst = gs_param_table_by_id(id);
            if (*return_tinst) {
                return GS_OK;
            }
        }
    }
    return GS_ERROR_NOT_FOUND;
}

gs_param_table_instance_t * gs_param_table_by_name(const char * name)
{
    gs_param_table_instance_t * tinst;
    gs_error_t error = gs_param_table_by_name2(name, &tinst);
    return (error == GS_OK) ? tinst : NULL;
}

gs_error_t gs_param_table_memory_initialize_zero(gs_param_table_instance_t * tinst)
{
    if (tinst && tinst->memory && tinst->memory_size) {
        memset(tinst->memory, 0, tinst->memory_size);
        return GS_OK;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

gs_error_t gs_param_table_memory_initialize(gs_param_table_instance_t * tinst)
{
    GS_CHECK_HANDLE(tinst != NULL);

    gs_error_t error = GS_ERROR_NOT_SUPPORTED;
    gs_error_t locked = gs_param_table_lock(tinst);
    {
        // use default values if specified
        if (tinst->default_values) {
            gs_param_table_memory_initialize_zero(tinst); // clear fillers, could this be done better, keepig consistent <type> write
            error = gs_param_table_copy_from_memory(tinst, tinst->default_values, tinst->memory_size, GS_PARAM_F_NO_CALLBACK, false, NULL);
            if (error) {
                error = gs_param_table_memory_initialize_zero(tinst);
            }
        }
        // use initializer if specified
        if (tinst->initializer_function) {
            error = tinst->initializer_function(tinst);
        }
        // in case of errors, clear all values
        if (error) {
            error = gs_param_table_memory_initialize_zero(tinst);
        }
    }
    if (locked == GS_OK) {
        gs_param_table_unlock(tinst);
    }

    return error;
}

static gs_error_t param_table_init_or_create(const gs_param_table_create_t * create,
                                             gs_param_table_instance_t * tinst,
                                             gs_param_table_instance_t ** return_tinst)
{
    // validate arguments
    GS_CHECK_ARG(create != NULL);

    gs_param_host_init();

    if (tinst) {
        // initialize table intance

    } else {
        // create and initialize  "global" table

        // Validate table name
        if (gs_string_empty(create->name)) {
            return GS_ERROR_ARG;
        }
        if (strnlen(create->name, GS_PARAM_MAX_TABLE_NAME+1) > GS_PARAM_MAX_TABLE_NAME) {
            log_warning("%s: table name: [%s] exceeds max length: %d", __FUNCTION__, create->name, GS_PARAM_MAX_TABLE_NAME);
        }

        // check if index has already been set/initialized
        if (gs_param_table_by_id(create->id)) {
            log_error("%s: table id: %u is already in use", __FUNCTION__, create->id);
            return GS_ERROR_IN_USE;
        }
        if (gs_param_table_by_name(create->name)) {
            log_error("%s: table name: [%s] (table id: %u) is already in use", __FUNCTION__, create->name, create->id);
            return GS_ERROR_IN_USE;
        }

        // check if table can be added on exact index
        if ((create->id < GS_PARAM_MAX_TABLES) && (g_table_instances[create->id].name == NULL)) {
            tinst = &g_table_instances[create->id];
        } else {
            // look for free slot
            tinst = NULL;
            for (unsigned int i = (GS_PARAM_MAX_TABLES - 1); i < GS_PARAM_MAX_TABLES; --i) {
                if (g_table_instances[i].name == NULL) {
                    tinst = &g_table_instances[i];
                    break;
                }
            }
        }
        if (tinst == NULL) {
            return GS_ERROR_FULL;
        }
    }

    if (tinst == NULL) {
        // makes cppcheck happy, even if this can never happen
        return GS_ERROR_HANDLE;
    }

    void * memory = create->memory;
    if (create->flags & GS_PARAM_TABLE_F_ALLOC_MEMORY) {
        if (create->memory_size == 0) {
            return GS_ERROR_ARG;
        }
        memory = calloc(1, create->memory_size);
        if (memory == NULL) {
            return GS_ERROR_ALLOC;
        }
    }

    memset(tinst, 0, sizeof(*tinst));
    tinst->id = create->id;
    tinst->rows = create->rows;
    tinst->row_count = create->row_count;
    if (create->function) {
        tinst->function_interface = *create->function;
    }
    tinst->memory = memory;
    tinst->memory_size = create->memory_size;
    tinst->callback = create->callback;
    tinst->initializer_function = create->initializer;
    tinst->default_values = create->default_values;
    tinst->stores = create->stores;
    tinst->auto_persist.store = create->auto_persist;
    tinst->name = create->name;
    tinst->flags = create->flags;

    if ((create->flags & GS_PARAM_TABLE_F_NO_LOCK) == 0) {
        gs_param_table_lock_init(tinst);
    }

    gs_param_table_memory_initialize(tinst);

    gs_param_table_check_all(tinst, create->flags);

    if (return_tinst) {
        *return_tinst = tinst;
    }

    return GS_OK;
}

gs_error_t gs_param_table_create(const gs_param_table_create_t * create,
                                 gs_param_table_instance_t ** return_tinst)
{
    gs_error_t error = param_table_init_or_create(create, NULL, return_tinst);
    if (error) {
        // make sure to set tinst to NULL - in case returned error-code isn't checked before use
        if (return_tinst) {
            *return_tinst = NULL;
        }
    }
    return error;
}

gs_error_t gs_param_table_init(gs_param_table_instance_t * tinst,
                               const gs_param_table_create_t * create)
{
    GS_CHECK_HANDLE(tinst != NULL);
    return param_table_init_or_create(create, tinst, NULL);
}

const char * gs_param_table_get_name(const gs_param_table_instance_t * tinst)
{
    if (tinst && tinst->name) {
        return tinst->name;
    }
    return "";
}

gs_param_table_id_t gs_param_table_get_id(const gs_param_table_instance_t * tinst)
{
    if (tinst) {
        return tinst->id;
    }
    return GS_PARAM_UNDEFINED_TABLE_ID;
}

gs_error_t gs_param_table_copy_to_memory(gs_param_table_instance_t * tinst, void * memory, size_t memory_size, uint32_t flags)
{
    GS_CHECK_HANDLE(tinst != NULL);
    GS_CHECK_ARG((memory != NULL) && (memory_size > 0));

    memset(memory, 0, memory_size);

    const gs_param_table_row_t * row = tinst->rows;
    for (unsigned int i = 0; i < tinst->row_count; ++i, ++row) {

        const unsigned int param_size = GS_PARAM_SIZE(row);
        const unsigned int param_array_size = GS_PARAM_ARRAY_SIZE(row);

        for (unsigned int j = 0; j < param_array_size; ++j) {
            const uint16_t addr = GS_PARAM_ADDR(row) + (param_size * j);

            if ((addr + param_size) > memory_size) {
                return GS_ERROR_OVERFLOW;
            }

            gs_error_t error = gs_param_get(tinst, addr, GS_PARAM_TYPE(row), ((uint8_t*)memory) + addr, param_size, flags);
            if (error) {
                return error;
            }
        }
    }

    return GS_OK;
}

gs_error_t gs_param_table_copy_from_memory(gs_param_table_instance_t * tinst, const void * memory, size_t memory_size, uint32_t flags,
                                           bool allow_partial, bool * partial_copy)
{
    GS_CHECK_HANDLE(tinst != NULL);
    GS_CHECK_ARG((memory != NULL) && (memory_size > 0));

    if (partial_copy) {
        *partial_copy = false;
    }

    const gs_param_table_row_t * row = tinst->rows;
    for (unsigned int i = 0; i < tinst->row_count; ++i, ++row) {

        const unsigned int param_size = GS_PARAM_SIZE(row);
        const unsigned int param_array_size = GS_PARAM_ARRAY_SIZE(row);

        for (unsigned int j = 0; j < param_array_size; ++j) {
            const uint16_t addr = GS_PARAM_ADDR(row) + (param_size * j);

            if (addr >= memory_size) {
                if (allow_partial == false) {
                    return GS_ERROR_DATA;
                }
                
                // table expanded - bigger than memory, but it is still consistent
                if (partial_copy) {
                    *partial_copy = true;
                }
                return GS_OK;
            }

            if ((addr + param_size) > memory_size) {
                return GS_ERROR_OVERFLOW;
            }

            gs_error_t error = gs_param_set(tinst, addr, GS_PARAM_TYPE(row), ((const uint8_t*)memory) + addr,
                                            param_size, (flags | row->flags) & ~GS_PARAM_F_AUTO_PERSIST);
            if (error) {
                return error;
            }
        }
    }

    return GS_OK;
}

gs_error_t gs_param_table_call_callback_for_all_addresses(gs_param_table_instance_t * tinst)
{
    if (tinst && tinst->callback && tinst->rows && tinst->row_count) {
        const gs_param_table_row_t * row = tinst->rows;
        for (unsigned int i = 0; i < tinst->row_count; ++i, ++row) {
            const unsigned int param_size = GS_PARAM_SIZE(row);
            const unsigned int param_array_size = GS_PARAM_ARRAY_SIZE(row);

            for (unsigned int j = 0; j < param_array_size; ++j) {
                const uint16_t addr = GS_PARAM_ADDR(row) + (param_size * j);
                tinst->callback(addr, tinst);
            }
        }
        return GS_OK;
    }
    return GS_ERROR_NOT_SUPPORTED;
}
