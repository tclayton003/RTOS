/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "store_local.h"
#include <gs/param/table.h>
#include <gs/param/internal/types.h>
#include <gs/util/check.h>
#include "../host_log.h"
#include "../host_table_local.h"

#define PSTORE_NAME(pstore)  (pstore ? pstore->name : "")

const gs_param_store_t * g_stores;

gs_error_t gs_param_store_register(const gs_param_store_t * map)
{
    g_stores = map;
    return GS_OK;
}

static gs_error_t find_store(const char * store, bool partial_match, const gs_param_store_t ** return_pstore)
{
    *return_pstore = NULL;
    if (gs_string_empty(store) == false) {
        unsigned int pcount = 0;
        for (const gs_param_store_t * pstore = g_stores; pstore && pstore->name && pstore->name[0]; ++pstore) {
            if (strcasecmp(pstore->name, store) == 0) {
                *return_pstore = pstore;
                pcount = 1;
                break;
            }
            if (strcasecmp(pstore->name, "*") == 0) {
                *return_pstore = pstore;
                pcount = 1;
                break;
            }
            if (partial_match && (strncasecmp(pstore->name, store, strlen(store)) == 0)) {
                *return_pstore = pstore;
                ++pcount;
            }
        }
        if (pcount == 1) {
            return ((*return_pstore)->drv) ? GS_OK : GS_ERROR_NOT_SUPPORTED;
        }
        if (pcount > 1) {
            *return_pstore = NULL;
            return GS_ERROR_AMBIGUOUS;
        }
    }
    return GS_ERROR_NOT_FOUND;
}

static const char * get_store_by_index(const char * stores, unsigned int idx, char * buf, size_t buf_size)
{
    const char * ss = stores;
    for (unsigned int i = 0; (ss != NULL) && (i < idx); ++i) {
        ss = strchr(ss, ',');
        if (ss) {
            ++ss;
        }
    }
    if (ss) {
        const char * es = strchr(ss, ',');
        size_t len;
        if (es) {
            len = (es - ss);
        } else {
            len = strlen(ss);
        }
        if ((len + 1) < buf_size) {
            strncpy(buf, ss, len);
            buf[len] = 0;
            return buf;
        }
    }
    return NULL;
}

static const char * get_slot(gs_param_table_instance_t * tinst, char * buf, size_t buf_size, uint32_t flags)
{
    const char * prefix;
    if (flags & GS_PARAM_STORE_F_SETUP_AUTO_PERSIST) {
        prefix = GS_PARAM_STORE_AP_PREFIX;
    } else {
        prefix = "";
    }

    // if table has a name, use it
    if (tinst->name && tinst->name[0]) {
        snprintf(buf, buf_size, "%s%s", prefix, tinst->name);
    } else {
        snprintf(buf, buf_size, "%s%u", prefix, tinst->id);
    }
    return buf;
}

static gs_error_t gs_param_store_load_auto_persist(gs_param_table_instance_t * tinst,
                                                   uint32_t flags)
{
    if (flags & GS_PARAM_STORE_F_SKIP_AUTO_PERSIST) {
        return GS_OK;
    }

    if (gs_string_empty(tinst->auto_persist.store)) {
        // no auto-persist store configured
        return GS_OK;
    }

    char slot[GS_STORE_MAX_SLOT_NAME + 1];
    get_slot(tinst, slot, sizeof(slot), flags | GS_PARAM_STORE_F_SETUP_AUTO_PERSIST);

    const gs_param_store_t * pstore;
    gs_error_t error = find_store(tinst->auto_persist.store, false, &pstore);
    if (error == GS_OK) {
        error = pstore->drv->load_auto_persist ? (pstore->drv->load_auto_persist)(pstore, tinst, slot, flags) : GS_ERROR_NOT_IMPLEMENTED;
    }

    if (error == GS_OK) {
        log_info("Table [%s] (id=%u) loaded auto-persist from [%s:%s]", tinst->name, tinst->id, PSTORE_NAME(pstore), slot);
    } else {
        log_warning("Table [%s] (id=%u) failed to load auto-persist from [%s:%s], error: %s",
                    tinst->name, tinst->id, PSTORE_NAME(pstore), slot, gs_error_string(error));
    }
    return error;
}

