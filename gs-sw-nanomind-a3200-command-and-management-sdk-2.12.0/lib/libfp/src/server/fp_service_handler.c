/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "fp_server.h"
#include <gs/fp/fp.h>
#include <csp/csp.h>
#include <gs/csp/csp.h>
#include <gs/csp/service_dispatcher.h>

#include <gs/util/log.h>

#include <stdio.h>

#define GS_FP_INTERNAL_USE 1
#include <gs/fp/internal/types.h>

gs_error_t gs_fp_start()
{
    // Initiate the timer list
    gs_error_t error = fp_init();
    if (error != GS_OK) {
        return error;
    }

    // Start the timer thread
    error = fp_timer_start();
    if (error != GS_OK) {
        return error;
    }

    // Start the service dispatcher
    return gs_fp_service_dispatcher_start();
}

gs_error_t gs_fp_stop(void)
{
    // Stop the timer thread
    gs_error_t error = fp_timer_stop();
    if (error != GS_OK) {
        log_error("Could not stop the timer thread, error: %s", gs_error_string(error));
        return error;
    }

    // Stop the service dispatcher
    error = gs_fp_service_stop();
    if (error != GS_OK) {
        log_error("Could not stop the service dispatcher, error: %s", gs_error_string(error));
    }

    return error;
}
