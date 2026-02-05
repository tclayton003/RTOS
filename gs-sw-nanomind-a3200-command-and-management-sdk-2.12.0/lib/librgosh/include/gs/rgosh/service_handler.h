#ifndef GS_RGOSH_SERVICE_HANDLER_H
#define GS_RGOSH_SERVICE_HANDLER_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file
   CSP RGOSH service handler (rgosh server).
*/

#include <gs/csp/csp.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   RGOSH service handler.

   The handler will process the request and queue it for execution.
   Use #GS_CSP_PORT_RGOSH as server port.

   @param[in] conn CSP connection.
   @return_gs_error_t
*/
gs_error_t gs_rgosh_service_handler(csp_conn_t * conn);

#ifdef __cplusplus
}
#endif
#endif
