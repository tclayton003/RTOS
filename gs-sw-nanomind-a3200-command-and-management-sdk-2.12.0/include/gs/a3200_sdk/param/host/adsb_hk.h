#ifndef GS_A3200_SDK_PARAM_HOST_ADSB_HK_H
#define GS_A3200_SDK_PARAM_HOST_ADSB_HK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a adsb_hk.
*/

#include <gs/a3200_sdk/param/adsb_hk.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200_sdk_adsb_hk_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Current consumption, 1v2 */
    uint16_t cur_1v2;
    /** Current consumption, 2v5 */
    uint16_t cur_2v5;
    /** Current consumption, 3v3 FPGA */
    uint16_t cur_3v3_fpga;
    /** Current consumption, 3v3 ADC */
    uint16_t cur_3v3_adc;
    /** Current consumption, 5v0 Board */
    uint16_t cur_5v0_board;
    /** Current consumption, 3v3 Board */
    uint16_t cur_3v3_board;
    /** Current consumption, 3v3 SD */
    uint16_t cur_3v3_sd;
    /** Average frames/sec over the last 10 seconds */
    uint16_t avg_fps_10sec;
    /** Average frames/sec over the last 1 minute */
    uint16_t avg_fps_1min;
    /** Average frames/sec over the last 5 minutes */
    uint16_t avg_fps_5min;
    /** Number of unique airplanes with at least one position decoded since boot */
    uint32_t plane_count;
    /** Number of frames received since boot */
    uint32_t frame_count;
    /** Number of CRC errors corrected since boot */
    uint32_t crc_corrected;
    /** ICAO24 address of the latest airplane received */
    uint32_t last_icao24;
    /** Latitude of latest airplane received */
    float    last_lat;
    /** Longitude of latest airplane received */
    float    last_lon;
    /** Altitude of latest airplane received */
    uint32_t last_alt;
    /** Timestamp of latest airplane received */
    uint32_t last_ts;
    /** Board boot count */
    uint32_t bootcount;
    /** Board boot cause */
    uint16_t bootcause;
    /** Filler for alignment */
    uint8_t  _filler_0x003a[2];
    /** Current board time in seconds since UNIX epoch */
    uint32_t current_time;
    /** Total number of frames received */
    uint32_t tot_frames;
    /** Total number of unique airplanes with at least one position decoded */
    uint32_t tot_planes;
    /** Total number of CRC errors corrected */
    uint32_t tot_crc_cor;
    /** Total number of CRC errors in FPGA configuration */
    uint32_t fpga_crc_cnt;
    /**  */
    uint32_t flash_lw;
} gs_a3200_sdk_adsb_hk_memory_t;