gs_error_t gs_param_table_load(gs_param_table_instance_t * tinst,
                               const char * store, const char * slot,
                               uint32_t flags)
{
    GS_CHECK_HANDLE(tinst != NULL);

    char tmp_slot[GS_STORE_MAX_SLOT_NAME + 1];
    if (gs_string_empty(slot)) {
        slot = get_slot(tinst, tmp_slot, sizeof(tmp_slot), flags);
    }

    char tmp_store[GS_STORE_MAX_STORE_NAME + 1];
    if (gs_string_empty(store)) {
        store = get_store_by_index(tinst->stores, 0, tmp_store, sizeof(tmp_store));
        if (store == NULL) {
            return GS_ERROR_NOT_SUPPORTED;
        }
    }
    const gs_param_store_t * pstore;
    gs_error_t error = find_store(store, true, &pstore);
    if (error == GS_OK) {
        error = pstore->drv->load ? pstore->drv->load(pstore, tinst, slot, flags | GS_PARAM_F_NO_CALLBACK) : GS_ERROR_NOT_IMPLEMENTED;
    }

    if (error == GS_OK) {
        log_info("Table [%s] (id=%u) loaded from [%s:%s]", tinst->name, tinst->id, PSTORE_NAME(pstore), slot);

        if ((flags & GS_PARAM_F_NO_CALLBACK) == 0) {
            gs_param_table_call_callback_for_all_addresses(tinst);
        }
    } else {
        log_debug("Failed to load table [%s] (id=%u) from store [%s:%s], error: %s", tinst->name, tinst->id,
                  PSTORE_NAME(pstore), slot, gs_error_string(error));
    }

    return error;
}

gs_error_t gs_param_table_load_fallback(gs_param_table_instance_t * tinst,
                                        uint32_t flags)
{
    GS_CHECK_HANDLE(tinst != NULL);

    bool store_configured = true;
    gs_error_t error = GS_ERROR_NOT_FOUND;
    if (gs_string_empty(tinst->stores)) {
        store_configured = false;
    } else {
        char tmp_store[GS_STORE_MAX_STORE_NAME + 1];
        for (unsigned int idx = 0; (idx < 10) && (error != GS_OK); ++idx) {
            const char * store = get_store_by_index(tinst->stores, idx, tmp_store, sizeof(tmp_store));
            if (store == NULL) {
                break;
            }
            error = gs_param_table_load(tinst, store, NULL, flags | (GS_PARAM_STORE_F_SKIP_AUTO_PERSIST | GS_PARAM_F_NO_CALLBACK));
            if (idx && (error == GS_OK)) {
                log_warning("Table [%s] (id=%u) loaded from fallback store [%s]", tinst->name, tinst->id, store);
            }
        }
    }
    if (error) {
        gs_param_table_memory_initialize(tinst);
        if (store_configured) {
            log_warning("Failed to load table [%s] (id=%u) - initialized to default values", tinst->name, tinst->id);
        }
        error = GS_OK;
    }

    gs_param_store_load_auto_persist(tinst, flags | GS_PARAM_F_NO_CALLBACK);

    if ((flags & GS_PARAM_F_NO_CALLBACK) == 0) {
        gs_param_table_call_callback_for_all_addresses(tinst);
    }

    return error;
}

gs_error_t gs_param_table_save(gs_param_table_instance_t * tinst,
                               const char * store, const char * slot,
                               uint32_t flags)
{
    GS_CHECK_HANDLE(tinst != NULL);

    char tmp_slot[GS_STORE_MAX_SLOT_NAME + 1];
    if (gs_string_empty(slot)) {
        slot = get_slot(tinst, tmp_slot, sizeof(tmp_slot), flags);
    }

    char tmp_store[GS_STORE_MAX_STORE_NAME + 1];
    if (gs_string_empty(store)) {
        store = get_store_by_index(tinst->stores, 0, tmp_store, sizeof(tmp_store));
        if (store == NULL) {
            return GS_ERROR_NOT_SUPPORTED;
        }
    }
    const gs_param_store_t * pstore;
    gs_error_t error = find_store(store, true, &pstore);
    if (error == GS_OK) {
        error = pstore->drv->save ? pstore->drv->save(pstore, tinst, slot, flags) : GS_ERROR_NOT_IMPLEMENTED;
    }

    if (error == GS_OK) {
        log_info("Table [%s] (id=%u) saved to [%s:%s]", tinst->name, tinst->id, PSTORE_NAME(pstore), slot);
    } else {
        log_error("Failed to save table [%s] (id=%u) to [%s:%s], error: %s%s",
                  tinst->name, tinst->id, PSTORE_NAME(pstore), slot, gs_error_string(error),
                  (error == GS_ERROR_DATA) ? " - perhaps the store is locked" : "");
    }

    return error;
}

gs_error_t gs_param_table_save_all(gs_param_table_instance_t * tinst,
                                   uint32_t flags)
{
    GS_CHECK_HANDLE(tinst != NULL);

    gs_error_t save_error = GS_OK;
    char tmp_store[GS_STORE_MAX_STORE_NAME + 1];
    unsigned int save_attempts = 0;
    for (unsigned int idx = 0; idx < 10; ++idx) {
        const char * store = get_store_by_index(tinst->stores, idx, tmp_store, sizeof(tmp_store));
        if (store == NULL) {
            break;
        }
        ++save_attempts;
        gs_error_t error = gs_param_table_save(tinst, store, NULL, flags | GS_PARAM_STORE_F_SKIP_AUTO_PERSIST);
        if (error && (save_error == GS_OK)) {
            save_error = error;
        }
    }

    if ((save_error == GS_OK) && (save_attempts == 0)) {
        return GS_ERROR_NOT_FOUND;
    }

    return save_error;
}

