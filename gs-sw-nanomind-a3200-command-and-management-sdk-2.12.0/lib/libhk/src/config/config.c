/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <sys/stat.h>
#include <stdlib.h>

#include <gs/hk/param/host/hkcfg.h>
#include <gs/hk/param/stores.h>
#include <gs/hk/config.h>
#include <gs/hk/internal/internal_config.h>
#include <persister/file.h>
#include <gs/util/log.h>
#include <gs/util/sem.h>
#include <gs/util/minmax.h>
#include <gs/util/check.h>
#include <gs/util/thread.h>
#include <gs/uthash/utlist.h>
#include <gs/param/rparam.h>
#include <cjson/cJSON.h>

#if (__linux__)
#include <dirent.h>
#else
#include <gs/storage/vfs/vfs.h> // -> GS_STORAGE_VFS
#endif

#if defined(GS_LOG_CAT_HK)
#define LOG_CATEGORY GS_LOG_CAT_HK
#else
#define LOG_CATEGORY GS_LOG_CAT_DEFAULT
#endif

GS_LOG_GROUP(hkcfg, "hk_config", LOG_CATEGORY, LOG_DEFAULT_MASK | LOG_INFO_MASK);
#define LOG_DEFAULT hkcfg

static gs_hk_table_cfg_t* tables = NULL;
static gs_hk_beacon_cfg_t* beacons = NULL;

#define MAX_READERS 4
#define VALID_HDL 0xEE
static gs_sem_t reader_semaphore;
static uint8_t initialized = 0;
static uint8_t loaded = 0;

bool gs_hk_check_priority(const char* priority) {
    if ((strcmp(priority, GS_HK_PRIORITY_LOWEST_NAME) == 0) || (strcmp(priority, GS_HK_PRIORITY_LOW_NAME) == 0) ||
       (strcmp(priority, GS_HK_PRIORITY_MEDIUM_NAME) == 0) || (strcmp(priority, GS_HK_PRIORITY_HIGH_NAME) == 0) ||
       (strcmp(priority, GS_HK_PRIORITY_HIGHEST_NAME) == 0)) {
        return true;
    }
    return false;
}

const char* gs_hk_priority_to_string(gs_hk_priority_t priority) {
    if (priority == GS_HK_PRIORITY_HIGHEST) {
        return GS_HK_PRIORITY_HIGHEST_NAME;
    } else if (priority == GS_HK_PRIORITY_HIGH) {
        return GS_HK_PRIORITY_HIGH_NAME;
    } else if (priority == GS_HK_PRIORITY_MEDIUM) {
        return GS_HK_PRIORITY_MEDIUM_NAME;
    } else if (priority == GS_HK_PRIORITY_LOW) {
        return GS_HK_PRIORITY_LOW_NAME;
    } else {
        return GS_HK_PRIORITY_LOWEST_NAME;
    }
}

gs_hk_priority_t gs_hk_priority_from_string(const char* priority) {
    gs_hk_priority_t ret_priority = GS_HK_PRIORITY_LOWEST;
    if (strcmp(priority, GS_HK_PRIORITY_LOWEST_NAME) == 0) {
        ret_priority = GS_HK_PRIORITY_LOWEST;
    } else if (strcmp(priority, GS_HK_PRIORITY_LOW_NAME) == 0) {
        ret_priority = GS_HK_PRIORITY_LOW;
    } else if (strcmp(priority, GS_HK_PRIORITY_MEDIUM_NAME) == 0) {
        ret_priority = GS_HK_PRIORITY_MEDIUM;
    } else if (strcmp(priority, GS_HK_PRIORITY_HIGH_NAME) == 0) {
        ret_priority = GS_HK_PRIORITY_HIGH;
    } else if (strcmp(priority, GS_HK_PRIORITY_HIGHEST_NAME) == 0) {
        ret_priority = GS_HK_PRIORITY_HIGHEST;
    }
    return ret_priority;
}

const char* gs_hk_beacon_policy_to_string(gs_hk_beacon_policy_t policy) {
    if (policy == GS_HK_AUTO_BEACONS_ALWAYS) {
        return "always";
    } else if (policy == GS_HK_AUTO_BEACONS_ONLY_COMPLETE) {
        return "complete";
    }
    return "off";
}

