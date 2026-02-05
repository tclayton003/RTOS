/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "vmem_common.h"
#include "../store_local.h"

void gs_param_store_vmem_format_vmem_entry(const char * store, char * buf, size_t bufsize)
{
    snprintf(buf, bufsize, "param-%s", store);
}

const gs_vmem_t * gs_param_store_vmem_get_vmem(const gs_param_store_t * store)
{
    if (store && store->name && store->drv_data) {
        char buf[128];
        gs_param_store_vmem_format_vmem_entry(store->name, buf, sizeof(buf));
        return gs_vmem_get_by_name(buf);
    }
    return NULL;
}

// ensure slot size is valid and fits within the vmem area
static gs_error_t validate_vmem_versus_slot(const gs_vmem_t * vmem, const gs_param_store_vmem_map_t * slot)
{
    if ((vmem->size == 0) || (slot->size == 0) || (vmem->size < (slot->start + slot->size))){
        return GS_ERROR_OVERFLOW;
    }
    return GS_OK;
}

gs_error_t gs_param_store_vmem_get_vmem_slot(const gs_param_store_t * store, const char * slot,
                                             const gs_vmem_t ** return_vmem, const gs_param_store_vmem_map_t ** return_slot)
{
    const gs_vmem_t * vmem = gs_param_store_vmem_get_vmem(store);
    if (vmem == NULL) {
        return GS_ERROR_NOT_FOUND;
    }
    *return_vmem = vmem;

    unsigned int slot_count = 0;
    const gs_param_store_vmem_map_t * partial_match = NULL;
    unsigned int partial_count = 0;
    for (const gs_param_store_vmem_map_t * map = store->drv_data; map->name != NULL; ++map, ++slot_count) {

        if (strcasecmp(slot, map->name) == 0) {
            *return_slot = map;
            return validate_vmem_versus_slot(*return_vmem, *return_slot);
        }

        if (strncasecmp(slot, map->name, strlen(slot)) == 0) {
            partial_match = map;
            ++partial_count;
        }
    }

    if (partial_count > 1) {
        return GS_ERROR_AMBIGUOUS;
    }
    if (partial_count == 1 ) {
        *return_slot = partial_match;
        return validate_vmem_versus_slot(*return_vmem, *return_slot);
    }

    uint8_t slot_index;
    gs_error_t error = gs_string_to_uint8(slot, &slot_index);
    if ((error == GS_OK) && (slot_index < slot_count)) {
        const gs_param_store_vmem_map_t * map = store->drv_data;
        *return_slot = &map[slot_index];
        return validate_vmem_versus_slot(*return_vmem, *return_slot);
    }

    return GS_ERROR_NOT_FOUND;
}

gs_error_t gs_param_store_vmem_clear_slot(const gs_param_store_t * store, const char * slot, uint32_t flags)
{
    const gs_vmem_t* vmem_entry;
    const gs_param_store_vmem_map_t* table_entry;
    gs_error_t error = gs_param_store_vmem_get_vmem_slot(store, slot, &vmem_entry, &table_entry);
    if (error == GS_OK) {

        uint8_t table_buf[table_entry->size];
        memset(table_buf, 0, table_entry->size);

        error = vmem_entry->drv->write(vmem_entry, vmem_entry->physmem.p + table_entry->start, table_buf, table_entry->size);
        if (error == GS_OK) {
            error = vmem_entry->drv->read(vmem_entry, table_buf, vmem_entry->physmem.p + table_entry->start, table_entry->size);
            if (error == GS_OK) {
                for (unsigned int i = 0; i < table_entry->size; ++i) {
                    if (table_buf[i] != 0) {
                        error = GS_ERROR_DATA;
                        break;
                    }
                }
            }
        }
    }
    return error;
}

gs_error_t gs_param_store_vmem_lock(const gs_param_store_t * store, bool on, uint32_t flags)
{
    char buf[128];
    gs_param_store_vmem_format_vmem_entry(store->name, buf, sizeof(buf));
    return gs_vmem_lock_by_name(buf, on);
}

