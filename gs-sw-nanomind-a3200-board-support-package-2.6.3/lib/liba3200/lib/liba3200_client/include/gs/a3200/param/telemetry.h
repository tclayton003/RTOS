#ifndef GS_A3200_PARAM_TELEMETRY_H
#define GS_A3200_PARAM_TELEMETRY_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions for \a telemetry.
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Table name */
#define GS_A3200_TELEMETRY_TABLE_NAME                    "telemetry"
/** Table id */
#define GS_A3200_TELEMETRY_TABLE_ID                      4
/** Legacy table id definition */
#define GS_A3200_TELEMETRY_TABLE_MEM_ID                  4
/** Table data size (bytes */
#define GS_A3200_TELEMETRY_TABLE_SIZE                    36 // bytes
/** Table rows (count) */
#define GS_A3200_TELEMETRY_TABLE_ROWS                    13

/**
   @defgroup param_addr Parameter addresses within the table.
   @{
*/
/** Address of fs_mounted: Filesystem mounted */
#define GS_A3200_TELEMETRY_FS_MOUNTED                                    0                  // 0x0000 bool
/** Address of ram_image: Is RAM image */
#define GS_A3200_TELEMETRY_RAM_IMAGE                                     1                  // 0x0001 bool
/** Address of temp_mcu: MCU Temperature */
#define GS_A3200_TELEMETRY_TEMP_MCU                                      2                  // 0x0002 int16
/** Address of temp_ram: RAM Temperature */
#define GS_A3200_TELEMETRY_TEMP_RAM                                      4                  // 0x0004 int16
/** Address of i_GSSB1: GSSB1 current */
#define GS_A3200_TELEMETRY_I_GSSB1                                       6                  // 0x0006 uint16
/** Address of i_GSSB2: GSSB2 current */
#define GS_A3200_TELEMETRY_I_GSSB2                                       8                  // 0x0008 uint16
/** Address of i_Flash: Flash current */
#define GS_A3200_TELEMETRY_I_FLASH                                      10                  // 0x000a uint16
/** Address of i_PWM: PWM current */
#define GS_A3200_TELEMETRY_I_PWM                                        12                  // 0x000c uint16
/** Address of resetcause: Reset cause */
#define GS_A3200_TELEMETRY_RESETCAUSE                                   16                  // 0x0010 uint32
/** Address of bootcause: Bootcause */
#define GS_A3200_TELEMETRY_BOOTCAUSE                                    20                  // 0x0014 uint32
/** Address of bootcount: Bootcount */
#define GS_A3200_TELEMETRY_BOOTCOUNT                                    24                  // 0x0018 uint16
/** Address of clock: Clock time */
#define GS_A3200_TELEMETRY_CLOCK                                        28                  // 0x001c uint32
/** Address of uptime: Uptime */
#define GS_A3200_TELEMETRY_UPTIME                                       32                  // 0x0020 uint32
/**@}*/
#ifdef __cplusplus
}
#endif
#endif
