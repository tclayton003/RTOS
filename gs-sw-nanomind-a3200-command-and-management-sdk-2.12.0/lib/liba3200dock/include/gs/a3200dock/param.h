#ifndef GS_A3200ODCK_PARAMETER_H
#define GS_A3200ODCK_PARAMETER_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Parameter interface for get/set parameters in the individually docks.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   A3200 dock parameters.

   These parameters maps to the docks own parameters.

   Not all parameters are supported by all docks.
*/
typedef enum {
    /**
       Enable/disable power to GPS.
       Supported on: ADCS3, ADCS6.
    */
    GS_A3200DOCK_PARAM_GPS_EN = 0,
    /**
       Enable/disable power to wheel 0.
       Supported on: ADCS6, ADCS8.
    */
    GS_A3200DOCK_PARAM_RW0_EN = 1,
    /**
       Enable/disable power to wheel 1.
       Supported on: ADCS6, ADCS8.
    */
    GS_A3200DOCK_PARAM_RW1_EN = 2,
    /**
       Enable/disable power to wheel 2.
       Supported on: ADCS6, ADCS8.
    */
    GS_A3200DOCK_PARAM_RW2_EN = 3,
    /**
       Enable/disable power to wheel 3.
       Supported on: ADCS6, ADCS8.
    */
    GS_A3200DOCK_PARAM_RW3_EN = 4,
    /**
       Enable/disable power to wheel driver electronics.
       Supported on: ADCS3.
    */
    GS_A3200DOCK_PARAM_WDE_EN = 5,
    /**
       Enable/disable power to GSSB C.
       Supported on: ADCS6, ADCS8.
    */
    GS_A3200DOCK_PARAM_GSSB_C_EN = 6,
    /**
       Enable/disable power to GSSB D.
       Supported on: ADCS6, ADCS8.
    */
    GS_A3200DOCK_PARAM_GSSB_D_EN = 7,
    /**
       Read/write current usage for the GPS.
       Supported on: ADCS3, ADCS6.
    */
    GS_A3200DOCK_PARAM_GPS_I = 8,
    /**
       Read/write current usage for the Fine Sun Sensor.
       Supported on: ADCS6, ADCS8.
    */
    GS_A3200DOCK_PARAM_GSSB_C_I = 9,
    /**
       Read/write current usage for the MAG.
       Supported on: ADCS6, ADCS8.
    */
    GS_A3200DOCK_PARAM_GSSB_D_I = 10,
    /**
       Read/write current usage for the Wheel Driver electronics.
       Supported on: ADCS3.
    */
    GS_A3200DOCK_PARAM_WDE_I = 13,
} gs_a3200dock_parameter_t;

/**
   Get parameter from the dock's table.
*/
gs_error_t gs_a3200dock_get_uint8(gs_a3200dock_parameter_t param, uint8_t *data);

/**
   Set parameter in the dock's table.
*/
gs_error_t gs_a3200dock_set_uint8(gs_a3200dock_parameter_t param, uint8_t data);

/**
   Get parameter from the dock's table.
*/
gs_error_t gs_a3200dock_get_uint16(gs_a3200dock_parameter_t param, uint16_t *data);

/**
   Set parameter in the dock's table.
*/
gs_error_t gs_a3200dock_set_uint16(gs_a3200dock_parameter_t param, uint16_t data);

/**
   Get parameter from the dock's table.
*/
gs_error_t gs_a3200dock_get_bool(gs_a3200dock_parameter_t param, bool *data);

/**
   Set parameter in the dock's table.
*/
gs_error_t gs_a3200dock_set_bool(gs_a3200dock_parameter_t param, bool data);

#ifdef __cplusplus
}
#endif
#endif
