/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "local.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <gs/util/log.h>
#include <gs/util/clock.h>
#include <gs/util/string.h>
#include <gs/util/time.h>
#include <gs/util/thread.h>
#include <gs/util/check.h>
#include <gs/util/gosh/command.h>

static GS_LOG_GROUP(gs_gscript_log, "gscript", GS_LOG_CAT_DEFAULT, LOG_DEFAULT_MASK);
#define LOG_DEFAULT gs_gscript_log

static uint16_t please_stop = 0;

static gs_error_t gscript_parse_line(const char * line, int * delay, int * retry, int * err_goto, bool * comment, char * cmd)
{
    for (; line && (*line == ' '); ++line);
    if ((line == NULL) || (line[0] == 0) || (*line == '#')) {
        *comment = true;
        return GS_OK;
    }
    *comment = false;

    int res = sscanf(line, "%d %d %d %" GS_DEF2STRING(GSCRIPT_CMD_LENGTH) "[^\n]", delay, retry, err_goto, cmd);

    if (res != 4) {
        return GS_ERROR_DATA;
    }
    if (*delay < 0) {
        return GS_ERROR_DATA;
    }
    if (*retry < 0) {
        return GS_ERROR_DATA;
    }
    if (*retry == 0) {
        *retry = 1;
    }
    if (*err_goto < 0) {
        return GS_ERROR_DATA;
    }
    if (cmd[0] == 0) {
        return GS_ERROR_DATA;
    }
    return GS_OK;
}

#define CHECK_STOP_RETURN(job)   if(job->please_stop != please_stop) {return GS_ERROR_INTR;}

static gs_error_t gscript_execute_job(gs_gscript_task_job_t * job)
{
    if (job == NULL) {
        return GS_ERROR_ARG;
    }
    
    CHECK_STOP_RETURN(job);

    if (job->is_file) {

        /* Open file */
        FILE * fd = fopen(job->script, "r");
        if (fd == NULL) {
            log_error("Could not open file [%s]", job->script);
            return GS_ERROR_NOT_FOUND;
        }

        /* Get size */
        struct stat file_stat;
        if (fstat(fileno(fd), &file_stat) != 0) {
            fclose(fd);
            log_error("Could not get file size [%s]", job->script);
            return GS_ERROR_IO;
        }

        log_info("Opened file %s size %u", job->script, (unsigned int) file_stat.st_size);

        const size_t alloc_size = (file_stat.st_size + 1);
        job->file_content = malloc(alloc_size);
        if (fread(job->file_content, 1, file_stat.st_size, fd) != (size_t) file_stat.st_size) {
            fclose(fd);
            log_error("Could not read full file [%s]", job->script);
            return GS_ERROR_IO;
        }
        job->file_content[alloc_size - 1] = 0;

        fclose(fd);
    }

    CHECK_STOP_RETURN(job);

    char * lines[GSCRIPT_MAX_LINES + 1];
    memset(lines, 0, sizeof(lines));
    int lines_count = 0;

    int delay;
    int retry;
    int err_goto;
    bool comment;
    char cmd[GSCRIPT_CMD_LENGTH + 1];

    // Check, split (and pre-parse) lines
    {
        char * next_line = (job->is_file) ? job->file_content : job->script;
        do {
            char * line = next_line;
            next_line = strchr(next_line, '\n'); // strtok_r skips empty lines, causing un-expected line numbers
            if (next_line) {
                *next_line++ = 0;
            }
            gs_error_t error = gscript_parse_line(line, &delay, &retry, &err_goto, &comment, cmd);
            if (error) {
                return error;
            }
            lines[lines_count++] = line;
        } while(next_line && (lines_count < GSCRIPT_MAX_LINES));

        if ((lines_count == 0) || (lines[0] == NULL)) {
            return GS_ERROR_NO_DATA;
        }
        if (lines_count >= GSCRIPT_MAX_LINES) {
            return GS_ERROR_FULL;
        }
    }

    CHECK_STOP_RETURN(job);

    // Check when/start time
    if (job->when_utc > 0) {
        timestamp_t ts;
        ts.tv_sec = job->when_utc;
        ts.tv_nsec = 0;
        char when[25];
        gs_clock_to_iso8601_string(&ts, when, sizeof(when));

        gs_clock_get_time(&ts);
        char now[25];
        gs_clock_to_iso8601_string(&ts, now, sizeof(now));

        if ((job->when_utc < ts.tv_sec) || (job->when_utc >= (ts.tv_sec + GS_TIME_DAYS_TO_SECS(8)))) {
            log_info("Invalid when/start time range: now: %s, when: %s", now, when);
            return GS_ERROR_RANGE;
        }

        log_info("Waiting until %"PRIu32" sec -> %s, now: %s", job->when_utc, when, now);
        while(ts.tv_sec < job->when_utc) {
            CHECK_STOP_RETURN(job);
            gs_time_sleep_ms(1000);
            gs_clock_get_time(&ts);
        }
    }

    CHECK_STOP_RETURN(job);

    // Execute
    int current_line = 0;
    while(current_line < lines_count) {

        gs_error_t error = gscript_parse_line(lines[current_line], &delay, &retry, &err_goto, &comment, cmd);
        if (error) {
            return error;
        }
        int result = GS_OK;
        if (comment == false) {
            // Run command - optionally re-run on failure
            int try = 0;
            do {
                CHECK_STOP_RETURN(job);
                
                if (delay > 0) {
                    gs_time_sleep_ms(delay);
                }

                CHECK_STOP_RETURN(job);
                log_debug("Line [%d] %d %d %d %s", current_line + 1, delay, retry - try, err_goto, cmd);
                ++try;
                gs_error_t cmd_result = GS_OK;
                result = gs_command_run(cmd, &cmd_result);
                if (result == GS_OK) {
                    result = cmd_result;
                }
            } while ((result != GS_OK) && (try < retry));
        }

        // Goto error line on error - otherwise goto next line
        if ((result != GS_OK) && (err_goto > 0)) {
            current_line = err_goto - 1;
        } else {
            current_line++;
        }
    }

    return GS_OK;
}

