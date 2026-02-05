#ifndef GS_A3200_SDK_PARAM_EPS_HK_H
#define GS_A3200_SDK_PARAM_EPS_HK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions for \a eps_hk.
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Table name */
#define GS_A3200_SDK_EPS_HK_TABLE_NAME                   "eps_hk"
/** Table id */
#define GS_A3200_SDK_EPS_HK_TABLE_ID                     90
/** Legacy table id definition */
#define GS_A3200_SDK_EPS_HK_TABLE_MEM_ID                 90
/** Table data size (bytes */
#define GS_A3200_SDK_EPS_HK_TABLE_SIZE                   101 // bytes
/** Table rows (count) */
#define GS_A3200_SDK_EPS_HK_TABLE_ROWS                   19

/**
   @defgroup param_addr Parameter addresses within the table.
   @{
*/
/** Address of vboost: Voltage of boost converters [mV] [PV1, PV2, PV3] */
#define GS_A3200_SDK_EPS_HK_VBOOST(idx)                             (    0 + ( 2 * (idx)))  // 0x0000 uint16
/** Array size of vboost: Voltage of boost converters [mV] [PV1, PV2, PV3] */
#define GS_A3200_SDK_EPS_HK_VBOOST_ARRAY_SIZE                            3
/** Address of vbatt: Voltage of battery */
#define GS_A3200_SDK_EPS_HK_VBATT                                        6                  // 0x0006 uint16
/** Address of curout: Current out */
#define GS_A3200_SDK_EPS_HK_CUROUT(idx)                             (    8 + ( 2 * (idx)))  // 0x0008 uint16
/** Array size of curout: Current out */
#define GS_A3200_SDK_EPS_HK_CUROUT_ARRAY_SIZE                            6
/** Address of curin: Current in */
#define GS_A3200_SDK_EPS_HK_CURIN(idx)                              (   20 + ( 2 * (idx)))  // 0x0014 uint16
/** Array size of curin: Current in */
#define GS_A3200_SDK_EPS_HK_CURIN_ARRAY_SIZE                             3
/** Address of cursun: Current from boost converters */
#define GS_A3200_SDK_EPS_HK_CURSUN                                      26                  // 0x001a uint16
/** Address of cursys: Current out of battery */
#define GS_A3200_SDK_EPS_HK_CURSYS                                      28                  // 0x001c uint16
/** Address of temp: Temperature sensors [0 = TEMP1, TEMP2, TEMP3, TEMP4, BATT0, BATT1] */
#define GS_A3200_SDK_EPS_HK_TEMP(idx)                               (   30 + ( 2 * (idx)))  // 0x001e int16
/** Array size of temp: Temperature sensors [0 = TEMP1, TEMP2, TEMP3, TEMP4, BATT0, BATT1] */
#define GS_A3200_SDK_EPS_HK_TEMP_ARRAY_SIZE                              6
/** Address of out_val: Status of outputs */
#define GS_A3200_SDK_EPS_HK_OUT_VAL(idx)                            (   42 + ( 1 * (idx)))  // 0x002a uint8
/** Array size of out_val: Status of outputs */
#define GS_A3200_SDK_EPS_HK_OUT_VAL_ARRAY_SIZE                           8
/** Address of battmode: Mode for battery [0 = normal, 1 = undervoltage, 2 = overvoltage] */
#define GS_A3200_SDK_EPS_HK_BATTMODE                                    50                  // 0x0032 uint8
/** Address of pptmode: Mode for PPT [0 = HW, 1 = MPPT, 2 = FIXED] */
#define GS_A3200_SDK_EPS_HK_PPTMODE                                     51                  // 0x0033 uint8
/** Address of wdtI2cS: Time left on I2C wdt */
#define GS_A3200_SDK_EPS_HK_WDTI2CS                                     52                  // 0x0034 uint32
/** Address of wdtGndS: Time left on Gnd wdt */
#define GS_A3200_SDK_EPS_HK_WDTGNDS                                     56                  // 0x0038 uint32
/** Address of bootcount: Number of EPS reboots */
#define GS_A3200_SDK_EPS_HK_BOOTCOUNT                                   60                  // 0x003c uint32
/** Address of cntWdtI2c: Number of WDT I2C reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTI2C                                   64                  // 0x0040 uint32
/** Address of cntWdtGnd: Number of WDT GND reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTGND                                   68                  // 0x0044 uint32
/** Address of cntWdtCsp: Number of WDT CSP reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTCSP(idx)                          (   72 + ( 4 * (idx)))  // 0x0048 uint32
/** Array size of cntWdtCsp: Number of WDT CSP reboots */
#define GS_A3200_SDK_EPS_HK_CNTWDTCSP_ARRAY_SIZE                         2
/** Address of wdtCspC: Pings left on CSP wdt */
#define GS_A3200_SDK_EPS_HK_WDTCSPC(idx)                            (   80 + ( 4 * (idx)))  // 0x0050 uint32
/** Array size of wdtCspC: Pings left on CSP wdt */
#define GS_A3200_SDK_EPS_HK_WDTCSPC_ARRAY_SIZE                           2
/** Address of latchups: Number of latch-ups */
#define GS_A3200_SDK_EPS_HK_LATCHUPS(idx)                           (   88 + ( 2 * (idx)))  // 0x0058 uint16
/** Array size of latchups: Number of latch-ups */
#define GS_A3200_SDK_EPS_HK_LATCHUPS_ARRAY_SIZE                          6
/** Address of bootcause: Cause of last EPS reset */
#define GS_A3200_SDK_EPS_HK_BOOTCAUSE                                  100                  // 0x0064 uint8
/**@}*/

