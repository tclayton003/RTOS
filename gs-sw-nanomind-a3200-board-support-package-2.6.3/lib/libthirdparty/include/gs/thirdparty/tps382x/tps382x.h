/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
#ifndef GS_INCLUDE_THIRDPARTY_TPS382X_TPS382X_H
#define GS_INCLUDE_THIRDPARTY_TPS382X_TPS382X_H

/**
   @file
   @brief Watchdog driver for the Texas Instruments TPS382x family of watchdog devices.
*/

#include <gs/util/drivers/watchdog/device.h>
#include <gs/util/drivers/gpio/gpio.h>

/** Max numbers of supported watchdogs */
#define GS_TPS382X_DEVICE_COUNT 2

/**
   @brief Device specifc info.

   Device specifc info that must be passed into driver_data of gs_watchdog_device_t when
   Initializing the watchdog.
   The driver can handle up to 2 watchdog devices at once
*/
typedef struct {
    uint32_t   left;                          /**< Internal use */
    uint32_t   timeout;                       /**< Internal use */
    uint8_t    gpio_count;                    /**< Number of configured gpio's */
    gs_gpio_t  gpio[GS_TPS382X_DEVICE_COUNT]; /**< gpio's used to control the watchdogs */
}gs_tps382x_watchdog_device_data;

/**
   Get device operations for libutil watchdog API
   @return pointer to device operations
*/
const gs_watchdog_dev_ops_t * gs_tps382x_watchdog_get_dev_ops(void);


#endif