GS_STATIC gs_error_t gs_gscript_task_execute(gs_gscript_task_job_t * job)
{
    log_trace("gscript task started");

    gs_error_t error = gscript_execute_job(job);

    gs_gscript_free_job(job);

    log_trace("gscript task stopped, error=%s", gs_error_string(error));

    return error;
}

static void * gscript_task(void * arg)
{
    gs_error_t error = gs_gscript_task_execute(arg);
    gs_thread_exit(GS_TYPES_INT2PTR(abs(error)));
}

GS_STATIC gs_gscript_task_job_t * gs_gscript_alloc_job(const char * script, size_t script_length, bool is_file, uint32_t when_utc)
{
    gs_gscript_task_job_t * job = malloc(sizeof(*job) + script_length + 1);;
    if (job) {
        job->please_stop = please_stop;
        job->is_file = is_file;
        job->when_utc = when_utc;
        job->file_content = NULL;
        strncpy(job->script, script, script_length);
        job->script[script_length] = 0;
    }
    return job;
}

GS_STATIC void gs_gscript_free_job(gs_gscript_task_job_t * job)
{
    if (job) {
        free(job->file_content);
        free(job);
    }
}

gs_error_t gs_gscript_init(void)
{
    return gs_log_group_register(LOG_DEFAULT);
}

gs_error_t gs_gscript_exec(const char * script, size_t script_length, bool is_file, uint32_t when_utc, size_t stack_size, int priority)
{
    GS_CHECK_ARG((script != NULL) && (script[0] != 0));
    const size_t MAX_LENGTH = 1000; // because of malloc
    if (script_length == 0) {
        script_length = strnlen(script, MAX_LENGTH);
    }
    GS_CHECK_ARG((script_length > 0) && (script_length < MAX_LENGTH));

    gs_gscript_task_job_t * job = gs_gscript_alloc_job(script, script_length, is_file, when_utc);
    if (job == NULL) {
        return GS_ERROR_ALLOC;
    }

    gs_error_t error = gs_thread_create("GSCRIPT", gscript_task, job, stack_size, priority, 0, NULL);
    if (error) {
        gs_gscript_free_job(job);
    }

    return error;
}

void gs_gscript_stop(void)
{
    ++please_stop;
}
