#ifndef GS_GOSH_REMOTE_REMOTE_H
#define GS_GOSH_REMOTE_REMOTE_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Execute remote shell command over CSP.
*/

#include <gs/csp/csp.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Initialize remote service.

   @version 3.4.3
   @return_gs_error_t
*/
gs_error_t gs_gosh_remote_init(void);
    
/**
   Process remote requests.

   @param[in] conn CSP connection.
   @return_gs_error_t
*/
gs_error_t gs_gosh_remote_csp_service_handler(csp_conn_t *conn);

#ifdef __cplusplus
}
#endif
#endif
