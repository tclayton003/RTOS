/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "fp.h"
#include "fp_list.h"
#include "fp_timer.h"
#include "fp_rwl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <gs/util/string.h>
#include <gs/util/error.h>
#include <gs/util/clock.h>
#include <gs/util/log.h>
#include <gs/util/stdio.h>

#if defined(GS_LOG_CAT_FP)
#define LOG_CATEGORY GS_LOG_CAT_FP
#else
#define LOG_CATEGORY GS_LOG_CAT_DEFAULT
#endif

GS_LOG_GROUP(gs_fp_log, "fp", LOG_CATEGORY, LOG_DEFAULT_MASK);

/*
 * The Epic Flight Planner Lock (EFPL)
 * This RWL allows either multiple readers or
 * a single writer to access the FP structure.
 */
static fp_rwl_t efpl;

gs_error_t fp_init(void)
{
    gs_log_group_register(LOG_DEFAULT);
    log_info("Initializing flight planner");

    /* Initialise EFPL */
    gs_error_t error = fp_rwl_init(&efpl);
    if (error == GS_OK) {
        /* Init timer list */
        error = fp_timer_init();
    }

    return error;
}

void fp_read_lock(void)
{
    fp_rwl_read_lock(&efpl);
}

void fp_read_unlock(void)
{
    fp_rwl_read_unlock(&efpl);
}

void fp_write_lock(void)
{
    fp_rwl_write_lock(&efpl);
}

void fp_write_unlock(void)
{
    fp_rwl_write_unlock(&efpl);
}

gs_error_t fp_flush(void)
{
    // Lock the write mutex
    fp_write_lock();

    /* Delete all timers and list entries*/
    gs_error_t error = fp_timer_flush();

    // Unlock the write mutex
    fp_write_unlock();

    return error;
}

static FILE * fd_fp;

/* Get all timers and write timers to file */
gs_error_t fp_read_timers(int pos, char * key, void * entry, void * ret, size_t retsize)
{
    if (fd_fp == NULL)
        return GS_ERROR_HANDLE;

    fp_timer_t * timer = (fp_timer_t *) entry;

    /* Print timer to file */
    fprintf(fd_fp, "%s,%s,%d,%d,%"PRIu32",%"PRIu32",%"PRIu32",%"PRIu32",%"PRIu16"\r\n",
            timer->timer,
            timer->cmd,
            timer->state,
            timer->basis,
            timer->last.tv_sec,
            timer->last.tv_nsec,
            timer->when.tv_sec,
            timer->when.tv_nsec,
            timer->repeat
            );

    return GS_OK;
}

gs_error_t fp_store_save(const char * path)
{
    if (fp_timer_count() == 0) {
        log_warning("Can't store empty timer list.");
        return GS_ERROR_NO_DATA;
    }

    if ((fd_fp = fopen(path, "w+")) == NULL) {
        return GS_ERROR_HANDLE;
    }

    if (fp_timer_iterate(fp_read_timers, NULL, 0) != GS_OK) {
        fflush(fd_fp);
        fclose(fd_fp);
        return GS_ERROR_HANDLE;
    }

    fclose(fd_fp);

    return GS_OK;
}

void fp_reset_fp(void)
{
    /* If the list is not empty then flush it */
    if (fp_timer_count() > 0) {
        fp_flush();
    }

    /* Reset FP list */
    fp_timer_init();
}

#define BUFFER_SIZE 256
bool check_string(char *string)
{
    if (string == NULL)
        return false;

    gs_string_trim(string, strnlen(string, BUFFER_SIZE));

    if (gs_string_empty(string))
        return false;

    return true;
}