static gs_error_t gs_param_table_check(gs_param_table_instance_t * tinst,
                                       const char * store, const char * slot,
                                       uint32_t flags)
{
    GS_CHECK_HANDLE(tinst != NULL);

    char tmp_slot[GS_STORE_MAX_SLOT_NAME + 1];
    if (slot == NULL) {
        slot = get_slot(tinst, tmp_slot, sizeof(tmp_slot), flags);
    }

    const gs_param_store_t * pstore;
    gs_error_t error = find_store(store, true, &pstore);
    if (error == GS_OK) {
        error = pstore->drv->check ? pstore->drv->check(pstore, tinst, slot, flags) : GS_ERROR_NOT_IMPLEMENTED;
    }

    if (error) {
        log_debug("Failed to check table [%s] (id=%u) against store: [%s:%s], error: %s",
                  tinst->name, tinst->id, store, slot, gs_error_string(error));
    }

    return error;
}

gs_error_t gs_param_table_check_all(gs_param_table_instance_t * tinst, uint32_t flags)
{
    GS_CHECK_HANDLE(tinst != NULL);

    gs_error_t check_error = GS_ERROR_NOT_FOUND;
    char tmp_store[GS_STORE_MAX_STORE_NAME + 1];
    for (unsigned int idx = 0; idx < 10; ++idx) {
        const char * store = get_store_by_index(tinst->stores, idx, tmp_store, sizeof(tmp_store));
        if (store == NULL) {
            break;
        }
        gs_error_t error = gs_param_table_check(tinst, store, NULL, flags);
        if (error && ((check_error == GS_ERROR_NOT_FOUND) || (check_error == GS_ERROR_NOT_SUPPORTED))) {
            check_error = error;
        }
    }
    {
        const char * store = get_store_by_index(tinst->auto_persist.store, 0, tmp_store, sizeof(tmp_store));
        if (store) {
            gs_error_t error = gs_param_table_check(tinst, store, NULL, flags | GS_PARAM_STORE_F_SETUP_AUTO_PERSIST);
            if (error && ((check_error == GS_ERROR_NOT_FOUND) || (check_error == GS_ERROR_NOT_SUPPORTED))) {
                check_error = error;
            }
        }
    }

    return check_error;
}

gs_error_t gs_param_store_clear(const char * store, const char * slot, uint32_t flags)
{
    GS_CHECK_ARG((slot != NULL) && (slot[0] != 0));

    const gs_param_store_t * pstore;
    gs_error_t error = find_store(store, true, &pstore);
    if (error == GS_OK) {
        error = pstore->drv->clear ? (pstore->drv->clear)(pstore, slot, flags) : GS_ERROR_NOT_IMPLEMENTED;
    }
    return error;
}

gs_error_t gs_param_store_lock(const char * store, bool on, uint32_t flags)
{
    const gs_param_store_t * pstore;
    gs_error_t error = find_store(store, true, &pstore);
    if (error == GS_OK) {
        error = pstore->drv->lock ? (pstore->drv->lock)(pstore, on, flags) : GS_ERROR_NOT_IMPLEMENTED;
    }
    return error;
}

static gs_error_t store_info(const char * store, uint8_t detail_level, FILE * out)
{
    const gs_param_store_t * pstore;
    gs_error_t error = find_store(store, true, &pstore);
    if (error == GS_OK) {
        error = pstore->drv->info ? (pstore->drv->info)(pstore, detail_level, out) : GS_ERROR_NOT_IMPLEMENTED;
    }
    if (error) {
        fprintf(out, GS_STORE_PRINTF_NAME "failed to show info, %s\r\n", PSTORE_NAME(pstore), gs_error_string(error));
    }
    return error;
}

gs_error_t gs_param_store_info(const char * store, uint8_t detail_level, FILE * out)
{
    gs_error_t error = GS_OK;
    if (gs_string_empty(store) == false) {
        error = store_info(store, detail_level, out);
    } else {
        fprintf(out, GS_STORE_PRINTF_NAME "info\r\n", "name");

        unsigned int found = 0;
        for (const gs_param_store_t * pstore = g_stores; pstore && pstore->name; ++pstore) {
            ++found;
            store_info(pstore->name, detail_level, out);
        }
        if (found == 0) {
            error = GS_ERROR_NOT_FOUND;
        }
    }
    return error;
}

gs_error_t gs_param_store_iterate(const char * name_pattern, void * ctx, bool(*iter)(void *ctx, const gs_param_store_t * store))
{
    bool found = false;
    if (name_pattern && name_pattern[0]) {
        for (const gs_param_store_t * pstore = g_stores; pstore && pstore->name; ++pstore) {
            if (gs_string_match(name_pattern, pstore->name)) {
                found = true;
                if ((iter)(ctx, pstore) == false) {
                    return GS_OK;
                }
            }
        }
    }
    return found ? GS_OK : GS_ERROR_NOT_FOUND;
}
