/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/hk/collector.h>

#include <unistd.h>
#include <stdlib.h>

#include <gs/util/log.h>
#include <gs/util/time.h>
#include <gs/util/clock.h>
#include <gs/util/thread.h>
#include <gs/hk/param/host/hkcfg.h>
#include <gs/hk/config.h>
#include <gs/hk/internal/internal_config.h>
#include <gs/hk/internal/internal_collector_store.h>
#include <beacon_service/beacon.h>
#include <gs/hk/beacon_service.h>

#define C1_THREADNAME "c1"
#define C2_THREADNAME "c2"

#if defined(GS_LOG_CAT_HK)
#define LOG_CATEGORY GS_LOG_CAT_HK
#else
#define LOG_CATEGORY GS_LOG_CAT_DEFAULT
#endif

GS_LOG_GROUP(log_hk, "hk_collector", LOG_CATEGORY, LOG_DEFAULT_MASK | LOG_INFO_MASK);
#define LOG_DEFAULT log_hk

static void collect_table(gs_hk_config_handle_t cfg_hdl,
                          gs_hk_table_cfg_t* table,
                          const char* thread_name,
                          uint32_t now_s)
{
    log_debug("[%s] collecting table, node=%i, idx=%i", thread_name,
              table->collected_table_node, table->collected_table_idx);
    int result = -1;

    if (!gs_hk_config_is_table_spec_downloaded(table)) {
        gs_hk_config_download_table_spec(cfg_hdl, table);
    }

    if (gs_hk_config_is_table_spec_downloaded(table)) {

        gs_error_t res = gs_rparam_get_full_table(
            table->collected_table,
            table->collected_table_node,
            table->collected_table_idx,
            table->collected_table_checksum,
            table->rparam_download_timeout);
        if (res == GS_OK) {
            result = 0;
        } else {
            log_debug("[%s] gs_rparam_get_full_table failed, res=%s, "
                      "node=%i, id=%i", thread_name, gs_error_string(res),
                      table->collected_table_node,
                      table->collected_table_idx);
        }
    }

    if (result >= 0) {

        log_trace("[%s] now storing fetched values, node=%i, idx=%i",
                  thread_name, table->collected_table_node,
                  table->collected_table_idx);

        /* reset TTL for this table because collection was a succes */
        table->ttl = INITIAL_TTL;

        /* log if sampling is late */
        if ((table->last_collection_ts > 0) &&
            ((table->last_collection_ts+table->collect_interval) < now_s)) {
            log_warning("[%s] sampling is late, node=%i, id=%i",
                        thread_name, table->collected_table_node,
                        table->collected_table_idx);
        }

        /* store collected sample */
        table->last_collection_ts = now_s;
        void* sample_data = gs_param_table_get_memory(table->collected_table, NULL);
        gs_error_t error = store_add_sample(table->store, sample_data,
                                            table->store->sample_size, now_s);
        if (error != GS_OK) {
            log_warning("[%s] store_add_sample failed, error=%s",
                        thread_name, gs_error_string(error));
        }
    } else {
        log_debug("[%s] failed to fetch table values, skipping store, node=%i, idx=%i",
                  thread_name, table->collected_table_node, table->collected_table_idx);

        if (csp_ping(table->collected_table_node, 100, 1, 0) >= 0) {

            /* If we are not getting any replies in gs_rparam_get_full_table(),
               but are getting replies to ping it could be due to a table
               crc mismatch, therefore we invalidate the table in our end and
               will eventually fetch it again from the other end.
               The persisted store will not load if table checksum in the file
               does not match collected_table_checksum, the persisted store will
               eventually be overwritten with data and a matching checksum.
            */
            table->ttl = table->ttl - 1;
        }
    }
}

static void invalidate_stale_tables(const char* thread_name,
                                    uint8_t priority_mask) {
    uint8_t stale_table_count = 0;
    gs_hk_config_handle_t cfg_hdl;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res == GS_OK) {
        gs_hk_table_cfg_t* tables = gs_hk_config_get_tables(cfg_hdl);
        if (tables != NULL) {
            gs_hk_table_cfg_t* table = NULL;
            LL_FOREACH(tables, table) {
                if (table->priority & priority_mask) {
                    if (table->ttl == 0) {
                        stale_table_count++;
                    }
                }
            }
        }
        gs_hk_config_close(cfg_hdl);
    } else {
        log_error("[%s] gs_hk_config_open() failed, res=%s",
                  thread_name, gs_error_string(res));
    }

    if (stale_table_count > 0) {
        invalidate_tables_that_are_not_being_collected();
    }
}

static void collect_tables(uint32_t now_s, const char* thread_name,
                           uint8_t priority_mask) {
    gs_hk_config_handle_t cfg_hdl;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res == GS_OK) {
        gs_hk_table_cfg_t* tables = gs_hk_config_get_tables(cfg_hdl);
        if (tables != NULL) {
            gs_hk_table_cfg_t* table = NULL;
            gs_hk_table_cfg_t* tmp = NULL;
            LL_FOREACH_SAFE(tables, table, tmp) {
                if (table->priority & priority_mask) {
                    if ((now_s % table->collect_interval) == 0) {
                        collect_table(cfg_hdl, table, thread_name, now_s);
                    }
                }
            }
        }
        gs_hk_config_close(cfg_hdl);
    } else {
        log_error("[%s] gs_hk_config_open() failed, res=%s",
                  thread_name, gs_error_string(res));
    }
}

