#ifndef FP_SERVER_H
#define FP_SERVER_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/util/error.h>
#include <csp/csp_types.h>

/**
    Start the service handler
    @return_gs_error_t
*/
gs_error_t gs_fp_service_dispatcher_start(void);

/**
    Stop the service handler
    @return_gs_error_t
*/
gs_error_t gs_fp_service_stop(void);

#endif
