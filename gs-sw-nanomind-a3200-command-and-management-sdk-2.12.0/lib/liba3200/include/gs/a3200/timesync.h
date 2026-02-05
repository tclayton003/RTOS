#ifndef GS_A3200_TIMESYNC_H
#define GS_A3200_TIMESYNC_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Time synchronization.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Start time synchronization task.
*/
gs_error_t gs_a3200_timesync(void);

#ifdef __cplusplus
}
#endif
#endif
