#ifndef GS_GOSH_REDIRECT_REDIRECT_H
#define GS_GOSH_REDIRECT_REDIRECT_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Redirection of stdio over TCP/IP socket.
*/

#include <gs/util/linux/argp.h>
#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Default TCP/IP port used for redirecting GOSH stdio.
*/
#define GOSH_DEFAULT_REDIRECT_PORT 5001

/**
   Starts thread listing on port and forward stdio.

   @param[in] port TCP/IP listen (server) port. If port is 0, default port or port specified on the command line will be used.
   @return_gs_error_t
*/
gs_error_t gs_gosh_redirect_stdio(uint16_t port);

/**
   Stop redirect thread.

   @return_gs_error_t
*/
gs_error_t gs_gosh_redirect_stdio_stop(void);
    
/**
   Starts thread listing on port and forward stdio.

   @deprecated use gs_gosh_redirect_stdio()
   @param[in] port TCP/IP listen (server) port. If port is 0, default port or port specified on the command line will be used.
   @return_gs_error_t
*/
static inline int gosh_redirect(unsigned short port)
{
    return gs_gosh_redirect_stdio(port);
}

/**
   Stop redirect thread.

   @deprecated use gs_gosh_redirect_stdio_stop()
   @return_gs_error_t
*/
static inline int gosh_redirect_stop(void)
{
    return gs_gosh_redirect_stdio_stop();
}
    
/**
   Starts thread listing on port and forward stdio - if specified on command line.
   @return #GS_ERROR_NOT_FOUND if no port is set
   @return_gs_error_t
*/
gs_error_t gs_gosh_redirect_stdio_command_line_option(void);
    
/**
   Command line options for redirect, e.g. setting port.
*/
extern const struct argp_child gs_gosh_redirect_stdio_command_line_argp;

#ifdef __cplusplus
}
#endif
#endif
