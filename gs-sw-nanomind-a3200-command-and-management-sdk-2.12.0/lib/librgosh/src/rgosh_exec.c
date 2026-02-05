/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/rgosh/rgosh_exec.h>
#include <gs/rgosh/rgosh.h>
#include <gs/rgosh/internal/serialize.h>
#include <gs/rgosh/internal/rgosh_log.h>
#include <gs/rgosh/service_handler.h>
#include <gs/util/types.h>
#include <gs/util/log.h>
#include <gs/util/gosh/command.h>
#include <gs/util/queue.h>
#include <gs/util/thread.h>
#include <proto/gosh.pb.h>
#include <pb_decode.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <stdio.h>
#include <stdlib.h>
#include "local.h"

#define GS_RGOSH_THREADS_MAX    2

typedef struct {
    uint8_t id;
    bool run;
    gs_queue_t job_queue;
    gs_thread_t thread;
    gs_rgosh_job_t *job;
} worker_thread_t;

static worker_thread_t g_worker_threads[GS_RGOSH_THREADS_MAX];

static gs_error_t gs_rgosh_send_response(gs_rgosh_job_t* job, bool completion)
{
    GoshResponse* r = &job->response;
    r->which_response = GoshResponse_command_tag;

    r->response.command.completion_flag = completion;

    r->response.command.return_code = job->command_return;

    fflush(job->std_out); /* Ensure data is committed to file */

    // We need to insert a zero termination at the end of the buffer, because memstream doesn't ensure 0 termination
    size_t cnt = ftell(job->std_out);
    if (cnt > (sizeof(job->response.response.command.std_out)-1)) {
        cnt = (sizeof(job->response.response.command.std_out)-1);
    }
    job->response.response.command.std_out[cnt] = '\0';

    uint8_t msg_buf[GoshResponse_size];
    uint32_t msg_size = GoshResponse_size;
    if (gs_rgosh_serialize_response(r, msg_buf, &msg_size) != GS_OK) {
        return GS_ERROR_DATA;
    }

    log_debug("Server: Created response with size: <%"PRIu32">", msg_size);

    if (csp_sfp_send(job->conn, msg_buf, msg_size, GS_RGOSH_MTU_SIZE, 1000) != 0) {
        return GS_ERROR_IO;
    }

    return GS_OK;
}

static gs_error_t command_rgosh_set_result(gs_command_context_t *ctx, const char *group, const char *key, const char *value)
{
    if (ctx == NULL || ctx->io_ctx == NULL) {
        return GS_ERROR_ARG;
    }

    gs_rgosh_job_t* job = (gs_rgosh_job_t*)ctx->io_ctx;
    CommandResponse *response = &job->response.response.command;
    if (response->results_count >= GS_ARRAY_SIZE(response->results))
    {
        gs_rgosh_send_response(job, false);
        /* Clear the response and prepare for new results */
        response->results_count = 0;
        response->std_out[0] = '\0';
        response->return_code = GS_OK;
    }

    ResultTuple *result = &response->results[response->results_count];
    response->results_count += 1;
    if (group) {
        if (strcmp("", group) != 0) {
            strncpy(result->group, group, sizeof(result->group));
            result->has_group = true;
        }
    }
    if (key) {
        strncpy(result->key, key, sizeof(result->key));
    }
    if (value) {
        strncpy(result->value, value, sizeof(result->value));
    }
    return GS_OK;
}

static gs_error_t command_rgosh_flush(gs_command_context_t *ctx)
{
    if (ctx == NULL || ctx->io_ctx == NULL) {
        return GS_ERROR_ARG;
    }

    gs_rgosh_job_t* job = (gs_rgosh_job_t*)ctx->io_ctx;
    fflush(job->std_out);
    return GS_ERROR_NOT_IMPLEMENTED;
}

static gs_error_t command_rgosh_wait_for_key(gs_command_context_t *ctx, int *ch, int timeout_ms)
{
    if (ctx == NULL || ch == NULL) {
        return GS_ERROR_ARG;
    }

    return GS_ERROR_HANDLE;
}

