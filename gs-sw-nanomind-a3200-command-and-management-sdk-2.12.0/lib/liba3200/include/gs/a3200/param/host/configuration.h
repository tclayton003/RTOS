#ifndef GS_A3200_PARAM_HOST_CONFIGURATION_H
#define GS_A3200_PARAM_HOST_CONFIGURATION_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a configuration.
*/

#include <gs/a3200/param/configuration.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200_configuration_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** GSSB1 power */
    bool     gssb1_pwr_en;
    /** GSSB2 power */
    bool     gssb2_pwr_en;
    /** Flash power */
    bool     flash_pwr_en;
    /** PWM power */
    bool     pwm_pwr_en;
    /** Primary program image */
    char     swload_img1[32];
    /** Primary program image boot count */
    uint16_t swload_cnt1;
    /** Secondary program image */
    char     swload_img2[32];
    /** Secondary program image boot count */
    uint16_t swload_cnt2;
    /** Default partition */
    uint8_t  fs_default;
    /** Enable /flash0 */
    bool     fs_en_p0;
    /** Enable /flash1 */
    bool     fs_en_p1;
    /** Log to FRAM */
    bool     log_fram_en;
    /** Log mask/level settings */
    char     log_mask[64];
    /** TSYNC CSP Addr */
    uint8_t  tsync_node;
    /** Filler for alignment */
    uint8_t  _filler_0x008d[1];
    /** TSYNC Interval */
    uint16_t tsync_intv;
    /** SPI0 CS Mode */
    uint8_t  spi0_cs_mode;
} gs_a3200_configuration_memory_t;

