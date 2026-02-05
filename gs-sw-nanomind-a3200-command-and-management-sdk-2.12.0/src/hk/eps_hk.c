/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "../scheduler.h"
#include <gs/a3200_sdk/param/host/eps_hk.h>
#include <io/nanopower2.h>
#include <string.h>

#define LOG_DEFAULT scheduler_log

static uint32_t eps_hk_collect(void * user_data)
{
    gs_a3200_sdk_eps_hk_memory_t * tel = gs_param_table_get_memory(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), NULL);
    if (tel) {
        const uint8_t addr = GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR_GET_NC();
        eps_hk_t values;
        const int res = eps_hk_get_p(&values, addr, EPS_PORT_HK, GS_CSP_HK_TIMEOUT);
        log_debug("%s: from: %u:%d, result: %d (expected %u)", __FUNCTION__, addr, EPS_PORT_HK, res, (unsigned int) sizeof(values));
        if (res == sizeof(values)) {
            unsigned int i;
            for (i = 0; i < GS_ARRAY_SIZE(tel->vboost); i++) {
                tel->vboost[i] = values.vboost[i];
            }
            for (i = 0; i < GS_ARRAY_SIZE(tel->curout); i++) {
                tel->curout[i] = values.curout[i];
            }
            for (i = 0; i < GS_ARRAY_SIZE(tel->curin); i++) {
                tel->curin[i] = values.curin[i];
            }
            for (i = 0; i < GS_ARRAY_SIZE(tel->temp); i++) {
                tel->temp[i] = values.temp[i];
            }
            for (i = 0; i < GS_ARRAY_SIZE(tel->out_val); i++) {
                tel->out_val[i] = values.output[i];
            }
            for (i = 0; i < GS_ARRAY_SIZE(tel->cntWdtCsp); i++) {
                tel->cntWdtCsp[i] = values.counter_wdt_csp[i];
            }
            for (i = 0; i < GS_ARRAY_SIZE(tel->wdtCspC); i++) {
                tel->wdtCspC[i] = 0;
            }
            for (i = 0; i < GS_ARRAY_SIZE(tel->latchups); i++){
                tel->latchups[i] = values.latchup[i];
            }

            tel->cursun = values.cursun;
            tel->cursys = values.cursys;
            tel->vbatt = values.vbatt;
            tel->wdtI2cS = values.wdt_i2c_time_left;
            tel->wdtGndS = values.wdt_gnd_time_left;
            tel->bootcount = values.counter_boot;
            tel->cntWdtI2c = values.counter_wdt_i2c;
            tel->cntWdtGnd = values.counter_wdt_gnd;
            tel->battmode = values.battmode;
            tel->pptmode = values.pptmode;
            tel->bootcause = values.bootcause;
        } else {
            memcpy(tel, &gs_a3200_sdk_eps_hk_default_values, sizeof(*tel));
        }
    }

    return GS_FUNCTION_SCHEDULER_REPEAT_INTERVAL;
}

void eps_hk_schedule(void)
{
    const gs_param_table_create_t create = {
        .name = GS_A3200_SDK_EPS_HK_TABLE_NAME,
        .id = GS_A3200_SDK_EPS_HK_TABLE_ID,
        .rows = gs_a3200_sdk_eps_hk_table,
        .row_count = GS_A3200_SDK_EPS_HK_TABLE_ROWS,
        .memory_size = GS_A3200_SDK_EPS_HK_TABLE_SIZE,
        .default_values = &gs_a3200_sdk_eps_hk_default_values,
        .flags = GS_PARAM_TABLE_F_ALLOC_MEMORY
    };

    const uint16_t collect_interval = GS_A3200_SDK_SCHEDULER_EPS_COL_INTV_GET_NC();
    if (collect_interval) {
        // collection is active - ensure table is created and schedule collector
        gs_param_table_instance_t * tinst = gs_param_table_by_id(create.id);
        if (tinst == NULL) {
            gs_param_table_create(&create, &tinst);
        }

        log_info("Added EPS HK table ID:%d - collecting from: %u:%d intv: %u seconds",
                 create.id, GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR_GET_NC(), EPS_PORT_HK, collect_interval);
    }

    scheduler_modify(create.name, collect_interval * 1000, eps_hk_collect, NULL);
}
