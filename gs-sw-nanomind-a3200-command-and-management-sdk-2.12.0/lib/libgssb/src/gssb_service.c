/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/gssb/gssb_all_devices.h>
#include <gs/gssb/rgssb.h>
#include <stdio.h>
#include <stdint.h>
#include <gs/util/thread.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <gs/util/drivers/i2c/master.h>
#include <gs/util/log.h>
#include <conf_gssb.h>
#define GS_GSSB_INTERNAL_USE 1
#include <gs/gssb/internal/gssb_common.h>
#include <gs/gssb/internal/rgssb.h>
#include <gs/gssb/internal/rgssb_cmd_id.h>

void gs_rgssb_cmd_parser(csp_conn_t * conn, csp_packet_t * packet)
{
    gs_error_t i2c_res = 0;

    gs_rgssb_cmd_t * cmd;
    cmd = (gs_rgssb_cmd_t *) packet->data;
    cmd->i2c_timeout_ms = csp_ntoh16(cmd->i2c_timeout_ms);
    cmd->cmd_id = csp_ntoh16(cmd->cmd_id);
    gs_rgssb_resp_t * resp;
    resp = (gs_rgssb_resp_t *) packet->data;

    if(cmd->prot_version != GS_RGSSB_PROTOCOL_VERSION) {
        resp->prot_version = GS_RGSSB_PROTOCOL_VERSION;
        resp->data_length = 0;
        resp->cmd_id = 0;
        resp->i2c_error = GS_OK;
        log_error("%s: client protocol version: %hhu, server protocol version: %hhu",
                  __FUNCTION__, cmd->prot_version, GS_RGSSB_PROTOCOL_VERSION);
    } else {
        switch (cmd->cmd_id) {
            case RGSSB_CMD_REBOOT: {
                i2c_res = gs_gssb_soft_reset(cmd->i2c_addr, cmd->i2c_timeout_ms);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_MODEL: {
                gs_gssb_model_t model;
                i2c_res = gs_gssb_get_model(cmd->i2c_addr, cmd->i2c_timeout_ms, &model);
                resp->data_length = 1;
                resp->data[0] = model;
                break;
            }
            case RGSSB_CMD_IDENT: {
                gs_gssb_ident_t ident;
                gs_gssb_get_version(cmd->i2c_addr, cmd->i2c_timeout_ms, (uint8_t *) &ident.version, 20);
                i2c_res = gs_gssb_get_uuid(cmd->i2c_addr, cmd->i2c_timeout_ms, &ident.uuid);
                resp->data_length = sizeof(ident);
                ident.uuid = csp_hton32(ident.uuid);
                memcpy(resp->data, &ident, sizeof(ident));
                break;
            }
#if defined GS_AR6_ENABLED || GS_ANT6_ENABLED || GS_I4_ENABLED
            case RGSSB_CMD_COMMON_GET_BACKUP_SETTINGS: {
                gs_gssb_backup_settings_t settings;
                i2c_res = gs_gssb_common_get_backup_settings(cmd->i2c_addr, cmd->i2c_timeout_ms, &settings);
                resp->data_length = sizeof(settings);
                settings.minutes = csp_ntoh16(settings.minutes);
                memcpy(resp->data, &settings, sizeof(settings));
                break;
            }
            case RGSSB_CMD_COMMON_SET_BACKUP_SETTINGS: {
                gs_gssb_backup_settings_t * settings;
                settings = (gs_gssb_backup_settings_t *) cmd->data;
                settings->minutes = csp_ntoh16(settings->minutes);
                i2c_res = gs_gssb_common_set_backup_settings(cmd->i2c_addr, cmd->i2c_timeout_ms, *settings);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_COMMON_INTERNAL_TEMP: {
                int16_t temp;
                i2c_res = gs_gssb_common_get_internal_temp(cmd->i2c_addr, cmd->i2c_timeout_ms, &temp);
                resp->data_length = sizeof(temp);
                temp = csp_hton16(temp);
                memcpy(resp->data, &temp, sizeof(temp));
                break;
            }
            case RGSSB_CMD_COMMON_STOP_BURN: {
                i2c_res = gs_gssb_common_stop_burn(cmd->i2c_addr, cmd->i2c_timeout_ms);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_COMMON_RESET_CNT: {
                i2c_res = gs_gssb_common_reset_count(cmd->i2c_addr, cmd->i2c_timeout_ms);
                resp->data_length = 0;
                break;
            }
#endif
#if defined GS_ANT6_ENABLED || GS_I4_ENABLED
            case RGSSB_CMD_COMMON_BURN_CHANNEL: {
                uint8_t channel = cmd->data[0];
                uint8_t burn_seconds = cmd->data[1];
                i2c_res = gs_gssb_common_burn_channel(cmd->i2c_addr, cmd->i2c_timeout_ms, channel, burn_seconds);
                resp->data_length = 0;
                break;
            }
#endif
#ifdef GS_I4_ENABLED
            case RGSSB_CMD_I4_STATUS: {
                gs_gssb_i4_status_t status;
                gs_gssb_i4_get_release_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.release);
                gs_gssb_i4_get_backup_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.backup);
                i2c_res = gs_gssb_i4_get_board_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.board);
                resp->data_length = sizeof(status);
                status.board.seconds_since_boot = csp_ntoh32(status.board.seconds_since_boot);
                status.backup.seconds_to_deploy = csp_ntoh32(status.backup.seconds_to_deploy);
                memcpy(resp->data, &status, sizeof(status));
                break;
            }
#endif
#ifdef GS_ANT6_ENABLED
            case RGSSB_CMD_ANT6_STATUS: {
                gs_gssb_ant6_status_t status;
                gs_gssb_ant6_get_release_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.release);
                gs_gssb_ant6_get_backup_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.backup);
                i2c_res = gs_gssb_ant6_get_board_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.board);
                resp->data_length = sizeof(status);
                status.board.seconds_since_boot = csp_ntoh32(status.board.seconds_since_boot);
                status.backup.seconds_to_deploy = csp_ntoh32(status.backup.seconds_to_deploy);
                memcpy(resp->data, &status, sizeof(status));
                break;
            }
