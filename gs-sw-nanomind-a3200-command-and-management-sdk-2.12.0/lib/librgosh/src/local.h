/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/csp/csp.h>
#include <proto/gosh.pb.h>

#include "memstream.h"

typedef struct gs_rgosh_job {
    // Incoming connection
    csp_conn_t* conn;
    char command[128];
    FILE* std_out;
    gs_error_t command_return;
    GoshResponse response;
    gs_memstream_handle_t memstream;
} gs_rgosh_job_t;

gs_error_t gs_rgosh_exec_job(gs_rgosh_job_t *job);
void gs_rgosh_free_job(gs_rgosh_job_t *job);