static gs_error_t vmem_param_auto_persist_set(gs_param_table_instance_t * tinst, uint16_t addr, gs_param_type_t type,
                                              const void * item, size_t size, uint32_t flags)
{
    if (tinst == NULL || item == NULL) {
        return GS_ERROR_ARG;
    }

    const gs_vmem_t * vmem_entry = (const gs_vmem_t *) tinst->auto_persist.context1;
    const gs_param_store_vmem_map_t * slot_entry = (const gs_param_store_vmem_map_t*) tinst->auto_persist.context2;

    if (vmem_entry && slot_entry) {
        return vmem_entry->drv->write(vmem_entry, vmem_entry->physmem.p + slot_entry->start + addr, item, size);
    }

    return GS_ERROR_NOT_FOUND;
}

gs_error_t gs_param_store_vmem_check(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    if (tinst == NULL || slot == NULL) {
        return GS_ERROR_ARG;
    }

    const gs_vmem_t * vmem_entry = gs_param_store_vmem_get_vmem(store);
    if (vmem_entry == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    uint32_t acc_size = 0;
    unsigned int slot_found = 0;
    const gs_param_store_vmem_map_t * myslot = NULL;
    for (const gs_param_store_vmem_map_t * map = store->drv_data; map->name; ++map) {

        if (map->start < acc_size) {
            return GS_ERROR_OVERFLOW;
        }

        acc_size = map->start + map->size;

        if (acc_size > vmem_entry->size) {
            return GS_ERROR_OVERFLOW;
        }

        if (strcasecmp(slot, map->name) == 0) {
            myslot = map;
            ++slot_found;
        }
    }

    if (myslot == NULL) {
        return GS_ERROR_NOT_FOUND;
    }
    if (slot_found > 1) {
        return GS_ERROR_AMBIGUOUS;
    }

    if (tinst->memory_size > myslot->size) {
        return GS_ERROR_OVERFLOW;
    }

    if (flags & GS_PARAM_STORE_F_SETUP_AUTO_PERSIST) {
        tinst->auto_persist.set = vmem_param_auto_persist_set;
        tinst->auto_persist.context1 = (void*) vmem_entry;
        tinst->auto_persist.context2 = (void*) myslot;
    }

    return GS_OK;
}

gs_error_t gs_param_store_vmem_load_auto_persist(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    const gs_vmem_t* vmem_entry;
    const gs_param_store_vmem_map_t* table_entry;
    gs_error_t error = gs_param_store_vmem_get_vmem_slot(store, slot, &vmem_entry, &table_entry);
    if (error) {
        return error;
    }

    /* Loop through all parameters, and reload the once marked persistent */
    for (unsigned int i = 0; i < tinst->row_count; i++) {
        const gs_param_table_row_t * param = &tinst->rows[i];
        if (GS_PARAM_FLAGS(param) & GS_PARAM_F_AUTO_PERSIST) {
            const gs_param_type_t param_type = GS_PARAM_TYPE(param);
            const unsigned int  param_size = GS_PARAM_SIZE(param);
            const unsigned int array_size = GS_PARAM_ARRAY_SIZE(param);
            uint8_t value[param_size];
            for (unsigned int j = 0; j < array_size; j++) {
                const uint16_t addr = GS_PARAM_ADDR(param) + (param_size * j);
                error = vmem_entry->drv->read(vmem_entry, value, vmem_entry->physmem.p + table_entry->start + addr, param_size);
                if (error) {
                    return error;
                }
                error = gs_param_set(tinst, addr, param_type, value, param_size, flags);
                if (error) {
                    return error;
                }
            }
        }
    }

    return GS_OK;
}

gs_error_t gs_param_store_vmem_info(const gs_param_store_t * store, uint8_t detail_level, FILE * out)
{
    const gs_vmem_t * vmem_entry = gs_param_store_vmem_get_vmem(store);

    char vinfo[100] = "";
    if (vmem_entry) {
        gs_vmem_info_by_name(vmem_entry->name, vinfo, sizeof(vinfo));
    }

    fprintf(out, GS_STORE_PRINTF_NAME "%s\r\n", store->name, vinfo);

    if ((detail_level >= 1) && store->drv_data) {
        unsigned int slot = 0;
        for (const gs_param_store_vmem_map_t * map = store->drv_data; map->name; ++map, ++slot) {
            fprintf(out, "  %2u %-20s 0x%04x, size 0x%04x\r\n",
                    slot, map->name, map->start, map->size);
        }
    }
    return GS_OK;
}
