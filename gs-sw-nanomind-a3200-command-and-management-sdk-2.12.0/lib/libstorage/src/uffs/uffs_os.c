/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

/**
   @file

   Implementation of required OS functions for UFFS.
*/

#include "uffs/uffs_os.h"
#include <gs/embed/freertos.h>
#include <gs/util/mutex.h>
#include <gs/util/log/log.h>
#include <gs/util/time.h>

int uffs_SemCreate(OSSEM *sem)
{
    gs_error_t error = gs_mutex_create((gs_mutex_t *) sem);
    if (error) {
        log_error("%s: gs_mutex_create() failed, error: %d", __FUNCTION__, error);
        *sem = OSSEM_NOT_INITED;
        return -1;
    }
    return 0;
}

int uffs_SemWait(OSSEM sem)
{
    return (gs_mutex_lock((gs_mutex_t) sem) == GS_OK) ? 0 : -1;
}

int uffs_SemSignal(OSSEM sem)
{
    return (gs_mutex_unlock((gs_mutex_t) sem) == GS_OK) ? 0 : -1;
}

int uffs_SemDelete(OSSEM *sem)
{
    gs_mutex_destroy((gs_mutex_t) *sem);
    *sem = OSSEM_NOT_INITED;
    return 0;
}

int uffs_OSGetTaskId(void)
{
    return (int)xTaskGetCurrentTaskHandle();
}

unsigned int uffs_GetCurDateTime(void)
{
    return gs_time_rel_ms();
}
