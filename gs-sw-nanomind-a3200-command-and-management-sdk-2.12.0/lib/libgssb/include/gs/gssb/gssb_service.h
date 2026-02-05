#ifndef INCLUDE_GS_GSSB_GSSB_SERVICE_H_
#define INCLUDE_GS_GSSB_GSSB_SERVICE_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/**
   @file

   CSP service handler
*/

#include <csp/csp.h>
#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   RGSSB server task

   @deprecated Replaced by gs_rgssb_csp_service_handler, which can be registered in a generic server task.

   @param[in] pvParameters Task parameters, which are unused.
 */
void gs_rgssb_server_task(void * pvParameters);


/**
   RGSSB cmd parser

   @deprecated Replaced by gs_rgssb_csp_service_handler, which can be registered in a generic server task.

   @param[in] conn CSP connection.
   @param[in] packet CSP packet.
 */
void gs_rgssb_cmd_parser(csp_conn_t * conn, csp_packet_t * packet);


/**
   CSP service handler.

   The handler will process all packets on the connection and close the connection.

   @param[in] conn CSP connection.
   @return_gs_error_t
*/
gs_error_t gs_rgssb_csp_service_handler(csp_conn_t * conn);

#ifdef __cplusplus
}
#endif
#endif
