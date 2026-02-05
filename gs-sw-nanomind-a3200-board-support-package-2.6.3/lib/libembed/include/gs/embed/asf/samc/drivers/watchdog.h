#ifndef LIB_LIBSAMC_INCLUDE_SAMC_DRIVERS_WATCHDOG_H_
#define LIB_LIBSAMC_INCLUDE_SAMC_DRIVERS_WATCHDOG_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/* --- includes ----------------------------------------------------------*/

#include <gs/util/error.h>

/* --- definitions -------------------------------------------------------*/

/**
 * Initialize watchdog to approx. 16 seconds timeout.
 */
gs_error_t gs_watchdog_init(void);

/**
 * Touch watchdog.
 */
void gs_watchdog_touch(void);

#endif
