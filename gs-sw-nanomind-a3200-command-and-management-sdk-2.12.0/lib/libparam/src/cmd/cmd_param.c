/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include <gs/param/host_command.h>

#include <gs/param/store/store.h>
#include "../host_table_local.h"
#include "../store/store_local.h"
#include <gs/param/internal/table.h>
#include <gs/param/internal/types.h>
#include <gs/util/gosh/command.h>
#include <gs/util/string.h>
#include <stdio.h>
#include <stdlib.h>

static gs_param_table_instance_t * cmd_tinst;

static gs_param_table_instance_t * get_tinst(void)
{
    // Find first table registered
    for (unsigned int id = 0; (cmd_tinst == NULL) && (id < 100); ++id) {
        cmd_tinst = gs_param_table_by_id(id);
    };
    return cmd_tinst;
}

static void fprintf_table(FILE * out, const gs_param_table_instance_t * tinst)
{
    fprintf(out, "  %s (%u)\r\n",
            gs_param_table_get_name(tinst),
            gs_param_table_get_id(tinst));
}

// iterator context
typedef struct {
    gs_command_context_t * ctx;
    gs_param_table_instance_t * first;
    bool do_read;
    uint32_t flags;
} iter_table_t;

void convert_show_options(const char * options, iter_table_t * it)
{
    gs_string_to_bool(options, &it->do_read);
    if (strchr(options, 'h')) {
        it->do_read = true;
        it->flags |= GS_PARAM_F_SHOW_HEX;
    }
    if (strchr(options, 's')) {
        it->do_read = true;
        it->flags |= GS_PARAM_F_SHOW_SCIENTIFIC;
    }
}

static bool iter_table_completer(void *ctx_in, gs_param_table_instance_t * tinst)
{
    iter_table_t * ctx = ctx_in;
    const unsigned int hits = gs_command_completer_add_token(ctx->ctx, tinst->name, false);
    if (hits == 1) {
        ctx->first = tinst;
    } else {
        if (hits == 2) {
            fprintf(ctx->ctx->out, "\r\n");
            fprintf_table(ctx->ctx->out, ctx->first);
        }
        fprintf_table(ctx->ctx->out, tinst);
    }
    return true;
}

static gs_error_t cmd_table_completer(gs_command_context_t * ctx, int table_arg)
{
    iter_table_t iter = {.ctx = ctx};
    char name[50];
    snprintf(name, sizeof(name), "%s*", (ctx->argc > table_arg) ? ctx->argv[table_arg] : "");
    gs_param_table_iterate(name, &iter, iter_table_completer);
    return GS_OK;
}

// iterator context
typedef struct {
    gs_command_context_t * ctx;
    const gs_param_store_t * first;
} iter_store_t;

static bool iter_store_completer(void *ctx_in, const gs_param_store_t * store)
{
    iter_store_t * ctx = ctx_in;
    unsigned int hits = gs_command_completer_add_token(ctx->ctx, store->name, false);
    if (hits == 1) {
        ctx->first = store;
    } else {
        if (hits == 2) {
            fprintf(ctx->ctx->out, "\r\n  %s\r\n", ctx->first->name);
        }
        fprintf(ctx->ctx->out, "  %s\r\n", store->name);
    }
    return true;
}

static gs_error_t cmd_store_completer(gs_command_context_t * ctx, int store_arg, const char * table)
{
    iter_store_t iter = {.ctx = ctx};
    char name[50];
    snprintf(name, sizeof(name), "%s*", (ctx->argc > store_arg) ? ctx->argv[store_arg] : "");
    gs_param_store_iterate(name, &iter, iter_store_completer);
    return GS_OK;
}

