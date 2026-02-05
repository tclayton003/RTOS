#ifndef GS_A3200_PARAM_HOST_TELEMETRY_H
#define GS_A3200_PARAM_HOST_TELEMETRY_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a telemetry.
*/

#include <gs/a3200/param/telemetry.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200_telemetry_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Filesystem mounted */
    bool     fs_mounted;
    /** Is RAM image */
    bool     ram_image;
    /** MCU Temperature */
    int16_t  temp_mcu;
    /** RAM Temperature */
    int16_t  temp_ram;
    /** GSSB1 current */
    uint16_t i_GSSB1;
    /** GSSB2 current */
    uint16_t i_GSSB2;
    /** Flash current */
    uint16_t i_Flash;
    /** PWM current */
    uint16_t i_PWM;
    /** Filler for alignment */
    uint8_t  _filler_0x000e[2];
    /** Reset cause */
    uint32_t resetcause;
    /** Bootcause */
    uint32_t bootcause;
    /** Bootcount */
    uint16_t bootcount;
    /** Filler for alignment */
    uint8_t  _filler_0x001a[2];
    /** Clock time */
    uint32_t clock;
    /** Uptime */
    uint32_t uptime;
} gs_a3200_telemetry_memory_t;

/** Table default values. */
extern const gs_a3200_telemetry_memory_t gs_a3200_telemetry_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set fs_mounted: Filesystem mounted */
#define GS_A3200_TELEMETRY_FS_MOUNTED_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_FS_MOUNTED, value, 0)
/** Set fs_mounted (no callback): Filesystem mounted */
#define GS_A3200_TELEMETRY_FS_MOUNTED_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_FS_MOUNTED, value, GS_PARAM_F_NO_CALLBACK)
/** Get fs_mounted: Filesystem mounted */
#define GS_A3200_TELEMETRY_FS_MOUNTED_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_FS_MOUNTED, value, 0)
/** Get fs_mounted (no check - return 0 on failure): Filesystem mounted */
#define GS_A3200_TELEMETRY_FS_MOUNTED_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_FS_MOUNTED, 0)
/** Set ram_image: Is RAM image */
#define GS_A3200_TELEMETRY_RAM_IMAGE_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_RAM_IMAGE, value, 0)
/** Set ram_image (no callback): Is RAM image */
#define GS_A3200_TELEMETRY_RAM_IMAGE_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_RAM_IMAGE, value, GS_PARAM_F_NO_CALLBACK)
/** Get ram_image: Is RAM image */
#define GS_A3200_TELEMETRY_RAM_IMAGE_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_RAM_IMAGE, value, 0)
/** Get ram_image (no check - return 0 on failure): Is RAM image */
#define GS_A3200_TELEMETRY_RAM_IMAGE_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_RAM_IMAGE, 0)
/** Set temp_mcu: MCU Temperature */
#define GS_A3200_TELEMETRY_TEMP_MCU_SET(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_TEMP_MCU, value, 0)
/** Set temp_mcu (no callback): MCU Temperature */
#define GS_A3200_TELEMETRY_TEMP_MCU_SET_NO_CALLBACK(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_TEMP_MCU, value, GS_PARAM_F_NO_CALLBACK)
/** Get temp_mcu: MCU Temperature */
#define GS_A3200_TELEMETRY_TEMP_MCU_GET(value) gs_param_get_int16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_TEMP_MCU, value, 0)
/** Get temp_mcu (no check - return 0 on failure): MCU Temperature */
#define GS_A3200_TELEMETRY_TEMP_MCU_GET_NC() gs_param_get_int16_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_TEMP_MCU, 0)
/** Set temp_ram: RAM Temperature */
#define GS_A3200_TELEMETRY_TEMP_RAM_SET(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_TEMP_RAM, value, 0)
/** Set temp_ram (no callback): RAM Temperature */
#define GS_A3200_TELEMETRY_TEMP_RAM_SET_NO_CALLBACK(value) gs_param_set_int16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_TEMP_RAM, value, GS_PARAM_F_NO_CALLBACK)
/** Get temp_ram: RAM Temperature */
#define GS_A3200_TELEMETRY_TEMP_RAM_GET(value) gs_param_get_int16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_TEMP_RAM, value, 0)
/** Get temp_ram (no check - return 0 on failure): RAM Temperature */
#define GS_A3200_TELEMETRY_TEMP_RAM_GET_NC() gs_param_get_int16_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_TEMP_RAM, 0)
/** Set i_GSSB1: GSSB1 current */
#define GS_A3200_TELEMETRY_I_GSSB1_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_GSSB1, value, 0)
/** Set i_GSSB1 (no callback): GSSB1 current */
#define GS_A3200_TELEMETRY_I_GSSB1_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_GSSB1, value, GS_PARAM_F_NO_CALLBACK)
/** Get i_GSSB1: GSSB1 current */
#define GS_A3200_TELEMETRY_I_GSSB1_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_GSSB1, value, 0)
/** Get i_GSSB1 (no check - return 0 on failure): GSSB1 current */
#define GS_A3200_TELEMETRY_I_GSSB1_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_GSSB1, 0)
/** Set i_GSSB2: GSSB2 current */
#define GS_A3200_TELEMETRY_I_GSSB2_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_GSSB2, value, 0)
/** Set i_GSSB2 (no callback): GSSB2 current */
#define GS_A3200_TELEMETRY_I_GSSB2_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_GSSB2, value, GS_PARAM_F_NO_CALLBACK)
/** Get i_GSSB2: GSSB2 current */
#define GS_A3200_TELEMETRY_I_GSSB2_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_GSSB2, value, 0)
/** Get i_GSSB2 (no check - return 0 on failure): GSSB2 current */
#define GS_A3200_TELEMETRY_I_GSSB2_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_GSSB2, 0)
/** Set i_Flash: Flash current */
#define GS_A3200_TELEMETRY_I_FLASH_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_FLASH, value, 0)
/** Set i_Flash (no callback): Flash current */
#define GS_A3200_TELEMETRY_I_FLASH_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_FLASH, value, GS_PARAM_F_NO_CALLBACK)
/** Get i_Flash: Flash current */
#define GS_A3200_TELEMETRY_I_FLASH_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_FLASH, value, 0)
/** Get i_Flash (no check - return 0 on failure): Flash current */
#define GS_A3200_TELEMETRY_I_FLASH_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_FLASH, 0)
/** Set i_PWM: PWM current */
#define GS_A3200_TELEMETRY_I_PWM_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_PWM, value, 0)
/** Set i_PWM (no callback): PWM current */
#define GS_A3200_TELEMETRY_I_PWM_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_PWM, value, GS_PARAM_F_NO_CALLBACK)
/** Get i_PWM: PWM current */
#define GS_A3200_TELEMETRY_I_PWM_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_PWM, value, 0)
/** Get i_PWM (no check - return 0 on failure): PWM current */
#define GS_A3200_TELEMETRY_I_PWM_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_I_PWM, 0)
/** Set resetcause: Reset cause */
#define GS_A3200_TELEMETRY_RESETCAUSE_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_RESETCAUSE, value, 0)
/** Set resetcause (no callback): Reset cause */
#define GS_A3200_TELEMETRY_RESETCAUSE_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_RESETCAUSE, value, GS_PARAM_F_NO_CALLBACK)
/** Get resetcause: Reset cause */
#define GS_A3200_TELEMETRY_RESETCAUSE_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_RESETCAUSE, value, 0)
/** Get resetcause (no check - return 0 on failure): Reset cause */
#define GS_A3200_TELEMETRY_RESETCAUSE_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_RESETCAUSE, 0)
/** Set bootcause: Bootcause */
#define GS_A3200_TELEMETRY_BOOTCAUSE_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_BOOTCAUSE, value, 0)
/** Set bootcause (no callback): Bootcause */
#define GS_A3200_TELEMETRY_BOOTCAUSE_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_BOOTCAUSE, value, GS_PARAM_F_NO_CALLBACK)
/** Get bootcause: Bootcause */
#define GS_A3200_TELEMETRY_BOOTCAUSE_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_BOOTCAUSE, value, 0)
/** Get bootcause (no check - return 0 on failure): Bootcause */
#define GS_A3200_TELEMETRY_BOOTCAUSE_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_BOOTCAUSE, 0)
/** Set bootcount: Bootcount */
#define GS_A3200_TELEMETRY_BOOTCOUNT_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_BOOTCOUNT, value, GS_PARAM_F_AUTO_PERSIST)
/** Set bootcount (no callback): Bootcount */
#define GS_A3200_TELEMETRY_BOOTCOUNT_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_BOOTCOUNT, value, GS_PARAM_F_AUTO_PERSIST | GS_PARAM_F_NO_CALLBACK)
/** Get bootcount: Bootcount */
#define GS_A3200_TELEMETRY_BOOTCOUNT_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_BOOTCOUNT, value, 0)
/** Get bootcount (no check - return 0 on failure): Bootcount */
#define GS_A3200_TELEMETRY_BOOTCOUNT_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_BOOTCOUNT, 0)
/** Set clock: Clock time */
#define GS_A3200_TELEMETRY_CLOCK_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_CLOCK, value, 0)
/** Set clock (no callback): Clock time */
#define GS_A3200_TELEMETRY_CLOCK_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_CLOCK, value, GS_PARAM_F_NO_CALLBACK)
/** Get clock: Clock time */
#define GS_A3200_TELEMETRY_CLOCK_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_CLOCK, value, 0)
/** Get clock (no check - return 0 on failure): Clock time */
#define GS_A3200_TELEMETRY_CLOCK_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_CLOCK, 0)
/** Set uptime: Uptime */
#define GS_A3200_TELEMETRY_UPTIME_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_UPTIME, value, 0)
/** Set uptime (no callback): Uptime */
#define GS_A3200_TELEMETRY_UPTIME_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_UPTIME, value, GS_PARAM_F_NO_CALLBACK)
/** Get uptime: Uptime */
#define GS_A3200_TELEMETRY_UPTIME_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_UPTIME, value, 0)
/** Get uptime (no check - return 0 on failure): Uptime */
#define GS_A3200_TELEMETRY_UPTIME_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_TELEMETRY_TABLE_ID), GS_A3200_TELEMETRY_UPTIME, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
