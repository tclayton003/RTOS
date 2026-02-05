/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * Watchdog interface.
 */

/* --- includes ----------------------------------------------------------*/

#include <gs/embed/asf/samc/drivers/watchdog.h>
#include <sam0/drivers/wdt/wdt.h>
#include <gs/embed/asf/samc/convert_asf_status_code.h>

/* --- definitions -------------------------------------------------------*/

/* --- code --------------------------------------------------------------*/

// NOTE: logging/stdout NOT allowed
gs_error_t gs_watchdog_init(void)
{
    struct wdt_conf config;

    wdt_get_config_defaults(&config);
    config.timeout_period = WDT_PERIOD_16384CLK;

    enum status_code status = wdt_set_config(&config);
    if (status != STATUS_OK) {
        return gs_convert_asf_status_code(status);
    }

    gs_watchdog_touch();

    return GS_OK;
}

void gs_watchdog_touch(void)
{
    wdt_reset_count();
}
