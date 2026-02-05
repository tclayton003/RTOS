#ifndef GS_PARAM_CSP_SERVICE_HANDLER_H
#define GS_PARAM_CSP_SERVICE_HANDLER_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   CSP service handler (rparam server).
*/

#include <csp/csp.h>
#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   CSP service handler.

   The handler will process all packets on the connection and close the connection.

   @param[in] conn CSP connection.
   @return_gs_error_t
*/
gs_error_t gs_rparam_csp_service_handler(csp_conn_t * conn);

#ifdef __cplusplus
}
#endif
#endif
