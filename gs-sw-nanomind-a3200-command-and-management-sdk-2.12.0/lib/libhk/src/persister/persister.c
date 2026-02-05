/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/hk/persister.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <gs/hk/internal/internal_collector_store.h>
#include <gs/util/log.h>
#include <gs/util/time.h>
#include <gs/util/thread.h>
#include <param/param.h>
#include <gs/hk/internal/internal_config.h>
#include <persister/file.h>
#include <gs/hk/param/host/hkcfg.h>

#if defined(GS_LOG_CAT_HK)
#define LOG_CATEGORY GS_LOG_CAT_HK
#else
#define LOG_CATEGORY GS_LOG_CAT_DEFAULT
#endif

GS_LOG_GROUP(hkpers, "hk_persister", LOG_CATEGORY, LOG_DEFAULT_MASK | LOG_INFO_MASK);
#define LOG_DEFAULT hkpers

static void * hk_persist(void * param)
{
    gs_error_t res = GS_OK;
    gs_hk_config_handle_t cfg_hdl;
    while (1) {

        /* Persisting enabled? */
        if (!GS_HK_HKCFG_PERSIST_EN_GET_NC()) {
            gs_time_sleep_ms(1000);
            break;
        }

        /* Wait interval */
        gs_time_sleep_ms(GS_HK_HKCFG_PERSIST_INTV_GET_NC() * 1000);

        log_debug("persisting samples - begin");
        res = gs_hk_config_open(&cfg_hdl);
        if (res == GS_OK) {
            gs_hk_table_cfg_t* tables = gs_hk_config_get_tables(cfg_hdl);
            if (tables != NULL) {
                gs_hk_table_cfg_t* table = NULL;
                gs_hk_table_cfg_t* tmp = NULL;
                LL_FOREACH_SAFE(tables, table, tmp) {

                    /* Check for disable flag */
                    if (!GS_HK_HKCFG_PERSIST_EN_GET_NC()) {
                        gs_hk_config_close(cfg_hdl);
                        continue;
                    }

                    if (table->store != NULL) {
                        /* Persist samples */
                        log_trace("persisting samples, node=%i, idx=%i",
                                  table->collected_table_node,
                                  table->collected_table_idx);
                        char buf[127];
                        file_create_filename(buf, sizeof(buf),
                                             table->collected_table_node,
                                             table->collected_table_idx);
                        res = file_save(table, buf, table->store_max_samples);
                        if (res != GS_OK) {
                            log_warning("failed to persist samples to file=%s, res=%s",
                                        buf, gs_error_string(res));
                        }
                    } else {
                        log_trace("skipping persisting as store is not"
                                  " initialized, node=%i, id=%i",
                                  table->collected_table_node,
                                  table->collected_table_idx);
                    }
                }
            }
            gs_hk_config_close(cfg_hdl);
            log_debug("persisting samples - done");
        } else {
            log_error("gs_hk_config_open failed, res=%s",
                      gs_error_string(res));
        }
    }

    gs_thread_exit(NULL);
}

gs_error_t gs_hk_create_persister_thread(void)
{
    gs_log_group_register(LOG_DEFAULT);

    bool enabled = false;
    GS_HK_HKCFG_HK_EN_GET(&enabled);
    if (enabled) {
        return gs_thread_create("HKPERS", hk_persist, NULL, 16000, GS_THREAD_PRIORITY_LOW, 0, NULL);
    } else {
        log_info("Housekeeping disabled, not starting persister thread");
        return GS_OK;
    }
}
