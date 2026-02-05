#ifndef FP_TIMER_H
#define FP_TIMER_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Flight planner timer-table
*/

#include <gs/fp/fp_timer.h>
#include "fp_list.h"

#define MAX_TIMERS 5000  // Maximum number of FP entries

typedef struct {
    char timer[FP_KEY_SIZE]; /**< Timer ID. Leave room for NULL termination */
    char cmd[FP_CMD_SIZE];   /**< Command. Leave room for NULL termination */
    fp_timer_state_t state;  /**< Timer state */
    fp_timer_basis_t basis;  /**< Timer basis */
    gs_timestamp_t last;     /**< Timer last execution */
    gs_timestamp_t when;     /**< Timer expiration time */
    uint16_t repeat;         /**< Timer Repetitions */
    uint16_t remain;         /**< Timer Remaining repetitions */
} fp_timer_t;

/**
 * Initialize timer list.
 * @return On success, 0 is returned. On error, -1 is returned.
 */
gs_error_t fp_timer_init(void);

/**
 * Create new timer.
 * @param id Timer to create.
 * @param basis Timer basis. Must either be FP_TIME_ABSOLUTE or
 * FP_TIME_RELATIVE.
 * @param when Timer expiration time.
 * @param repeat Number of repetitions. Use FP_TIME_UNLIMITED
 * @param key_surely_is_unique When set, key existence is NOT verified before being inserted in list.
 * Should be false unless you're sure no duplicated keys
 *
 * for at recurring timer.
 * @return_gs_error_t
 */
gs_error_t fp_timer_create(const char* key, const char* cmd, fp_timer_basis_t basis, fp_timer_state_t state,
                           gs_timestamp_t* when, uint16_t repeat, bool key_surely_is_unique);

/**
 * Delete timer.
 * @param id Timer to delete.
 * @return_gs_error_t
 */
gs_error_t fp_timer_delete(const char* key);

/**
 * Delete all timers in list and the list it self.
 * @return_gs_error_t
 */
gs_error_t fp_timer_flush(void);

/**
 * Iterate over all timers and apply function.
 * @param apply Function to apply.
 * @param ret Optional pointer to a return value.
 * @param retsize Size of return value.
 * @return_gs_error_t
 */
gs_error_t fp_timer_iterate(fp_iter_func_t apply, void* ret, size_t retsize);

/**
 * Number of timers.
 * @return On success, the number of timers is returned.
 * On error, -1 is returned.
 */
int fp_timer_count(void);

/**
 * Search for specific timer.
 * @param id Timer to search for.
 * @return On success, a pointer to the timer is returned.
 * On error, NULL is returned.
 */
fp_timer_t* fp_timer_search(const char* key);

/**
 * Get timer at position.
 * @param pos Timer position.
 * @return On success, a pointer to the timer is returned.
 * On error, NULL is returned.
 */
fp_timer_t* fp_timer_get(int pos);

/**
 * Set timer active.
 * @param id: Timer to set active.
 * @return_gs_error_t
 */
gs_error_t fp_timer_set_active(fp_timer_t* t);

/**
 * Set all timers active.
 * @return_gs_error_t
 */
gs_error_t fp_timer_set_all_active(void);

/**
 * Set timer dormant.
 * @param id: Timer to set dormant.
 * @return_gs_error_t
 */
gs_error_t fp_timer_set_dormant(const char* key);

/**
 * Set repeat count on timer
 * @param id: Timer to set number of repeats on
 * @param count number of repeats
 * @return_gs_error_t
 */
gs_error_t fp_timer_set_repeat(fp_timer_t* t, uint16_t count);

/**
 * Set execution time on timer
 * @param key id: Timer to set
 * @param basis Timer basis. Must either be FP_TIME_ABSOLUTE or
 * FP_TIME_RELATIVE
 * @param when Timer expiration time.
 * @return_gs_error_t
 */
gs_error_t fp_timer_set_time(const char* key, uint8_t basis, gs_timestamp_t* when);

#endif
