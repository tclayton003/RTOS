/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Flight planner Readers-Writer lock.
   An RWL allows mutual exclusion of either one writer or multiple readers.
*/

#include "fp.h"
#include "fp_rwl.h"
#include <gs/util/stdio.h>
#include <gs/util/string.h>
#include <gs/util/check.h>

/* NOTE: Consider/prevent writer starvation	*
 * The current implementation allows additional *
 * readers to lock, even if a writer is pending */

gs_error_t fp_rwl_init(fp_rwl_t * lock)
{
    GS_CHECK_HANDLE(lock != NULL);

    memset(lock, 0, sizeof(*lock));

    gs_error_t error = gs_mutex_create(&lock->reader_sem);
    if (error == GS_OK) {
        error = gs_mutex_create(&lock->access_sem);
    }

    return error;
}

gs_error_t fp_rwl_write_lock(fp_rwl_t * lock)
{
    GS_CHECK_HANDLE(lock != NULL);

    return gs_mutex_lock(lock->access_sem);
}

gs_error_t fp_rwl_write_unlock(fp_rwl_t * lock)
{
    GS_CHECK_HANDLE(lock != NULL);

    return gs_mutex_unlock(lock->access_sem);
}

gs_error_t fp_rwl_read_lock(fp_rwl_t * lock)
{
    GS_CHECK_HANDLE(lock != NULL);

    /* Lock the reader semaphore */
    gs_error_t error = gs_mutex_lock(lock->reader_sem);
    if (error == GS_OK) {

        /* Increment number of readers */
        lock->readers++;

        /* If we are the first reader, lock the access */
        if (lock->readers == 1) {
            error = gs_mutex_lock(lock->access_sem);
            if (error) {
                lock->readers = 0;
            }
        }

        /* Allow next reader to lock */
        gs_mutex_unlock(lock->reader_sem);
    }

    return error;
}

gs_error_t fp_rwl_read_unlock(fp_rwl_t * lock)
{
    GS_CHECK_HANDLE(lock != NULL);

    gs_error_t error = gs_mutex_lock(lock->reader_sem);
    if (error == GS_OK) {

        /* Decrement number of readers */
        lock->readers--;

        /* If we are the last reader, unlock the access */
        if (lock->readers == 0) {
            error = gs_mutex_unlock(lock->access_sem);
            if (error) {
                lock->readers = 1;
            }
        }

        gs_mutex_unlock(lock->reader_sem);
    }

    return error;
}
