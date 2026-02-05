/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#define GS_GOSH_GSCRIPT_INTERNAL_USE 1

#include <gs/gosh/gscript/gscript.h>
#include <gs/gosh/internal/gscript.h>
#include <csp/csp_endian.h>
#include <gs/util/gosh/command.h>
#include <gs/util/thread.h>
#include <gs/csp/csp.h>

void gscript_service_handler(csp_conn_t * conn, csp_packet_t * packet)
{
    gs_gscript_req_type_t type = packet->data[0];

    switch(type) {
	case GS_GSCRIPT_REQ_TYPE_EXEC:
	case GS_GSCRIPT_REQ_TYPE_EXEC_FILE:
            {
                gs_gscript_req_exec_t * req = (void *) packet->data;
                req->priority = csp_ntoh16(req->priority);
                req->stack = csp_ntoh16(req->stack);
                req->when = csp_ntoh32(req->when);

                gs_gscript_exec(req->cmd, packet->length - sizeof(*req),
                                (type == GS_GSCRIPT_REQ_TYPE_EXEC_FILE),
                                req->when,
                                gs_command_get_stack_size(), GS_THREAD_PRIORITY_LOW);
            }
            break;

	case GS_GSCRIPT_REQ_TYPE_STOP:
            gs_gscript_stop();
            break;
    }
    csp_buffer_free(packet);
}

gs_error_t gs_gscript_csp_service_handler(csp_conn_t * conn)
{
    csp_packet_t *packet;
    while ((packet = csp_read(conn, 0))) {
        gscript_service_handler(conn, packet);
    }
    csp_close(conn);
    return GS_OK;
}
