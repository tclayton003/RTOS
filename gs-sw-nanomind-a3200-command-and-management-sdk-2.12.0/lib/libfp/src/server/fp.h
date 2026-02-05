#ifndef SRC_SERVER_FP_H
#define SRC_SERVER_FP_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/util/log.h>

GS_LOG_GROUP_EXTERN(gs_fp_log);
#define LOG_DEFAULT gs_fp_log

/**
 * Lock flight planner lock for reading
 * @return On success, 0 is returned. On error, -1 is returned.
 */
void fp_read_lock(void);

/**
 * Unlock flight planner lock after reading
 * @return On success, 0 is returned. On error, -1 is returned.
 */
void fp_read_unlock(void);

/**
 * Lock flight planner lock for writing
 * @return On success, 0 is returned. On error, -1 is returned.
 */
void fp_write_lock(void);

/**
 * Unlock flight planner lock after writing
 * @return On success, 0 is returned. On error, -1 is returned.
 */
void fp_write_unlock(void);

/**
 * Flush current flight plan.
 * @return_gs_error_t
 */
gs_error_t fp_flush(void);

/**
 * Store the current flight plan
 * @param path
 * @return_gs_error_t
 */
gs_error_t fp_store_save(const char* path);

/**
 * Load a stored flight plan and make it the current one
 * @param path
 * @param fast_load_with_unique_keys When set the key uniqueness in the flightplan aren't verified, they MUST be unique.
 *
 *  * @return_gs_error_t
 */
gs_error_t fp_store_load(const char* path, bool fast_load_with_unique_keys);

#endif
