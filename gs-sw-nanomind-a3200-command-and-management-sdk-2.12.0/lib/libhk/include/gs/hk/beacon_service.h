#ifndef GS_HK_BEACON_SERVICE_H
#define GS_HK_BEACON_SERVICE_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/**
   @file

   @brief Housekeeping system Beacon Service.
*/

#include <gs/util/error.h>

/**
   Create beacon service thread.
   @return_gs_error_t
*/
gs_error_t gs_hk_create_beacon_service_thread(void);

#endif
