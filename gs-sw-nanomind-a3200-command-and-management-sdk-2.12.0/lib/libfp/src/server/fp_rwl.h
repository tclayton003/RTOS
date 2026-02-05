#ifndef _FP_RWL_H_
#define _FP_RWL_H_
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file fp_rwl.h

   Flight planner Readers-Writer lock.
   An RWL allows mutual exclusion of either one writer or multiple readers.
*/

#include <gs/util/error.h>
#include <gs/util/mutex.h>

/**
   RWL structure.
*/
typedef struct {
    uint32_t readers;
    gs_mutex_t reader_sem;
    gs_mutex_t access_sem;
} fp_rwl_t;

gs_error_t fp_rwl_init(fp_rwl_t * lock);

gs_error_t fp_rwl_write_lock(fp_rwl_t * lock);
gs_error_t fp_rwl_write_unlock(fp_rwl_t * lock);

gs_error_t fp_rwl_read_lock(fp_rwl_t * lock);
gs_error_t fp_rwl_read_unlock(fp_rwl_t * lock);

#endif
