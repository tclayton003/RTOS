/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "../local.h"
#include <gs/util/watchdog/watchdog_task.h>


static int hw_wd_stop(gs_watchdog_device_t *wd_dev)
{
    (void)wd_dev;

    wdt_disable();
    wdt_clear();
    return 0;
}

static int hw_wd_start(gs_watchdog_device_t *wd_dev)
{
    wdt_disable();
    wdt_clear();
    wdt_opt_t g_wdtopt = { wd_dev->timeout*1000*1000, 0,0,0,0,0,0};
    wdt_enable(&g_wdtopt);
    return 0;
}

static int hw_wd_ping(gs_watchdog_device_t *wd_dev)
{
    (void)wd_dev;
    wdt_clear();

    return 0;
}

static int hw_wd_set_timeout(gs_watchdog_device_t *wd_dev, unsigned int timeout_s)
{
    wd_dev->timeout = timeout_s;
    return 0;
}

void gs_a3200_sdk_watchdog_init(void)
{
    gs_watchdog_device_t* wd_dev = calloc(1, sizeof(*wd_dev));
    gs_watchdog_dev_ops_t* wd_ops = calloc(1, sizeof(*wd_ops));

    wd_dev->id = 0;
    wd_dev->max_timeout = 50;
    wd_dev->min_timeout = 1;
    wd_dev->timeout = 5;
    wd_dev->ops = wd_ops;
    wd_ops->start = hw_wd_start;
    wd_ops->stop = hw_wd_stop;
    wd_ops->ping = hw_wd_ping;
    wd_ops->set_timeout = hw_wd_set_timeout;

    gs_swwd_create(10, wd_dev);
    gs_swwd_monitor_task_start();
}