/** Table default values. */
extern const gs_a3200_sdk_adsb_hk_memory_t gs_a3200_sdk_adsb_hk_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set cur_1v2: Current consumption, 1v2 */
#define GS_A3200_SDK_ADSB_HK_CUR_1V2_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_1V2, value, 0)
/** Set cur_1v2 (no callback): Current consumption, 1v2 */
#define GS_A3200_SDK_ADSB_HK_CUR_1V2_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_1V2, value, GS_PARAM_F_NO_CALLBACK)
/** Get cur_1v2: Current consumption, 1v2 */
#define GS_A3200_SDK_ADSB_HK_CUR_1V2_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_1V2, value, 0)
/** Get cur_1v2 (no check - return 0 on failure): Current consumption, 1v2 */
#define GS_A3200_SDK_ADSB_HK_CUR_1V2_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_1V2, 0)
/** Set cur_2v5: Current consumption, 2v5 */
#define GS_A3200_SDK_ADSB_HK_CUR_2V5_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_2V5, value, 0)
/** Set cur_2v5 (no callback): Current consumption, 2v5 */
#define GS_A3200_SDK_ADSB_HK_CUR_2V5_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_2V5, value, GS_PARAM_F_NO_CALLBACK)
/** Get cur_2v5: Current consumption, 2v5 */
#define GS_A3200_SDK_ADSB_HK_CUR_2V5_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_2V5, value, 0)
/** Get cur_2v5 (no check - return 0 on failure): Current consumption, 2v5 */
#define GS_A3200_SDK_ADSB_HK_CUR_2V5_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_2V5, 0)
/** Set cur_3v3_fpga: Current consumption, 3v3 FPGA */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_FPGA_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_FPGA, value, 0)
/** Set cur_3v3_fpga (no callback): Current consumption, 3v3 FPGA */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_FPGA_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_FPGA, value, GS_PARAM_F_NO_CALLBACK)
/** Get cur_3v3_fpga: Current consumption, 3v3 FPGA */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_FPGA_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_FPGA, value, 0)
/** Get cur_3v3_fpga (no check - return 0 on failure): Current consumption, 3v3 FPGA */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_FPGA_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_FPGA, 0)
/** Set cur_3v3_adc: Current consumption, 3v3 ADC */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_ADC_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_ADC, value, 0)
/** Set cur_3v3_adc (no callback): Current consumption, 3v3 ADC */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_ADC_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_ADC, value, GS_PARAM_F_NO_CALLBACK)
/** Get cur_3v3_adc: Current consumption, 3v3 ADC */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_ADC_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_ADC, value, 0)
/** Get cur_3v3_adc (no check - return 0 on failure): Current consumption, 3v3 ADC */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_ADC_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_ADC, 0)
/** Set cur_5v0_board: Current consumption, 5v0 Board */
#define GS_A3200_SDK_ADSB_HK_CUR_5V0_BOARD_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_5V0_BOARD, value, 0)
/** Set cur_5v0_board (no callback): Current consumption, 5v0 Board */
#define GS_A3200_SDK_ADSB_HK_CUR_5V0_BOARD_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_5V0_BOARD, value, GS_PARAM_F_NO_CALLBACK)
/** Get cur_5v0_board: Current consumption, 5v0 Board */
#define GS_A3200_SDK_ADSB_HK_CUR_5V0_BOARD_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_5V0_BOARD, value, 0)
/** Get cur_5v0_board (no check - return 0 on failure): Current consumption, 5v0 Board */
#define GS_A3200_SDK_ADSB_HK_CUR_5V0_BOARD_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_5V0_BOARD, 0)
/** Set cur_3v3_board: Current consumption, 3v3 Board */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_BOARD_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_BOARD, value, 0)
/** Set cur_3v3_board (no callback): Current consumption, 3v3 Board */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_BOARD_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_BOARD, value, GS_PARAM_F_NO_CALLBACK)
/** Get cur_3v3_board: Current consumption, 3v3 Board */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_BOARD_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_BOARD, value, 0)
/** Get cur_3v3_board (no check - return 0 on failure): Current consumption, 3v3 Board */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_BOARD_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_BOARD, 0)
/** Set cur_3v3_sd: Current consumption, 3v3 SD */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_SD_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_SD, value, 0)
/** Set cur_3v3_sd (no callback): Current consumption, 3v3 SD */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_SD_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_SD, value, GS_PARAM_F_NO_CALLBACK)
/** Get cur_3v3_sd: Current consumption, 3v3 SD */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_SD_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_SD, value, 0)
/** Get cur_3v3_sd (no check - return 0 on failure): Current consumption, 3v3 SD */
#define GS_A3200_SDK_ADSB_HK_CUR_3V3_SD_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CUR_3V3_SD, 0)
/** Set avg_fps_10sec: Average frames/sec over the last 10 seconds */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_10SEC_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_10SEC, value, 0)
/** Set avg_fps_10sec (no callback): Average frames/sec over the last 10 seconds */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_10SEC_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_10SEC, value, GS_PARAM_F_NO_CALLBACK)
/** Get avg_fps_10sec: Average frames/sec over the last 10 seconds */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_10SEC_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_10SEC, value, 0)
/** Get avg_fps_10sec (no check - return 0 on failure): Average frames/sec over the last 10 seconds */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_10SEC_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_10SEC, 0)
/** Set avg_fps_1min: Average frames/sec over the last 1 minute */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_1MIN_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_1MIN, value, 0)
/** Set avg_fps_1min (no callback): Average frames/sec over the last 1 minute */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_1MIN_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_1MIN, value, GS_PARAM_F_NO_CALLBACK)
/** Get avg_fps_1min: Average frames/sec over the last 1 minute */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_1MIN_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_1MIN, value, 0)
/** Get avg_fps_1min (no check - return 0 on failure): Average frames/sec over the last 1 minute */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_1MIN_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_1MIN, 0)
/** Set avg_fps_5min: Average frames/sec over the last 5 minutes */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_5MIN_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_5MIN, value, 0)
/** Set avg_fps_5min (no callback): Average frames/sec over the last 5 minutes */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_5MIN_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_5MIN, value, GS_PARAM_F_NO_CALLBACK)
/** Get avg_fps_5min: Average frames/sec over the last 5 minutes */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_5MIN_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_5MIN, value, 0)
/** Get avg_fps_5min (no check - return 0 on failure): Average frames/sec over the last 5 minutes */
#define GS_A3200_SDK_ADSB_HK_AVG_FPS_5MIN_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_AVG_FPS_5MIN, 0)
/** Set plane_count: Number of unique airplanes with at least one position decoded since boot */
#define GS_A3200_SDK_ADSB_HK_PLANE_COUNT_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_PLANE_COUNT, value, 0)
/** Set plane_count (no callback): Number of unique airplanes with at least one position decoded since boot */
#define GS_A3200_SDK_ADSB_HK_PLANE_COUNT_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_PLANE_COUNT, value, GS_PARAM_F_NO_CALLBACK)
/** Get plane_count: Number of unique airplanes with at least one position decoded since boot */
#define GS_A3200_SDK_ADSB_HK_PLANE_COUNT_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_PLANE_COUNT, value, 0)
/** Get plane_count (no check - return 0 on failure): Number of unique airplanes with at least one position decoded since boot */
#define GS_A3200_SDK_ADSB_HK_PLANE_COUNT_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_PLANE_COUNT, 0)
/** Set frame_count: Number of frames received since boot */
#define GS_A3200_SDK_ADSB_HK_FRAME_COUNT_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FRAME_COUNT, value, 0)
/** Set frame_count (no callback): Number of frames received since boot */
#define GS_A3200_SDK_ADSB_HK_FRAME_COUNT_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FRAME_COUNT, value, GS_PARAM_F_NO_CALLBACK)
/** Get frame_count: Number of frames received since boot */
#define GS_A3200_SDK_ADSB_HK_FRAME_COUNT_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FRAME_COUNT, value, 0)
/** Get frame_count (no check - return 0 on failure): Number of frames received since boot */
#define GS_A3200_SDK_ADSB_HK_FRAME_COUNT_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FRAME_COUNT, 0)
/** Set crc_corrected: Number of CRC errors corrected since boot */
#define GS_A3200_SDK_ADSB_HK_CRC_CORRECTED_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CRC_CORRECTED, value, 0)
/** Set crc_corrected (no callback): Number of CRC errors corrected since boot */
#define GS_A3200_SDK_ADSB_HK_CRC_CORRECTED_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CRC_CORRECTED, value, GS_PARAM_F_NO_CALLBACK)
/** Get crc_corrected: Number of CRC errors corrected since boot */
#define GS_A3200_SDK_ADSB_HK_CRC_CORRECTED_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CRC_CORRECTED, value, 0)
/** Get crc_corrected (no check - return 0 on failure): Number of CRC errors corrected since boot */
#define GS_A3200_SDK_ADSB_HK_CRC_CORRECTED_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CRC_CORRECTED, 0)
/** Set last_icao24: ICAO24 address of the latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_ICAO24_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_ICAO24, value, 0)
/** Set last_icao24 (no callback): ICAO24 address of the latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_ICAO24_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_ICAO24, value, GS_PARAM_F_NO_CALLBACK)
/** Get last_icao24: ICAO24 address of the latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_ICAO24_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_ICAO24, value, 0)
/** Get last_icao24 (no check - return 0 on failure): ICAO24 address of the latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_ICAO24_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_ICAO24, 0)
/** Set last_lat: Latitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_LAT_SET(value) gs_param_set_float(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_LAT, value, 0)
/** Set last_lat (no callback): Latitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_LAT_SET_NO_CALLBACK(value) gs_param_set_float(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_LAT, value, GS_PARAM_F_NO_CALLBACK)
/** Get last_lat: Latitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_LAT_GET(value) gs_param_get_float(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_LAT, value, 0)
/** Get last_lat (no check - return 0 on failure): Latitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_LAT_GET_NC() gs_param_get_float_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_LAT, 0)
/** Set last_lon: Longitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_LON_SET(value) gs_param_set_float(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_LON, value, 0)
/** Set last_lon (no callback): Longitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_LON_SET_NO_CALLBACK(value) gs_param_set_float(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_LON, value, GS_PARAM_F_NO_CALLBACK)
/** Get last_lon: Longitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_LON_GET(value) gs_param_get_float(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_LON, value, 0)
/** Get last_lon (no check - return 0 on failure): Longitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_LON_GET_NC() gs_param_get_float_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_LON, 0)
/** Set last_alt: Altitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_ALT_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_ALT, value, 0)
/** Set last_alt (no callback): Altitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_ALT_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_ALT, value, GS_PARAM_F_NO_CALLBACK)
/** Get last_alt: Altitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_ALT_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_ALT, value, 0)
/** Get last_alt (no check - return 0 on failure): Altitude of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_ALT_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_ALT, 0)
/** Set last_ts: Timestamp of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_TS_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_TS, value, 0)
/** Set last_ts (no callback): Timestamp of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_TS_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_TS, value, GS_PARAM_F_NO_CALLBACK)
/** Get last_ts: Timestamp of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_TS_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_TS, value, 0)
/** Get last_ts (no check - return 0 on failure): Timestamp of latest airplane received */
#define GS_A3200_SDK_ADSB_HK_LAST_TS_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_LAST_TS, 0)
/** Set bootcount: Board boot count */
#define GS_A3200_SDK_ADSB_HK_BOOTCOUNT_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_BOOTCOUNT, value, 0)
/** Set bootcount (no callback): Board boot count */
#define GS_A3200_SDK_ADSB_HK_BOOTCOUNT_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_BOOTCOUNT, value, GS_PARAM_F_NO_CALLBACK)
/** Get bootcount: Board boot count */
#define GS_A3200_SDK_ADSB_HK_BOOTCOUNT_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_BOOTCOUNT, value, 0)
/** Get bootcount (no check - return 0 on failure): Board boot count */
#define GS_A3200_SDK_ADSB_HK_BOOTCOUNT_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_BOOTCOUNT, 0)
/** Set bootcause: Board boot cause */
#define GS_A3200_SDK_ADSB_HK_BOOTCAUSE_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_BOOTCAUSE, value, 0)
/** Set bootcause (no callback): Board boot cause */
#define GS_A3200_SDK_ADSB_HK_BOOTCAUSE_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_BOOTCAUSE, value, GS_PARAM_F_NO_CALLBACK)
/** Get bootcause: Board boot cause */
#define GS_A3200_SDK_ADSB_HK_BOOTCAUSE_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_BOOTCAUSE, value, 0)
/** Get bootcause (no check - return 0 on failure): Board boot cause */
#define GS_A3200_SDK_ADSB_HK_BOOTCAUSE_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_BOOTCAUSE, 0)
/** Set current_time: Current board time in seconds since UNIX epoch */
#define GS_A3200_SDK_ADSB_HK_CURRENT_TIME_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CURRENT_TIME, value, 0)
/** Set current_time (no callback): Current board time in seconds since UNIX epoch */
#define GS_A3200_SDK_ADSB_HK_CURRENT_TIME_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CURRENT_TIME, value, GS_PARAM_F_NO_CALLBACK)
/** Get current_time: Current board time in seconds since UNIX epoch */
#define GS_A3200_SDK_ADSB_HK_CURRENT_TIME_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CURRENT_TIME, value, 0)
/** Get current_time (no check - return 0 on failure): Current board time in seconds since UNIX epoch */
#define GS_A3200_SDK_ADSB_HK_CURRENT_TIME_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_CURRENT_TIME, 0)
/** Set tot_frames: Total number of frames received */
#define GS_A3200_SDK_ADSB_HK_TOT_FRAMES_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_FRAMES, value, 0)
/** Set tot_frames (no callback): Total number of frames received */
#define GS_A3200_SDK_ADSB_HK_TOT_FRAMES_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_FRAMES, value, GS_PARAM_F_NO_CALLBACK)
/** Get tot_frames: Total number of frames received */
#define GS_A3200_SDK_ADSB_HK_TOT_FRAMES_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_FRAMES, value, 0)
/** Get tot_frames (no check - return 0 on failure): Total number of frames received */
#define GS_A3200_SDK_ADSB_HK_TOT_FRAMES_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_FRAMES, 0)
/** Set tot_planes: Total number of unique airplanes with at least one position decoded */
#define GS_A3200_SDK_ADSB_HK_TOT_PLANES_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_PLANES, value, 0)
/** Set tot_planes (no callback): Total number of unique airplanes with at least one position decoded */
#define GS_A3200_SDK_ADSB_HK_TOT_PLANES_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_PLANES, value, GS_PARAM_F_NO_CALLBACK)
/** Get tot_planes: Total number of unique airplanes with at least one position decoded */
#define GS_A3200_SDK_ADSB_HK_TOT_PLANES_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_PLANES, value, 0)
/** Get tot_planes (no check - return 0 on failure): Total number of unique airplanes with at least one position decoded */
#define GS_A3200_SDK_ADSB_HK_TOT_PLANES_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_PLANES, 0)
/** Set tot_crc_cor: Total number of CRC errors corrected */
#define GS_A3200_SDK_ADSB_HK_TOT_CRC_COR_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_CRC_COR, value, 0)
/** Set tot_crc_cor (no callback): Total number of CRC errors corrected */
#define GS_A3200_SDK_ADSB_HK_TOT_CRC_COR_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_CRC_COR, value, GS_PARAM_F_NO_CALLBACK)
/** Get tot_crc_cor: Total number of CRC errors corrected */
#define GS_A3200_SDK_ADSB_HK_TOT_CRC_COR_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_CRC_COR, value, 0)
/** Get tot_crc_cor (no check - return 0 on failure): Total number of CRC errors corrected */
#define GS_A3200_SDK_ADSB_HK_TOT_CRC_COR_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_TOT_CRC_COR, 0)
/** Set fpga_crc_cnt: Total number of CRC errors in FPGA configuration */
#define GS_A3200_SDK_ADSB_HK_FPGA_CRC_CNT_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FPGA_CRC_CNT, value, 0)
/** Set fpga_crc_cnt (no callback): Total number of CRC errors in FPGA configuration */
#define GS_A3200_SDK_ADSB_HK_FPGA_CRC_CNT_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FPGA_CRC_CNT, value, GS_PARAM_F_NO_CALLBACK)
/** Get fpga_crc_cnt: Total number of CRC errors in FPGA configuration */
#define GS_A3200_SDK_ADSB_HK_FPGA_CRC_CNT_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FPGA_CRC_CNT, value, 0)
/** Get fpga_crc_cnt (no check - return 0 on failure): Total number of CRC errors in FPGA configuration */
#define GS_A3200_SDK_ADSB_HK_FPGA_CRC_CNT_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FPGA_CRC_CNT, 0)
/** Set flash_lw:  */
#define GS_A3200_SDK_ADSB_HK_FLASH_LW_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FLASH_LW, value, 0)
/** Set flash_lw (no callback):  */
#define GS_A3200_SDK_ADSB_HK_FLASH_LW_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FLASH_LW, value, GS_PARAM_F_NO_CALLBACK)
/** Get flash_lw:  */
#define GS_A3200_SDK_ADSB_HK_FLASH_LW_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FLASH_LW, value, 0)
/** Get flash_lw (no check - return 0 on failure):  */
#define GS_A3200_SDK_ADSB_HK_FLASH_LW_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_ADSB_HK_TABLE_ID), GS_A3200_SDK_ADSB_HK_FLASH_LW, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
