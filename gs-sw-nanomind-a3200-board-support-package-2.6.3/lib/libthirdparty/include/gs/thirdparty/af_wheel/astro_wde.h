#ifndef GS_THIRDPARTY_AF_WHEEL_ASTRO_WDE_H
#define GS_THIRDPARTY_AF_WHEEL_ASTRO_WDE_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Astro wheel driver
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the current speed from a wheel
 * @param wheel 0-3 wheel to get speed from
 * @param speed output
 * @return_gs_error_t
 */
gs_error_t af_get_speed(uint8_t wheel, float *speed);

/**
 * Configure the spi channel towards the wheel
 * @param spi_slave SPI addr of the slave
 * @return_gs_error_t
 */
gs_error_t af_wheels_init(uint8_t spi_slave);

/**
 * Set the torque on the wheel
 * @param wheel 0-3 wheel to set torque on
 * @param torque the requested torque
 * @param inertia the inertia of the wheel
 * @return_gs_error_t
 */
gs_error_t af_set_torque(uint8_t wheel, float torque, float inertia);

/**
 * Set enable on the wheel
 * @param wheel 0-3 set status on wheel. 5 set on all
 * @param status 0 or 1 set disable or enable
 * @param wheelPointer array[4] of enabled status. Modified by function
 * @return_gs_error_t
 */
gs_error_t af_set_enable(uint8_t wheel, uint8_t status, uint8_t *wheelPointer);

#ifdef __cplusplus
}
#endif
#endif
