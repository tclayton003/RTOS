/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Flight planner timer event thread.
*/

#include "fp_timer.h"
#include <gs/util/clock.h>
#include <gs/util/gosh/command.h>
#include <gs/util/log.h>
#include <gs/util/minmax.h>
#include <gs/util/stdio.h>
#include <gs/util/string.h>
#include <gs/util/thread.h>
#include <gs/util/time.h>
#include <stdlib.h>
#include "fp.h"

static fp_list_t times;

int fp_timer_init(void)
{
    return fp_list_init(&times);
}

gs_error_t fp_timer_create(const char *key, const char *cmd, fp_timer_basis_t basis, fp_timer_state_t state,
                           gs_timestamp_t *when, uint16_t repeat, bool key_surely_is_unique)
{
    gs_error_t error;
    if (fp_timer_count() >= MAX_TIMERS) {
        log_warning("Failed to create timer. To many entries in list.");
        return GS_ERROR_RANGE;
    }

    if (strnlen(key, FP_KEY_SIZE) >= FP_KEY_SIZE) {
        log_warning("Timer key too long.");
        return GS_ERROR_OVERFLOW;
    }

    if (strnlen(cmd, FP_CMD_SIZE) >= FP_CMD_SIZE) {
        log_warning("Timer command too long.");
        return GS_ERROR_OVERFLOW;
    }

    /* Allocate timer */
    fp_timer_t *t = (fp_timer_t *)calloc(1, sizeof(*t));
    if (t == NULL) {
        log_warning("Failed to allocate memory for new time");
        return GS_ERROR_ALLOC;
    }

    strcpy(t->timer, key);
    strcpy(t->cmd, cmd);

    t->basis = basis;
    t->state = state;

    if (basis == FP_TIME_ABSOLUTE) {
        t->repeat = 1;
        t->when.tv_sec = when->tv_sec;
        t->when.tv_nsec = when->tv_nsec;
    } else if (basis == FP_TIME_RELATIVE) {
        gs_clock_get_monotonic(&(t->last));
        t->repeat = repeat;
        t->when.tv_sec = when->tv_sec;
        t->when.tv_nsec = when->tv_nsec;
    }

    t->remain = t->repeat;

    // Lock the write mutex
    fp_write_lock();

    /* Insert in list */
    if (key_surely_is_unique) {
        // Here assuming that the key is unique (that it isn't already in list)
        error = fp_list_append(&times, key, t);
    } else {
        error = fp_list_append_unique(&times, key, t);
    }
    if (error != GS_OK) {
        free(t);
        log_warning("Failed to insert timer in list");
    }

    // Unlock the write mutex
    fp_write_unlock();

    return error;
}

gs_error_t fp_timer_delete(const char *key)
{
    /* Get timer */
    fp_timer_t *t = fp_timer_search(key);
    if (t == NULL) {
        log_warning("Unknown timer: %s", key);
        return GS_ERROR_NO_DATA;
    }

    // Lock the write mutex
    fp_write_lock();

    fp_list_entry_t *entry = fp_list_remove(&times, key);
    if (entry == NULL) {
        return GS_ERROR_HANDLE;
    } else {
        /* Free memory used by timer */
        free(t);
        free(entry);
    }

    // Unlock the write mutex
    fp_write_unlock();

    return GS_OK;
}

gs_error_t fp_timer_flush(void)
{
    /* List empty */
    if (fp_list_empty(&times)) {
        log_warning("Timer list is empty.");
        return GS_ERROR_NO_DATA;
    }

    while (times.head != NULL) {
        fp_list_entry_t *next_entry = times.head->next;

        /* Free the timer and list entry */
        free(times.head->entry);
        free(times.head);

        times.head = next_entry;
    }

    /* Make sure the list is reset */
    fp_list_init(&times);

    return GS_OK;
}

fp_timer_t *fp_timer_search(const char *key)
{
    return (fp_timer_t *)fp_list_search(&times, key);
}

fp_timer_t *fp_timer_get(int pos)
{
    return (fp_timer_t *)fp_list_get(&times, pos);
}

gs_error_t fp_timer_iterate(fp_iter_func_t apply, void *ret, size_t retsize)
{
    return fp_list_iterate(&times, apply, ret, retsize);
}

int fp_timer_count(void)
{
    return fp_list_count(&times);
}

gs_error_t fp_timer_set_active(fp_timer_t *t)
{
    if (t == NULL) {
        return GS_ERROR_NO_DATA;
    }

    // Lock the write mutex
    fp_write_lock();

    /* If timer is on interval, set last execution to now */
    if (t->basis == FP_TIME_RELATIVE) {
        gs_clock_get_monotonic(&(t->last));
    }

    t->state = FP_TIME_ACTIVE;
    t->remain = t->repeat;

    // Unlock the write mutex
    fp_write_unlock();

    return GS_OK;
}

gs_error_t fp_timer_activate_all(int pos, char *key, void *entry, void *ret, size_t retsize)
{
    if (entry == NULL)
        return GS_ERROR_NOT_FOUND;

    fp_timer_t *t = (fp_timer_t *)entry;

    gs_error_t error = fp_timer_set_active(t);
    if (error != GS_OK) {
        return error;
    }

    error = fp_timer_set_repeat(t, 1);

    return error;
}