gs_error_t fp_store_load(const char * path, bool fast_load_with_unique_keys)
{
    FILE * fd;
    fp_timer_t timer;
    gs_timestamp_t now;

    if ((fd = fopen(path, "r")) == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    fp_reset_fp();

    // Lock the write mutex
    fp_write_lock();

    // Process the flightplan file. It should be a CSV file with 9 fields
    char buffer[BUFFER_SIZE];
    char *field;
    char *rest = NULL;
    while (fgets(buffer, BUFFER_SIZE, fd)) {
        // Check for an empty line
        if (!check_string(buffer)) {
            continue;
        }
        // Check if line is a comment
        if (buffer[0] == '#') {
            continue;
        }

        // Get the key parameter
        field = strtok_r(buffer, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No key.");
            continue;
        }
        if (strnlen(field, FP_KEY_SIZE) >= FP_KEY_SIZE) {
            log_warning("Illegal flight planner entry. Key too long.");
            continue;
        }
        strcpy(timer.timer, field);

        // Get the command parameter
        field = strtok_r(NULL, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No command.");
            continue;
        }
        if (strnlen(field, FP_CMD_SIZE) >= FP_CMD_SIZE) {
            log_warning("Illegal flight planner entry. Command too long.");
            continue;
        }
        strcpy(timer.cmd, field);

        // Get the state parameter
        field = strtok_r(NULL, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No state.");
            continue;
        }
        uint8_t tmp_u8 = 0;
        if (((strcmp(field, "0") != 0) && (strcmp(field, "1") != 0)) || (gs_string_to_uint8(field, &tmp_u8) != GS_OK)) {
            log_warning("Illegal flight planner entry: state must be 0 or 1.");
            continue;
        }
        timer.state = tmp_u8;

        // Get the basis parameter
        field = strtok_r(NULL, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No basis.");
            continue;
        }
        if (((strcmp(field, "0") != 0) && (strcmp(field, "1") != 0)) || (gs_string_to_uint8(field, &tmp_u8) != GS_OK)) {
            log_warning("Illegal flight planner entry: basis must be 0 or 1.");
            continue;
        }
        timer.basis = tmp_u8;

        // Get the last execution time [s]
        field = strtok_r(NULL, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No last execution time [s].");
            continue;
        }
        uint32_t tmp_u32 = 0;
        if (gs_string_to_uint32(field, &tmp_u32) != GS_OK) {
            log_warning("Illegal flight planner entry: last execution time [s]");
            continue;
        }
        timer.last.tv_sec = tmp_u32;

        // Get the last execution time [ns]
        field = strtok_r(NULL, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No last execution time [ns].");
            continue;
        }
        if (gs_string_to_uint32(field, &tmp_u32) != GS_OK) {
            log_warning("Illegal flight planner entry: last execution time [ns]");
            continue;
        }
        timer.last.tv_nsec = tmp_u32;

        // Get the execution time [s]
        field = strtok_r(NULL, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No execution time [s].");
            continue;
        }
        if (gs_string_to_uint32(field, &tmp_u32) != GS_OK) {
            log_warning("Illegal flight planner entry: execution time [s]");
            continue;
        }
        timer.when.tv_sec = tmp_u32;

        // Get the execution time [ns]
        field = strtok_r(NULL, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No execution time [ns].");
            continue;
        }
        if (gs_string_to_uint32(field, &tmp_u32) != GS_OK) {
            log_warning("Illegal flight planner entry: execution time [ns]");
            continue;
        }
        timer.when.tv_nsec = tmp_u32;

        // Get the repeat parameter
        field = strtok_r(NULL, ",", &rest);
        if (!check_string(field)) {
            log_warning("Illegal flight planner entry. No repeat parameter.");
            continue;
        }
        uint16_t tmp_u16 = 0;
        if (gs_string_to_uint16(field, &tmp_u16) != GS_OK) {
            log_warning("Illegal flight planner entry: repeat");
            continue;
        }
        timer.repeat = tmp_u16;

        // Drop timer if time has expired
        gs_clock_get_time(&now);
        if (timestamp_ge(&timer.when, &now) && (FP_TIME_ABSOLUTE == timer.basis)) {
            log_warning("dropping expired flight planner entry: %s", timer.timer);
            continue;
        }

        if (fp_timer_create(timer.timer, timer.cmd, timer.basis, timer.state, &timer.when, timer.repeat, 
                            fast_load_with_unique_keys) == GS_ERROR_RANGE) {
            log_warning("Flight planner list is full.");
            break;
        }
    }

    // Unlock the write mutex
    fp_write_unlock();

    fclose(fd);

    return GS_OK;
}
