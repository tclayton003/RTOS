#ifndef GS_A3200DOCK_PARAM_HOST_ADCS3_DOCK_H
#define GS_A3200DOCK_PARAM_HOST_ADCS3_DOCK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions (host) for \a adcs3_dock.
*/

#include <gs/a3200dock/param/adcs3_dock.h>
#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Table rows (layout). */
extern const gs_param_table_row_t gs_a3200dock_adcs3_dock_table[];

/** Table data memory as a C struct. */
typedef struct __attribute__ ((packed)) {
    /** Power GPS */
    bool     gps_en;
    /** Power Wheels */
    bool     wde_en;
    /** GPS current */
    uint16_t gps_i;
    /** Wheel current */
    uint16_t wde_i;
} gs_a3200dock_adcs3_dock_memory_t;

/** Table default values. */
extern const gs_a3200dock_adcs3_dock_memory_t gs_a3200dock_adcs3_dock_default_values;

/**
   @defgroup param_getters_setters Table getters/setters based on table id.
   @note include <gs/param/host_table.h> in order to use these macros.
   @{
*/
/** Set gps_en: Power GPS */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_GPS_EN, value, 0)
/** Set gps_en (no callback): Power GPS */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_GPS_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get gps_en: Power GPS */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_GPS_EN, value, 0)
/** Get gps_en (no check - return 0 on failure): Power GPS */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_GPS_EN, 0)
/** Set wde_en: Power Wheels */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_EN_SET(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_WDE_EN, value, 0)
/** Set wde_en (no callback): Power Wheels */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_EN_SET_NO_CALLBACK(value) gs_param_set_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_WDE_EN, value, GS_PARAM_F_NO_CALLBACK)
/** Get wde_en: Power Wheels */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_EN_GET(value) gs_param_get_bool(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_WDE_EN, value, 0)
/** Get wde_en (no check - return 0 on failure): Power Wheels */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_EN_GET_NC() gs_param_get_bool_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_WDE_EN, 0)
/** Set gps_i: GPS current */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_I_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_GPS_I, value, 0)
/** Set gps_i (no callback): GPS current */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_I_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_GPS_I, value, GS_PARAM_F_NO_CALLBACK)
/** Get gps_i: GPS current */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_I_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_GPS_I, value, 0)
/** Get gps_i (no check - return 0 on failure): GPS current */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_I_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_GPS_I, 0)
/** Set wde_i: Wheel current */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_I_SET(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_WDE_I, value, 0)
/** Set wde_i (no callback): Wheel current */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_I_SET_NO_CALLBACK(value) gs_param_set_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_WDE_I, value, GS_PARAM_F_NO_CALLBACK)
/** Get wde_i: Wheel current */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_I_GET(value) gs_param_get_uint16(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_WDE_I, value, 0)
/** Get wde_i (no check - return 0 on failure): Wheel current */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_I_GET_NC() gs_param_get_uint16_nc(gs_param_table_by_id(GS_A3200DOCK_ADCS3_DOCK_TABLE_ID), GS_A3200DOCK_ADCS3_DOCK_WDE_I, 0)
/**@}*/

#ifdef __cplusplus
}
#endif
#endif