/**
   @defgroup Invalid values for table parameters.
   @{
*/
/** vboost invalid value. */
#define GS_A3200_SDK_EPS_HK_VBOOST_INVALID               0
/** vbatt invalid value. */
#define GS_A3200_SDK_EPS_HK_VBATT_INVALID                0
/** curout invalid value. */
#define GS_A3200_SDK_EPS_HK_CUROUT_INVALID               65535
/** curin invalid value. */
#define GS_A3200_SDK_EPS_HK_CURIN_INVALID                65535
/** cursun invalid value. */
#define GS_A3200_SDK_EPS_HK_CURSUN_INVALID               65535
/** cursys invalid value. */
#define GS_A3200_SDK_EPS_HK_CURSYS_INVALID               65535
/** temp invalid value. */
#define GS_A3200_SDK_EPS_HK_TEMP_INVALID                 -32768
/** out_val invalid value. */
#define GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID              255
/** battmode invalid value. */
#define GS_A3200_SDK_EPS_HK_BATTMODE_INVALID             255
/** pptmode invalid value. */
#define GS_A3200_SDK_EPS_HK_PPTMODE_INVALID              255
/** wdtI2cS invalid value. */
#define GS_A3200_SDK_EPS_HK_WDTI2CS_INVALID              4294967295
/** wdtGndS invalid value. */
#define GS_A3200_SDK_EPS_HK_WDTGNDS_INVALID              4294967295
/** bootcount invalid value. */
#define GS_A3200_SDK_EPS_HK_BOOTCOUNT_INVALID            4294967295
/** cntWdtI2c invalid value. */
#define GS_A3200_SDK_EPS_HK_CNTWDTI2C_INVALID            4294967295
/** cntWdtGnd invalid value. */
#define GS_A3200_SDK_EPS_HK_CNTWDTGND_INVALID            4294967295
/** cntWdtCsp invalid value. */
#define GS_A3200_SDK_EPS_HK_CNTWDTCSP_INVALID            4294967295
/** wdtCspC invalid value. */
#define GS_A3200_SDK_EPS_HK_WDTCSPC_INVALID              4294967295
/** latchups invalid value. */
#define GS_A3200_SDK_EPS_HK_LATCHUPS_INVALID             65535
/** bootcause invalid value. */
#define GS_A3200_SDK_EPS_HK_BOOTCAUSE_INVALID            255
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
