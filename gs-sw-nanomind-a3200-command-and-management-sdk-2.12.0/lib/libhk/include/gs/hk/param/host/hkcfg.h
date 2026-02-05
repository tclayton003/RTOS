#ifndef GS_HK_PARAM_HOST_HKCFG_H
#define GS_HK_PARAM_HOST_HKCFG_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a hkcfg.
*/

#include <gs/hk/param/hkcfg.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_hk_hkcfg_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Temporarily disable all automatic beacon transmits for this number of seconds */
    uint16_t bcn_atx_tpds;
    /** CSP destination address for automatic beacon transmits */
    uint8_t  bcn_atx_da;
    /** CSP destination port for automatic beacon transmits */
    uint8_t  bcn_atx_dp;
    /** Identifier to stamp into each beacon to show from which satellite the beacon originates */
    uint16_t bcn_satid;
    /** Filler for alignment */
    uint8_t  _filler_0x0006[2];
    /** Interval for persisting table samples */
    uint32_t persist_intv;
    /** Flag to completely enable/disable persisting of table samples */
    bool     persist_en;
    /** Filesystem path used by the housekeeping system to read and write data */
    char     data_path[50];
    /** Flag to completely enable/disable the housekeeping system */
    bool     hk_en;
} gs_hk_hkcfg_memory_t;

