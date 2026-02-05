/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Flight planner server
*/

#include "fp.h"
#include "fp_list.h"
#include "fp_timer.h"
#include "fp_server.h"

#define GS_FP_INTERNAL_USE 1
#include <gs/fp/internal/types.h>

#include <gs/util/error.h>
#include <gs/util/stdio.h>
#include <gs/util/string.h>
#include <gs/util/log.h>
#include <gs/util/thread.h>
#include <gs/csp/error.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <gs/csp/csp.h>
#include <gs/csp/service_dispatcher.h>

static csp_conn_t * conn;

static gs_error_t fp_timer_iterator(int pos, char * key, void * entry, void * ret, size_t retsize)
{
    fp_server_packet_t tmp;
    fp_timer_t *timer = (fp_timer_t *)entry;

    csp_packet_t *packet = csp_buffer_get(FP_SERVER_PACKET_SIZE(fp_server_timer_list_t));
    if (!packet) {
        return GS_ERROR_ALLOC;
    }

    tmp.type = FP_TYPE_REPLY;
    tmp.cmd = FP_CMD_TIMER_LIST;
    tmp.reply = FP_REPLY_OK;
    tmp.pad = 0;

    strncpy(tmp.timer.list.timer, key, FP_KEY_SIZE);
    strncpy(tmp.timer.list.cmd, timer->cmd, FP_CMD_SIZE);
    tmp.timer.list.total = csp_hton16(fp_timer_count());
    tmp.timer.list.seq = csp_hton16(pos + 1);
    tmp.timer.list.state = timer->state;
    tmp.timer.list.basis = timer->basis;
    tmp.timer.list.repeat = csp_hton16(timer->repeat);
    tmp.timer.list.remain = csp_hton16(timer->remain);
    tmp.timer.list.when.tv_sec = csp_hton32(timer->when.tv_sec);
    tmp.timer.list.when.tv_nsec = csp_hton32(timer->when.tv_nsec);
    tmp.timer.list.last.tv_sec = 0;
    tmp.timer.list.last.tv_nsec = 0;

    memcpy(packet->data, &tmp, sizeof(tmp));
    packet->length = FP_SERVER_PACKET_SIZE(fp_server_timer_list_t);

    if (!csp_send(conn, packet, 1000)) {
        csp_buffer_free(packet);
        return GS_ERROR_IO;
    }

    return GS_OK;
}

static void fp_process_packet(csp_packet_t * packet)
{
    int ret = -1;
    int silent = 0;
    uint16_t received_length = packet->length;

    /* Assume standard reply */
    packet->length = FP_SERVER_PACKET_EMPTY;

    /* Copy packet data to avoid alignment issues */
    fp_server_packet_t tmp;
    memcpy(&tmp, packet->data, sizeof(tmp));
    switch (tmp.cmd) {
        /* General FP commands */
        case FP_CMD_STORE_FP:
            ret = fp_store_save(tmp.fp.store.path);
            break;
        case FP_CMD_LOAD_FP:
            if (received_length == FP_SERVER_PACKET_SIZE(fp_server_load_fp_t)) {
                ret = fp_store_load(tmp.fp.load.path, tmp.fp.load.fast_load_with_unique_keys);
            } else {
                ret = fp_store_load(tmp.fp.load.path, false);
            }
            break;
        case FP_CMD_FLUSH_FP:
            ret = fp_flush();
            break;
            /* Timer commands */
        case FP_CMD_TIMER_CREATE:
            tmp.timer.create.when.tv_sec = csp_ntoh32(tmp.timer.create.when.tv_sec);
            tmp.timer.create.when.tv_nsec = csp_ntoh32(tmp.timer.create.when.tv_nsec);
            ret = fp_timer_create(tmp.timer.create.timer,
                                  tmp.timer.create.cmd,
                                  tmp.timer.create.basis,
                                  tmp.timer.create.state,
                                  &tmp.timer.create.when,
                                  csp_ntoh16(tmp.timer.create.repeat),
                                  false);
            break;
        case FP_CMD_TIMER_DELETE:
            ret = fp_timer_delete(tmp.timer.delete.timer);
            break;
        case FP_CMD_TIMER_SET_ACTIVE:
        {
            fp_timer_t * t = fp_timer_search(tmp.timer.active.timer);
            if (t == NULL) {
                log_warning("Unknown timer: %s", tmp.timer.active.timer);
                ret = GS_ERROR_NO_DATA;
            }
            else {
                ret = fp_timer_set_active(t);
            }
            break;
        }
        case FP_CMD_TIMER_SET_ALL_ACTIVE:
            ret = fp_timer_set_all_active();
            break;
        case FP_CMD_TIMER_SET_DORMANT:
            ret = fp_timer_set_dormant(tmp.timer.dormant.timer);
            break;
        case FP_CMD_TIMER_SET_REPEAT:
        {
            fp_timer_t * t = fp_timer_search(tmp.timer.repeat.timer);
            if (t == NULL) {
                log_warning("Unknown timer: %s", tmp.timer.repeat.timer);
                ret = GS_ERROR_NO_DATA;
            }
            else {
                ret = fp_timer_set_repeat(t, csp_ntoh16(tmp.timer.repeat.repeat));
            }
            break;
        }
        case FP_CMD_TIMER_SET_TIME:
        {
            gs_timestamp_t when;
            when.tv_sec = csp_ntoh32(tmp.timer.time.when.tv_sec);
            when.tv_nsec = csp_ntoh32(tmp.timer.time.when.tv_nsec);
            ret = fp_timer_set_time(tmp.timer.time.timer, tmp.timer.time.basis, &when);
            break;
        }
        case FP_CMD_TIMER_LIST:
            if (fp_timer_count() > 0) {
                fp_timer_iterate(fp_timer_iterator, NULL, 0);
                silent = 1;
            } else {
                ret = 0;
                tmp.cmd = FP_CMD_TIMER_LIST;
                tmp.timer.list.total = csp_hton16(0);
                packet->length = FP_SERVER_PACKET_SIZE(fp_server_timer_list_t);
            }
            break;
        default:
            log_warning("Unknown command: %u", tmp.cmd);
            silent = 1;
            break;
    }

    /* Build reply unless silenced */
    if (!silent) {
        tmp.type = FP_TYPE_REPLY;
        tmp.reply = ret ? FP_REPLY_INVAL : FP_REPLY_OK;
        tmp.pad = 0;

        /* Copy reply to packet */
        memcpy(packet->data, &tmp, sizeof(tmp));

        /* Packet length has been determined above */
        if (!csp_send(conn, packet, 100))
            csp_buffer_free(packet);
    } else {
        csp_buffer_free(packet);
    }
}

