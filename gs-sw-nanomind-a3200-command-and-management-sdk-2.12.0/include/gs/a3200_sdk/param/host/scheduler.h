#ifndef GS_A3200_SDK_PARAM_HOST_SCHEDULER_H
#define GS_A3200_SDK_PARAM_HOST_SCHEDULER_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a scheduler.
*/

#include <gs/a3200_sdk/param/scheduler.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200_sdk_scheduler_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Enable/disable scheduling */
    bool     enabled;
    /** Filler for alignment */
    uint8_t  _filler_0x0001[1];
    /** Delay before scheduling is started after boot/re-load */
    uint16_t delayed_start;
    /** BPX HK collection interval */
    uint16_t bpx_col_intv;
    /** BPX CSP address */
    uint8_t  bpx_csp_addr;
    /** Filler for alignment */
    uint8_t  _filler_0x0007[1];
    /** EPS HK collection interval */
    uint16_t eps_col_intv;
    /** EPS CSP address */
    uint8_t  eps_csp_addr;
    /** Filler for alignment */
    uint8_t  _filler_0x000b[1];
    /** ADSB collection interval */
    uint16_t adsb_col_intv;
    /** ADSB CSP address */
    uint8_t  adsb_csp_addr;
} gs_a3200_sdk_scheduler_memory_t;

/** Table default values. */
extern const gs_a3200_sdk_scheduler_memory_t gs_a3200_sdk_scheduler_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set enabled: Enable/disable scheduling */
#define GS_A3200_SDK_SCHEDULER_ENABLED_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ENABLED, value, 0)
/** Set enabled (no callback): Enable/disable scheduling */
#define GS_A3200_SDK_SCHEDULER_ENABLED_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ENABLED, value, GS_PARAM_F_NO_CALLBACK)
/** Get enabled: Enable/disable scheduling */
#define GS_A3200_SDK_SCHEDULER_ENABLED_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ENABLED, value, 0)
/** Get enabled (no check - return 0 on failure): Enable/disable scheduling */
#define GS_A3200_SDK_SCHEDULER_ENABLED_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ENABLED, 0)
/** Set delayed_start: Delay before scheduling is started after boot/re-load */
#define GS_A3200_SDK_SCHEDULER_DELAYED_START_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_DELAYED_START, value, 0)
/** Set delayed_start (no callback): Delay before scheduling is started after boot/re-load */
#define GS_A3200_SDK_SCHEDULER_DELAYED_START_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_DELAYED_START, value, GS_PARAM_F_NO_CALLBACK)
/** Get delayed_start: Delay before scheduling is started after boot/re-load */
#define GS_A3200_SDK_SCHEDULER_DELAYED_START_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_DELAYED_START, value, 0)
/** Get delayed_start (no check - return 0 on failure): Delay before scheduling is started after boot/re-load */
#define GS_A3200_SDK_SCHEDULER_DELAYED_START_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_DELAYED_START, 0)
/** Set bpx_col_intv: BPX HK collection interval */
#define GS_A3200_SDK_SCHEDULER_BPX_COL_INTV_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_BPX_COL_INTV, value, 0)
/** Set bpx_col_intv (no callback): BPX HK collection interval */
#define GS_A3200_SDK_SCHEDULER_BPX_COL_INTV_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_BPX_COL_INTV, value, GS_PARAM_F_NO_CALLBACK)
/** Get bpx_col_intv: BPX HK collection interval */
#define GS_A3200_SDK_SCHEDULER_BPX_COL_INTV_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_BPX_COL_INTV, value, 0)
/** Get bpx_col_intv (no check - return 0 on failure): BPX HK collection interval */
#define GS_A3200_SDK_SCHEDULER_BPX_COL_INTV_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_BPX_COL_INTV, 0)
/** Set bpx_csp_addr: BPX CSP address */
#define GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR, value, 0)
/** Set bpx_csp_addr (no callback): BPX CSP address */
#define GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR, value, GS_PARAM_F_NO_CALLBACK)
/** Get bpx_csp_addr: BPX CSP address */
#define GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR, value, 0)
/** Get bpx_csp_addr (no check - return 0 on failure): BPX CSP address */
#define GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR, 0)
/** Set eps_col_intv: EPS HK collection interval */
#define GS_A3200_SDK_SCHEDULER_EPS_COL_INTV_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_EPS_COL_INTV, value, 0)
/** Set eps_col_intv (no callback): EPS HK collection interval */
#define GS_A3200_SDK_SCHEDULER_EPS_COL_INTV_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_EPS_COL_INTV, value, GS_PARAM_F_NO_CALLBACK)
/** Get eps_col_intv: EPS HK collection interval */
#define GS_A3200_SDK_SCHEDULER_EPS_COL_INTV_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_EPS_COL_INTV, value, 0)
/** Get eps_col_intv (no check - return 0 on failure): EPS HK collection interval */
#define GS_A3200_SDK_SCHEDULER_EPS_COL_INTV_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_EPS_COL_INTV, 0)
/** Set eps_csp_addr: EPS CSP address */
#define GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR, value, 0)
/** Set eps_csp_addr (no callback): EPS CSP address */
#define GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR, value, GS_PARAM_F_NO_CALLBACK)
/** Get eps_csp_addr: EPS CSP address */
#define GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR, value, 0)
/** Get eps_csp_addr (no check - return 0 on failure): EPS CSP address */
#define GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR, 0)
/** Set adsb_col_intv: ADSB collection interval */
#define GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV, value, 0)
/** Set adsb_col_intv (no callback): ADSB collection interval */
#define GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV, value, GS_PARAM_F_NO_CALLBACK)
/** Get adsb_col_intv: ADSB collection interval */
#define GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV, value, 0)
/** Get adsb_col_intv (no check - return 0 on failure): ADSB collection interval */
#define GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV, 0)
/** Set adsb_csp_addr: ADSB CSP address */
#define GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR, value, 0)
/** Set adsb_csp_addr (no callback): ADSB CSP address */
#define GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR, value, GS_PARAM_F_NO_CALLBACK)
/** Get adsb_csp_addr: ADSB CSP address */
#define GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR, value, 0)
/** Get adsb_csp_addr (no check - return 0 on failure): ADSB CSP address */
#define GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_A3200_SDK_SCHEDULER_TABLE_ID), GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
