#ifndef GS_A3200_PARAM_HOST_BOARD_H
#define GS_A3200_PARAM_HOST_BOARD_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a board.
*/

#include <gs/a3200/param/board.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200_board_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Unit IDentification */
    char     uid[16];
    /** Type */
    uint8_t  type;
    /** Revision */
    uint8_t  rev;
    /** Dock type */
    char     dock_type[16];
    /** CSP address */
    uint8_t  csp_addr;
    /** CSP routing table */
    char     csp_rtable[96];
    /** GOSH enabled */
    bool     gosh_en;
    /** GOSH UART */
    uint8_t  gosh_uart;
    /** CAN enabled */
    bool     can_en;
    /** Filler for alignment */
    uint8_t  _filler_0x0086[2];
    /** CAN bitrate */
    uint32_t can_brate;
    /** I2C enabled */
    bool     i2c_en;
    /** Filler for alignment */
    uint8_t  _filler_0x008d[3];
    /** I2C bitrate */
    uint32_t i2c_brate;
    /** KISS enabled */
    bool     kiss_en;
    /** Filler for alignment */
    uint8_t  _filler_0x0095[3];
    /** KISS baudrate */
    uint32_t kiss_brate;
    /** KISS UART */
    uint8_t  kiss_uart;
} gs_a3200_board_memory_t;

