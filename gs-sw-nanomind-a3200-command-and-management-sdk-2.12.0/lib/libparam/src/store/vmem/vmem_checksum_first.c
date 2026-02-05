/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "vmem_common.h"
#include <gs/param/table.h>
#include <gs/param/serialize.h>
#include <gs/util/vmem.h>
#include <gs/util/fletcher.h>
#include <gs/util/string.h>
#include <gs/util/byteorder.h>
#include <stdlib.h>
#include "../../host_table_local.h"
#include "../../host_log.h"

typedef struct __attribute__((__packed__)) vmem_storage_hdr_t {
    uint16_t  crc_table_meta;
    uint16_t  crc_table_data;
    uint16_t  sz_table_data;
    uint16_t  crc_hdr;
} vmem_storage_hdr_t;

static uint16_t calculate_table_desc_checksum_be(gs_param_table_instance_t * tinst, unsigned int rows)
{
    gs_fletcher16_t f16;
    gs_fletcher16_init(&f16);
    for (unsigned int i = 0; i < rows; ++i) {
        gs_param_table_row_t row = tinst->rows[i];
        row.addr = util_htons(row.addr);
        gs_fletcher16_update(&f16, &row, sizeof(row));
    }
    const uint16_t checksum = gs_fletcher16_finalize(&f16);
    return checksum;
}

/*
 * This local function checks if a table (description) has been updated in a way where table rows has been added
 * to the end of the table and the original part of the table remains unchanged.
 * In such a case we allow the existing table data to be loaded and let the newly added parameters be set to it's
 * default values.
 * */
static gs_error_t check_for_accumulative_table_update(gs_param_table_instance_t * tinst, vmem_storage_hdr_t *hdr)
{
    unsigned int skipped_rows = 1;

    uint16_t table_size = util_ntohs(hdr->sz_table_data);
    while (skipped_rows <= tinst->row_count)
    {
        if (table_size == gs_param_calc_table_size(tinst->rows, tinst->row_count - skipped_rows))
        {
            uint16_t table_desc_checksum = calculate_table_desc_checksum_be(tinst, (tinst->row_count - skipped_rows));
            if (hdr->crc_table_meta == table_desc_checksum)
            {
                gs_param_table_memory_initialize(tinst);
                log_warning("Table name: [%s] has expanded. Last: [%d] row(s) was set to default values", tinst->name, skipped_rows);
                return GS_OK;
            }
        }
        skipped_rows++;
    }
    return GS_ERROR_DATA;
}

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
    hdr.crc_table_meta = gs_param_table_checksum_be(tinst);
    uint16_t table_size  = gs_param_calc_table_size(tinst->rows, tinst->row_count);

    uint8_t table_buf[table_size] __attribute__ ((aligned (8)));
    error = gs_param_table_copy_to_memory(tinst, table_buf, table_size, GS_PARAM_SF_TO_BIG_ENDIAN);
    if (error == GS_OK) {

        hdr.crc_table_data = util_htons(gs_fletcher16(table_buf, table_size));
        hdr.sz_table_data = util_htons(table_size);
        hdr.crc_hdr = util_htons(gs_fletcher16(&hdr, sizeof(hdr) - sizeof(hdr.crc_hdr)));

        /* Write to VMEM */
        error = vmem_entry->drv->write(vmem_entry, vmem_entry->physmem.p + table_entry->start, &hdr, sizeof(hdr));
        if (error == GS_OK) {

            error = vmem_entry->drv->write(vmem_entry, vmem_entry->physmem.p + table_entry->start + sizeof(hdr), table_buf, table_size);
            if (error == GS_OK) {
                /* Verify that the table header was correctly written */
                vmem_storage_hdr_t hdr_rb;
                vmem_entry->drv->read(vmem_entry, &hdr_rb, vmem_entry->physmem.p + table_entry->start, sizeof(hdr_rb));
                if (memcmp(&hdr, &hdr_rb, sizeof(hdr)) != 0) {
                    //log_error("CRC does not match (wrote <0x%04X> != read <0x%04X>)", hdr.crc_hdr, hdr_rb.crc_hdr);
                    error = GS_ERROR_DATA;
                }
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
    vmem_entry->drv->read(vmem_entry, &hdr, vmem_entry->physmem.p + table_entry->start, sizeof(hdr));
    uint16_t table_size = util_ntohs(hdr.sz_table_data);

    if ((gs_fletcher16(&hdr, sizeof(hdr)-sizeof(hdr.crc_hdr)) != util_ntohs(hdr.crc_hdr)) || (table_size == 0))
        return GS_ERROR_DATA;

    /* Check HDR Table description CRC */
    if (hdr.crc_table_meta != gs_param_table_checksum_be(tinst)) {
        if (check_for_accumulative_table_update(tinst, &hdr) != GS_OK) {
            return GS_ERROR_DATA;
        }
    }

    /* Check Hdr Table data CRC */
    uint8_t table_buf[table_size] __attribute__ ((aligned (8)));
    vmem_entry->drv->read(vmem_entry, table_buf, vmem_entry->physmem.p + table_entry->start + sizeof(hdr), sizeof(table_buf));
    if (gs_fletcher16(table_buf, sizeof(table_buf)) != util_ntohs(hdr.crc_table_data)) {
        return GS_ERROR_DATA;
    }

    return gs_param_table_copy_from_memory(tinst, table_buf, table_size, GS_PARAM_SF_FROM_BIG_ENDIAN | flags, true, NULL);
}

const gs_param_store_driver_t gs_param_store_vmem_driver_checksum_first = {
    .save = vmem_param_save,
    .load = vmem_param_load,
    // common
    .check             = gs_param_store_vmem_check,
    .load_auto_persist = gs_param_store_vmem_load_auto_persist,
    .clear             = gs_param_store_vmem_clear_slot,
    .lock              = gs_param_store_vmem_lock,
    .info              = gs_param_store_vmem_info,
};
