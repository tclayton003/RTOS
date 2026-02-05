/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "vmem_common.h"
#include <gs/param/table.h>
#include <gs/util/vmem.h>
#include <gs/util/fletcher.h>
#include <gs/util/string.h>
#include "../../host_table_local.h"

typedef struct __attribute__((__packed__)) vmem_storage_hdr_t {
    uint16_t  crc_table_data;
    uint16_t  crc_table_meta;
} vmem_storage_hdr_t;

static gs_error_t vmem_param_save(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    if (tinst == NULL) {
        return GS_ERROR_ARG;
    }

    const gs_vmem_t* vmem_entry;
    const gs_param_store_vmem_map_t* table_entry;
    gs_error_t error = gs_param_store_vmem_get_vmem_slot(store, slot, &vmem_entry, &table_entry);
    if (error) {
        return error;
    }

    if (tinst->memory_size > table_entry->size) {
        return GS_ERROR_OVERFLOW;
    }

    /* Calculate table definition checksum */
    vmem_storage_hdr_t hdr;
    hdr.crc_table_meta = gs_param_table_checksum(tinst);
    //printf("Table CRC %u\r\n", hdr.crc_table_meta);

    uint8_t table_buf[tinst->memory_size] __attribute__ ((aligned (8)));
    memset(table_buf, 0, sizeof(table_buf));

    error = gs_param_table_copy_to_memory(tinst, table_buf, tinst->memory_size, 0);
    if (error) {
        return error;
    }

    hdr.crc_table_data = gs_fletcher16(table_buf, sizeof(table_buf));
    //printf("Data CRC %u\r\n", hdr.crc_table_data);

    /* Write to VMEM */
    error = vmem_entry->drv->write(vmem_entry, vmem_entry->physmem.p + table_entry->start, table_buf, sizeof(table_buf));
    if (error == GS_OK) {

        error = vmem_entry->drv->write(vmem_entry, vmem_entry->physmem.p + table_entry->start + table_entry->size - sizeof(hdr), &hdr, sizeof(hdr));
        if (error == GS_OK) {
            /* Verify that the table header was correctly written */
            vmem_storage_hdr_t hdr_rb;
            vmem_entry->drv->read(vmem_entry, &hdr_rb, vmem_entry->physmem.p + table_entry->start + table_entry->size - sizeof(hdr_rb), sizeof(hdr_rb));
            if (memcmp(&hdr, &hdr_rb, sizeof(hdr)) != 0) {
                error = GS_ERROR_DATA;
            }
        }
    }

    return error;
}

static gs_error_t vmem_param_load(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    if (tinst == NULL) {
        return GS_ERROR_ARG;
    }

    const gs_vmem_t* vmem_entry;
    const gs_param_store_vmem_map_t* table_entry;
    gs_error_t error = gs_param_store_vmem_get_vmem_slot(store, slot, &vmem_entry, &table_entry);
    if (error) {
        return error;
    }

    if (tinst->memory_size > table_entry->size) {
        return GS_ERROR_OVERFLOW;
    }

    if (table_entry->size < sizeof(vmem_storage_hdr_t)) {
        return GS_ERROR_OVERFLOW;
    }

    /* Check Header CRC to validate valid data in Storage location */
    vmem_storage_hdr_t hdr;
    vmem_entry->drv->read(vmem_entry, &hdr, vmem_entry->physmem.p + table_entry->start + table_entry->size - sizeof(hdr), sizeof(hdr));

    /* Check HDR Table description CRC */
    if (hdr.crc_table_meta != gs_param_table_checksum(tinst)) {
        return GS_ERROR_DATA;
    }

    /* Check Hdr Table data CRC */
    uint8_t table_buf[tinst->memory_size];
    vmem_entry->drv->read(vmem_entry, table_buf, vmem_entry->physmem.p + table_entry->start, sizeof(table_buf));
    if (gs_fletcher16(table_buf, sizeof(table_buf)) != hdr.crc_table_data) {
        return GS_ERROR_DATA;
    }

    return gs_param_table_copy_from_memory(tinst, table_buf, sizeof(table_buf), flags, false, NULL);
}

const gs_param_store_driver_t gs_param_store_vmem_driver_checksum_last = {
    .save = vmem_param_save,
    .load = vmem_param_load,
    // common
    .check             = gs_param_store_vmem_check,
    .load_auto_persist = gs_param_store_vmem_load_auto_persist,
    .clear             = gs_param_store_vmem_clear_slot,
    .lock              = gs_param_store_vmem_lock,
    .info              = gs_param_store_vmem_info,
};