/** Table default values. */
extern const gs_a3200_board_memory_t gs_a3200_board_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set uid: Unit IDentification */
#define GS_A3200_BOARD_UID_SET(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_UID, value, 0)
/** Set uid (no callback): Unit IDentification */
#define GS_A3200_BOARD_UID_SET_NO_CALLBACK(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_UID, value, GS_PARAM_F_NO_CALLBACK)
/** Get uid: Unit IDentification */
#define GS_A3200_BOARD_UID_GET(buf, buf_size) gs_param_get_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_UID, buf, buf_size, 0)
/** Set type: Type */
#define GS_A3200_BOARD_TYPE_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_TYPE, value, 0)
/** Set type (no callback): Type */
#define GS_A3200_BOARD_TYPE_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_TYPE, value, GS_PARAM_F_NO_CALLBACK)
/** Get type: Type */
#define GS_A3200_BOARD_TYPE_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_TYPE, value, 0)
/** Get type (no check - return 0 on failure): Type */
#define GS_A3200_BOARD_TYPE_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_TYPE, 0)
/** Set rev: Revision */
#define GS_A3200_BOARD_REV_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_REV, value, 0)
/** Set rev (no callback): Revision */
#define GS_A3200_BOARD_REV_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_REV, value, GS_PARAM_F_NO_CALLBACK)
/** Get rev: Revision */
#define GS_A3200_BOARD_REV_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_REV, value, 0)
/** Get rev (no check - return 0 on failure): Revision */
#define GS_A3200_BOARD_REV_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_REV, 0)
/** Set dock_type: Dock type */
#define GS_A3200_BOARD_DOCK_TYPE_SET(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_DOCK_TYPE, value, 0)
/** Set dock_type (no callback): Dock type */
#define GS_A3200_BOARD_DOCK_TYPE_SET_NO_CALLBACK(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_DOCK_TYPE, value, GS_PARAM_F_NO_CALLBACK)
/** Get dock_type: Dock type */
#define GS_A3200_BOARD_DOCK_TYPE_GET(buf, buf_size) gs_param_get_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_DOCK_TYPE, buf, buf_size, 0)
/** Set csp_addr: CSP address */
#define GS_A3200_BOARD_CSP_ADDR_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CSP_ADDR, value, 0)
/** Set csp_addr (no callback): CSP address */
#define GS_A3200_BOARD_CSP_ADDR_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CSP_ADDR, value, GS_PARAM_F_NO_CALLBACK)
/** Get csp_addr: CSP address */
#define GS_A3200_BOARD_CSP_ADDR_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CSP_ADDR, value, 0)
/** Get csp_addr (no check - return 0 on failure): CSP address */
#define GS_A3200_BOARD_CSP_ADDR_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CSP_ADDR, 0)
/** Set csp_rtable: CSP routing table */
#define GS_A3200_BOARD_CSP_RTABLE_SET(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CSP_RTABLE, value, 0)
/** Set csp_rtable (no callback): CSP routing table */
#define GS_A3200_BOARD_CSP_RTABLE_SET_NO_CALLBACK(value) gs_param_set_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CSP_RTABLE, value, GS_PARAM_F_NO_CALLBACK)
/** Get csp_rtable: CSP routing table */
#define GS_A3200_BOARD_CSP_RTABLE_GET(buf, buf_size) gs_param_get_string(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CSP_RTABLE, buf, buf_size, 0)
/** Set gosh_en: GOSH enabled */
#define GS_A3200_BOARD_GOSH_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_GOSH_EN, value, 0)
/** Set gosh_en (no callback): GOSH enabled */
#define GS_A3200_BOARD_GOSH_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_GOSH_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get gosh_en: GOSH enabled */
#define GS_A3200_BOARD_GOSH_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_GOSH_EN, value, 0)
/** Get gosh_en (no check - return 0 on failure): GOSH enabled */
#define GS_A3200_BOARD_GOSH_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_GOSH_EN, 0)
/** Set gosh_uart: GOSH UART */
#define GS_A3200_BOARD_GOSH_UART_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_GOSH_UART, value, 0)
/** Set gosh_uart (no callback): GOSH UART */
#define GS_A3200_BOARD_GOSH_UART_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_GOSH_UART, value, GS_PARAM_F_NO_CALLBACK)
/** Get gosh_uart: GOSH UART */
#define GS_A3200_BOARD_GOSH_UART_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_GOSH_UART, value, 0)
/** Get gosh_uart (no check - return 0 on failure): GOSH UART */
#define GS_A3200_BOARD_GOSH_UART_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_GOSH_UART, 0)
/** Set can_en: CAN enabled */
#define GS_A3200_BOARD_CAN_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CAN_EN, value, 0)
/** Set can_en (no callback): CAN enabled */
#define GS_A3200_BOARD_CAN_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CAN_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get can_en: CAN enabled */
#define GS_A3200_BOARD_CAN_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CAN_EN, value, 0)
/** Get can_en (no check - return 0 on failure): CAN enabled */
#define GS_A3200_BOARD_CAN_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CAN_EN, 0)
/** Set can_brate: CAN bitrate */
#define GS_A3200_BOARD_CAN_BRATE_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CAN_BRATE, value, 0)
/** Set can_brate (no callback): CAN bitrate */
#define GS_A3200_BOARD_CAN_BRATE_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CAN_BRATE, value, GS_PARAM_F_NO_CALLBACK)
/** Get can_brate: CAN bitrate */
#define GS_A3200_BOARD_CAN_BRATE_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CAN_BRATE, value, 0)
/** Get can_brate (no check - return 0 on failure): CAN bitrate */
#define GS_A3200_BOARD_CAN_BRATE_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_CAN_BRATE, 0)
/** Set i2c_en: I2C enabled */
#define GS_A3200_BOARD_I2C_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_I2C_EN, value, 0)
/** Set i2c_en (no callback): I2C enabled */
#define GS_A3200_BOARD_I2C_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_I2C_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get i2c_en: I2C enabled */
#define GS_A3200_BOARD_I2C_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_I2C_EN, value, 0)
/** Get i2c_en (no check - return 0 on failure): I2C enabled */
#define GS_A3200_BOARD_I2C_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_I2C_EN, 0)
/** Set i2c_brate: I2C bitrate */
#define GS_A3200_BOARD_I2C_BRATE_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_I2C_BRATE, value, 0)
/** Set i2c_brate (no callback): I2C bitrate */
#define GS_A3200_BOARD_I2C_BRATE_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_I2C_BRATE, value, GS_PARAM_F_NO_CALLBACK)
/** Get i2c_brate: I2C bitrate */
#define GS_A3200_BOARD_I2C_BRATE_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_I2C_BRATE, value, 0)
/** Get i2c_brate (no check - return 0 on failure): I2C bitrate */
#define GS_A3200_BOARD_I2C_BRATE_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_I2C_BRATE, 0)
/** Set kiss_en: KISS enabled */
#define GS_A3200_BOARD_KISS_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_EN, value, 0)
/** Set kiss_en (no callback): KISS enabled */
#define GS_A3200_BOARD_KISS_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get kiss_en: KISS enabled */
#define GS_A3200_BOARD_KISS_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_EN, value, 0)
/** Get kiss_en (no check - return 0 on failure): KISS enabled */
#define GS_A3200_BOARD_KISS_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_EN, 0)
/** Set kiss_brate: KISS baudrate */
#define GS_A3200_BOARD_KISS_BRATE_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_BRATE, value, 0)
/** Set kiss_brate (no callback): KISS baudrate */
#define GS_A3200_BOARD_KISS_BRATE_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_BRATE, value, GS_PARAM_F_NO_CALLBACK)
/** Get kiss_brate: KISS baudrate */
#define GS_A3200_BOARD_KISS_BRATE_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_BRATE, value, 0)
/** Get kiss_brate (no check - return 0 on failure): KISS baudrate */
#define GS_A3200_BOARD_KISS_BRATE_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_BRATE, 0)
/** Set kiss_uart: KISS UART */
#define GS_A3200_BOARD_KISS_UART_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_UART, value, 0)
/** Set kiss_uart (no callback): KISS UART */
#define GS_A3200_BOARD_KISS_UART_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_UART, value, GS_PARAM_F_NO_CALLBACK)
/** Get kiss_uart: KISS UART */
#define GS_A3200_BOARD_KISS_UART_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_UART, value, 0)
/** Get kiss_uart (no check - return 0 on failure): KISS UART */
#define GS_A3200_BOARD_KISS_UART_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_BOARD_TABLE_ID), GS_A3200_BOARD_KISS_UART, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
