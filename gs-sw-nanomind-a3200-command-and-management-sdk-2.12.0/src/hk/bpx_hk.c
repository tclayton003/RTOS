/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "../scheduler.h"
#include <gs/a3200_sdk/param/host/bpx_hk.h>
#include <gs/bpx/io.h>
#include <string.h>

#define LOG_DEFAULT scheduler_log

static uint32_t bpx_hk_collect(void * user_data)
{
    gs_a3200_sdk_bpx_hk_memory_t * tel = gs_param_table_get_memory(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), NULL);
    if (tel) {
        const uint8_t addr = GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR_GET_NC();
        bpx_hk_t values;
        const int res = bpx_hk_get_p(&values, addr, BPX_PORT_HK, GS_CSP_HK_TIMEOUT);
        log_debug("%s: from: %u:%d, result: %d (expected %u)", __FUNCTION__, addr, BPX_PORT_HK, res, (unsigned int) sizeof(values));
        if (res == sizeof(values)) {
            tel->c_charge = values.cur_charge;
            tel->c_dcharge = values.cur_discharge;
            tel->c_heater = values.cur_heater;
            tel->vbatt = values.vbatt;
            tel->bat_temp1 = values.bat_temperature1;
            tel->bat_temp2 = values.bat_temperature2;
            tel->bat_temp3 = values.bat_temperature3;
            tel->bat_temp4 = values.bat_temperature4;
            tel->bootcount = values.counter_boot;
            tel->bootcause = values.bootcause;
        } else {
            memcpy(tel, &gs_a3200_sdk_bpx_hk_default_values, sizeof(*tel));
        }
    }
    return GS_FUNCTION_SCHEDULER_REPEAT_INTERVAL;
}

void bpx_hk_schedule(void)
{
    const gs_param_table_create_t create = {
        .name = GS_A3200_SDK_BPX_HK_TABLE_NAME,
        .id = GS_A3200_SDK_BPX_HK_TABLE_ID,
        .rows = gs_a3200_sdk_bpx_hk_table,
        .row_count = GS_A3200_SDK_BPX_HK_TABLE_ROWS,
        .memory_size = GS_A3200_SDK_BPX_HK_TABLE_SIZE,
        .default_values = &gs_a3200_sdk_bpx_hk_default_values,
        .flags = GS_PARAM_TABLE_F_ALLOC_MEMORY
    };

    const uint16_t collect_interval = GS_A3200_SDK_SCHEDULER_BPX_COL_INTV_GET_NC();
    if (collect_interval) {
        // collection is active - ensure table is created and schedule collector
        gs_param_table_instance_t * tinst = gs_param_table_by_id(create.id);
        if (tinst == NULL) {
            gs_param_table_create(&create, &tinst);
        }

        log_info("Added BPX HK table ID:%d - collecting from: %u:%d intv: %u seconds",
                 create.id, GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR_GET_NC(), BPX_PORT_HK, collect_interval);
    }

    scheduler_modify(create.name, collect_interval * 1000, bpx_hk_collect, NULL);
}