gs_error_t fp_timer_set_all_active(void)
{
    /* List empty */
    if (fp_list_empty(&times)) {
        log_warning("Timer list is empty.");
        return GS_ERROR_NO_DATA;
    }

    return fp_list_iterate(&times, fp_timer_activate_all, NULL, 0);
}

gs_error_t fp_timer_set_dormant(const char *key)
{
    fp_timer_t *t = fp_timer_search(key);
    if (t == NULL) {
        log_warning("Unknown timer: %s", key);
        return GS_ERROR_NO_DATA;
    }

    // Lock the write mutex
    fp_write_lock();

    t->state = FP_TIME_DORMANT;

    // Unlock the write mutex
    fp_write_unlock();

    return GS_OK;
}

gs_error_t fp_timer_set_repeat(fp_timer_t *t, uint16_t count)
{
    if (t == NULL) {
        return GS_ERROR_NO_DATA;
    }

    // Lock the write mutex
    fp_write_lock();

    t->repeat = count;

    // Unlock the write mutex
    fp_write_unlock();

    return GS_OK;
}

gs_error_t fp_timer_set_time(const char *key, uint8_t basis, gs_timestamp_t *when)
{
    fp_timer_t *t = fp_timer_search(key);
    if (t == NULL) {
        log_warning("Unknown timer: %s", key);
        return GS_ERROR_NO_DATA;
    }

    // Lock the write mutex
    fp_write_lock();

    t->basis = basis;
    t->when.tv_sec = when->tv_sec;
    t->when.tv_nsec = when->tv_nsec;

    // Unlock the write mutex
    fp_write_unlock();

    return GS_OK;
}

gs_error_t fp_timer_process(fp_timer_t *t)
{
    gs_timestamp_t now;

    if (t == NULL)
        return GS_ERROR_NO_DATA;

    /* Check if timer is active */
    if (t->state == FP_TIME_DORMANT)
        return GS_OK;

    switch (t->basis) {
        case FP_TIME_ABSOLUTE:

            log_trace("Absolute timer");

            /* Check execution time */
            gs_clock_get_time(&now);
            if (timestamp_ge(&(t->when), &now) != 1)
                return GS_OK;

            /* When executed, the timer goes dormant */
            log_debug("Setting timer dormant");
            t->state = FP_TIME_DORMANT;
            t->remain = 0;

            break;

        case FP_TIME_RELATIVE:

            log_trace("Relative timer");

            /* Check interval */
            gs_clock_get_monotonic(&now);
            gs_timestamp_t tm = {0, 0};
            timestamp_add(&tm, &(t->last));
            timestamp_add(&tm, &(t->when));
            if (timestamp_ge(&tm, &now) != 1)
                return GS_OK;

            /* Decrement repeat counter */
            if (t->remain > 0 && t->repeat != FP_TIME_UNLIMITED) {
                t->remain--;
                log_debug("Timer remain: %u", t->remain);
            }

            /* Check repeat counter */
            if (t->remain == 0) {
                log_debug("Setting timer dormant");
                t->state = FP_TIME_DORMANT;
            }

            break;

        default:
            return GS_OK;
    }

    // Run command
    int cmd_result = 0;
    int result = gs_command_run(t->cmd, &cmd_result);
    if ((result == 0) && (cmd_result == 0)) {
        log_info("\"%s\" ret: %d", t->cmd, result);
    } else {
        log_warning("\"%s\" ret: %d / %d", t->cmd, result, cmd_result);
    }

    /* Update last execution time */
    timestamp_copy(&now, &(t->last));

    return GS_OK;
}

static gs_thread_t timer_thread;
static bool timer_run = false;
static void *fp_timer_task(void *param)
{
    const char *load_cmd = "fp load ";
    while (timer_run) {
        fp_read_lock();

        /* Loop list */
        fp_list_entry_t *entry = times.head;
        while (entry != NULL) {
            fp_timer_t *t = (fp_timer_t *)entry->entry;

            // Check if the entry is a "fp load" command
            if (strncmp(t->cmd, load_cmd, strlen(load_cmd)) == 0) {
                char path[FP_MAX_PATH_LENGTH];
                strncpy(path, t->cmd + strlen(load_cmd), FP_MAX_PATH_LENGTH);

                fp_read_unlock();
                fp_store_load(path, false);
                fp_read_lock();
                break;
            } else {
                fp_timer_process(t);
                entry = entry->next;
            }
        }

        fp_read_unlock();

        gs_time_sleep_ms(100);
    }

    gs_thread_exit(NULL);
}

gs_error_t fp_timer_start(void)
{
    const size_t stack_size = gs_max(10000U, gs_command_get_stack_size());
    timer_run = true;
    gs_error_t error = gs_thread_create("FP_TIMER", fp_timer_task, NULL, stack_size, GS_THREAD_PRIORITY_LOW,
                                        GS_THREAD_CREATE_JOINABLE, &timer_thread);
    if (error) {
        log_error("Failed to start FP timer task, error: %s", gs_error_string(error));
    }

    return error;
}

gs_error_t fp_timer_stop(void)
{
    timer_run = false;
    return gs_thread_join(timer_thread, NULL);
}
