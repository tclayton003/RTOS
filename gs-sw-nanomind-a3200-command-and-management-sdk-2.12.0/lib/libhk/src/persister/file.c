/* Copyright (c) 2013-2020 GomSpace A/S. All rights reserved. */

#include <persister/file.h>

#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gs/util/clock.h>
#include <gs/util/log.h>
#include <gs/util/check.h>
#include <gs/util/byteorder.h>
#include <gs/uthash/utlist.h>
#include <gs/hk/internal/internal_collector_store.h>
#include <gs/hk/param/host/hkcfg.h>

#define HK_FILE_MAGIC 0x686b0001

extern gs_log_group_t* hkpers;
#define LOG_DEFAULT hkpers

static gs_error_t noop_initializer(gs_param_table_instance_t* tinst) {
    return GS_OK;
}

static gs_error_t initialize_anonymous_table(gs_param_table_instance_t* tinst,
                                             gs_hk_table_cfg_t* hk_table,
                                             void* data_ptr) {
    size_t row_count = 0;
    const gs_param_table_row_t* rows =
            gs_param_table_get_rows(hk_table->collected_table,
                                    &row_count);
    const gs_param_table_create_t create = {
        .rows = rows,
        .row_count = row_count,
        .memory = data_ptr,
        .memory_size = hk_table->store->sample_size,
        .initializer = noop_initializer,
        .flags = GS_PARAM_TABLE_F_NO_LOCK
    };
    return gs_param_table_init(tinst, &create);
}

void file_create_filename(char* buf, size_t buf_size,
                          uint8_t node, uint8_t id) {
    char root_path[64];
    GS_HK_HKCFG_DATA_PATH_GET(root_path, sizeof(root_path));
    snprintf(buf, buf_size, "%s/%s/tbl_%03u_%03u.bin", root_path,
             GS_HK_CACHE_PATH_PREFIX, node, id);
}


gs_error_t file_save(gs_hk_table_cfg_t* hk_table, const char * filename, uint32_t max_elem)
{
    GS_CHECK_ARG(hk_table != NULL &&
                 filename != NULL &&
                 strlen(filename) < 128);

    /* Load data into local cache */
    gs_hk_sample_t* local_cache = NULL;
    gs_hk_sample_t* elem = NULL;
    uint32_t no_elem = 0;

    gs_error_t res = GS_OK;
    store_lock(hk_table->store);
    DL_FOREACH(hk_table->store->head, elem) {

        if (elem->data == NULL) {
            // we reached the end of the elements in use
            break;
        }

        // prepare anonymous table to use in copy
        GS_PARAM_TINST_VAR(tinst);
        res = initialize_anonymous_table(tinst, hk_table, elem->data);
        if (res != GS_OK) {
            log_warning("initialize_anonymous_table() failed, "
                        "data not stored to file, res=%s", gs_error_string(res));
            break;
        }

        // allocate room for element
        gs_hk_sample_t* local_elem = malloc(sizeof(gs_hk_sample_t));
        if (local_elem == NULL) {
            log_warning("allocation failed, data not stored to file");
            break;
        }

        local_elem->data = malloc(hk_table->store->sample_size);
        if (local_elem->data == NULL) {
            log_warning("allocation failed, data not stored to file");
            free(local_elem);
            break;
        }

        // copy into local element
        res = gs_param_table_copy_to_memory(tinst, local_elem->data,
                                            hk_table->store->sample_size,
                                            GS_PARAM_SF_TO_BIG_ENDIAN);
        if (res != GS_OK) {
            log_warning("gs_param_table_copy_to_memory() failed, "
                        "data not stored to file, res=%s", gs_error_string(res));
            free(local_elem->data);
            free(local_elem);
            break;
        }
        local_elem->timestamp = elem->timestamp;

        // add local element to local cache
        DL_PREPEND(local_cache, local_elem);

        if (++no_elem >= max_elem) {
            break;
        }
    }
    store_unlock(hk_table->store);

    if (res == GS_OK) {
        // open file to write to
        char hk_filename_new[128 + 4];
        snprintf(hk_filename_new, sizeof(hk_filename_new), "%s.new", filename);
        FILE * fp = fopen(hk_filename_new, "w+");

        if (fp != NULL) {
            log_trace("now writing samples to file, samples=%"PRIu32", "
                      "file=%s, samplesize=%lu", no_elem, filename,
                      (unsigned long) hk_table->store->sample_size);

            // write file header - big endian
            uint32_t magic = util_hton32(HK_FILE_MAGIC);
            fwrite(&magic, 1, sizeof(uint32_t), fp);
            timestamp_t clock_now;
            gs_clock_get_time(&clock_now);
            uint32_t ts_save = util_hton32(clock_now.tv_sec);
            fwrite(&ts_save, 1, sizeof(uint32_t), fp);
            uint16_t table_crc = util_hton16(hk_table->collected_table_checksum);
            fwrite(&table_crc, 1, sizeof(uint16_t), fp);
            uint32_t elements_written = util_hton32(no_elem);
            fwrite(&elements_written, 1, sizeof(uint32_t), fp);

            // write data
            gs_hk_sample_t* tmp = NULL;
            gs_hk_sample_t* elem2 = NULL;
            DL_FOREACH_SAFE(local_cache, elem2, tmp) {
                uint32_t timestamp = elem2->timestamp;
                fwrite(&timestamp, 1, sizeof(uint32_t), fp);
                fwrite(elem2->data, 1, hk_table->store->sample_size, fp);
                free(elem2->data);
                free(elem2);
            }

            fsync(fileno(fp));
            fclose(fp);

            remove(filename);
            rename(hk_filename_new, filename);
            hk_table->last_persist = clock_now.tv_sec;

            log_trace("done writing samples to file=%s", filename);
        } else {
            gs_hk_sample_t* tmp = NULL;
            gs_hk_sample_t* elem2 = NULL;
            log_warning("hk file_save() - fopen failed on:%s", filename);
            DL_FOREACH_SAFE(local_cache, elem2, tmp) {
                free(elem2->data);
                free(elem2);
            }
            res = GS_ERROR_IO;
        }
    }

    return res;
}

