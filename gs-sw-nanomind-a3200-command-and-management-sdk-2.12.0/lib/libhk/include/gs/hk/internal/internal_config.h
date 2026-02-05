#ifndef GS_HK_CONFIG_H
#define GS_HK_CONFIG_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

/**
   @file

   @brief Data types and functions used in housekeeping.

   The data types defined here are internal data types to the housekeeping system.
*/

#include <gs/util/error.h>
#include <gs/param/table.h>
#include <gs/hk/internal/internal_collector_store.h>

#define INITIAL_TTL 10

#define GS_HK_PRIORITY_LOWEST_NAME "lowest"
#define GS_HK_PRIORITY_LOW_NAME    "low"
#define GS_HK_PRIORITY_MEDIUM_NAME "medium"
#define GS_HK_PRIORITY_HIGH_NAME   "high"
#define GS_HK_PRIORITY_HIGHEST_NAME "highest"

typedef enum {
    GS_HK_AUTO_BEACONS_OFF = 0,
    GS_HK_AUTO_BEACONS_ALWAYS,
    GS_HK_AUTO_BEACONS_ONLY_COMPLETE,
} gs_hk_beacon_policy_t;

typedef enum {
    GS_HK_PRIORITY_LOWEST  = 1,
    GS_HK_PRIORITY_LOW     = 2,
    GS_HK_PRIORITY_MEDIUM  = 4,
    GS_HK_PRIORITY_HIGH    = 8,
    GS_HK_PRIORITY_HIGHEST = 16,
} gs_hk_priority_t;

typedef struct gs_hk_table_cfg_s {
    gs_param_table_instance_t* collected_table;
    uint16_t collected_table_checksum;
    uint8_t collected_table_node;
    gs_param_table_id_t collected_table_idx;
    uint16_t rparam_download_timeout;

    // store for samples of the collected table
    gs_hk_store_t* store;
    uint32_t store_max_samples;
    uint32_t last_persist;

    // state
    uint32_t last_collection_ts;
    uint8_t used_in_beacons_count;
    uint16_t collect_interval;
    gs_hk_priority_t priority;
    uint8_t ttl; // cleanup if this hits zero

    // pointer for linked list (utlist.h)
    struct gs_hk_table_cfg_s* next;

} gs_hk_table_cfg_t;

typedef struct gs_hk_beacon_parameter_s {
    char name[GS_PARAM_MAX_NAME];
    const gs_param_table_row_t* p_row;
    uint8_t array_idx_begin;
    uint8_t array_idx_end;

    // pointer for linked list (utlist.h)
    struct gs_hk_beacon_parameter_s* next;
} gs_hk_beacon_parameter_t;

typedef struct gs_hk_beacon_data_element_s {
    gs_hk_beacon_parameter_t* parameters;
    gs_hk_table_cfg_t* table;

    // pointer for linked list (utlist.h)
    struct gs_hk_beacon_data_element_s* next;
} gs_hk_beacon_data_element_t;

typedef struct gs_hk_beacon_cfg_s {

    // data elements that go into this beacon type
    gs_hk_beacon_data_element_t* data_elements;

    // auto transmit
    uint16_t auto_tx_interval;
    gs_hk_beacon_policy_t policy;

    // beacon identification
    uint8_t type;
    uint8_t version;
    gs_hk_priority_t priority;

    // pointer for linked list (utlist.h)
    struct gs_hk_beacon_cfg_s* next;
} gs_hk_beacon_cfg_t;


#define GS_HK_BCN_TYPES_PATH_PREFIX  "bcn_types"
#define GS_HK_CACHE_PATH_PREFIX      "cache"
#define GS_HK_SPECS_PATH_PREFIX      "specs"

typedef uint8_t gs_hk_config_handle_t;


/****************************** HELPERS **********************************/
size_t gs_hk_config_count_beacon_files_in_data_path(const char* path);
const char* gs_hk_beacon_policy_to_string(gs_hk_beacon_policy_t policy);
gs_hk_beacon_policy_t gs_hk_beacon_policy_from_string(const char* policy);
bool gs_hk_check_priority(const char* priority);
const char* gs_hk_priority_to_string(gs_hk_priority_t priority);
bool gs_hk_config_is_table_spec_downloaded(gs_hk_table_cfg_t* table);


/**************************** CONFIGURATION *******************************/

gs_error_t gs_hk_config_open(gs_hk_config_handle_t* hdl);
gs_error_t gs_hk_config_close(gs_hk_config_handle_t hdl);

gs_error_t gs_hk_config_load();
gs_error_t gs_hk_config_unload();

gs_error_t gs_hk_config_invalidate_table_spec(gs_hk_config_handle_t hdl,
                                              gs_hk_table_cfg_t* table);
gs_error_t gs_hk_config_download_table_spec(gs_hk_config_handle_t hdl,
                                            gs_hk_table_cfg_t* table);

gs_hk_table_cfg_t* gs_hk_config_get_tables(gs_hk_config_handle_t hdl);
gs_hk_beacon_cfg_t* gs_hk_config_get_beacons(gs_hk_config_handle_t hdl);
gs_error_t gs_hk_config_get_table(gs_hk_config_handle_t hdl, uint8_t node,
                                  gs_param_table_id_t idx,
                                  gs_hk_table_cfg_t** return_table);
gs_error_t gs_hk_config_get_beacon(gs_hk_config_handle_t hdl, uint8_t type,
                                   gs_hk_beacon_cfg_t** return_beacon);

gs_error_t gs_hk_config_update_beacon_priority(gs_hk_config_handle_t hdl,
                                               uint8_t type,
                                               const char* priority_string);
gs_error_t invalidate_tables_that_are_not_being_collected();

#endif /* GS_HK_CONFIG_H */
