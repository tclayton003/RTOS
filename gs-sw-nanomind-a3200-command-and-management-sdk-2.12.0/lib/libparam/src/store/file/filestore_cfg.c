/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "filestore_common.h"
#include <gs/param/internal/types.h>
#include <gs/util/string.h>
#include <gs/util/gosh/command.h> // borrow argc/argv splitter
#include <stdlib.h>
#include "../../host_table_local.h"
#include "../../host_log.h"

#define FILE_EXTENSION  "cfg"

static gs_error_t file_store_check(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    const gs_param_store_file_map_t* store_map = store->drv_data;
    if ((store_map == NULL) || gs_string_empty(store_map->path)) {
        return GS_ERROR_ARG;
    }

    if (flags & GS_PARAM_STORE_F_SETUP_AUTO_PERSIST) {
        log_debug("table; [%s] - auto-persist is not supported for .%s", tinst->name, FILE_EXTENSION);
    }

    return GS_OK;
}

static gs_error_t file_store_save(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    const gs_param_store_file_map_t* store_map = store->drv_data;
    if ((store_map == NULL) || gs_string_empty(store_map->path)) {
        return GS_ERROR_ARG;
    }

    if ((tinst->rows == NULL) || (tinst->row_count == 0)) {
        log_debug("table: [%s] - no rows defined, can't save table", tinst->name);
        return GS_ERROR_DATA;
    }

    char param_file_name[MAX_FILE_PATH_LEN];
    gs_param_file_store_make_filename(store_map->path, slot, FILE_EXTENSION, param_file_name, sizeof(param_file_name));
    FILE* fp = fopen(param_file_name, "w"); // file should be created under a tmp name, and moved when writen.
    if (fp == NULL) {
        log_debug("Failed saving table [%s] - couldn't create file [%s]", tinst->name, param_file_name);
        return GS_ERROR_IO;
    }

    fprintf(fp, "# Table %s (%d)\n", tinst->name, tinst->id);
    gs_error_t error = GS_OK;
    const gs_param_table_row_t * row = tinst->rows;
    for (unsigned int i = 0; (i < tinst->row_count) && (error == GS_OK); ++row, ++i) {

        fprintf(fp, " %-16.14s = ", row->name); // ensure missing NUL termination doesn't cause problems.

        const unsigned int param_array_size = GS_PARAM_ARRAY_SIZE(row);
        const unsigned int param_size = GS_PARAM_SIZE(row);
        uint8_t value[param_size];
        char buf[500];
        for (unsigned int j = 0; (j < param_array_size) && (error == GS_OK); ++j) {
            error = gs_param_get(tinst, row->addr + (param_size * j), row->type, value, param_size, 0);
            if (error == GS_OK) {
                error = gs_param_to_string2(row, value, false, GS_PARAM_F_SHOW_SCIENTIFIC, buf, sizeof(buf), 0, NULL);
                if (error == GS_OK) {
                    fprintf(fp,"%s ", buf);
                }
            }
        }
        fprintf(fp, "\n");
    }

    fclose(fp);

    if (error) {
        log_debug("Failed to save table: [%s] - deleting [%s] as it may be incomplete", tinst->name, param_file_name);
        remove(param_file_name);
        return error;
    }

    return error;
}

static gs_error_t file_store_load(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags)
{
    const gs_param_store_file_map_t* store_map = store->drv_data;
    if ((store_map == NULL) || gs_string_empty(store_map->path)) {
        return GS_ERROR_ARG;
    }

    char param_file_name[MAX_FILE_PATH_LEN];
    gs_param_file_store_make_filename(store_map->path, slot, FILE_EXTENSION, param_file_name, sizeof(param_file_name));
    FILE* fp = fopen(param_file_name, "r");
    if (fp == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    gs_param_table_memory_initialize(tinst); // set default value

    char buf[1000];
    gs_error_t error = GS_OK;
    unsigned int line = 0;
    while ((error == GS_OK) && fgets(buf, sizeof(buf), fp)) {
        buf[sizeof(buf) - 1] = 0;
        ++line;
        int argc = 0;
        char * argv[50];
        gs_command_build_argv(buf, &argc, argv, 50);
        if ((argc >= 3) && (argv[0][0] != '#')) {
            const gs_param_table_row_t * row = gs_param_row_by_name(argv[0], tinst->rows, tinst->row_count);
            if (row == NULL) {
                log_error("Failed to load table: [%s] - unknown parameter; [%s] in [%s:%u]", tinst->name, argv[0], param_file_name, line);
                error = GS_ERROR_DATA;
                break;
            }
            if (strcmp(argv[1], "=") != 0) {
                log_error("Failed to load table: [%s] - invalid separator [%s] in [%s:%u]", tinst->name, argv[1], param_file_name, line);
                error = GS_ERROR_DATA;
                break;
            }
            const unsigned int param_array_size = GS_PARAM_ARRAY_SIZE(row);
            const unsigned int param_size = GS_PARAM_SIZE(row);
            const gs_param_type_t param_type = GS_PARAM_TYPE(row);
            for (int i = 0; (i < (argc - 2)) && (i < (int)param_array_size) && (error == GS_OK); ++i) {
                uint8_t value[param_size];
                const char * data = argv[i + 2];
                error = gs_param_from_string(row, data, value);
                if (error) {
                    log_error("Failed to load table: [%s] - invalid data [%s] in [%s:%u:%u]", tinst->name, data, param_file_name, line, i);
                    break;
                }
                error = gs_param_set(tinst, GS_PARAM_ADDR(row) + (param_size * i), param_type, value, param_size, flags);
            }
        }
    }

    fclose(fp);

    if (error) {
        gs_param_table_memory_initialize(tinst); // set default values
    }

    return error;
}

static gs_error_t file_store_clear(const gs_param_store_t * store, const char * slot, uint32_t flags)
{
    return gs_param_file_store_clear(store, slot, FILE_EXTENSION, flags);
}

static gs_error_t file_store_info(const gs_param_store_t * store, uint8_t detail_level, FILE * out)
{
    return gs_param_file_store_info(store, FILE_EXTENSION, detail_level, out);
}

const gs_param_store_driver_t gs_param_store_file_driver_cfg = {
    .check = file_store_check,
    .save  = file_store_save,
    .load  = file_store_load,
    .clear = file_store_clear,
    .info  = file_store_info,
};