static gs_error_t get_table_name(gs_command_context_t * ctx, int arg_idx, bool use_selected, const char ** name)
{
    if (ctx->argc > arg_idx) {
        if (gs_string_has_wildcards(ctx->argv[arg_idx])) {
            *name = ctx->argv[arg_idx];
            return GS_OK;
        }
        gs_param_table_instance_t * tinst = NULL;
        gs_error_t error = gs_param_table_by_name2(ctx->argv[arg_idx], &tinst);
        if (error == GS_OK) {
            *name = tinst->name;
        }
        return error;
    }
    if (use_selected) {
        gs_param_table_instance_t * tinst = get_tinst();
        if (tinst) {
            *name = tinst->name;
            return GS_OK;
        }
    }
    return GS_ERROR_NOT_FOUND;
}

static gs_error_t cmd_table_arg1_completer(gs_command_context_t * ctx, int arg_to_complete)
{
    if (arg_to_complete == 1) {
        return cmd_table_completer(ctx, 1);
    }
    return GS_ERROR_AMBIGUOUS;
}

static int cmd_select(gs_command_context_t *ctx)
{
    if (ctx->argc > 1) {
        return gs_param_table_by_name2(ctx->argv[1], &cmd_tinst);
    }

    gs_param_table_instance_t * tinst = get_tinst();
    if (tinst == NULL) {
        fprintf(ctx->out, "  No table selected yet\r\n");
    } else {
        fprintf(ctx->out, "  %s (%u)\r\n", gs_param_table_name(tinst), tinst->id);
    }
    return GS_OK;
}

static bool iter_list_parameters(void *ctx_in, gs_param_table_instance_t * tinst)
{
    iter_table_t * ctx = ctx_in;
    fprintf(ctx->ctx->out, "Table %s (%u):\r\n", tinst->name, tinst->id);
    gs_param_list_to_stream(tinst, ctx->do_read, ctx->flags, ctx->ctx->out);
    return true;
}

static int cmd_list_parameters(gs_command_context_t *ctx)
{
    const char * name;
    gs_error_t error = get_table_name(ctx, 1, true, &name);
    if (error == GS_OK) {
        iter_table_t iter = {.ctx = ctx, .do_read = true};
        if (ctx->argc > 2) {
            convert_show_options(ctx->argv[2], &iter);
        }
        error = gs_param_table_iterate(name, &iter, iter_list_parameters);
    }
    return error;
}

static int cmd_tableinfo(gs_command_context_t *ctx)
{
    if (ctx->argc > 1) {
        // show specified table
        gs_param_table_instance_t * tinst;
        gs_error_t error = gs_param_table_by_name2(ctx->argv[1], &tinst);
        if (error) {
            return error;
        }
        uint8_t detail_level = 0;
        if (ctx->argc > 2) {
            gs_string_to_uint8(ctx->argv[2], &detail_level);
        }

        fprintf(ctx->out, "name               = \"%s\"\r\n", gs_param_table_get_name(tinst));
        fprintf(ctx->out, "id                 = %u\r\n", gs_param_table_get_id(tinst));

        if (detail_level)
            fprintf(ctx->out, "rows               = %p\r\n", tinst->rows);
        fprintf(ctx->out, "row_count          = %u\r\n", tinst->row_count);

        if (detail_level)
            fprintf(ctx->out, "memory             = %p\r\n", tinst->memory);
        fprintf(ctx->out, "memory_size        = %u\r\n", tinst->memory_size);

        fprintf(ctx->out, "checksum (host)    = %u\r\n", gs_param_table_checksum(tinst));
        fprintf(ctx->out, "checksum (le)      = %u\r\n", gs_param_table_checksum_le(tinst));
        fprintf(ctx->out, "checksum (be)      = %u\r\n", gs_param_table_checksum_be(tinst));

        fprintf(ctx->out, "stores             = \"%s\"\r\n", tinst->stores ? tinst->stores : "");
        fprintf(ctx->out, "ap-store           = \"%s\"\r\n", tinst->auto_persist.store ? tinst->auto_persist.store : "");

        if (detail_level) {
            fprintf(ctx->out, "ap-set             = %p\r\n", tinst->auto_persist.set);
            fprintf(ctx->out, "ap-ctx1            = %p\r\n", tinst->auto_persist.context1);
            fprintf(ctx->out, "ap-ctx2            = %p\r\n", tinst->auto_persist.context2);

            fprintf(ctx->out, "lock               = %p\r\n", tinst->lock);

            fprintf(ctx->out, "function-if get    = %p\r\n", tinst->function_interface.get);
            fprintf(ctx->out, "function-if set    = %p\r\n", tinst->function_interface.set);
            fprintf(ctx->out, "function-if ctx    = %p\r\n", tinst->function_interface.context);
            
            fprintf(ctx->out, "callback           = %p\r\n", tinst->callback);
            fprintf(ctx->out, "memory initializer = %p\r\n", tinst->initializer_function);
            fprintf(ctx->out, "flags              = 0x%x\r\n", (unsigned int) tinst->flags);
        }

    } else {

        // show all tables
        unsigned int found = 0;
        for (unsigned int i = 0; i < gs_param_table_max_instances(); i++) {

            const gs_param_table_instance_t * tinst = gs_param_table_raw(i);

            if ((tinst == NULL) || (tinst->rows == NULL)) {
                continue;
            }

            if (found == 0) {
                fprintf(ctx->out, "%-3s %s\r\n", "id", "name");
            }
            ++found;
            fprintf(ctx->out, "%3u %s\r\n",
                    gs_param_table_get_id(tinst),
                    gs_param_table_get_name(tinst));
        }
    }

    return GS_OK;
}