static const gs_command_io_functions_t rgosh_io_functions = {
    .set_result   = command_rgosh_set_result,
    .flush        = command_rgosh_flush,
    .wait_for_key = command_rgosh_wait_for_key
};

static void * gs_rgosh_worker_thread(void * param)
{
    worker_thread_t * wt = param;

    while (wt->run) {
        if (gs_queue_dequeue(wt->job_queue, 1000, &wt->job) == GS_OK) {
            gs_rgosh_job_t *job = wt->job;
            log_debug("EX[%u] new job: %p, q-pending: %u", wt->id, job, gs_queue_size(wt->job_queue));

            job->command_return = GS_OK;
            gs_error_t error = gs_command_execute(job->command, &job->command_return,
                                                  job->std_out, &rgosh_io_functions, job);
            log_info("EX[%u] Command <%s>, error: framework %d, command %d", wt->id, job->command, error, job->command_return);
            if (error != GS_OK) {
                job->command_return = error;
            }

            error = gs_rgosh_send_response(job, true);
            log_debug("EX[%u] finished job: %p, send_resp.error: %d", wt->id, job, error);

            gs_rgosh_free_job(job);
            wt->job = NULL;
        }
    }
    log_debug("EX[%u]: terminating ...", wt->id);
    gs_thread_exit(NULL);
}

gs_error_t gs_rgosh_exec_engine_start2(uint8_t threads, uint32_t stack_size, uint8_t queue_length)
{
    gs_rgosh_init(); // register log group

    if (threads > GS_RGOSH_THREADS_MAX) {
        log_error("Too many RGOSH threads requested. At most: <%d> threads supported.", GS_RGOSH_THREADS_MAX);
        return GS_ERROR_ARG;
    }

    if (stack_size == 0) {
        stack_size = gs_command_get_stack_size();
    }

    for (uint8_t i = 0; i < threads; i++) {
        worker_thread_t * wt = &g_worker_threads[i];
        wt->id = i;
        wt->run = true;

        char exec_name[20];
        snprintf(exec_name, sizeof(exec_name), "RGOSH_EX_%u", i);

        gs_error_t error = gs_queue_create(queue_length, sizeof(gs_rgosh_job_t*), &wt->job_queue);
        if (error) {
            log_error("Failed creating EX[%u] queue, error: %d", i, error);
            return error;
        }

        error = gs_thread_create(exec_name, gs_rgosh_worker_thread, wt, stack_size,
                                 GS_THREAD_PRIORITY_NORMAL, GS_THREAD_CREATE_JOINABLE, &wt->thread);
        if (error) {
            log_error("Failed creating EX[%u] thread, error: %d", i, error);
            return error;
        }
    }
    return GS_OK;
}

gs_error_t gs_rgosh_exec_engine_stop()
{
    for (unsigned int i = 0; i < GS_RGOSH_THREADS_MAX; i++) {
        worker_thread_t * wt = &g_worker_threads[i];

        wt->run = false;
        if (wt->thread) {
            gs_thread_join(wt->thread, NULL);
        }
        if (wt->job_queue) {
            gs_rgosh_job_t *job;
            while (gs_queue_dequeue(wt->job_queue, 0, &job) == GS_OK) {
                gs_rgosh_free_job(job);
            }
            gs_queue_destroy(wt->job_queue);
        }
    }

    // clear data, making valgrind reveal unfreed stuff
    memset(g_worker_threads, 0, sizeof(g_worker_threads));

    return GS_OK;
}

static inline unsigned int queue_size(worker_thread_t * wt)
{
    return (gs_queue_size(wt->job_queue) + (wt->job ? 1 : 0));
}

gs_error_t gs_rgosh_exec_job(gs_rgosh_job_t *job)
{
    worker_thread_t * wt = NULL;
    for (unsigned int i = 0; i < GS_RGOSH_THREADS_MAX; i++) {
        if (g_worker_threads[i].job_queue) {
            if ((wt == NULL) || (queue_size(&g_worker_threads[i]) < queue_size(wt))) {
                wt = &g_worker_threads[i];
            }
        }
    }

    if (wt && (gs_queue_enqueue(wt->job_queue, &job, 1000) == GS_OK)) {
        return GS_OK;
    }

    return GS_ERROR_FULL;
}