/** Table default values. */
extern const gs_a3200_configuration_memory_t gs_a3200_configuration_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set gssb1_pwr_en: GSSB1 power */
#define GS_A3200_CONFIGURATION_GSSB1_PWR_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_GSSB1_PWR_EN, value, 0)
/** Set gssb1_pwr_en (no callback): GSSB1 power */
#define GS_A3200_CONFIGURATION_GSSB1_PWR_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_GSSB1_PWR_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get gssb1_pwr_en: GSSB1 power */
#define GS_A3200_CONFIGURATION_GSSB1_PWR_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_GSSB1_PWR_EN, value, 0)
/** Get gssb1_pwr_en (no check - return 0 on failure): GSSB1 power */
#define GS_A3200_CONFIGURATION_GSSB1_PWR_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_GSSB1_PWR_EN, 0)
/** Set gssb2_pwr_en: GSSB2 power */
#define GS_A3200_CONFIGURATION_GSSB2_PWR_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_GSSB2_PWR_EN, value, 0)
/** Set gssb2_pwr_en (no callback): GSSB2 power */
#define GS_A3200_CONFIGURATION_GSSB2_PWR_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_GSSB2_PWR_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get gssb2_pwr_en: GSSB2 power */
#define GS_A3200_CONFIGURATION_GSSB2_PWR_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_GSSB2_PWR_EN, value, 0)
/** Get gssb2_pwr_en (no check - return 0 on failure): GSSB2 power */
#define GS_A3200_CONFIGURATION_GSSB2_PWR_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_GSSB2_PWR_EN, 0)
/** Set flash_pwr_en: Flash power */
#define GS_A3200_CONFIGURATION_FLASH_PWR_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FLASH_PWR_EN, value, 0)
/** Set flash_pwr_en (no callback): Flash power */
#define GS_A3200_CONFIGURATION_FLASH_PWR_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FLASH_PWR_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get flash_pwr_en: Flash power */
#define GS_A3200_CONFIGURATION_FLASH_PWR_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FLASH_PWR_EN, value, 0)
/** Get flash_pwr_en (no check - return 0 on failure): Flash power */
#define GS_A3200_CONFIGURATION_FLASH_PWR_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FLASH_PWR_EN, 0)
/** Set pwm_pwr_en: PWM power */
#define GS_A3200_CONFIGURATION_PWM_PWR_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_PWM_PWR_EN, value, 0)
/** Set pwm_pwr_en (no callback): PWM power */
#define GS_A3200_CONFIGURATION_PWM_PWR_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_PWM_PWR_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get pwm_pwr_en: PWM power */
#define GS_A3200_CONFIGURATION_PWM_PWR_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_PWM_PWR_EN, value, 0)
/** Get pwm_pwr_en (no check - return 0 on failure): PWM power */
#define GS_A3200_CONFIGURATION_PWM_PWR_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_PWM_PWR_EN, 0)
/** Set swload_img1: Primary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG1_SET(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_IMG1, value, GS_PARAM_F_AUTO_PERSIST)
/** Set swload_img1 (no callback): Primary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG1_SET_NO_CALLBACK(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_IMG1, value, GS_PARAM_F_AUTO_PERSIST | GS_PARAM_F_NO_CALLBACK)
/** Get swload_img1: Primary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG1_GET(buf, buf_size) gs_param_get_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_IMG1, buf, buf_size, 0)
/** Set swload_cnt1: Primary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT1_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_CNT1, value, GS_PARAM_F_AUTO_PERSIST)
/** Set swload_cnt1 (no callback): Primary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT1_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_CNT1, value, GS_PARAM_F_AUTO_PERSIST | GS_PARAM_F_NO_CALLBACK)
/** Get swload_cnt1: Primary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT1_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_CNT1, value, 0)
/** Get swload_cnt1 (no check - return 0 on failure): Primary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT1_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_CNT1, 0)
/** Set swload_img2: Secondary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG2_SET(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_IMG2, value, GS_PARAM_F_AUTO_PERSIST)
/** Set swload_img2 (no callback): Secondary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG2_SET_NO_CALLBACK(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_IMG2, value, GS_PARAM_F_AUTO_PERSIST | GS_PARAM_F_NO_CALLBACK)
/** Get swload_img2: Secondary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG2_GET(buf, buf_size) gs_param_get_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_IMG2, buf, buf_size, 0)
/** Set swload_cnt2: Secondary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT2_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_CNT2, value, GS_PARAM_F_AUTO_PERSIST)
/** Set swload_cnt2 (no callback): Secondary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT2_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_CNT2, value, GS_PARAM_F_AUTO_PERSIST | GS_PARAM_F_NO_CALLBACK)
/** Get swload_cnt2: Secondary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT2_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_CNT2, value, 0)
/** Get swload_cnt2 (no check - return 0 on failure): Secondary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT2_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SWLOAD_CNT2, 0)
/** Set fs_default: Default partition */
#define GS_A3200_CONFIGURATION_FS_DEFAULT_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_DEFAULT, value, 0)
/** Set fs_default (no callback): Default partition */
#define GS_A3200_CONFIGURATION_FS_DEFAULT_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_DEFAULT, value, GS_PARAM_F_NO_CALLBACK)
/** Get fs_default: Default partition */
#define GS_A3200_CONFIGURATION_FS_DEFAULT_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_DEFAULT, value, 0)
/** Get fs_default (no check - return 0 on failure): Default partition */
#define GS_A3200_CONFIGURATION_FS_DEFAULT_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_DEFAULT, 0)
/** Set fs_en_p0: Enable /flash0 */
#define GS_A3200_CONFIGURATION_FS_EN_P0_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_EN_P0, value, 0)
/** Set fs_en_p0 (no callback): Enable /flash0 */
#define GS_A3200_CONFIGURATION_FS_EN_P0_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_EN_P0, value, GS_PARAM_F_NO_CALLBACK)
/** Get fs_en_p0: Enable /flash0 */
#define GS_A3200_CONFIGURATION_FS_EN_P0_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_EN_P0, value, 0)
/** Get fs_en_p0 (no check - return 0 on failure): Enable /flash0 */
#define GS_A3200_CONFIGURATION_FS_EN_P0_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_EN_P0, 0)
/** Set fs_en_p1: Enable /flash1 */
#define GS_A3200_CONFIGURATION_FS_EN_P1_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_EN_P1, value, 0)
/** Set fs_en_p1 (no callback): Enable /flash1 */
#define GS_A3200_CONFIGURATION_FS_EN_P1_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_EN_P1, value, GS_PARAM_F_NO_CALLBACK)
/** Get fs_en_p1: Enable /flash1 */
#define GS_A3200_CONFIGURATION_FS_EN_P1_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_EN_P1, value, 0)
/** Get fs_en_p1 (no check - return 0 on failure): Enable /flash1 */
#define GS_A3200_CONFIGURATION_FS_EN_P1_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_FS_EN_P1, 0)
/** Set log_fram_en: Log to FRAM */
#define GS_A3200_CONFIGURATION_LOG_FRAM_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_LOG_FRAM_EN, value, 0)
/** Set log_fram_en (no callback): Log to FRAM */
#define GS_A3200_CONFIGURATION_LOG_FRAM_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_LOG_FRAM_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get log_fram_en: Log to FRAM */
#define GS_A3200_CONFIGURATION_LOG_FRAM_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_LOG_FRAM_EN, value, 0)
/** Get log_fram_en (no check - return 0 on failure): Log to FRAM */
#define GS_A3200_CONFIGURATION_LOG_FRAM_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_LOG_FRAM_EN, 0)
/** Set log_mask: Log mask/level settings */
#define GS_A3200_CONFIGURATION_LOG_MASK_SET(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_LOG_MASK, value, 0)
/** Set log_mask (no callback): Log mask/level settings */
#define GS_A3200_CONFIGURATION_LOG_MASK_SET_NO_CALLBACK(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_LOG_MASK, value, GS_PARAM_F_NO_CALLBACK)
/** Get log_mask: Log mask/level settings */
#define GS_A3200_CONFIGURATION_LOG_MASK_GET(buf, buf_size) gs_param_get_string(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_LOG_MASK, buf, buf_size, 0)
/** Set tsync_node: TSYNC CSP Addr */
#define GS_A3200_CONFIGURATION_TSYNC_NODE_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_TSYNC_NODE, value, 0)
/** Set tsync_node (no callback): TSYNC CSP Addr */
#define GS_A3200_CONFIGURATION_TSYNC_NODE_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_TSYNC_NODE, value, GS_PARAM_F_NO_CALLBACK)
/** Get tsync_node: TSYNC CSP Addr */
#define GS_A3200_CONFIGURATION_TSYNC_NODE_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_TSYNC_NODE, value, 0)
/** Get tsync_node (no check - return 0 on failure): TSYNC CSP Addr */
#define GS_A3200_CONFIGURATION_TSYNC_NODE_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_TSYNC_NODE, 0)
/** Set tsync_intv: TSYNC Interval */
#define GS_A3200_CONFIGURATION_TSYNC_INTV_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_TSYNC_INTV, value, 0)
/** Set tsync_intv (no callback): TSYNC Interval */
#define GS_A3200_CONFIGURATION_TSYNC_INTV_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_TSYNC_INTV, value, GS_PARAM_F_NO_CALLBACK)
/** Get tsync_intv: TSYNC Interval */
#define GS_A3200_CONFIGURATION_TSYNC_INTV_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_TSYNC_INTV, value, 0)
/** Get tsync_intv (no check - return 0 on failure): TSYNC Interval */
#define GS_A3200_CONFIGURATION_TSYNC_INTV_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_TSYNC_INTV, 0)
/** Set spi0_cs_mode: SPI0 CS Mode */
#define GS_A3200_CONFIGURATION_SPI0_CS_MODE_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SPI0_CS_MODE, value, 0)
/** Set spi0_cs_mode (no callback): SPI0 CS Mode */
#define GS_A3200_CONFIGURATION_SPI0_CS_MODE_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SPI0_CS_MODE, value, GS_PARAM_F_NO_CALLBACK)
/** Get spi0_cs_mode: SPI0 CS Mode */
#define GS_A3200_CONFIGURATION_SPI0_CS_MODE_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SPI0_CS_MODE, value, 0)
/** Get spi0_cs_mode (no check - return 0 on failure): SPI0 CS Mode */
#define GS_A3200_CONFIGURATION_SPI0_CS_MODE_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_ID), GS_A3200_CONFIGURATION_SPI0_CS_MODE, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