static int cmd_set(gs_command_context_t *ctx)
{
    gs_param_table_instance_t * tinst = get_tinst();
    if (tinst == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    char shortname[GS_PARAM_MAX_NAME + 20];
    uint8_t array_index = 0;
    if (gs_param_parse_name_and_array_index(ctx->argv[1], shortname, sizeof(shortname), &array_index, NULL)) {
        return GS_ERROR_ARG;
    }

    const gs_param_table_row_t * param = gs_param_row_by_name(shortname, tinst->rows, tinst->row_count);
    if (param == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    const unsigned int param_array_size = GS_PARAM_ARRAY_SIZE(param);
    const unsigned int param_size = GS_PARAM_SIZE(param);
    const gs_param_type_t param_type = GS_PARAM_TYPE(param);

    if (array_index >= param_array_size) {
        return GS_ERROR_RANGE;
    }

    // Check arg count vs. array size
    if ((ctx->argc - 2) > (int)(param_array_size - array_index)) {
        return GS_ERROR_OVERFLOW;
    }

    // parse and set argument(s)
    int res = GS_OK;
    for (int i = 0; (i < (ctx->argc - 2)) && (res == GS_OK); i++) {
        uint8_t value[param_size];
        res = gs_param_from_string(param, ctx->argv[i + 2], value);
        if (res == GS_OK) {
            res = gs_param_set(tinst, GS_PARAM_ADDR(param) + (param_size * (array_index + i)), param_type,
                               value, param_size, GS_PARAM_FLAGS(param));
        }
    }

    return res;
}

static gs_error_t cmd_parameter_completer(gs_command_context_t * ctx, int arg_to_complete)
{
    if (arg_to_complete != 1) {
        return GS_ERROR_AMBIGUOUS;
    }

    gs_param_table_instance_t * tinst = get_tinst();
    if ((tinst == NULL) || (tinst->rows == NULL)) {
        return GS_ERROR_NOT_FOUND;
    }

    const char * parameter = NULL;
    size_t len = 0;
    if (ctx->argc > 1) {
        parameter = ctx->argv[1];
        len = strlen(parameter);
    }

    const gs_param_table_row_t * first = NULL;
    for (unsigned int i = 0; i < tinst->row_count; i++) {
        if ((len == 0) || (strncmp(parameter, tinst->rows[i].name, len) == 0)) {
            const unsigned int hits = gs_command_completer_add_token(ctx, tinst->rows[i].name, false);
            if (hits == 1) {
                // wait for more hits
                first = &tinst->rows[i];
            } else {
                if (hits == 2) {
                    fprintf(ctx->out, "\r\n");
                    gs_param_list_single_to_stream(tinst, first, true, 0, ctx->out);
                }
                gs_param_list_single_to_stream(tinst, &tinst->rows[i], true, 0, ctx->out);
            }
        }
    }

    return GS_OK;
}

static int cmd_get(gs_command_context_t *ctx)
{
    gs_param_table_instance_t * tinst = get_tinst();
    if (tinst == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    char shortname[GS_PARAM_MAX_NAME + 20];
    uint8_t array_index = 0;
    bool is_array;
    if (gs_param_parse_name_and_array_index(ctx->argv[1], shortname, sizeof(shortname), &array_index, &is_array)) {
        return GS_ERROR_ARG;
    }

    iter_table_t it = {.flags = 0};
    if (ctx->argc > 2) {
        convert_show_options(ctx->argv[2], &it);
    }

    const gs_param_table_row_t * param = gs_param_row_by_name(shortname, tinst->rows, tinst->row_count);
    if (param == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    const unsigned int param_array_size = GS_PARAM_ARRAY_SIZE(param);
    const unsigned int param_size = GS_PARAM_SIZE(param);
    const gs_param_type_t param_type = GS_PARAM_TYPE(param);

    if (array_index >= param_array_size) {
        return GS_ERROR_RANGE;
    }

    if (is_array && (param_array_size <= 1)) {
        is_array = false;
    }

    uint8_t value[param_size];
    char buf[100];
    int res = GS_OK;
    if (is_array) {
        fprintf(ctx->out, "  %s[%u] = ", shortname, array_index);
    } else {
        fprintf(ctx->out, "  %s = ", shortname);
    }
    for (unsigned int i = array_index; (i < param_array_size) && (res == GS_OK); i++) {
        res = gs_param_get(tinst, GS_PARAM_ADDR(param) + (param_size * i), param_type, value, param_size, 0);
        if (res == GS_OK) {
            gs_param_to_string2(param, value, false, it.flags, buf, sizeof(buf), 0, NULL);
            fprintf(ctx->out, "%s ", buf);
        }
        if (is_array) {
            break;
        }
    }
    fprintf(ctx->out, "\r\n");

    return res;
}

static gs_error_t cmd_table_store_completer(gs_command_context_t * ctx, int arg_to_complete)
{
    if (arg_to_complete == 1) {
        return cmd_table_completer(ctx, 1);
    }
    if (arg_to_complete == 2) {
        gs_param_table_instance_t * tinst = NULL;
        gs_error_t error = gs_param_table_by_name2(ctx->argv[1], &tinst);
        if ((error == GS_OK) && gs_string_empty(tinst->stores) == false) {
            const size_t cmp_len = (arg_to_complete < ctx->argc) ? strlen(ctx->argv[arg_to_complete]) : 0;
            const size_t lstores = strnlen(tinst->stores, 100);
            char tmpstores[lstores + 1];
            strncpy(tmpstores, tinst->stores, lstores);
            tmpstores[lstores] = 0;
            char * tmp;
            char * store = strtok_r(tmpstores, ",", &tmp);
            char first[50] = "";
            while (store) {
                if ((cmp_len == 0) || (strncasecmp(store, ctx->argv[arg_to_complete], cmp_len) == 0)) {
                    const unsigned int hits = gs_command_completer_add_token(ctx, store, false);
                    if (hits == 1) {
                        // wait for more hits
                        GS_STRNCPY(first, store);
                    } else {
                        if (hits == 2) {
                            fprintf(ctx->out, "\r\n  %s\r\n", first);
                        }
                        fprintf(ctx->out, "  %s\r\n", store);
                    }
                }
                store = strtok_r(NULL, ",", &tmp);
            }
            return GS_OK;
        }
    }
    return GS_ERROR_NOT_FOUND;
}

static int cmd_load(gs_command_context_t *ctx)
{
    gs_param_table_instance_t * tinst;
    gs_error_t error = gs_param_table_by_name2(ctx->argv[1], &tinst);
    if (error) {
        return error;
    }

    const char * store = NULL;
    if (ctx->argc > 2) {
        store = ctx->argv[2];
    }

    const char * slot = tinst->name;
    if (ctx->argc > 3) {
        slot = ctx->argv[3];
    }

    return gs_param_table_load(tinst, store, slot, 0);
}

static int cmd_save(gs_command_context_t *ctx)
{
    gs_param_table_instance_t * tinst;
    gs_error_t error = gs_param_table_by_name2(ctx->argv[1], &tinst);
    if (error) {
        return error;
    }

    const char * store = NULL;
    if (ctx->argc > 2) {
        store = ctx->argv[2];
    }

    const char * slot = NULL;
    if (ctx->argc > 3) {
        slot = ctx->argv[3];
    }

    return gs_param_table_save(tinst, store, slot, 0);
}

static gs_error_t cmd_store_arg1_completer(gs_command_context_t * ctx, int arg_to_complete)
{
    if (arg_to_complete == 1) {
        return cmd_store_completer(ctx, 1, NULL);
    }
    return GS_ERROR_NOT_FOUND;
}

static int cmd_storeinfo(gs_command_context_t *ctx)
{
    const char * store = NULL;
    uint8_t detail_level = 0;
    if (ctx->argc > 1) {
        detail_level = 1;
        store = ctx->argv[1];
    }
    if (ctx->argc > 2) {
        gs_string_to_uint8(ctx->argv[2], &detail_level);
    }
    return gs_param_store_info(store, detail_level, ctx->out);
}

static int cmd_param_clear_slot(gs_command_context_t *ctx)
{
    return gs_param_store_clear(ctx->argv[1], ctx->argv[2], 0);
}

static int cmd_param_lock_store(gs_command_context_t *ctx)
{
    return gs_param_store_lock(ctx->argv[1], true, 0);
}

static int cmd_param_unlock_store(gs_command_context_t *ctx)
{
    return gs_param_store_lock(ctx->argv[1], false, 0);
}

static int cmd_export(gs_command_context_t *ctx)
{
    gs_param_table_instance_t * tinst = get_tinst();
    if (ctx->argc > 1) {
        gs_error_t error = gs_param_table_by_name2(ctx->argv[1], &tinst);
        if (error) {
            return error;
        }
    }

    fprintf(ctx->out, "param mem %u\r\n", tinst->id); // 'mem' is legacy command for select

    int res = GS_OK;
    for (unsigned int i = 0; (i < tinst->row_count) && (res == GS_OK); i++) {
        const gs_param_table_row_t * param = &tinst->rows[i];

        char name_buf[GS_PARAM_MAX_NAME + 1];
        GS_PGM_MEMCPY(name_buf, param->name, GS_PARAM_MAX_NAME);
        name_buf[GS_PARAM_MAX_NAME] = 0;
        fprintf(ctx->out, "param set %-16s ", name_buf);
        
        const unsigned int param_type = GS_PARAM_TYPE(param);
        const unsigned int param_array_size = GS_PARAM_ARRAY_SIZE(param);
        const unsigned int param_size = GS_PARAM_SIZE(param);
        uint8_t value[param_size];
        for (unsigned int j = 0; (j < param_array_size) && (res == GS_OK); j++) {
            char value_buf[100];
            res = gs_param_get(tinst, GS_PARAM_ADDR(param) + (param_size * j), param_type, value, param_size, 0);
            if (res == GS_OK) {
                res = gs_param_to_string2(param, value, false, GS_PARAM_F_SHOW_SCIENTIFIC, value_buf, sizeof(value_buf), 0, NULL);
                if (res == GS_OK) {
                    fprintf(ctx->out, " %s", value_buf);
                }
            }
        }

        fprintf(ctx->out, "\r\n");
    }
    return res;
}

static const gs_command_t GS_COMMAND_SUB param_subcommands[] = {
    {
        .name = "select",
        .help = "Select working table",
        .usage = "[table name|id]",
        .completer = cmd_table_arg1_completer,
        .handler = cmd_select,
        .optional_args = 1,
    },{
        .name = "list",
        .help = "List all parameters",
        .usage = "[table name|id] [0|1|h|s]",
        .completer = cmd_table_arg1_completer,
        .handler = cmd_list_parameters,
        .optional_args = 2,
    },{
        .name = "tableinfo",
        .help = "Show table information",
        .usage = "[table name|id] [detail-level]",
        .completer = cmd_table_arg1_completer,
        .handler = cmd_tableinfo,
        .optional_args = 2,
    },{
        .name = "export",
        .help = "Export parameters to stdout",
        .usage = "[table name|id]",
        .completer = cmd_table_arg1_completer,
        .handler = cmd_export,
        .optional_args = 1,
    },{
        .name = "set",
        .help = "Set parameter value",
        .usage = "<parameter> <value> [value] ...",
        .completer = cmd_parameter_completer,
        .handler = cmd_set,
        .mandatory_args = 2,
        .optional_args = 100,
    },{
        .name = "get",
        .help = "Get parameter value",
        .usage = "<parameter> [h|s]",
        .completer = cmd_parameter_completer,
        .handler = cmd_get,
        .mandatory_args = 1,
        .optional_args = 1,
    },{
        .name = "load",
        .help = "Load table",
        .usage = "<table name|id> [store] [slot]",
        .completer = cmd_table_store_completer,
        .handler = cmd_load,
        .mandatory_args = 1,
        .optional_args = 2,
    },{
        .name = "save",
        .help = "Save table",
        .usage = "<table name|id> [store] [slot]",
        .completer = cmd_table_store_completer,
        .handler = cmd_save,
        .mandatory_args = 1,
        .optional_args = 2,
    },{
        .name = "storeinfo",
        .help = "Show store information",
        .usage = "[store] [detail-level]",
        .completer = cmd_store_arg1_completer,
        .handler = cmd_storeinfo,
        .optional_args = 2,
    },{
        .name = "clear",
        .help = "Clear/invalidate store slot",
        .usage = "<store> <slot>",
        .completer = cmd_store_arg1_completer,
        .handler = cmd_param_clear_slot,
        .mandatory_args = 2,
    },{
        .name = "lock",
        .help = "Lock a store",
        .usage = "<store>",
        .completer = cmd_store_arg1_completer,
        .handler = cmd_param_lock_store,
        .mandatory_args = 1,
    },{
        .name = "unlock",
        .help = "Unlock a store",
        .usage = "<store>",
        .completer = cmd_store_arg1_completer,
        .handler = cmd_param_unlock_store,
        .mandatory_args = 1,
    },{
        .name = "mem",
        .handler = cmd_select,
        .mode = GS_COMMAND_FLAG_HIDDEN,
        .optional_args = 1,
    },{
        .name = "tables",
        .handler = cmd_tableinfo,
        .mode = GS_COMMAND_FLAG_HIDDEN,
        .optional_args = 2,
    }
};

static const gs_command_t GS_COMMAND_ROOT param_commands[] = {
    {
        .name = "param",
        .help = "Parameter System (local)",
        .chain = GS_COMMAND_INIT_CHAIN(param_subcommands),
    },
};

gs_error_t gs_param_register_commands(void)
{
    return GS_COMMAND_REGISTER(param_commands);
}
