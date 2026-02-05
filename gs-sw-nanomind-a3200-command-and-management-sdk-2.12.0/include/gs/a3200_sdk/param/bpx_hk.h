#ifndef GS_A3200_SDK_PARAM_BPX_HK_H
#define GS_A3200_SDK_PARAM_BPX_HK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions for \a bpx_hk.
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Table name */
#define GS_A3200_SDK_BPX_HK_TABLE_NAME                   "bpx_hk"
/** Table id */
#define GS_A3200_SDK_BPX_HK_TABLE_ID                     91
/** Legacy table id definition */
#define GS_A3200_SDK_BPX_HK_TABLE_MEM_ID                 91
/** Table data size (bytes */
#define GS_A3200_SDK_BPX_HK_TABLE_SIZE                   21 // bytes
/** Table rows (count) */
#define GS_A3200_SDK_BPX_HK_TABLE_ROWS                   10

/**
   @defgroup param_addr Parameter addresses within the table.
   @{
*/
/** Address of c_charge: Charging current  */
#define GS_A3200_SDK_BPX_HK_C_CHARGE                                     0                  // 0x0000 uint16
/** Address of c_dcharge: Discharging current  */
#define GS_A3200_SDK_BPX_HK_C_DCHARGE                                    2                  // 0x0002 uint16
/** Address of c_heater: Heater current  */
#define GS_A3200_SDK_BPX_HK_C_HEATER                                     4                  // 0x0004 uint16
/** Address of vbatt: Voltage of battery */
#define GS_A3200_SDK_BPX_HK_VBATT                                        6                  // 0x0006 uint16
/** Address of bat_temp1: Battery temperature 1 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP1                                    8                  // 0x0008 int16
/** Address of bat_temp2: Battery temperature 2 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP2                                   10                  // 0x000a int16
/** Address of bat_temp3: Battery temperature 3 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP3                                   12                  // 0x000c int16
/** Address of bat_temp4: Battery temperature 4 */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP4                                   14                  // 0x000e int16
/** Address of bootcount: Number of BPX reboots */
#define GS_A3200_SDK_BPX_HK_BOOTCOUNT                                   16                  // 0x0010 uint32
/** Address of bootcause: Cause of last BPX reset */
#define GS_A3200_SDK_BPX_HK_BOOTCAUSE                                   20                  // 0x0014 uint8
/**@}*/

/**
   @defgroup Invalid values for table parameters.
   @{
*/
/** c_charge invalid value. */
#define GS_A3200_SDK_BPX_HK_C_CHARGE_INVALID             65535
/** c_dcharge invalid value. */
#define GS_A3200_SDK_BPX_HK_C_DCHARGE_INVALID            65535
/** c_heater invalid value. */
#define GS_A3200_SDK_BPX_HK_C_HEATER_INVALID             65535
/** vbatt invalid value. */
#define GS_A3200_SDK_BPX_HK_VBATT_INVALID                0
/** bat_temp1 invalid value. */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP1_INVALID            -32768
/** bat_temp2 invalid value. */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP2_INVALID            -32768
/** bat_temp3 invalid value. */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP3_INVALID            -32768
/** bat_temp4 invalid value. */
#define GS_A3200_SDK_BPX_HK_BAT_TEMP4_INVALID            -32768
/** bootcount invalid value. */
#define GS_A3200_SDK_BPX_HK_BOOTCOUNT_INVALID            4294967295
/** bootcause invalid value. */
#define GS_A3200_SDK_BPX_HK_BOOTCAUSE_INVALID            255
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
