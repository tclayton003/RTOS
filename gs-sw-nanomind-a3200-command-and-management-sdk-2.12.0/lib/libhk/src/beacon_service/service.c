/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#define GS_HK_INTERNAL_USE 1
#include <gs/hk/internal/types.h>
#include <gs/hk/param/host/hkcfg.h>
#include <gs/hk/config.h>
#include <gs/hk/internal/internal_config.h>
#include <beacon_service/beacon_file.h>
#include <beacon_service/beacon.h>
#include <gs/csp/port.h>
#include <gs/util/clock.h>
#include <gs/util/byteorder.h>
#include <gs/util/log.h>
#include <gs/util/thread.h>
#include <gs/param/host_table.h>

#if defined(GS_LOG_CAT_HK)
#define LOG_CATEGORY GS_LOG_CAT_HK
#else
#define LOG_CATEGORY GS_LOG_CAT_DEFAULT
#endif

GS_LOG_GROUP(hksrv, "hk_service", LOG_CATEGORY, LOG_DEFAULT_MASK | LOG_INFO_MASK);
#define LOG_DEFAULT hksrv

static void send_beacon_packet(csp_conn_t* conn, FILE* fp,
                               gs_hk_beacon_cfg_t* bcn, csp_packet_t* bcn_packet) {
    if (fp == NULL) {
        if (conn == NULL) {
            uint8_t dst_addr = GS_HK_HKCFG_BCN_ATX_DA_GET_NC();
            uint8_t dst_port = GS_HK_HKCFG_BCN_ATX_DP_GET_NC();
            if (dst_addr < 1 || dst_addr > 31)
                dst_addr = 10;
            if (dst_port < 1)
                dst_port = GS_CSP_PORT_HK_BEACON;

            if (csp_sendto(CSP_PRIO_NORM, dst_addr, dst_port, 0,
                           CSP_O_CRC32, bcn_packet, 0) != CSP_ERR_NONE) {
                csp_buffer_free(bcn_packet);
            }
        } else {
            if (!csp_send(conn, bcn_packet, 0)) {
                csp_buffer_free(bcn_packet);
            }
        }
    } else {
        hk_bcn_file_append(fp, bcn->type, bcn_packet->length, bcn_packet->data);
        csp_buffer_free(bcn_packet);
    }
}

gs_error_t hk_service_send(csp_conn_t* conn, gs_hk_beacon_cfg_t* bcn,
                           uint32_t time, FILE* fp)
{
    uint16_t satid = GS_HK_HKCFG_BCN_SATID_GET_NC();
    csp_packet_t* bcn_packet = create_bcn_packet(bcn, time, satid);
    if (bcn_packet != NULL) {
        log_debug("sending beacon, type=%d, version=%d, length=%i",
                  bcn->type, bcn->version, bcn_packet->length);
        send_beacon_packet(conn, fp, bcn, bcn_packet);
    } else {
        log_debug("failed to created beacon packet, type=%d, version=%d",
                  bcn->type, bcn->version);
    }
    return GS_OK;
}


static void handle_housekeeping_request(csp_packet_t* packet)
{
    gs_hk_housekeeping_request_t* req =
            (gs_hk_housekeeping_request_t*)packet->data;

    /* Fix endianness */
    req->count = util_ntoh32(req->count);
    req->interval = util_ntoh32(req->interval);
    req->t0 = util_ntoh32(req->t0);

    /* Check protocol version */
    if (req->p_version != GS_HK_REQUEST_PROTOCOL_V1) {
        log_warning("will not handle request with unknown protocol "
                    "version, version=0x%02x", req->p_version);
        return;
    }

    // Note: if req->path is set, a file will be created in the call to hk_bcn_file_open.
    // Otherwise, fp will be returned as NULL.
    // fp is passed through hk_service_send and further to send_beacon_packet
    // where it is decided whether the beacon should be sent trough CSP or written
    // to storage.
    FILE* fp = hk_bcn_file_open(req->path, 1);

        /* Calculate t0 if not set */
    if (req->t0 == 0) {
            timestamp_t time;
            gs_clock_get_time(&time);
            req->t0 = time.tv_sec;
    }

    gs_hk_config_handle_t cfg_hdl;
    gs_error_t res = gs_hk_config_open(&cfg_hdl);
    if (res == GS_OK) {
        gs_hk_beacon_cfg_t* bcn;
        res = gs_hk_config_get_beacon(cfg_hdl, req->type, &bcn);
        if (res == GS_OK && bcn != NULL) {

            /* send away...*/
            log_info("Request, type: %"PRIu8", interval: %"PRIu32", count: %"PRIu32", t0: %"PRIu32", path: %s",
                     req->type, req->interval, req->count, req->t0, req->path);

            uint32_t t = req->t0;
            for (unsigned int i = 0; i < req->count; i++) {
                if (hk_service_send(NULL, bcn, t, fp) != GS_OK)
                    break;

                // decrement time
                t = t - req->interval;
            }
        } else {
            log_warning("trying to send beacon that was not found, aborting, "
                        "type=%i, res=%s", req->type, gs_error_string(res));
        }
        gs_hk_config_close(cfg_hdl);
    } else {
        log_warning("gs_hk_config_open() failed, res=%s",
                    gs_error_string(res));
    }

    hk_bcn_file_close(fp);
    return;
}

static void handle_reload_request(csp_packet_t* packet)
{
    gs_hk_reload_request_t* req =
            (gs_hk_reload_request_t*)packet->data;
    if (req->magic == GS_HK_RELOAD_MAGIC) {
        gs_hk_config_reload();
    } else {
        log_warning("got reload_request_t with invalid magic, ignoring, "
                    "magic=%i", req->magic);
    }
}


static void * hk_service(void * param)
{
    // create socket
    csp_socket_t* sock = csp_socket(0);

    // bind every port to socket
    csp_bind(sock, GS_CSP_PORT_HK);

    // create 10 connections backlog queue
    csp_listen(sock, 10);

    // pointer to current connection and packet
    csp_conn_t* conn;
    csp_packet_t* packet;

    // process incoming connections
    while (1) {
        // wait for connection
        if ((conn = csp_accept(sock, CSP_MAX_DELAY)) == NULL) {
            continue;
        }

        // read request packet
        packet = csp_read(conn, 0);
        if (packet == NULL) {
            csp_close(conn);
            continue;
        }
        uint8_t* request_type = (uint8_t*)packet->data;

        // handle request packet
        if (*request_type <= GS_HK_REQUEST_PROTOCOL_MAX) {
            if (gs_hk_config_is_loaded()) {
                handle_housekeeping_request(packet);
            } else {
                log_info("configuration not loaded, ignoring beacon request");
            }
        } else if (*request_type == GS_HK_RELOAD_REQUEST) {
            handle_reload_request(packet);
        } else {
            log_warning("got request with unknown type, ignoring, type=%i",
                        *request_type);
        }

        csp_buffer_free(packet);
        csp_close(conn);
    }

    gs_thread_exit(NULL);
}

gs_error_t gs_hk_create_beacon_service_thread(void)
{
    gs_log_group_register(LOG_DEFAULT);
    gs_hk_beacon_init();

    bool enabled = false;
    GS_HK_HKCFG_HK_EN_GET(&enabled);
    if (enabled) {
        return gs_thread_create("HKSRV", hk_service, NULL, 16000, GS_THREAD_PRIORITY_LOW, 0, NULL);
    } else {
        log_info("Housekeeping disabled, not starting beacon service");
        return GS_OK;
    }
}
