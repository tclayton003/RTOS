#ifndef GS_RGOSH_EXEC_H
#define GS_RGOSH_EXEC_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   RGOSH job execution engine.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Create RGOSH backend(s) handling RGOSH requests from clients.

   @param[in] threads Number of worker threads to start.
   @param[in] stack_size thread stack size for each worker thread. If 0, gs_command_get_stack_size() is used to get stack size.
   @param[in] queue_length length of processing queue for each worker thread.
   @return_gs_error_t
*/
gs_error_t gs_rgosh_exec_engine_start2(uint8_t threads, uint32_t stack_size, uint8_t queue_length);

/**
   Create RGOSH backend(s) handling RGOSH requests from clients.

   @param[in] threads Number of worker threads to start.
   @param[in] stack_size thread stack size for each worker thread. If 0, gs_command_get_stack_size() is used to get stack size.
   @return_gs_error_t
*/
static inline gs_error_t gs_rgosh_exec_engine_start(uint8_t threads, uint32_t stack_size)
{
    return gs_rgosh_exec_engine_start2(threads, stack_size, 10);
}

/**
   Stop/Destroy RGOSH backend(s) (for testing use only).

   Signal stop to the worker threads and waits for them to terminate (join).

   @note Join is performed, which may hang forever if the worker threads doesn't respond to the stop request.
   @return_gs_error_t
*/
gs_error_t gs_rgosh_exec_engine_stop(void);

#ifdef __cplusplus
}
#endif
#endif
