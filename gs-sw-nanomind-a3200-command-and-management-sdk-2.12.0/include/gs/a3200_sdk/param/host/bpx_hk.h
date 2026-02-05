#ifndef GS_A3200_SDK_PARAM_HOST_BPX_HK_H
#define GS_A3200_SDK_PARAM_HOST_BPX_HK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a bpx_hk.
*/

#include <gs/a3200_sdk/param/bpx_hk.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200_sdk_bpx_hk_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Charging current  */
    uint16_t c_charge;
    /** Discharging current  */
    uint16_t c_dcharge;
    /** Heater current  */
    uint16_t c_heater;
    /** Voltage of battery */
    uint16_t vbatt;
    /** Battery temperature 1 */
    int16_t  bat_temp1;
    /** Battery temperature 2 */
    int16_t  bat_temp2;
    /** Battery temperature 3 */
    int16_t  bat_temp3;
    /** Battery temperature 4 */
    int16_t  bat_temp4;
    /** Number of BPX reboots */
    uint32_t bootcount;
    /** Cause of last BPX reset */
    uint8_t  bootcause;
} gs_a3200_sdk_bpx_hk_memory_t;

/** Table default values. */
extern const gs_a3200_sdk_bpx_hk_memory_t gs_a3200_sdk_bpx_hk_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set c_charge: Charging current  */
#define GS_A3200_SDK_BPX_HK_C_CHARGE_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_CHARGE, value, 0)
/** Set c_charge (no callback): Charging current  */
#define GS_A3200_SDK_BPX_HK_C_CHARGE_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_CHARGE, value, GS_PARAM_F_NO_CALLBACK)
/** Get c_charge: Charging current  */
#define GS_A3200_SDK_BPX_HK_C_CHARGE_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_CHARGE, value, 0)
/** Get c_charge (no check - return 0 on failure): Charging current  */
#define GS_A3200_SDK_BPX_HK_C_CHARGE_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_CHARGE, 0)
/** Set c_dcharge: Discharging current  */
#define GS_A3200_SDK_BPX_HK_C_DCHARGE_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_DCHARGE, value, 0)
/** Set c_dcharge (no callback): Discharging current  */
#define GS_A3200_SDK_BPX_HK_C_DCHARGE_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_DCHARGE, value, GS_PARAM_F_NO_CALLBACK)
/** Get c_dcharge: Discharging current  */
#define GS_A3200_SDK_BPX_HK_C_DCHARGE_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_DCHARGE, value, 0)
/** Get c_dcharge (no check - return 0 on failure): Discharging current  */
#define GS_A3200_SDK_BPX_HK_C_DCHARGE_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_DCHARGE, 0)
/** Set c_heater: Heater current  */
#define GS_A3200_SDK_BPX_HK_C_HEATER_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_HEATER, value, 0)
/** Set c_heater (no callback): Heater current  */
#define GS_A3200_SDK_BPX_HK_C_HEATER_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_HEATER, value, GS_PARAM_F_NO_CALLBACK)
/** Get c_heater: Heater current  */
#define GS_A3200_SDK_BPX_HK_C_HEATER_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_HEATER, value, 0)
/** Get c_heater (no check - return 0 on failure): Heater current  */
#define GS_A3200_SDK_BPX_HK_C_HEATER_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_C_HEATER, 0)
/** Set vbatt: Voltage of battery */
#define GS_A3200_SDK_BPX_HK_VBATT_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_VBATT, value, 0)
/** Set vbatt (no callback): Voltage of battery */
#define GS_A3200_SDK_BPX_HK_VBATT_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_VBATT, value, GS_PARAM_F_NO_CALLBACK)
/** Get vbatt: Voltage of battery */
#define GS_A3200_SDK_BPX_HK_VBATT_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_VBATT, value, 0)
/** Get vbatt (no check - return 0 on failure): Voltage of battery */
#define GS_A3200_SDK_BPX_HK_VBATT_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_VBATT, 0)
/** Set bat_temp1: Battery temperature 1 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP1_SET(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP1, value, 0)
/** Set bat_temp1 (no callback): Battery temperature 1 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP1_SET_NO_CALLBACK(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP1, value, GS_PARAM_F_NO_CALLBACK)
/** Get bat_temp1: Battery temperature 1 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP1_GET(value) gs_param_get_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP1, value, 0)
/** Get bat_temp1 (no check - return 0 on failure): Battery temperature 1 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP1_GET_NC() gs_param_get_int16_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP1, 0)
/** Set bat_temp2: Battery temperature 2 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP2_SET(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP2, value, 0)
/** Set bat_temp2 (no callback): Battery temperature 2 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP2_SET_NO_CALLBACK(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP2, value, GS_PARAM_F_NO_CALLBACK)
/** Get bat_temp2: Battery temperature 2 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP2_GET(value) gs_param_get_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP2, value, 0)
/** Get bat_temp2 (no check - return 0 on failure): Battery temperature 2 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP2_GET_NC() gs_param_get_int16_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP2, 0)
/** Set bat_temp3: Battery temperature 3 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP3_SET(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP3, value, 0)
/** Set bat_temp3 (no callback): Battery temperature 3 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP3_SET_NO_CALLBACK(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP3, value, GS_PARAM_F_NO_CALLBACK)
/** Get bat_temp3: Battery temperature 3 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP3_GET(value) gs_param_get_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP3, value, 0)
/** Get bat_temp3 (no check - return 0 on failure): Battery temperature 3 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP3_GET_NC() gs_param_get_int16_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP3, 0)
/** Set bat_temp4: Battery temperature 4 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP4_SET(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP4, value, 0)
/** Set bat_temp4 (no callback): Battery temperature 4 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP4_SET_NO_CALLBACK(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP4, value, GS_PARAM_F_NO_CALLBACK)
/** Get bat_temp4: Battery temperature 4 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP4_GET(value) gs_param_get_int16(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP4, value, 0)
/** Get bat_temp4 (no check - return 0 on failure): Battery temperature 4 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP4_GET_NC() gs_param_get_int16_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BAT_TEMP4, 0)
/** Set bootcount: Number of BPX reboots */
#define GS_A3200_SDK_BPX_HK_BOOTCOUNT_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BOOTCOUNT, value, 0)
/** Set bootcount (no callback): Number of BPX reboots */
#define GS_A3200_SDK_BPX_HK_BOOTCOUNT_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BOOTCOUNT, value, GS_PARAM_F_NO_CALLBACK)
/** Get bootcount: Number of BPX reboots */
#define GS_A3200_SDK_BPX_HK_BOOTCOUNT_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BOOTCOUNT, value, 0)
/** Get bootcount (no check - return 0 on failure): Number of BPX reboots */
#define GS_A3200_SDK_BPX_HK_BOOTCOUNT_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BOOTCOUNT, 0)
/** Set bootcause: Cause of last BPX reset */
#define GS_A3200_SDK_BPX_HK_BOOTCAUSE_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BOOTCAUSE, value, 0)
/** Set bootcause (no callback): Cause of last BPX reset */
#define GS_A3200_SDK_BPX_HK_BOOTCAUSE_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BOOTCAUSE, value, GS_PARAM_F_NO_CALLBACK)
/** Get bootcause: Cause of last BPX reset */
#define GS_A3200_SDK_BPX_HK_BOOTCAUSE_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BOOTCAUSE, value, 0)
/** Get bootcause (no check - return 0 on failure): Cause of last BPX reset */
#define GS_A3200_SDK_BPX_HK_BOOTCAUSE_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_SDK_BPX_HK_TABLE_ID), GS_A3200_SDK_BPX_HK_BOOTCAUSE, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
