#ifndef GS_A3200_SDK_PARAM_HOST_EPS_HK_H
#define GS_A3200_SDK_PARAM_HOST_EPS_HK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a eps_hk.
*/

#include <gs/a3200_sdk/param/eps_hk.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200_sdk_eps_hk_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Voltage of boost converters [mV] [PV1, PV2, PV3] */
    uint16_t vboost[3];
    /** Voltage of battery */
    uint16_t vbatt;
    /** Current out */
    uint16_t curout[6];
    /** Current in */
    uint16_t curin[3];
    /** Current from boost converters */
    uint16_t cursun;
    /** Current out of battery */
    uint16_t cursys;
    /** Temperature sensors [0 = TEMP1, TEMP2, TEMP3, TEMP4, BATT0, BATT1] */
    int16_t  temp[6];
    /** Status of outputs */
    uint8_t  out_val[8];
    /** Mode for battery [0 = normal, 1 = undervoltage, 2 = overvoltage] */
    uint8_t  battmode;
    /** Mode for PPT [0 = HW, 1 = MPPT, 2 = FIXED] */
    uint8_t  pptmode;
    /** Time left on I2C wdt */
    uint32_t wdtI2cS;
    /** Time left on Gnd wdt */
    uint32_t wdtGndS;
    /** Number of EPS reboots */
    uint32_t bootcount;
    /** Number of WDT I2C reboots */
    uint32_t cntWdtI2c;
    /** Number of WDT GND reboots */
    uint32_t cntWdtGnd;
    /** Number of WDT CSP reboots */
    uint32_t cntWdtCsp[2];
    /** Pings left on CSP wdt */
    uint32_t wdtCspC[2];
    /** Number of latch-ups */
    uint16_t latchups[6];
    /** Cause of last EPS reset */
    uint8_t  bootcause;
} gs_a3200_sdk_eps_hk_memory_t;