static gs_error_t gs_fp_service_handler(csp_conn_t * connection)
{
    if (connection) {
        conn = connection;
        csp_packet_t * packet = NULL;
        while ((packet = csp_read(connection, GS_CSP_TIMEOUT))) {
            if (packet != NULL) {
                fp_process_packet(packet);

                csp_close(conn);
            }
        }
    }

    return GS_OK;
}

static gs_csp_service_dispatcher_t dispatcher;
gs_error_t gs_fp_service_dispatcher_start()
{
    // Create service handler
    static const gs_csp_service_handler_t handlers[30] = {
        [GS_CSP_PORT_FP] = gs_fp_service_handler
    };

    // Configuration for service handler - must remain valid as long service handler is running
    static const gs_csp_service_dispatcher_conf_t conf = {
        .name = "FP",
        .handler_array = handlers,
        .handler_array_size = GS_ARRAY_SIZE(handlers),
        .disable_watchdog = true
    };

    return gs_csp_service_dispatcher_create(&conf, 10000, GS_THREAD_PRIORITY_LOW, &dispatcher);
}

static bool server_run = false;
static void * fp_server_task(void * param)
{
    csp_socket_t * socket = param;
    while (server_run) {
        if ((conn = csp_accept(socket, 10000)) == NULL) {
            continue;
        }

        gs_fp_service_handler(conn);
    }

    gs_thread_exit(NULL);
}

gs_error_t gs_fp_service_stop(void)
{
    return gs_csp_service_dispatcher_destroy(dispatcher);
}

static gs_thread_t server_thread;
gs_error_t fp_server_start(uint8_t port)
{
    csp_socket_t * socket = csp_socket(CSP_SO_NONE);
    if (!socket) {
        log_error("Failed to create socket for FP server task");
        return GS_ERROR_ALLOC;
    }

    gs_error_t error = gs_csp_error(csp_bind(socket, port));
    if (error) {
        log_error("Failed to bind to FP server port %d, error: %s", port, gs_error_string(error));
        return error;
    }

    error = gs_csp_error(csp_listen(socket, 10));
    if (error) {
        log_error("Failed to listen on FP server socket, error: %s", gs_error_string(error));
        return error;
    }

    server_run = true;
    error = gs_thread_create("FPSRV", fp_server_task, socket, 10000, GS_THREAD_PRIORITY_LOW, GS_THREAD_CREATE_JOINABLE, &server_thread);
    if (error) {
        log_error("Failed to start FP server task, error: %s", gs_error_string(error));
    }

    return error;
}

gs_error_t fp_server_stop(void)
{
    server_run = false;
    return gs_thread_join(server_thread, NULL);
}