#endif
#ifdef GS_AR6_ENABLED
            case RGSSB_CMD_AR6_STATUS: {
                gs_gssb_ar6_status_t status;
                gs_gssb_ar6_get_release_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.release);
                gs_gssb_ar6_get_backup_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.backup);
                i2c_res = gs_gssb_ar6_get_board_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status.board);
                resp->data_length = sizeof(status);
                status.board.seconds_since_boot = csp_ntoh32(status.board.seconds_since_boot);
                status.backup.seconds_to_deploy = csp_ntoh32(status.backup.seconds_to_deploy);
                memcpy(resp->data, &status, sizeof(status));
                break;
            }
            case RGSSB_CMD_AR6_BURN: {
                uint8_t burn_seconds = cmd->data[0];
                i2c_res = gs_gssb_ar6_burn(cmd->i2c_addr, cmd->i2c_timeout_ms, burn_seconds);
                resp->data_length = 0;
                break;
            }
#endif
#ifdef GS_MSP_ENABLED
            case RGSSB_CMD_MSP_SENSORS: {
                int16_t tmp_i;
                uint16_t tmp_u;
                gs_gssb_msp_sensors_t sensors;
                gs_gssb_common_get_internal_temp(cmd->i2c_addr, cmd->i2c_timeout_ms, &tmp_i);
                sensors.internal_temp = csp_hton16(tmp_i);
                gs_gssb_msp_get_outside_temp(cmd->i2c_addr, cmd->i2c_timeout_ms, &tmp_i);
                sensors.outside_temp = csp_hton16(tmp_i);
                i2c_res = gs_gssb_common_get_sun_voltage(cmd->i2c_addr, cmd->i2c_timeout_ms, &tmp_u);
                sensors.sun_voltage = csp_hton16(tmp_u);
                resp->data_length = sizeof(sensors);
                memcpy(resp->data, &sensors, sizeof(sensors));
                break;
            }
