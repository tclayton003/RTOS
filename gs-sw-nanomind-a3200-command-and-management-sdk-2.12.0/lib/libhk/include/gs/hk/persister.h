#ifndef GS_HK_PERSISTER_H
#define GS_HK_PERSISTER_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/**
   @file

   @brief Housekeeping system Persister, part of the Telemetry Cache.
*/

#include <gs/util/error.h>

/**
   Create persister thread.
   @return_gs_error_t
*/
gs_error_t gs_hk_create_persister_thread(void);

#endif
