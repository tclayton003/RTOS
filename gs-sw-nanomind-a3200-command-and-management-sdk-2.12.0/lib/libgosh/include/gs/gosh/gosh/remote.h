#ifndef GS_GOSH_GOSH_REMOTE_H
#define GS_GOSH_GOSH_REMOTE_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/gosh/remote/remote.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Process remote requests.

   @deprecated use gs_gosh_remote_csp_service_handler()
   @param[in] conn CSP connection.
   @return_gs_error_t
*/
static inline gs_error_t gosh_remote_shell_process(csp_conn_t *conn)
{
    return gs_gosh_remote_csp_service_handler(conn);
}
 
#ifdef __cplusplus
}
#endif
#endif
