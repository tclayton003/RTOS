#ifndef GS_GOSH_GSCRIPT_GSCRIPT_H
#define GS_GOSH_GSCRIPT_GSCRIPT_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   gscript (GOMspace script).

   Gscripts consists of a number of GOSH commands and some simple control logic.
   The format is as follows:
   \<delay ms\> \<retry\> \<goto on error\> \<comamnd\> \[options\]
   delays ms: delay in mS before the command is executed.
   retry: number of times the command is tried executed, if it fails.
   goto on error: which line to goto, if command fails (after retry).
   command: GOSH command
   options: option(s) for command.
*/

#include <gs/gosh/gscript/client.h>
#include <gs/csp/csp.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Initialize gscript module.
   @return_gs_error_t
*/
gs_error_t gs_gscript_init(void);    

/**
   Execute script locally.

   @param[in] script script or commands - depends on \a is_file.
   @param[in] script_length length of \a script - if 0 specified, \a script must be 0 terminated.
   @param[in] is_file true if \a script is a filename, false if \a script contains script commands.
   @param[in] when_utc_seconds when to execute the script (UTC), use 0 to execute now.
   @param[in] stack_size stack size for the executing thread. Use gs_command_get_stack_size().
   @param[in] priority priotity for the executing thread. Use GS_THREAD_PRIORITY_LOW.
   @return_gs_error_t
*/
gs_error_t gs_gscript_exec(const char * script, size_t script_length, bool is_file, uint32_t when_utc_seconds, size_t stack_size, int priority);

/**
   Request all scripts to terminate.
   @note this function signals all running scripts to terminate - it does not wait for them to terminate.
*/
void gs_gscript_stop(void);

/**
   Service handler for handling remote script requests.

   @param[in] conn incoming connection
   @return_gs_error_t
*/
gs_error_t gs_gscript_csp_service_handler(csp_conn_t * conn);

/**
   Service handler for handling remote script requests.

   @deprecated use gs_gscript_csp_service_handler()
   @param[in] conn incoming connection
   @param[in] packet packet from incoming connection.
   @return_gs_error_t
*/
void gscript_service_handler(csp_conn_t * conn, csp_packet_t * packet);

#ifdef __cplusplus
}
#endif
#endif
