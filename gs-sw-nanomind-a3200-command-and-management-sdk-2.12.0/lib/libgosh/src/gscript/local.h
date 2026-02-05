#ifndef GS_SRC_GSCRIPT_GSCRIPT_LOCAL_H
#define GS_SRC_GSCRIPT_GSCRIPT_LOCAL_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/gosh/gscript/gscript.h>

#define GSCRIPT_CMD_LENGTH 100
#define GSCRIPT_MAX_LINES  100

typedef struct {
    //! please_stop value when task created (ensure we catch stop signal during fast stop/start).
    uint16_t please_stop;
    //! true if command is actually a filename
    bool is_file;
    //! when/start time for executing
    uint32_t when_utc;
    //! Allocated buffer - must be freed, use gs_gscript_free_job()
    char * file_content;
    //! script command or filename - allocated as part of the entire job
    char script[0];
} gs_gscript_task_job_t;

GS_STATIC gs_error_t gs_gscript_task_execute(gs_gscript_task_job_t * job);
GS_STATIC gs_gscript_task_job_t * gs_gscript_alloc_job(const char * script, size_t script_length, bool is_file, uint32_t when_utc);
GS_STATIC void gs_gscript_free_job(gs_gscript_task_job_t * job);

#endif
