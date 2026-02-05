/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "filestore_common.h"
#include <gs/param/internal/types.h>
#include <gs/param/serialize.h>
#include <gs/util/fletcher.h>
#include <gs/util/string.h>
#include <gs/util/byteorder.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../../host_table_local.h"
#include "../../host_log.h"

#define FILE_EXTENSION  "bin"

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

/* This local function checks if a table (description) has been updated in a way where table rows has been added
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

static gs_error_t file_store_auto_persist_set(gs_param_table_instance_t * tinst, uint16_t addr, gs_param_type_t type, const void * item, size_t size, uint32_t flags)
{
    FILE* auto_persist_fp = (FILE*)tinst->auto_persist.context1;

    if (auto_persist_fp == NULL) {
        log_debug("%s: FP is NULL", __FUNCTION__);
        return GS_ERROR_NOT_FOUND;
    }

    if (fseek(auto_persist_fp, addr, SEEK_SET) != 0) {
        log_debug("%s: fseek(addr: %u, SEEK_SET) failed", __FUNCTION__, addr);
        return GS_ERROR_IO;
    }

    if (fwrite(item, 1, size, auto_persist_fp) != size) {
        log_debug("%s: fwrite() failed", __FUNCTION__);
        return GS_ERROR_IO;
    }

    fflush(auto_persist_fp);

    return GS_OK;
}

static gs_error_t file_store_check(const gs_param_store_t* store, gs_param_table_instance_t* tinst, const char* slot,
                                   uint32_t flags)
{
    if (slot == NULL) {
        return GS_ERROR_ARG;
    }

    const gs_param_store_file_map_t* store_map = store->drv_data;

    if ((store_map == NULL) || gs_string_empty(store_map->path)) {
        return GS_ERROR_ARG;
    }

    char param_file_name[MAX_FILE_PATH_LEN];
    gs_param_file_store_make_filename(store_map->path, slot, FILE_EXTENSION, param_file_name, sizeof(param_file_name));

    FILE* fp = NULL;
    if (flags & GS_PARAM_STORE_F_SETUP_AUTO_PERSIST) {
        fp = fopen(param_file_name, "rb+");
        if (fp == NULL) {
            fp = fopen(param_file_name, "wb+");
        }
    } else {
        fp = fopen(param_file_name, "rb");
    }

    if (fp == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    if (flags & GS_PARAM_STORE_F_SETUP_AUTO_PERSIST) {
        /* For auto-persist files, we keep them open for faster parameters saving. */
        tinst->auto_persist.set = file_store_auto_persist_set;
        tinst->auto_persist.context1 = (void*)fp;
        tinst->auto_persist.context2 = NULL; /* Not used */
    } else {
        fclose(fp);
    }

    return GS_OK;
}

static gs_error_t file_store_save(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    if (slot == NULL)
        return GS_ERROR_ARG;

    const gs_param_store_file_map_t* store_map = store->drv_data;

    if ((store_map == NULL) || gs_string_empty(store_map->path)) {
        return GS_ERROR_ARG;
    }

    char param_file_name[MAX_FILE_PATH_LEN];
    gs_param_file_store_make_filename(store_map->path, slot, FILE_EXTENSION, param_file_name, sizeof(param_file_name));
    FILE* fp = fopen(param_file_name, "wb");
    if (fp == NULL)
    {
        log_error("Failed saving table: [%s]. Couldn't create file: %s", tinst->name, param_file_name);
        return GS_ERROR_IO;
    }

    uint16_t table_size = gs_param_calc_table_size(tinst->rows, tinst->row_count);
    /* Calculate table definition checksum */
    vmem_storage_hdr_t hdr;
    hdr.crc_table_meta = gs_param_table_checksum_be(tinst);
    hdr.sz_table_data  = util_htons(table_size);

    gs_error_t ret;
    uint8_t table_buf[table_size] __attribute__ ((aligned (8)));
    memset(table_buf, 0, sizeof(table_buf));

    ret = gs_param_table_copy_to_memory(tinst, table_buf, sizeof(table_buf), GS_PARAM_SF_TO_BIG_ENDIAN);
    if (ret != GS_OK) {
        fclose(fp);
        return ret;
    }

    hdr.crc_table_data = util_htons(gs_fletcher16(table_buf, sizeof(table_buf)));
    hdr.crc_hdr = util_htons(gs_fletcher16(&hdr, sizeof(hdr) - sizeof(hdr.crc_hdr)));

    /* Write to file */
    ret = GS_OK;
    size_t bytesWritten = 0;
    bytesWritten += fwrite(&hdr, 1, sizeof(hdr), fp);
    bytesWritten += fwrite(table_buf, 1, sizeof(table_buf), fp);
    if (bytesWritten != (sizeof(table_buf) + sizeof(hdr)))
    {
        ret = GS_ERROR_IO;
    }

    fclose(fp);
    return ret;
}

