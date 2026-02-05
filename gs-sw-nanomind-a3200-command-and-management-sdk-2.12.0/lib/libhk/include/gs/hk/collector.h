#ifndef GS_HK_COLLECTOR_H_
#define GS_HK_COLLECTOR_H_
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

/**
   @file

   @brief Housekeeping system Collector.
*/

#include <gs/util/error.h>

/**
   Create collector threads.
*/
void gs_hk_create_collector_threads(void);

/**
   Temporarily update the priority of all beacons.
   Update is reverted on next reload or reboot
   @param[in] priority new priority
   @return_gs_error_t
*/
gs_error_t gs_hk_update_priority_beacons(const char *priority);

#endif