#endif
#ifdef GS_ISTAGE_ENABLED
            case RGSSB_CMD_ISTAGE_STATUS: {
                gs_gssb_istage_status_t status;
                i2c_res = gs_gssb_istage_status(cmd->i2c_addr, cmd->i2c_timeout_ms, &status);
                resp->data_length = sizeof(status);
                status.deploy_in_s = csp_ntoh16(status.deploy_in_s);
                status.total_number_of_deploys = csp_ntoh16(status.total_number_of_deploys);
                memcpy(resp->data, &status, sizeof(status));
                break;
            }
            case RGSSB_CMD_ISTAGE_CONF_GET: {
                gs_gssb_istage_burn_settings_t settings;
                i2c_res = gs_gssb_istage_get_burn_settings(cmd->i2c_addr, cmd->i2c_timeout_ms, &settings);
                resp->data_length = sizeof(settings);
                settings.std_time_ms = csp_hton16(settings.std_time_ms);
                settings.increment_ms = csp_hton16(settings.increment_ms);
                settings.short_cnt_down = csp_hton16(settings.short_cnt_down);
                memcpy(resp->data, &settings, sizeof(settings));
                break;
            }
            case RGSSB_CMD_ISTAGE_CONF_SET: {
                gs_gssb_istage_burn_settings_t * settings;
                settings = (gs_gssb_istage_burn_settings_t *) cmd->data;
                if (gs_gssb_istage_settings_unlock(cmd->i2c_addr, cmd->i2c_timeout_ms) != GS_OK) {
                    i2c_res = GS_ERROR_ACCESS;
                    break;
                }
                settings->std_time_ms = csp_ntoh16(settings->std_time_ms);
                settings->increment_ms = csp_ntoh16(settings->increment_ms);
                settings->short_cnt_down = csp_ntoh16(settings->short_cnt_down);
                if ((i2c_res = gs_gssb_istage_set_burn_settings(cmd->i2c_addr, cmd->i2c_timeout_ms, settings)) != GS_OK)
                    break;
                /* We need to have a delay as we write the settings to EEPROM which takes some time */
                gs_thread_sleep_ms(50);
                if ((i2c_res = gs_gssb_istage_set_burn_settings_cnt(cmd->i2c_addr, cmd->i2c_timeout_ms, settings)) != GS_OK)
                    break;
                /* We need to have a delay as we write the settings to EEPROM which takes some time */
                gs_thread_sleep_ms(20);
                if ((i2c_res = gs_gssb_istage_settings_lock(cmd->i2c_addr, cmd->i2c_timeout_ms)) != GS_OK)
                    break;
                i2c_res = gs_gssb_soft_reset(cmd->i2c_addr, cmd->i2c_timeout_ms);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_ISTAGE_ARM: {
                if (gs_gssb_istage_settings_unlock(cmd->i2c_addr, cmd->i2c_timeout_ms) != GS_OK) {
                    i2c_res = GS_ERROR_ACCESS;
                    break;
                }
                i2c_res = gs_gssb_istage_set_arm(cmd->i2c_addr, cmd->i2c_timeout_ms, cmd->data[0]);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_ISTAGE_DEPLOY: {
                i2c_res = gs_gssb_istage_burn(cmd->i2c_addr, cmd->i2c_timeout_ms);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_ISTAGE_STATE: {
                if (gs_gssb_istage_settings_unlock(cmd->i2c_addr, cmd->i2c_timeout_ms) != GS_OK) {
                    i2c_res = GS_ERROR_ACCESS;
                    break;
                }
                i2c_res = gs_gssb_istage_set_state(cmd->i2c_addr, cmd->i2c_timeout_ms, cmd->data[0]);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_ISTAGE_SENSORS: {
                uint16_t tmp_u;
                int16_t tmp_i;
                float tmp_flt;
                gs_gssb_istage_sensors_t sensors;
                gs_gssb_common_get_sun_voltage(cmd->i2c_addr, cmd->i2c_timeout_ms, &tmp_u);
                sensors.sun_voltage = csp_hton16(tmp_u);
                gs_gssb_common_get_internal_temp(cmd->i2c_addr, cmd->i2c_timeout_ms, &tmp_i);
                sensors.internal_temp = csp_hton16(tmp_i);
                i2c_res = gs_gssb_istage_get_temp(cmd->i2c_addr, cmd->i2c_timeout_ms, &tmp_flt);
                sensors.panel_temp = csp_htonflt(tmp_flt);
                resp->data_length = sizeof(sensors);
                memcpy(resp->data, &sensors, sizeof(sensors));
                break;
            }
#endif
#ifdef GS_SUNSENSOR_ENABLED
            case RGSSB_CMD_SUN_SAMPLE_SUNSENSOR: {
                i2c_res = gs_gssb_sun_sample_sensor(cmd->i2c_addr, cmd->i2c_timeout_ms);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_SUN_GET_SUNSENSOR: {
                uint16_t samples[4];
                uint8_t count = cmd->data[0];
                i2c_res = gs_gssb_sun_read_sensor_samples(cmd->i2c_addr, cmd->i2c_timeout_ms, samples);
                for (uint8_t i = 0 ; i < count && i < 4 ; i++) {
                    samples[i] = csp_hton16(samples[i]);
                }
                memcpy(resp->data, samples, (sizeof(samples[0]) * count));
                resp->data_length = sizeof(samples[0]) * count;
                break;
            }
            case RGSSB_CMD_SUN_SAMPLE_GET_SUNSENSOR: {
                uint16_t samples[4];
                uint8_t count = cmd->data[0];
                gs_gssb_sun_sample_sensor(cmd->i2c_addr, cmd->i2c_timeout_ms);
                gs_thread_sleep_ms(20);
                i2c_res = gs_gssb_sun_read_sensor_samples(cmd->i2c_addr, cmd->i2c_timeout_ms, samples);
                for (uint8_t i = 0 ; i < count && i < 4 ; i++) {
                    samples[i] = csp_hton16(samples[i]);
                }
                memcpy(resp->data, samples, (sizeof(samples[0]) * count));
                resp->data_length = sizeof(samples[0]) * count;
                break;
            }
            case RGSSB_CMD_SUN_SAMPLE_TEMP: {
                i2c_res = gs_gssb_sun_sample_temp(cmd->i2c_addr, cmd->i2c_timeout_ms);
                resp->data_length = 0;
                break;
            }
            case RGSSB_CMD_SUN_GET_TEMP: {
                float temp;
                i2c_res = gs_gssb_sun_get_temp(cmd->i2c_addr, cmd->i2c_timeout_ms, &temp);
                temp = csp_htonflt(temp);
                memcpy(resp->data, &temp, sizeof(temp));
                resp->data_length = sizeof(temp);
                break;
            }
            case RGSSB_CMD_SUN_SAMPLE_GET_TEMP: {
                float temp;
                gs_gssb_sun_sample_temp(cmd->i2c_addr, cmd->i2c_timeout_ms);
                gs_thread_sleep_ms(20);
                i2c_res = gs_gssb_sun_get_temp(cmd->i2c_addr, cmd->i2c_timeout_ms, &temp);
                temp = csp_htonflt(temp);
                memcpy(resp->data, &temp, sizeof(temp));
                resp->data_length = sizeof(temp);
                break;
            }
#endif
            default: {
                cmd->cmd_id = RGSSB_CMD_NOT_FOUND;
                resp->data_length = 0;
            }
        }
        resp->cmd_id = cmd->cmd_id;
        resp->i2c_error = i2c_res;
    }
    resp->cmd_id = csp_hton16(resp->cmd_id);
    resp->i2c_error = csp_hton32(resp->i2c_error);
    packet->length = resp->data_length + GS_RGSSB_RESP_HEADER_SIZE;
    if (!csp_send(conn, packet, 0))
        csp_buffer_free(packet);
    return;
}

void gs_rgssb_server_task(void * pvParameters)
{
    csp_conn_t * conn;
    csp_packet_t * packet;
    csp_socket_t * socket = csp_socket(CSP_SO_NONE);
    csp_bind(socket, GS_GSSB_PORT);

    csp_listen(socket, 2);

    while (1) {
        /* Wait for incoming connection, or timeout */
        conn = csp_accept(socket, CSP_MAX_DELAY);

        /* Only continue if a valid connection is present */
        if (conn == NULL)
            continue;

        while ((packet = csp_read(conn, 0)) != NULL) {

            if (packet == NULL)
                break;

            switch (csp_conn_dport(conn)) {
                case GS_GSSB_PORT: {
                    gs_rgssb_cmd_parser(conn, packet);
                    break;
                }
            }
        }

        csp_close(conn);
    }

}

gs_error_t gs_rgssb_csp_service_handler(csp_conn_t * conn)
{
    if (conn) {
        csp_packet_t * packet;
        while ((packet = csp_read(conn, 0)) != NULL) {
            gs_rgssb_cmd_parser(conn, packet);
        }
        csp_close(conn);
    }
    return GS_OK;
}