/** Table default values. */
extern const gs_a3200_sdk_eps_hk_memory_t gs_a3200_sdk_eps_hk_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set vboost: Voltage of boost converters [mV] [PV1, PV2, PV3] */
#define GS_A3200_SDK_EPS_HK_VBOOST_SET(idx, value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_VBOOST(idx), value, 0)
/** Set vboost (no callback): Voltage of boost converters [mV] [PV1, PV2, PV3] */
#define GS_A3200_SDK_EPS_HK_VBOOST_SET_NO_CALLBACK(idx, value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_VBOOST(idx), value, GS_PARAM_F_NO_CALLBACK)
/** Get vboost: Voltage of boost converters [mV] [PV1, PV2, PV3] */
#define GS_A3200_SDK_EPS_HK_VBOOST_GET(idx, value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_VBOOST(idx), value, 0)
/** Get vboost (no check - return 0 on failure): Voltage of boost converters [mV] [PV1, PV2, PV3] */
#define GS_A3200_SDK_EPS_HK_VBOOST_GET_NC(idx) gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_VBOOST(idx), 0)
/** Set vbatt: Voltage of battery */
#define GS_A3200_SDK_EPS_HK_VBATT_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_VBATT, value, 0)
/** Set vbatt (no callback): Voltage of battery */
#define GS_A3200_SDK_EPS_HK_VBATT_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_VBATT, value, GS_PARAM_F_NO_CALLBACK)
/** Get vbatt: Voltage of battery */
#define GS_A3200_SDK_EPS_HK_VBATT_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_VBATT, value, 0)
/** Get vbatt (no check - return 0 on failure): Voltage of battery */
#define GS_A3200_SDK_EPS_HK_VBATT_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_VBATT, 0)
/** Set curout: Current out */
#define GS_A3200_SDK_EPS_HK_CUROUT_SET(idx, value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CUROUT(idx), value, 0)
/** Set curout (no callback): Current out */
#define GS_A3200_SDK_EPS_HK_CUROUT_SET_NO_CALLBACK(idx, value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CUROUT(idx), value, GS_PARAM_F_NO_CALLBACK)
/** Get curout: Current out */
#define GS_A3200_SDK_EPS_HK_CUROUT_GET(idx, value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CUROUT(idx), value, 0)
/** Get curout (no check - return 0 on failure): Current out */
#define GS_A3200_SDK_EPS_HK_CUROUT_GET_NC(idx) gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CUROUT(idx), 0)
/** Set curin: Current in */
#define GS_A3200_SDK_EPS_HK_CURIN_SET(idx, value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURIN(idx), value, 0)
/** Set curin (no callback): Current in */
#define GS_A3200_SDK_EPS_HK_CURIN_SET_NO_CALLBACK(idx, value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURIN(idx), value, GS_PARAM_F_NO_CALLBACK)
/** Get curin: Current in */
#define GS_A3200_SDK_EPS_HK_CURIN_GET(idx, value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURIN(idx), value, 0)
/** Get curin (no check - return 0 on failure): Current in */
#define GS_A3200_SDK_EPS_HK_CURIN_GET_NC(idx) gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURIN(idx), 0)
/** Set cursun: Current from boost converters */
#define GS_A3200_SDK_EPS_HK_CURSUN_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURSUN, value, 0)
/** Set cursun (no callback): Current from boost converters */
#define GS_A3200_SDK_EPS_HK_CURSUN_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURSUN, value, GS_PARAM_F_NO_CALLBACK)
/** Get cursun: Current from boost converters */
#define GS_A3200_SDK_EPS_HK_CURSUN_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURSUN, value, 0)
/** Get cursun (no check - return 0 on failure): Current from boost converters */
#define GS_A3200_SDK_EPS_HK_CURSUN_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURSUN, 0)
/** Set cursys: Current out of battery */
#define GS_A3200_SDK_EPS_HK_CURSYS_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURSYS, value, 0)
/** Set cursys (no callback): Current out of battery */
#define GS_A3200_SDK_EPS_HK_CURSYS_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURSYS, value, GS_PARAM_F_NO_CALLBACK)
/** Get cursys: Current out of battery */
#define GS_A3200_SDK_EPS_HK_CURSYS_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURSYS, value, 0)
/** Get cursys (no check - return 0 on failure): Current out of battery */
#define GS_A3200_SDK_EPS_HK_CURSYS_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CURSYS, 0)
/** Set temp: Temperature sensors [0 = TEMP1, TEMP2, TEMP3, TEMP4, BATT0, BATT1] */
#define GS_A3200_SDK_EPS_HK_TEMP_SET(idx, value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_TEMP(idx), value, 0)
/** Set temp (no callback): Temperature sensors [0 = TEMP1, TEMP2, TEMP3, TEMP4, BATT0, BATT1] */
#define GS_A3200_SDK_EPS_HK_TEMP_SET_NO_CALLBACK(idx, value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_TEMP(idx), value, GS_PARAM_F_NO_CALLBACK)
/** Get temp: Temperature sensors [0 = TEMP1, TEMP2, TEMP3, TEMP4, BATT0, BATT1] */
#define GS_A3200_SDK_EPS_HK_TEMP_GET(idx, value) gs_param_get_int16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_TEMP(idx), value, 0)
/** Get temp (no check - return 0 on failure): Temperature sensors [0 = TEMP1, TEMP2, TEMP3, TEMP4, BATT0, BATT1] */
#define GS_A3200_SDK_EPS_HK_TEMP_GET_NC(idx) gs_param_get_int16_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_TEMP(idx), 0)
/** Set out_val: Status of outputs */
#define GS_A3200_SDK_EPS_HK_OUT_VAL_SET(idx, value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_OUT_VAL(idx), value, 0)
/** Set out_val (no callback): Status of outputs */
#define GS_A3200_SDK_EPS_HK_OUT_VAL_SET_NO_CALLBACK(idx, value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_OUT_VAL(idx), value, GS_PARAM_F_NO_CALLBACK)
/** Get out_val: Status of outputs */
#define GS_A3200_SDK_EPS_HK_OUT_VAL_GET(idx, value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_OUT_VAL(idx), value, 0)
/** Get out_val (no check - return 0 on failure): Status of outputs */
#define GS_A3200_SDK_EPS_HK_OUT_VAL_GET_NC(idx) gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_OUT_VAL(idx), 0)
/** Set battmode: Mode for battery [0 = normal, 1 = undervoltage, 2 = overvoltage] */
#define GS_A3200_SDK_EPS_HK_BATTMODE_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BATTMODE, value, 0)
/** Set battmode (no callback): Mode for battery [0 = normal, 1 = undervoltage, 2 = overvoltage] */
#define GS_A3200_SDK_EPS_HK_BATTMODE_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BATTMODE, value, GS_PARAM_F_NO_CALLBACK)
/** Get battmode: Mode for battery [0 = normal, 1 = undervoltage, 2 = overvoltage] */
#define GS_A3200_SDK_EPS_HK_BATTMODE_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BATTMODE, value, 0)
/** Get battmode (no check - return 0 on failure): Mode for battery [0 = normal, 1 = undervoltage, 2 = overvoltage] */
#define GS_A3200_SDK_EPS_HK_BATTMODE_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BATTMODE, 0)
/** Set pptmode: Mode for PPT [0 = HW, 1 = MPPT, 2 = FIXED] */
#define GS_A3200_SDK_EPS_HK_PPTMODE_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_PPTMODE, value, 0)
/** Set pptmode (no callback): Mode for PPT [0 = HW, 1 = MPPT, 2 = FIXED] */
#define GS_A3200_SDK_EPS_HK_PPTMODE_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_PPTMODE, value, GS_PARAM_F_NO_CALLBACK)
/** Get pptmode: Mode for PPT [0 = HW, 1 = MPPT, 2 = FIXED] */
#define GS_A3200_SDK_EPS_HK_PPTMODE_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_PPTMODE, value, 0)
/** Get pptmode (no check - return 0 on failure): Mode for PPT [0 = HW, 1 = MPPT, 2 = FIXED] */
#define GS_A3200_SDK_EPS_HK_PPTMODE_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_PPTMODE, 0)
/** Set wdtI2cS: Time left on I2C wdt */
#define GS_A3200_SDK_EPS_HK_WDTI2CS_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTI2CS, value, 0)
/** Set wdtI2cS (no callback): Time left on I2C wdt */
#define GS_A3200_SDK_EPS_HK_WDTI2CS_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTI2CS, value, GS_PARAM_F_NO_CALLBACK)
/** Get wdtI2cS: Time left on I2C wdt */
#define GS_A3200_SDK_EPS_HK_WDTI2CS_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTI2CS, value, 0)
/** Get wdtI2cS (no check - return 0 on failure): Time left on I2C wdt */
#define GS_A3200_SDK_EPS_HK_WDTI2CS_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTI2CS, 0)
/** Set wdtGndS: Time left on Gnd wdt */
#define GS_A3200_SDK_EPS_HK_WDTGNDS_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTGNDS, value, 0)
/** Set wdtGndS (no callback): Time left on Gnd wdt */
#define GS_A3200_SDK_EPS_HK_WDTGNDS_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTGNDS, value, GS_PARAM_F_NO_CALLBACK)
/** Get wdtGndS: Time left on Gnd wdt */
#define GS_A3200_SDK_EPS_HK_WDTGNDS_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTGNDS, value, 0)
/** Get wdtGndS (no check - return 0 on failure): Time left on Gnd wdt */
#define GS_A3200_SDK_EPS_HK_WDTGNDS_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTGNDS, 0)
/** Set bootcount: Number of EPS reboots */
#define GS_A3200_SDK_EPS_HK_BOOTCOUNT_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BOOTCOUNT, value, 0)
/** Set bootcount (no callback): Number of EPS reboots */
#define GS_A3200_SDK_EPS_HK_BOOTCOUNT_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BOOTCOUNT, value, GS_PARAM_F_NO_CALLBACK)
/** Get bootcount: Number of EPS reboots */
#define GS_A3200_SDK_EPS_HK_BOOTCOUNT_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BOOTCOUNT, value, 0)
/** Get bootcount (no check - return 0 on failure): Number of EPS reboots */
#define GS_A3200_SDK_EPS_HK_BOOTCOUNT_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BOOTCOUNT, 0)
/** Set cntWdtI2c: Number of WDT I2C reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTI2C_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTI2C, value, 0)
/** Set cntWdtI2c (no callback): Number of WDT I2C reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTI2C_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTI2C, value, GS_PARAM_F_NO_CALLBACK)
/** Get cntWdtI2c: Number of WDT I2C reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTI2C_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTI2C, value, 0)
/** Get cntWdtI2c (no check - return 0 on failure): Number of WDT I2C reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTI2C_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTI2C, 0)
/** Set cntWdtGnd: Number of WDT GND reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTGND_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTGND, value, 0)
/** Set cntWdtGnd (no callback): Number of WDT GND reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTGND_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTGND, value, GS_PARAM_F_NO_CALLBACK)
/** Get cntWdtGnd: Number of WDT GND reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTGND_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTGND, value, 0)
/** Get cntWdtGnd (no check - return 0 on failure): Number of WDT GND reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTGND_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTGND, 0)
/** Set cntWdtCsp: Number of WDT CSP reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTCSP_SET(idx, value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTCSP(idx), value, 0)
/** Set cntWdtCsp (no callback): Number of WDT CSP reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTCSP_SET_NO_CALLBACK(idx, value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTCSP(idx), value, GS_PARAM_F_NO_CALLBACK)
/** Get cntWdtCsp: Number of WDT CSP reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTCSP_GET(idx, value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTCSP(idx), value, 0)
/** Get cntWdtCsp (no check - return 0 on failure): Number of WDT CSP reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTCSP_GET_NC(idx) gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_CNTWDTCSP(idx), 0)
/** Set wdtCspC: Pings left on CSP wdt */
#define GS_A3200_SDK_EPS_HK_WDTCSPC_SET(idx, value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTCSPC(idx), value, 0)
/** Set wdtCspC (no callback): Pings left on CSP wdt */
#define GS_A3200_SDK_EPS_HK_WDTCSPC_SET_NO_CALLBACK(idx, value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTCSPC(idx), value, GS_PARAM_F_NO_CALLBACK)
/** Get wdtCspC: Pings left on CSP wdt */
#define GS_A3200_SDK_EPS_HK_WDTCSPC_GET(idx, value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTCSPC(idx), value, 0)
/** Get wdtCspC (no check - return 0 on failure): Pings left on CSP wdt */
#define GS_A3200_SDK_EPS_HK_WDTCSPC_GET_NC(idx) gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_WDTCSPC(idx), 0)
/** Set latchups: Number of latch-ups */
#define GS_A3200_SDK_EPS_HK_LATCHUPS_SET(idx, value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_LATCHUPS(idx), value, 0)
/** Set latchups (no callback): Number of latch-ups */
#define GS_A3200_SDK_EPS_HK_LATCHUPS_SET_NO_CALLBACK(idx, value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_LATCHUPS(idx), value, GS_PARAM_F_NO_CALLBACK)
/** Get latchups: Number of latch-ups */
#define GS_A3200_SDK_EPS_HK_LATCHUPS_GET(idx, value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_LATCHUPS(idx), value, 0)
/** Get latchups (no check - return 0 on failure): Number of latch-ups */
#define GS_A3200_SDK_EPS_HK_LATCHUPS_GET_NC(idx) gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_LATCHUPS(idx), 0)
/** Set bootcause: Cause of last EPS reset */
#define GS_A3200_SDK_EPS_HK_BOOTCAUSE_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BOOTCAUSE, value, 0)
/** Set bootcause (no callback): Cause of last EPS reset */
#define GS_A3200_SDK_EPS_HK_BOOTCAUSE_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BOOTCAUSE, value, GS_PARAM_F_NO_CALLBACK)
/** Get bootcause: Cause of last EPS reset */
#define GS_A3200_SDK_EPS_HK_BOOTCAUSE_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BOOTCAUSE, value, 0)
/** Get bootcause (no check - return 0 on failure): Cause of last EPS reset */
#define GS_A3200_SDK_EPS_HK_BOOTCAUSE_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_SDK_EPS_HK_TABLE_ID), GS_A3200_SDK_EPS_HK_BOOTCAUSE, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