gs_error_t file_load(gs_hk_table_cfg_t * hk_table, const char * filename,
                     uint32_t max_elem)
{
    GS_CHECK_ARG(hk_table != NULL &&
                 filename != NULL &&
                 strlen(filename) < 128);

    FILE * fp = fopen(filename, "r+");
    if (fp == NULL) {
        return GS_ERROR_IO;
    }
    gs_error_t retval = GS_ERROR_DATA;

    // read file header
    uint32_t magic = 0;
    size_t bread = fread(&magic, 1, sizeof(uint32_t), fp);
    magic = util_ntoh32(magic);
    uint32_t ts_save = 0;
    bread += fread(&ts_save, 1, sizeof(uint32_t), fp);
    ts_save = util_ntoh32(ts_save);
    uint16_t table_crc = 0;
    bread += fread(&table_crc, 1, sizeof(uint16_t), fp);
    table_crc = util_ntoh16(table_crc);
    uint32_t no_elem = 0;
    bread += fread(&no_elem, 1, sizeof(uint32_t), fp);
    no_elem = util_ntoh32(no_elem);
    if (magic != HK_FILE_MAGIC) {
        log_warning("bad magic while reading file header, file=%s, "
                    "magic=%"PRIu32", expected=%i",
                    filename, magic, HK_FILE_MAGIC);
    } else if (table_crc != hk_table->collected_table_checksum) {
        log_warning("table checksum does not match in file header, file=%s, crc=%i, expected=%i",
                    filename, table_crc, hk_table->collected_table_checksum);
    } else if (bread != 14) {
        log_warning("malformed file header, file=%s, bytes read=%lu, expected=14",
                    filename, (unsigned long) bread);
    } else if (no_elem < 1) {
        log_debug("no data in file, file=%s", filename);
        retval = GS_ERROR_NO_DATA;
    } else {
        uint32_t cnt = 0;
        if (no_elem > max_elem) {
            no_elem = max_elem;
        }

        // load data
        while (cnt < no_elem) {

            // read timestamp
            uint32_t timestamp = 0;
            if (fread(&timestamp, 1, sizeof(uint32_t), fp) != sizeof(uint32_t)) {
                log_warning("failed to read timestamp, data not loaded from file");
                break;
            }

            // read packed data
            void* data_raw = malloc(hk_table->store->sample_size);
            if (data_raw == NULL) {
                log_warning("allocation failed, data not loaded from file");
                break;
            }
            if (fread(data_raw, 1, hk_table->store->sample_size, fp) != hk_table->store->sample_size) {
                log_warning("failed to read data, data not loaded from file");
                free(data_raw);
                break;
            }

            // prepare anonymous table to use in copy
            GS_PARAM_TINST_VAR(tinst);
            void* data = malloc(hk_table->store->sample_size);
            retval = initialize_anonymous_table(tinst, hk_table, data);
            if (retval != GS_OK) {
                log_warning("initialize_anonymous_table() failed, "
                            "data not loaded from file, res=%s",
                            gs_error_string(retval));
                free(data);
                free(data_raw);
                break;
            }

            // copy using anom table
            retval = gs_param_table_copy_from_memory(
                tinst, data_raw,
                hk_table->store->sample_size, GS_PARAM_SF_FROM_BIG_ENDIAN,
                false, NULL);
            if (retval != GS_OK) {
                log_warning("gs_param_table_copy_from_memory() failed, res=%s",
                            gs_error_string(retval));
            }

            // add to store
            log_trace("adding sample from file, ts=%"PRIu32", filename=%s",
                      timestamp, filename);
            retval = store_add_sample(hk_table->store, data,
                                      hk_table->store->sample_size, timestamp);
            if (retval != GS_OK) {
                log_warning("store_add_sample_failed, retval=%s",
                            gs_error_string(retval));
                free(data);
                free(data_raw);
                break;
            }
            free(data);
            free(data_raw);
            cnt++;
            retval = GS_OK;
        }
    }
    fclose(fp);
    return retval;
}