gs_hk_beacon_policy_t gs_hk_beacon_policy_from_string(const char* policy) {
    if (strcmp(policy, "always") == 0) {
        return GS_HK_AUTO_BEACONS_ALWAYS;
    } else if (strcmp(policy, "complete") == 0) {
        return GS_HK_AUTO_BEACONS_ONLY_COMPLETE;
    }
    return GS_HK_AUTO_BEACONS_OFF;
}

uint16_t interval_from_priority(gs_hk_priority_t priority) {
    uint16_t interval = 0;
    if (priority == GS_HK_PRIORITY_LOWEST) {
        interval = 180;
    } else if (priority == GS_HK_PRIORITY_LOW) {
        interval = 60;
    } else if (priority == GS_HK_PRIORITY_MEDIUM) {
        interval = 30;
    } else if (priority == GS_HK_PRIORITY_HIGH) {
        interval = 10;
    } else if (priority == GS_HK_PRIORITY_HIGHEST) {
        interval = 1;
    }
    return interval;
}

static void create_dir(const char* dir) {
    struct stat statbuf;
    if ((stat(dir, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode)) {
        if (mkdir(dir, 0777) != 0) {
            log_warning("error creating directory=%s", dir);
        }
    }
}
static void create_directories() {
    char root_path[GS_HK_HKCFG_DATA_PATH_LENGTH+1];
    GS_HK_HKCFG_DATA_PATH_GET(root_path, sizeof(root_path));
    create_dir(root_path);
    char tmp[50];
    snprintf(tmp, sizeof(tmp), "%s/%s", root_path, GS_HK_SPECS_PATH_PREFIX);
    create_dir(tmp);
    snprintf(tmp, sizeof(tmp), "%s/%s", root_path, GS_HK_CACHE_PATH_PREFIX);
    create_dir(tmp);
    snprintf(tmp, sizeof(tmp), "%s/%s", root_path, GS_HK_BCN_TYPES_PATH_PREFIX);
    create_dir(tmp);
}

bool gs_hk_config_is_initialized() {
    return (initialized == 1);
}

bool gs_hk_config_is_loaded() {
    return (loaded == 1);
}

void gs_hk_config_init() {

    gs_log_group_register(LOG_DEFAULT);

    if (initialized == 1) {
        return;
    }
    log_info("initializing housekeeping configuration");

    // create semaphores for read locking the configuration
    gs_sem_create(MAX_READERS, &reader_semaphore);

    // initialize configuration table
    gs_param_table_instance_t * tinst = NULL;
    const gs_param_table_create_t create = {
        .name = GS_HK_HKCFG_TABLE_NAME,
        .id = GS_HK_HKCFG_TABLE_MEM_ID,
        .rows = gs_hk_hkcfg_table,
        .row_count = GS_HK_HKCFG_TABLE_ROWS,
        .memory_size = GS_HK_HKCFG_TABLE_SIZE,
        .stores = GS_HK_PARAM_STORE,
        .flags = GS_PARAM_TABLE_F_ALLOC_MEMORY,
        .default_values = &gs_hk_hkcfg_default_values,
    };
    gs_param_table_create(&create, &tinst);
    gs_param_table_load_fallback(tinst, 0);

    initialized = 1;
    return;
}

gs_error_t gs_hk_config_open(gs_hk_config_handle_t* hdl) {
    gs_error_t res = gs_sem_wait(reader_semaphore, 5000);
    if (res == GS_OK) {
        *hdl = VALID_HDL;
    }
    return res;
}
gs_error_t gs_hk_config_close(gs_hk_config_handle_t hdl) {
    return gs_sem_post(reader_semaphore);
}


static bool obtain_write_lock_on_configuration() {
    for (int i=0; i<MAX_READERS; ++i) {
        if (gs_sem_wait(reader_semaphore, 5000) != GS_OK) {
            for (; i>=0; --i) {
                gs_sem_post(reader_semaphore);
            }
            return false;
        }
    }
    return true;
}

static void release_write_lock_on_configuration() {
    for (int i=0; i<MAX_READERS; ++i) {
        gs_sem_post(reader_semaphore);
    }
}

static bool is_handle_valid(gs_hk_config_handle_t hdl) {
    // this check could be made into something more meaningful if needed.
    return (hdl == VALID_HDL);
}

static void destroy_unused_tables() {
    if (tables != NULL) {
        gs_hk_table_cfg_t* elem = NULL;
        gs_hk_table_cfg_t* tmp = NULL;
        LL_FOREACH_SAFE(tables, elem, tmp) {
            if (elem->used_in_beacons_count < 1) {
                log_debug("table no longer in use - unloading, node=%i, id=%i",
                          elem->collected_table_node,
                          elem->collected_table_idx);
                gs_param_table_free(elem->collected_table);
                free(elem->collected_table);
                store_free(elem->store);
                free(elem->store);
                LL_DELETE(tables, elem);
                free(elem);
            }
        }
    }
}

static void destroy_beacon(gs_hk_beacon_cfg_t* beacon) {
    gs_hk_beacon_data_element_t* de = NULL;
    gs_hk_beacon_data_element_t* de_tmp = NULL;
    LL_FOREACH_SAFE(beacon->data_elements, de, de_tmp) {
        gs_hk_beacon_parameter_t* pe = NULL;
        gs_hk_beacon_parameter_t* pe_tmp = NULL;
        LL_FOREACH_SAFE(de->parameters, pe, pe_tmp) {
            LL_DELETE(de->parameters, pe);
            free(pe);
        }
        de->table->used_in_beacons_count--;
        LL_DELETE(beacon->data_elements, de);
        free(de);
    }

    destroy_unused_tables();
}

gs_error_t gs_hk_config_unload() {
    log_info("unloading all beacon configurations");
    if (!obtain_write_lock_on_configuration()) {
        log_error("could not obtain configuration write lock");
        return GS_ERROR_BUSY;
    }
    loaded = 0;

    if (beacons != NULL) {
        gs_hk_beacon_cfg_t *elem = NULL;
        gs_hk_beacon_cfg_t *tmp = NULL;
        LL_FOREACH_SAFE(beacons, elem, tmp) {
            destroy_beacon(elem);
            LL_DELETE(beacons, elem);
            free(elem);
        }
        beacons = NULL;
    }

    gs_hk_table_cfg_t* t_elem;
    uint32_t cnt;
    LL_COUNT(tables, t_elem, cnt);
    if (cnt != 0) {
        log_error("%"PRIu32" tables still in use after unloading all beacons",
                  cnt);
    }
    tables = NULL;

    release_write_lock_on_configuration();
    return GS_OK;
}

gs_error_t gs_hk_config_reload() {
    gs_error_t res = gs_hk_config_unload();
    if (res == GS_OK) {
        res = gs_hk_config_load();
    }
    return res;
}


#define MAX_TABLE_SPEC_FILE 50
static gs_error_t download_table_spec(gs_hk_table_cfg_t* table) {

    char path[GS_HK_HKCFG_DATA_PATH_LENGTH+1];
    GS_HK_HKCFG_DATA_PATH_GET(path, sizeof(path));
    char table_spec_file[MAX_TABLE_SPEC_FILE];
    snprintf(table_spec_file, MAX_TABLE_SPEC_FILE, "%s/%s/hk_st_%i_%i.bin",
             path, GS_HK_SPECS_PATH_PREFIX, table->collected_table_node,
             table->collected_table_idx);

    /* attempt download from remote node */
    gs_error_t res = gs_rparam_download_table_spec(
        table->collected_table,
        table_spec_file,
        table->collected_table_node,
        table->collected_table_idx,
        table->rparam_download_timeout,
        &table->collected_table_checksum);

    if (res != GS_OK) {
        /* attempt load from local file */
        res = gs_rparam_load_table_spec(table->collected_table,
                                        table_spec_file,
                                        &table->collected_table_checksum);
    }

    if (res == GS_OK) {
        log_debug("table specification succesfully loaded, node=%i, id=%i",
                  table->collected_table_node, table->collected_table_idx);

        gs_param_table_get_memory(table->collected_table, NULL);
        size_t row_count;
        const gs_param_table_row_t* rows = gs_param_table_get_rows(
            table->collected_table, &row_count);
        size_t datasize = gs_param_calc_table_size(rows, row_count);

        table->store = calloc(1, sizeof(gs_hk_store_t));
        res = store_initialize(table->store, table->store_max_samples, datasize);
        if (res != GS_OK) {
            log_error("failed to initialize store, res=%s",
                      gs_error_string(res));
        } else {
            // attempt to load cache from persistent storage
            char buf[127];
            file_create_filename(buf, sizeof(buf),
                                 table->collected_table_node,
                                 table->collected_table_idx);
            res = file_load(table, buf, table->store_max_samples);
            if (res == GS_ERROR_IO) {
                // OK - could be a newly configured
                log_trace("failed to open filename=%s, res=%s",
                          buf, gs_error_string(res));
                res = GS_OK;
            } else if (res == GS_ERROR_NO_DATA) {
                // OK - could have been created without any samples available
                log_debug("cache file empty, filename=%s, res=%s",
                          buf, gs_error_string(res));
                res = GS_OK;
            } else if (res != GS_OK) {
                log_warning("failed to load cache from file=%s, res=%s",
                            buf, gs_error_string(res));
            } else {
                log_debug("samples succesfully loaded from storage, "
                         "node=%i, id=%i",
                         table->collected_table_node,
                         table->collected_table_idx);
            }
        }
    }

    return res;
}

gs_error_t invalidate_tables_that_are_not_being_collected() {

    if (!obtain_write_lock_on_configuration()) {
        log_warning("could not obtain configuration write lock");
        return GS_ERROR_BUSY;
    }

    gs_hk_table_cfg_t* table = NULL;
    LL_FOREACH(tables, table) {
        if (table->ttl == 0) {
            log_warning("table not being collected correctly, "
                        "could be wrong table crc, invalidating table and "
                        "freeing store, node=%i, id=%i",
                        table->collected_table_node,
                        table->collected_table_idx);

            /* Reset row pointers in beacons that are using this table,
               as they will become invalid when calling gs_param_table_free
               below.
            */
            if (beacons != NULL) {
                gs_hk_beacon_cfg_t *b_elem = NULL;
                LL_FOREACH(beacons, b_elem) {
                    gs_hk_beacon_data_element_t* d_elem = NULL;
                    LL_FOREACH(b_elem->data_elements, d_elem) {
                        if (d_elem->table == table) {
                            log_debug("invalidating pointers to table rows in "
                                      "type=%i, table_idx=%i", b_elem->type,
                                      table->collected_table_idx);
                            gs_hk_beacon_parameter_t* p = NULL;
                            LL_FOREACH(d_elem->parameters, p) {
                                p->p_row = NULL;
                            }
                        }
                    }
                }
            }

            store_free(table->store);
            free(table->store);
            table->store = NULL;
            gs_param_table_free(table->collected_table);
            table->ttl = INITIAL_TTL;
            table->last_collection_ts = 0;
        }
    }

    release_write_lock_on_configuration();
    return GS_OK;
}

#define DEFAULT_RPARAM_DOWNLOAD_TIMEOUT 200
static gs_error_t create_and_add_table_element(cJSON* cfg,
                                               gs_hk_table_cfg_t** rinst) {

    cJSON* source_node = cJSON_GetObjectItem(cfg, "node_address");
    if (source_node == NULL || !cJSON_IsNumber(source_node)) {
        log_error("json error - could not load 'node_address' number");
        return GS_ERROR_DATA;
    }

    cJSON* source_table_id = cJSON_GetObjectItem(cfg, "table_id");
    if (source_table_id == NULL || !cJSON_IsNumber(source_table_id)) {
        log_error("json error - could not load 'table_id' number");
        return GS_ERROR_DATA;
    }

    cJSON* download_timeout = cJSON_GetObjectItem(cfg, "download_timeout");
    uint16_t rparam_download_timeout = DEFAULT_RPARAM_DOWNLOAD_TIMEOUT;
    if (download_timeout != NULL && cJSON_IsNumber(download_timeout)) {
        rparam_download_timeout = download_timeout->valueint;
    }

    // check if this table has already been created
    gs_hk_table_cfg_t*elem = NULL;
    gs_hk_table_cfg_t* tmp = NULL;
    LL_FOREACH_SAFE(tables, elem, tmp) {
        if (elem->collected_table_node == source_node->valueint &&
            elem->collected_table_idx == source_table_id->valueint) {
            log_debug("table already exists - skipping, node=%i, id=%i",
                      elem->collected_table_node, elem->collected_table_idx);
            *rinst = elem;
            return GS_OK;
        }
    }

    gs_hk_table_cfg_t* table = calloc(1, sizeof(gs_hk_table_cfg_t));
    table->collected_table_node = source_node->valueint;
    table->collected_table_idx = source_table_id->valueint;
    table->rparam_download_timeout = rparam_download_timeout;

    log_info("creating table instance, node=%i, id=%i",
              table->collected_table_node, table->collected_table_idx);
    table->collected_table = gs_param_table_instance_alloc();
    gs_param_table_create_t create = { .name = NULL };
    gs_error_t res = gs_param_table_init(table->collected_table, &create);
    if (res != GS_OK) {
        gs_param_table_free(table->collected_table);
        free(table->collected_table);
        free(table);
        log_error("gs_param_table_init() failed, error=%s",
                  gs_error_string(res));
        return res;
    }

    *rinst = table;
    LL_APPEND(tables, table);

    return GS_OK;

}

static void update_beacon_priority(gs_hk_beacon_cfg_t* beacon,
                                   gs_hk_beacon_cfg_t* all_beacons,
                                   gs_hk_priority_t priority) {

    // update beacon priority
    beacon->priority = priority;

    // update beacon autobeacon interval
    beacon->auto_tx_interval = interval_from_priority(priority);

    // update priority and sample interval of tables related to beacon
    gs_hk_beacon_data_element_t* de = NULL;
    LL_FOREACH(beacon->data_elements, de) {
        gs_hk_table_cfg_t* table_to_update = de->table;
        gs_hk_priority_t highest_priority = beacon->priority;

        gs_hk_beacon_cfg_t* b = NULL;
        LL_FOREACH(all_beacons, b) {
            gs_hk_beacon_data_element_t* de_int = NULL;
            LL_FOREACH(b->data_elements, de_int) {
                if (de_int->table == table_to_update) {
                    highest_priority = gs_max(highest_priority, b->priority);
                }
            }
        }
        table_to_update->priority = highest_priority;
        table_to_update->collect_interval =
                interval_from_priority(table_to_update->priority);

        /* reset last_collection_ts to avoid warnings about late sampling */
        table_to_update->last_collection_ts = 0;
    }
}

gs_error_t gs_hk_config_update_beacon_priority(gs_hk_config_handle_t hdl,
                                               uint8_t type,
                                               const char* priority_string) {
    if (!is_handle_valid(hdl)) {
        return GS_ERROR_ARG;
    }

    gs_hk_beacon_cfg_t* beacon_to_update;
    gs_error_t res = gs_hk_config_get_beacon(hdl, type, &beacon_to_update);
    if (res != GS_OK) {
        return res;
    }

    gs_hk_beacon_cfg_t* all_beacons = gs_hk_config_get_beacons(hdl);
    update_beacon_priority(beacon_to_update, all_beacons,
                           gs_hk_priority_from_string(priority_string));
    return GS_OK;
}

static void parse_array_index_string(gs_hk_beacon_parameter_t* p,
                                     char* array_index_string)
{
    if (array_index_string == NULL) {
        return;
    }

    // support a single value by checking if the string is a number
    uint8_t single_value;
    gs_error_t error = gs_string_to_uint8(array_index_string, &single_value);
    if (error == GS_OK) {
            p->array_idx_begin = single_value;
            p->array_idx_end = single_value;
    } else {
        char* tmp;
        char* tok = strtok_r(array_index_string, "-", &tmp);
        error = gs_string_to_uint8(tok, &p->array_idx_begin);
        if (!error) {
            error = gs_string_to_uint8(strtok_r(NULL, "-", &tmp),
                                       &p->array_idx_end);
        }

        // end < begin will screw things up - set defaults instead
        if (error || p->array_idx_end < p->array_idx_begin) {
            p->array_idx_begin = 0;
            p->array_idx_end = 0;
        }
    }

    return;
}

gs_error_t load_beacon_from_string(const char* config) {

    /* load json data for beacon */
    cJSON* root = cJSON_Parse(config);
    if (root == NULL || !cJSON_IsObject(root)) {
        log_error("json error - could not load 'root' object");
        cJSON_Delete(root);
        return GS_ERROR_DATA;
    }

    cJSON* data = cJSON_GetObjectItem(root, "elements");
    if (data == NULL || !cJSON_IsArray(data)) {
        log_error("json error - could not load 'elements' array");
        cJSON_Delete(root);
        return GS_ERROR_DATA;
    }

    int elements = cJSON_GetArraySize(data);
    if (elements < 1) {
        log_error("json error - 'data' array has no items");
        cJSON_Delete(root);
        return GS_ERROR_DATA;
    }

    cJSON* type = cJSON_GetObjectItem(root, "type");
    if (type == NULL || !cJSON_IsNumber(type)) {
        log_error("json error - could not load 'type' number");
        cJSON_Delete(root);
        return GS_ERROR_DATA;
    }

    cJSON* version = cJSON_GetObjectItem(root, "version");
    if (version == NULL || !cJSON_IsNumber(version)) {
        log_error("json error - could not load 'version' number");
        cJSON_Delete(root);
        return GS_ERROR_DATA;
    }

    gs_hk_priority_t beacon_priority = GS_HK_PRIORITY_LOW;
    cJSON* priority = cJSON_GetObjectItem(root, "samplerate");
    if (priority == NULL || !cJSON_IsString(priority)) {
        log_trace("defaulting beacon samplerate to low, type=%i, version=%i",
                  type->valueint, version->valueint);
    } else {
        beacon_priority = gs_hk_priority_from_string(priority->valuestring);
    }

    uint32_t max_samples = 10;
    cJSON* max_samples_json = cJSON_GetObjectItem(root, "max_samples");
    if (max_samples_json == NULL || !cJSON_IsNumber(max_samples_json)) {
        log_trace("defaulting max number of samples to %"PRIu32", type=%i, version=%i",
                  max_samples, type->valueint, version->valueint);
    } else {
        max_samples = max_samples_json->valueint;
    }

    gs_hk_beacon_policy_t auto_beacon_pol = GS_HK_AUTO_BEACONS_OFF;
    cJSON* auto_beacon_pol_json = cJSON_GetObjectItem(root, "auto_beacon_policy");
    if (auto_beacon_pol_json == NULL || !cJSON_IsString(auto_beacon_pol_json)) {
        log_trace("defaulting autobeacon policy to [%s], type=%i, version=%i",
                  gs_hk_beacon_policy_to_string(auto_beacon_pol),
                  type->valueint, version->valueint);
    } else {
        auto_beacon_pol = gs_hk_beacon_policy_from_string(
            auto_beacon_pol_json->valuestring);
    }

    /* check if this beacon already exists */
    gs_hk_beacon_cfg_t* elem = NULL;
    gs_hk_beacon_cfg_t* tmp = NULL;
    LL_FOREACH_SAFE(beacons, elem, tmp) {
        if (elem->type == type->valueint) {
            log_debug("beacon already exists, reloading, type=%i",
                      type->valueint);
            LL_DELETE(beacons, elem);
            destroy_beacon(elem);
            free(elem);
        }
    }

    /* allocate beacon structure */
    gs_hk_beacon_cfg_t* beacon = calloc(1, sizeof(gs_hk_beacon_cfg_t));
    beacon->type = type->valueint;
    beacon->version = version->valueint;
    beacon->policy = auto_beacon_pol;

    log_info("loading beacon configuration, type=%i, version=%i",
              beacon->type, beacon->version);

    /* iterate data elements */
    gs_error_t result = GS_OK;
    for (int i = 0; i < elements; ++i) {

        /* load json data for element */
        cJSON* element = cJSON_GetArrayItem(data, i);
        if (element == NULL || !cJSON_IsObject(element)) {
            log_error("json error - could not load 'element' object");
            result = GS_ERROR_DATA;
            break;
        }

        cJSON* source_node = cJSON_GetObjectItem(element, "node_address");
        if (source_node == NULL || !cJSON_IsNumber(source_node)) {
            log_error("json error - could not load 'node_address' number");
            result = GS_ERROR_DATA;
            break;
        }

        cJSON* source_table_id = cJSON_GetObjectItem(element, "table_id");
        if (source_table_id == NULL || !cJSON_IsNumber(source_table_id)) {
            log_error("json error - could not load 'table_id' number");
            result = GS_ERROR_DATA;
            break;
        }

        cJSON* parameters = cJSON_GetObjectItem(element, "params");
        if (parameters == NULL || !cJSON_IsArray(parameters)) {
            log_error("json error - could not load 'params' array");
            result = GS_ERROR_DATA;
            break;
        }

        int parameter_count = cJSON_GetArraySize(parameters);
        if (parameter_count < 1) {
            log_error("json error - 'parameters' array has no items");
            result = GS_ERROR_DATA;
            break;
        }

        /* allocate and create table element */
        gs_hk_table_cfg_t* table;
        if (create_and_add_table_element(element, &table) != GS_OK) {
            result = GS_ERROR_DATA;
            break;
        }
        table->ttl = INITIAL_TTL;
        table->used_in_beacons_count++;
        table->store_max_samples =
                gs_max(max_samples, table->store_max_samples);

        /* allocate beacon data element */
        gs_hk_beacon_data_element_t* data_element =
                calloc(1, sizeof(gs_hk_beacon_data_element_t));
        data_element->table = table;

        /* add parameters to beacon data element */
        for (int j=0; j<parameter_count; ++j) {
            gs_hk_beacon_parameter_t* p = calloc(1, sizeof(gs_hk_beacon_parameter_t));
            cJSON* name = cJSON_GetObjectItem(
                cJSON_GetArrayItem(parameters, j), "name");
            if (name == NULL || !cJSON_IsString(name)) {
                log_error("json error - parameter name missing or not string");
                free(p);
                result = GS_ERROR_DATA;
                break;
            } else {
                strncpy(p->name, name->valuestring, sizeof(p->name));
            }

            cJSON* array_index = cJSON_GetObjectItem(
                cJSON_GetArrayItem(parameters, j), "array_index");
            if (array_index != NULL && cJSON_IsString(array_index)) {
                parse_array_index_string(p, array_index->valuestring);
            }

            LL_APPEND(data_element->parameters, p);
        }
        LL_APPEND(beacon->data_elements, data_element);

        if (result != GS_OK) {
            break;
        }
    }

    if (result == GS_OK) {
        LL_APPEND(beacons, beacon);
        update_beacon_priority(beacon, beacons, beacon_priority);
    } else {
        log_error("error while creating beacon configuration, skipping");
        destroy_beacon(beacon);
        free(beacon);
    }

    cJSON_Delete(root);
    return result;
}

size_t gs_hk_config_count_beacon_files_in_data_path(const char* path) {
    DIR *dp;
    struct dirent *ep;
    size_t cnt = 0;
    dp = opendir (path);
    if (dp == NULL) {
        log_error("could not open path, path=%s", path);
        return 0;
    }

    while ((ep = readdir (dp))) {
        if (gs_string_endswith(ep->d_name, ".json")) {
            ++cnt;
        }
    }
    closedir(dp);
    return cnt;
}

gs_error_t gs_hk_config_load() {

    // create required directories
    create_directories();

    char root_path[GS_HK_HKCFG_DATA_PATH_LENGTH+1];
    GS_HK_HKCFG_DATA_PATH_GET(root_path, sizeof(root_path));
    char path[50];
    snprintf(path, sizeof(path), "%s/%s", root_path, GS_HK_BCN_TYPES_PATH_PREFIX);

    log_info("loading beacon configuration from path=%s", path);
    uint8_t errors = 0;
    DIR *dp;
    struct dirent *ep;
    dp = opendir (path);
    if (dp == NULL) {
        log_warning("could not open path, path=%s", path);
        return GS_ERROR_ARG;
    }

    if (!obtain_write_lock_on_configuration()) {
        log_warning("could not obtain configuration write lock");
        closedir(dp);
        return GS_ERROR_BUSY;
    }

    while ((ep = readdir (dp))) {
        if (gs_string_endswith(ep->d_name, ".json")) {
            char fullpath[sizeof(path)+1+sizeof(ep->d_name)];
            snprintf(fullpath, sizeof(path)+1+sizeof(ep->d_name),
                     "%s/%s", path, ep->d_name);
            FILE* f = fopen(fullpath, "r");
            if (f == NULL) {
                log_warning("failed to open file, file=%s", fullpath);
                errors++;
                continue;
            }

            char buf[4096];
            memset(buf, 0, sizeof(buf));
            if (!fread(&buf, 1, sizeof(buf), f)) {
                log_warning("error reading from file, file=%s", fullpath);
                fclose(f);
                errors++;
                continue;
            } else {
                gs_error_t error = load_beacon_from_string(buf);
                if (error != GS_OK) {
                    log_warning("error load beacon from string, error=%s, "
                                "file=%s", gs_error_string(error), fullpath);
                    errors++;
                }
                fclose(f);
            }
        }
    }
    release_write_lock_on_configuration();
    closedir(dp);
    log_info("done loading beacon configuration, errors=%i", errors);
    loaded = 1;

    if (errors) {
        return GS_ERROR_UNKNOWN;
    }

    return GS_OK;
}

gs_hk_table_cfg_t* gs_hk_config_get_tables(gs_hk_config_handle_t hdl) {
    if (!is_handle_valid(hdl)) {
        return NULL;
    }
    return tables;
}
gs_hk_beacon_cfg_t* gs_hk_config_get_beacons(gs_hk_config_handle_t hdl) {
    if (!is_handle_valid(hdl)) {
        return NULL;
    }
    return beacons;
}

gs_error_t gs_hk_config_get_table(gs_hk_config_handle_t hdl, uint8_t node,
                                  gs_param_table_id_t id,
                                  gs_hk_table_cfg_t** return_table) {
    GS_CHECK_ARG(is_handle_valid(hdl));
    gs_hk_table_cfg_t* elem = NULL;
    LL_FOREACH(tables, elem) {
        if (elem->collected_table_node == node &&
            elem->collected_table_idx == id) {
            *return_table = elem;
            return GS_OK;
        }
    }
    return GS_ERROR_NOT_FOUND;
}

gs_error_t gs_hk_config_get_beacon(gs_hk_config_handle_t hdl, uint8_t type,
                                   gs_hk_beacon_cfg_t** return_beacon) {
    GS_CHECK_ARG(is_handle_valid(hdl));
    gs_hk_beacon_cfg_t* elem = NULL;
    LL_FOREACH(beacons, elem) {
        if (elem->type == type) {
            *return_beacon = elem;
            return GS_OK;
        }
    }
    return GS_ERROR_NOT_FOUND;
}

gs_error_t gs_hk_config_invalidate_table_spec(gs_hk_config_handle_t hdl,
                                        gs_hk_table_cfg_t* table) {
    GS_CHECK_ARG(is_handle_valid(hdl));
    GS_CHECK_ARG(table != NULL);
    return gs_param_table_free(table->collected_table);
}

bool gs_hk_config_is_table_spec_downloaded(gs_hk_table_cfg_t* table) {
    size_t row_cnt = 0;
    gs_param_table_get_rows(table->collected_table, &row_cnt);
    return (row_cnt > 0);
}

gs_error_t gs_hk_config_download_table_spec(gs_hk_config_handle_t hdl,
                                            gs_hk_table_cfg_t* table) {
    GS_CHECK_ARG(is_handle_valid(hdl));
    return download_table_spec(table);
}


static void * hk_config_init_task(void* param)
{
    gs_error_t res = gs_hk_config_load();
    log_info("gs_hk_config_load() done, error=%s",
             gs_error_string(res));
    gs_thread_exit(NULL);
}

gs_error_t gs_hk_create_config_init_thread(void)
{
    bool enabled = false;
    GS_HK_HKCFG_HK_EN_GET(&enabled);
    if (enabled) {
        return gs_thread_create("HKCFG", hk_config_init_task, NULL, 16000,
                                GS_THREAD_PRIORITY_LOW, 0, NULL);
    } else {
        log_info("Housekeeping disabled, not loading beacon configurations");
        return GS_OK;
    }
}