static void send_automatic_beacons(uint32_t now_s, const char* thread_name,
                                   uint8_t priority_mask) {

    if (GS_HK_HKCFG_BCN_ATX_TPDS_GET_NC() > 0) {
        log_debug("[%s] beacons temporarily disabled", thread_name);
        if (strncmp(thread_name, C1_THREADNAME, sizeof(C1_THREADNAME)) == 0) {
            GS_HK_HKCFG_BCN_ATX_TPDS_SET(GS_HK_HKCFG_BCN_ATX_TPDS_GET_NC() - 1);
        }
        return;
    }

    gs_hk_config_handle_t cfg_hdl;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res == GS_OK) {
        gs_hk_beacon_cfg_t* beacons = gs_hk_config_get_beacons(cfg_hdl);
        if (beacons != NULL) {
            gs_hk_beacon_cfg_t* beacon = NULL;
            gs_hk_beacon_cfg_t* tmp_b = NULL;
            LL_FOREACH_SAFE(beacons, beacon, tmp_b) {
                if (!(beacon->priority & priority_mask)) {
                    continue;
                }

                if (beacon->policy == GS_HK_AUTO_BEACONS_OFF) {
                    continue;
                }

                if (now_s % beacon->auto_tx_interval != 0) {
                    continue;
                }

                /* Check policy */
                if (beacon->policy != GS_HK_AUTO_BEACONS_ALWAYS) {
                    log_trace("[%s] beacon policy in place, type=%i, policy=%s",
                              thread_name, beacon->type,
                              gs_hk_beacon_policy_to_string(beacon->policy));

                    /* Check if any tables failed to be collected */
                    uint8_t tables_with_stale_data = 0;
                    gs_hk_beacon_data_element_t* elem = NULL;
                    gs_hk_beacon_data_element_t* tmp = NULL;
                    LL_FOREACH_SAFE(beacon->data_elements, elem, tmp) {
                        if ((elem->table->last_collection_ts +
                             elem->table->collect_interval) < now_s) {
                            tables_with_stale_data++;
                        }
                    }

                    /* Send or skip */
                    if (beacon->policy == GS_HK_AUTO_BEACONS_ONLY_COMPLETE &&
                        tables_with_stale_data > 0) {
                        log_debug("[%s] skipping beacon, pol=%s, "
                                  "stale_tables=%i, beacon_type=%i",
                                  thread_name,
                                  gs_hk_beacon_policy_to_string(beacon->policy),
                                  tables_with_stale_data, beacon->type);
                        continue;
                    }
                } else {
                    log_trace("[%s] 'always send beacon'-policy, type=%i",
                              thread_name, beacon->type);
                }

                /* Send auto beacon */
                hk_service_send(NULL, beacon, 0, NULL);
            }
        }
    }
    gs_hk_config_close(cfg_hdl);
}

typedef struct {
    uint8_t priority_mask;
    const char* thread_name;
} gs_hk_collector_cfg_t;

static gs_hk_collector_cfg_t cfg1 =
{
    .priority_mask = 0 | GS_HK_PRIORITY_HIGH |
    GS_HK_PRIORITY_MEDIUM | GS_HK_PRIORITY_LOW | GS_HK_PRIORITY_LOWEST,
    .thread_name = C1_THREADNAME,
};

static gs_hk_collector_cfg_t cfg2 =
{
    .priority_mask = 0 | GS_HK_PRIORITY_HIGHEST,
    .thread_name = C2_THREADNAME,
};

static void* collector_task(void* param)
{
    /* wait for configuration to be loaded */
    while (!gs_hk_config_is_loaded()) {
        gs_time_sleep_ms(100);
    }

    gs_hk_collector_cfg_t* cfg = (gs_hk_collector_cfg_t*)param;
    log_info("[%s] starting collector task, priority_mask=%i",
             cfg->thread_name, cfg->priority_mask);

    uint32_t ref_ms = 0;
    while (1) {

        /* wakeup every second */
        gs_time_sleep_until_ms(&ref_ms, 1000);

        /* get time */
        timestamp_t now;
        gs_clock_get_time(&now);

        /* collect tables */
        collect_tables(now.tv_sec, cfg->thread_name, cfg->priority_mask);

        /* invalidate tables that are not been collected */
        invalidate_stale_tables(cfg->thread_name, cfg->priority_mask);

        /* transmit automatic beacons */
        send_automatic_beacons(now.tv_sec, cfg->thread_name,
                               cfg->priority_mask);
    }

    gs_thread_exit(NULL);
}

void gs_hk_create_collector_threads(void)
{
    gs_log_group_register(LOG_DEFAULT);

    bool enabled = false;
    GS_HK_HKCFG_HK_EN_GET(&enabled);
    if (enabled) {
        gs_thread_create("HKC1", collector_task, &cfg1,
                         16000, GS_THREAD_PRIORITY_LOW, 0, NULL);
        gs_thread_create("HKC2", collector_task, &cfg2,
                         16000, GS_THREAD_PRIORITY_LOW, 0, NULL);
    } else {
        log_info("Housekeeping disabled, not starting collectors");
    }
}

gs_error_t gs_hk_update_priority_beacons(const char *priority)
{
    // Do a sanity check of the priority
    if (!gs_hk_check_priority(priority)) {
        return GS_ERROR_ARG;
    }

    if (!gs_hk_config_is_initialized()) {
        return GS_ERROR_STATE;
    }

    gs_hk_config_handle_t cfg_hdl;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res != GS_OK) {
        return res;
    }

    // Update all beacons to the new priority
    gs_hk_beacon_cfg_t* all_beacons = gs_hk_config_get_beacons(cfg_hdl);
    gs_hk_beacon_cfg_t* b = NULL;
    LL_FOREACH(all_beacons, b) {
        res = gs_hk_config_update_beacon_priority(cfg_hdl, b->type, priority);
    }

    gs_hk_config_close(cfg_hdl);
    return res;
}