static gs_error_t file_store_load(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    if (slot == NULL)
        return GS_ERROR_ARG;

    const gs_param_store_file_map_t* store_map = store->drv_data;

    if ((store_map == NULL) || gs_string_empty(store_map->path)) {
        return GS_ERROR_ARG;
    }

    char param_file_name[MAX_FILE_PATH_LEN];
    gs_param_file_store_make_filename(store_map->path, slot, FILE_EXTENSION, param_file_name, sizeof(param_file_name));
    FILE* fp = fopen(param_file_name, "rb");
    if (fp == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    gs_error_t ret = GS_ERROR_DATA;
    /* Calculate table definition checksum */
    vmem_storage_hdr_t hdr;
    if (fread(&hdr, 1, sizeof(hdr), fp) == sizeof(hdr))
    {
        uint16_t table_size = util_ntohs(hdr.sz_table_data);

        /* Check HDR Table description CRC */
        if ((gs_fletcher16(&hdr, sizeof(hdr)-sizeof(hdr.crc_hdr)) == util_ntohs(hdr.crc_hdr)) &&
            (table_size != 0))
        {
            if (gs_param_table_checksum_be(tinst) != hdr.crc_table_meta)
            {
                if (check_for_accumulative_table_update(tinst, &hdr) == GS_OK)
                {
                    ret = GS_OK;
                }
            }
            else
            {
                /* Check HDR Table size */
                if (table_size == gs_param_calc_table_size(tinst->rows, tinst->row_count))
                {
                    ret = GS_OK;
                }
            }

            if (ret == GS_OK)
            {
                ret = GS_ERROR_DATA;
                /* Check Hdr Table data CRC */
                uint8_t table_buf[table_size] __attribute__ ((aligned (8)));
                if (fread(table_buf, 1, sizeof(table_buf), fp) == sizeof(table_buf))
                {
                    if (gs_fletcher16(table_buf, sizeof(table_buf)) == util_ntohs(hdr.crc_table_data))
                    {
                        ret = gs_param_table_copy_from_memory(tinst, table_buf, sizeof(table_buf), GS_PARAM_SF_FROM_BIG_ENDIAN | flags, true, NULL);
                    }
                }
            }
        }
    }

    fclose(fp);
    return ret;
}

static gs_error_t file_store_load_auto_persist(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    const gs_param_store_file_map_t* store_map = store->drv_data;

    if ((store_map == NULL) || gs_string_empty(store_map->path)) {
        return GS_ERROR_ARG;
    }

    char param_file_name[MAX_FILE_PATH_LEN];
    gs_param_file_store_make_filename(store_map->path, slot, FILE_EXTENSION, param_file_name, sizeof(param_file_name));
    FILE* fp = (FILE*)tinst->auto_persist.context1;
    if (fp == NULL) {
        return GS_ERROR_NOT_FOUND;
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
                // don't fail due to seek error - the parameter may not have been auto-persistet yet.
                if (fseek(fp, addr, SEEK_SET) == 0) {
                    if (fread(value, 1, param_size, fp) == param_size) {
                        gs_param_set(tinst, addr, param_type, value, param_size, flags);
                    }
                }
            }
        }
    }
    return GS_OK;
}

static gs_error_t file_store_clear(const gs_param_store_t * store, const char * slot, uint32_t flags)
{
    return gs_param_file_store_clear(store, slot, FILE_EXTENSION, flags);
}

static gs_error_t file_store_info(const gs_param_store_t * store, uint8_t detail_level, FILE * out)
{
    return gs_param_file_store_info(store, FILE_EXTENSION, detail_level, out);
}

const gs_param_store_driver_t gs_param_store_file_driver_bin = {
    .check = file_store_check,
    .save  = file_store_save,
    .load  = file_store_load,
    .load_auto_persist = file_store_load_auto_persist,
    .clear = file_store_clear,
    .info  = file_store_info,
};