/** Table default values. */
extern const gs_hk_hkcfg_memory_t gs_hk_hkcfg_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set bcn_atx_tpds: Temporarily disable all automatic beacon transmits for this number of seconds */
#define GS_HK_HKCFG_BCN_ATX_TPDS_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_TPDS, value, 0)
/** Set bcn_atx_tpds (no callback): Temporarily disable all automatic beacon transmits for this number of seconds */
#define GS_HK_HKCFG_BCN_ATX_TPDS_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_TPDS, value, GS_PARAM_F_NO_CALLBACK)
/** Get bcn_atx_tpds: Temporarily disable all automatic beacon transmits for this number of seconds */
#define GS_HK_HKCFG_BCN_ATX_TPDS_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_TPDS, value, 0)
/** Get bcn_atx_tpds (no check - return 0 on failure): Temporarily disable all automatic beacon transmits for this number of seconds */
#define GS_HK_HKCFG_BCN_ATX_TPDS_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_TPDS, 0)
/** Set bcn_atx_da: CSP destination address for automatic beacon transmits */
#define GS_HK_HKCFG_BCN_ATX_DA_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_DA, value, 0)
/** Set bcn_atx_da (no callback): CSP destination address for automatic beacon transmits */
#define GS_HK_HKCFG_BCN_ATX_DA_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_DA, value, GS_PARAM_F_NO_CALLBACK)
/** Get bcn_atx_da: CSP destination address for automatic beacon transmits */
#define GS_HK_HKCFG_BCN_ATX_DA_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_DA, value, 0)
/** Get bcn_atx_da (no check - return 0 on failure): CSP destination address for automatic beacon transmits */
#define GS_HK_HKCFG_BCN_ATX_DA_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_DA, 0)
/** Set bcn_atx_dp: CSP destination port for automatic beacon transmits */
#define GS_HK_HKCFG_BCN_ATX_DP_SET(value) gs_param_set_uint8(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_DP, value, 0)
/** Set bcn_atx_dp (no callback): CSP destination port for automatic beacon transmits */
#define GS_HK_HKCFG_BCN_ATX_DP_SET_NO_CALLBACK(value) gs_param_set_uint8(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_DP, value, GS_PARAM_F_NO_CALLBACK)
/** Get bcn_atx_dp: CSP destination port for automatic beacon transmits */
#define GS_HK_HKCFG_BCN_ATX_DP_GET(value) gs_param_get_uint8(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_DP, value, 0)
/** Get bcn_atx_dp (no check - return 0 on failure): CSP destination port for automatic beacon transmits */
#define GS_HK_HKCFG_BCN_ATX_DP_GET_NC() gs_param_get_uint8_nc(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_ATX_DP, 0)
/** Set bcn_satid: Identifier to stamp into each beacon to show from which satellite the beacon originates */
#define GS_HK_HKCFG_BCN_SATID_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_SATID, value, 0)
/** Set bcn_satid (no callback): Identifier to stamp into each beacon to show from which satellite the beacon originates */
#define GS_HK_HKCFG_BCN_SATID_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_SATID, value, GS_PARAM_F_NO_CALLBACK)
/** Get bcn_satid: Identifier to stamp into each beacon to show from which satellite the beacon originates */
#define GS_HK_HKCFG_BCN_SATID_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_SATID, value, 0)
/** Get bcn_satid (no check - return 0 on failure): Identifier to stamp into each beacon to show from which satellite the beacon originates */
#define GS_HK_HKCFG_BCN_SATID_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_BCN_SATID, 0)
/** Set persist_intv: Interval for persisting table samples */
#define GS_HK_HKCFG_PERSIST_INTV_SET(value) gs_param_set_uint32(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_PERSIST_INTV, value, 0)
/** Set persist_intv (no callback): Interval for persisting table samples */
#define GS_HK_HKCFG_PERSIST_INTV_SET_NO_CALLBACK(value) gs_param_set_uint32(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_PERSIST_INTV, value, GS_PARAM_F_NO_CALLBACK)
/** Get persist_intv: Interval for persisting table samples */
#define GS_HK_HKCFG_PERSIST_INTV_GET(value) gs_param_get_uint32(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_PERSIST_INTV, value, 0)
/** Get persist_intv (no check - return 0 on failure): Interval for persisting table samples */
#define GS_HK_HKCFG_PERSIST_INTV_GET_NC() gs_param_get_uint32_nc(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_PERSIST_INTV, 0)
/** Set persist_en: Flag to completely enable/disable persisting of table samples */
#define GS_HK_HKCFG_PERSIST_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_PERSIST_EN, value, 0)
/** Set persist_en (no callback): Flag to completely enable/disable persisting of table samples */
#define GS_HK_HKCFG_PERSIST_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_PERSIST_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get persist_en: Flag to completely enable/disable persisting of table samples */
#define GS_HK_HKCFG_PERSIST_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_PERSIST_EN, value, 0)
/** Get persist_en (no check - return 0 on failure): Flag to completely enable/disable persisting of table samples */
#define GS_HK_HKCFG_PERSIST_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_PERSIST_EN, 0)
/** Set data_path: Filesystem path used by the housekeeping system to read and write data */
#define GS_HK_HKCFG_DATA_PATH_SET(value) gs_param_set_string(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_DATA_PATH, value, 0)
/** Set data_path (no callback): Filesystem path used by the housekeeping system to read and write data */
#define GS_HK_HKCFG_DATA_PATH_SET_NO_CALLBACK(value) gs_param_set_string(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_DATA_PATH, value, GS_PARAM_F_NO_CALLBACK)
/** Get data_path: Filesystem path used by the housekeeping system to read and write data */
#define GS_HK_HKCFG_DATA_PATH_GET(buf, buf_size) gs_param_get_string(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_DATA_PATH, buf, buf_size, 0)
/** Set hk_en: Flag to completely enable/disable the housekeeping system */
#define GS_HK_HKCFG_HK_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_HK_EN, value, 0)
/** Set hk_en (no callback): Flag to completely enable/disable the housekeeping system */
#define GS_HK_HKCFG_HK_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_HK_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get hk_en: Flag to completely enable/disable the housekeeping system */
#define GS_HK_HKCFG_HK_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_HK_EN, value, 0)
/** Get hk_en (no check - return 0 on failure): Flag to completely enable/disable the housekeeping system */
#define GS_HK_HKCFG_HK_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_HK_HKCFG_TABLE_ID), GS_HK_HKCFG_HK_EN, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
