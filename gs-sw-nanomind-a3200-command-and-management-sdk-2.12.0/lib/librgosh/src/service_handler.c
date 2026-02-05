/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/rgosh/service_handler.h>
#include <gs/rgosh/rgosh.h>
#include <gs/util/gosh/command.h>
#include <gs/rgosh/rgosh_exec.h>
#include <gs/rgosh/internal/serialize.h>
#include <gs/rgosh/internal/rgosh_log.h>
#include <proto/gosh.pb.h>
#include <pb_decode.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <csp/arch/csp_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "local.h"


void gs_rgosh_free_job(gs_rgosh_job_t *job)
{
    if (job) {
        if (job->std_out) {
            fclose(job->std_out);
        }
        if (job->conn) {
            csp_close(job->conn);
        }
        free(job);
    }
}

static gs_error_t gs_rgosh_run_handler(csp_conn_t * conn, const char* command, uint32_t id)
{
    gs_rgosh_job_t *job = calloc(1, sizeof(*job));
    if (job == NULL) {
        return GS_ERROR_ALLOC;
    }

    job->conn = conn;
    job->response.id = id;
    job->std_out = gs_memstream_open_static(job->response.response.command.std_out, sizeof(job->response.response.command.std_out),
                                            &job->memstream);
    job->response.response.command.has_std_out = true;
    GS_STRNCPY(job->command, command);

    gs_error_t error = gs_rgosh_exec_job(job);
    if (error) {
        job->conn = NULL; // will be freed else where
        gs_rgosh_free_job(job);
    }

    return error;
}

static gs_error_t gs_rgosh_process_packet(csp_conn_t * conn, void* packet, int packet_size)
{
    GoshRequest request;
    gs_error_t error = gs_rgosh_deserialize_request(packet, packet_size, &request);
    if (error == GS_OK) {
        if (request.which_command == GoshRequest_run_tag) {
            error = gs_rgosh_run_handler(conn, request.command.run.command, request.id);
        } else {
            error = GS_ERROR_NOT_IMPLEMENTED;
        }
    }

    return error;
}

gs_error_t gs_rgosh_service_handler(csp_conn_t * conn)
{
    if (conn) {
        void* packet_data = NULL; /* SFP will allocate necessary memory for incoming packet. */
        int packet_size = 0;

        /* With current implementation, we can only handle 1 command per connection, as the "job executor" will
           close connection when complete.
           Timeout should match RDP connection timeout, in order to compensate for retransmission of the actual
           command (automatically handled by libcsp 1.6) - BUT too large timeout causes the thread to hang. see
           improvement: LBRGSH-17 */
        int ret = csp_sfp_recv(conn, &packet_data, &packet_size, 3000);
        if (ret == 0) {
            ret = gs_rgosh_process_packet(conn, packet_data, packet_size);
            if (ret == GS_OK) {
                conn = NULL; // freed by executor on completion
            } else {
                log_warning("Server: failed to decode/queue command, size: %d, source: %u:%u, error: %d",
                            packet_size, csp_conn_src(conn), csp_conn_sport(conn), ret);
            }
        }

        if (conn) {
            csp_close(conn);
        }

        csp_free(packet_data);
    }
    return GS_OK;
}
