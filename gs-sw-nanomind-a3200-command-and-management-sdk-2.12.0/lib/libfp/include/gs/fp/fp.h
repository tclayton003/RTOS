#ifndef _FP_H_
#define _FP_H_
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Flight planner - server interface.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Initialize flight planner.

   @return_gs_error_t
*/
gs_error_t fp_init(void);

/**
   Start server task.

   @param[in] port port number to start server on.
   @return_gs_error_t
*/
gs_error_t fp_server_start(uint8_t port);

/**
   Stop server task.
   @return_gs_error_t
*/
gs_error_t fp_server_stop(void);

/**
   Start timer task.
   @return_gs_error_t
*/
gs_error_t fp_timer_start(void);

/**
   Stop timer task.
   @return_gs_error_t
*/
gs_error_t fp_timer_stop(void);

/**
    Start the service handler and timer thread.
    @return_gs_error_t
*/
gs_error_t gs_fp_start();

/**
    Stop the service handler and timer thread.
    @return_gs_error_t
*/
gs_error_t gs_fp_stop(void);

#ifdef __cplusplus
}
#endif
#endif
