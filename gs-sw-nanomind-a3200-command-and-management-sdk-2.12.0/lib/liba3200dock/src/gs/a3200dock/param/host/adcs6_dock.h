#ifndef GS_A3200DOCK_PARAM_HOST_ADCS6_DOCK_H
#define GS_A3200DOCK_PARAM_HOST_ADCS6_DOCK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a adcs6_dock.
*/

#include <gs/a3200dock/param/adcs6_dock.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200dock_adcs6_dock_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Power GPS */
    bool     gps_en;
    /** Power Wheel 0 */
    bool     rw0_en;
    /** Power Wheel 1 */
    bool     rw1_en;
    /** Power Wheel 2 */
    bool     rw2_en;
    /** Power Wheel 3 */
    bool     rw3_en;
    /** Power GSSB C  */
    bool     gssb_c_en;
    /** Power GSSB D */
    bool     gssb_d_en;
    /** Filler for alignment */
    uint8_t  _filler_0x0007[1];
    /** GPS current */
    uint16_t gps_i;
    /** GSSB C current */
    uint16_t gssb_c_i;
    /** GSSB D current */
    uint16_t gssb_d_i;
} gs_a3200dock_adcs6_dock_memory_t;

/** Table default values. */
extern const gs_a3200dock_adcs6_dock_memory_t gs_a3200dock_adcs6_dock_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set gps_en: Power GPS */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GPS_EN, value, 0)
/** Set gps_en (no callback): Power GPS */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GPS_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get gps_en: Power GPS */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GPS_EN, value, 0)
/** Get gps_en (no check - return 0 on failure): Power GPS */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GPS_EN, 0)
/** Set rw0_en: Power Wheel 0 */
#define GS_A3200DOCK_ADCS6_DOCK_RW0_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW0_EN, value, 0)
/** Set rw0_en (no callback): Power Wheel 0 */
#define GS_A3200DOCK_ADCS6_DOCK_RW0_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW0_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get rw0_en: Power Wheel 0 */
#define GS_A3200DOCK_ADCS6_DOCK_RW0_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW0_EN, value, 0)
/** Get rw0_en (no check - return 0 on failure): Power Wheel 0 */
#define GS_A3200DOCK_ADCS6_DOCK_RW0_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW0_EN, 0)
/** Set rw1_en: Power Wheel 1 */
#define GS_A3200DOCK_ADCS6_DOCK_RW1_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW1_EN, value, 0)
/** Set rw1_en (no callback): Power Wheel 1 */
#define GS_A3200DOCK_ADCS6_DOCK_RW1_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW1_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get rw1_en: Power Wheel 1 */
#define GS_A3200DOCK_ADCS6_DOCK_RW1_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW1_EN, value, 0)
/** Get rw1_en (no check - return 0 on failure): Power Wheel 1 */
#define GS_A3200DOCK_ADCS6_DOCK_RW1_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW1_EN, 0)
/** Set rw2_en: Power Wheel 2 */
#define GS_A3200DOCK_ADCS6_DOCK_RW2_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW2_EN, value, 0)
/** Set rw2_en (no callback): Power Wheel 2 */
#define GS_A3200DOCK_ADCS6_DOCK_RW2_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW2_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get rw2_en: Power Wheel 2 */
#define GS_A3200DOCK_ADCS6_DOCK_RW2_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW2_EN, value, 0)
/** Get rw2_en (no check - return 0 on failure): Power Wheel 2 */
#define GS_A3200DOCK_ADCS6_DOCK_RW2_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW2_EN, 0)
/** Set rw3_en: Power Wheel 3 */
#define GS_A3200DOCK_ADCS6_DOCK_RW3_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW3_EN, value, 0)
/** Set rw3_en (no callback): Power Wheel 3 */
#define GS_A3200DOCK_ADCS6_DOCK_RW3_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW3_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get rw3_en: Power Wheel 3 */
#define GS_A3200DOCK_ADCS6_DOCK_RW3_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW3_EN, value, 0)
/** Get rw3_en (no check - return 0 on failure): Power Wheel 3 */
#define GS_A3200DOCK_ADCS6_DOCK_RW3_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_RW3_EN, 0)
/** Set gssb_c_en: Power GSSB C  */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN, value, 0)
/** Set gssb_c_en (no callback): Power GSSB C  */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get gssb_c_en: Power GSSB C  */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN, value, 0)
/** Get gssb_c_en (no check - return 0 on failure): Power GSSB C  */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN, 0)
/** Set gssb_d_en: Power GSSB D */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN, value, 0)
/** Set gssb_d_en (no callback): Power GSSB D */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get gssb_d_en: Power GSSB D */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN, value, 0)
/** Get gssb_d_en (no check - return 0 on failure): Power GSSB D */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN, 0)
/** Set gps_i: GPS current */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_I_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GPS_I, value, 0)
/** Set gps_i (no callback): GPS current */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_I_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GPS_I, value, GS_PARAM_F_NO_CALLBACK)
/** Get gps_i: GPS current */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_I_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GPS_I, value, 0)
/** Get gps_i (no check - return 0 on failure): GPS current */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_I_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GPS_I, 0)
/** Set gssb_c_i: GSSB C current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I, value, 0)
/** Set gssb_c_i (no callback): GSSB C current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I, value, GS_PARAM_F_NO_CALLBACK)
/** Get gssb_c_i: GSSB C current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I, value, 0)
/** Get gssb_c_i (no check - return 0 on failure): GSSB C current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I, 0)
/** Set gssb_d_i: GSSB D current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I, value, 0)
/** Set gssb_d_i (no callback): GSSB D current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I, value, GS_PARAM_F_NO_CALLBACK)
/** Get gssb_d_i: GSSB D current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I, value, 0)
/** Get gssb_d_i (no check - return 0 on failure): GSSB D current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS6_DOCK_TABLE_ID), GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
