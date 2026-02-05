/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/tps382x/tps382x.h>
#include <gs/util/thread.h>
#include <gs/util/minmax.h>

#define WDT_SERVICE_INTERVAL_MS 100

static void * tps382x_watchdog_thread(void * device_data)
{
    gs_tps382x_watchdog_device_data *dev = (gs_tps382x_watchdog_device_data*)device_data;

    while(1) {
        if (dev->left > 0)
        {
            for (uint8_t i=0; i < dev->gpio_count; i++) {
                gs_gpio_set(dev->gpio[i], !gs_gpio_get_nc(dev->gpio[i]));
            }
            dev->left--;
        }
        gs_thread_sleep_ms(WDT_SERVICE_INTERVAL_MS);
    }
    gs_thread_exit(NULL);
    return NULL;
}

/***************** Hooks for implementation of watchdog in libutil *****************/

static gs_error_t gs_watchdog_start(gs_watchdog_device_t * watchdog)
{
    gs_thread_create("TPS382x", tps382x_watchdog_thread, watchdog->driver_data, 500,
                     GS_THREAD_PRIORITY_CRITICAL, 0, NULL);
    return GS_OK;
}


static gs_error_t gs_watchdog_ping(gs_watchdog_device_t * watchdog)
{
    gs_tps382x_watchdog_device_data *dev = (gs_tps382x_watchdog_device_data*)watchdog->driver_data;
    uint32_t ticks = dev->timeout / WDT_SERVICE_INTERVAL_MS;
    dev->left = gs_max(ticks, (uint32_t)1);
    return GS_OK;
}


static gs_error_t gs_watchdog_stop(gs_watchdog_device_t * watchdog)
{
    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t gs_watchdog_set_timeout(gs_watchdog_device_t * watchdog, unsigned int timeout)
{
    gs_tps382x_watchdog_device_data *dev = (gs_tps382x_watchdog_device_data*)watchdog->driver_data;
    dev->timeout = timeout*1000;
    uint32_t ticks = dev->timeout / WDT_SERVICE_INTERVAL_MS;
    dev->left = gs_max(ticks, (uint32_t)1);
    return GS_OK;
}

static const gs_watchdog_dev_ops_t dev_ops = {
    .start = gs_watchdog_start,
    .ping = gs_watchdog_ping,
    .stop = gs_watchdog_stop,
    .set_timeout = gs_watchdog_set_timeout
};


/**
   Get device operations for libutil watchdog API
   @return pointer to device operations
*/
const gs_watchdog_dev_ops_t * gs_tps382x_watchdog_get_dev_ops(void)
{
    return &dev_ops;
}
