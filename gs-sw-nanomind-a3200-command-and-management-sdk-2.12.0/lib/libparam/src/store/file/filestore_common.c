/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include "filestore_common.h"

void gs_param_file_store_make_filename(const char * root, const char * slot, const char * ext, char * filename_buf, size_t buf_size)
{
    snprintf(filename_buf, buf_size, "%s/%s.%s", root, slot, ext);
}

gs_error_t gs_param_file_store_clear(const gs_param_store_t * store, const char * slot, const char * ext, uint32_t flags)
{
    const gs_param_store_file_map_t* store_map = store->drv_data;
    if ((store_map == NULL) || gs_string_empty(store_map->path)) {
        return GS_ERROR_ARG;
    }

    char param_file_name[MAX_FILE_PATH_LEN];
    gs_param_file_store_make_filename(store_map->path, slot, ext, param_file_name, sizeof(param_file_name));

    remove(param_file_name);

    return GS_OK;
}

gs_error_t gs_param_file_store_info(const gs_param_store_t * store, const char * ext, uint8_t detail_level, FILE * out)
{
    const gs_param_store_file_map_t* store_map = store->drv_data;

    fprintf(out, GS_STORE_PRINTF_NAME "file: %s/<table-name>.%s\r\n",
            store->name,
            (store_map != NULL) ? store_map->path : "",
            ext);

    return GS_OK;
}
