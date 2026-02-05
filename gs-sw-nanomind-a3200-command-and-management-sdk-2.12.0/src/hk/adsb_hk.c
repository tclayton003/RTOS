/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "../scheduler.h"
#include <gs/a3200_sdk/param/host/adsb_hk.h>
#include <io/gatoss.h>
#include <string.h>

#define LOG_DEFAULT scheduler_log

static uint32_t adsb_hk_collect(void * user_data)
{
    gs_a3200_sdk_adsb_hk_memory_t * tel = gs_param_table_get_memory(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), NULL);
    if (tel) {
        const uint8_t addr = GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR_GET_NC();
        gatoss_hk_t values;
        const int res = gatoss_hk_p(&values, 0, addr, GATOSS_PORT_HK, GS_CSP_HK_TIMEOUT);
        log_debug("%s: from: %u:%d, result: %d (expected %u)", __FUNCTION__, addr, GATOSS_PORT_HK, res, (unsigned int) sizeof(values));
        if (res == sizeof(values)) {
            tel->cur_1v2 = values.current_1v2;
            tel->cur_2v5 = values.current_2v5;
            tel->cur_3v3_fpga = values.current_3v3_fpga;
            tel->cur_3v3_adc = values.current_3v3_adc;
            tel->cur_5v0_board = values.current_5v0_board;
            tel->cur_3v3_board = values.current_3v3_board;
            tel->cur_3v3_sd = values.current_3v3_sd;
            tel->avg_fps_10sec = values.average_fps_10sec;
            tel->avg_fps_1min = values.average_fps_1min;
            tel->avg_fps_5min = values.average_fps_5min;
            tel->plane_count = values.plane_count;
            tel->crc_corrected = values.crc_corrected;
            tel->last_icao24 = values.last_icao24;
            tel->last_lat = values.last_pos.lat;
            tel->last_lon = values.last_pos.lon;
            tel->last_alt = values.last_pos.altitude;
            tel->last_ts = values.last_pos.timestamp;
            tel->bootcount = values.bootcount;
            tel->bootcause = values.bootcause;
            tel->current_time = values.current_time;
            tel->tot_frames = values.tot_frames;
            tel->tot_planes = values.tot_planes;
            tel->tot_crc_cor = values.tot_crc_corrected;
            tel->fpga_crc_cnt =  values.fpga_crc_cnt;
            tel->flash_lw = values.flash_last_write;
        } else {
            memcpy(tel, &gs_a3200_sdk_adsb_hk_default_values, sizeof(*tel));
        }
    }
    return GS_FUNCTION_SCHEDULER_REPEAT_INTERVAL;
}

void adsb_hk_schedule(void)
{
    const gs_param_table_create_t create = {
        .name = GS_A3200_SDK_ADSB_HK_TABLE_NAME,
        .id = GS_A3200_SDK_ADSB_HK_TABLE_ID,
        .rows = gs_a3200_sdk_adsb_hk_table,
        .row_count = GS_A3200_SDK_ADSB_HK_TABLE_ROWS,
        .memory_size = GS_A3200_SDK_ADSB_HK_TABLE_SIZE,
        .default_values = &gs_a3200_sdk_adsb_hk_default_values,
        .flags = GS_PARAM_TABLE_F_ALLOC_MEMORY
    };

    const uint16_t collect_interval = GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV_GET_NC();
    if (collect_interval) {
        // collection is active - ensure table is created and schedule collector
        gs_param_table_instance_t * tinst = gs_param_table_by_id(create.id);
        if (tinst == NULL) {
            gs_param_table_create(&create, &tinst);
        }

        log_info("Added ADSB HK table ID:%d - collecting from: %u:%d intv: %u seconds",
                 create.id, GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR_GET_NC(), GATOSS_PORT_HK, collect_interval);
    }

    scheduler_modify(create.name, collect_interval * 1000, adsb_hk_collect, NULL);
}
