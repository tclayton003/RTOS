/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/novatel_gnss/novatel_gnss.h>
#include "novatel_gnss_internal.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <gs/util/hexdump.h>
#include <gs/util/byteorder.h>
#include <gs/util/log.h>
#include <gs/util/time.h>
#include <gs/embed/freertos.h>
#include <gs/embed/drivers/uart/uart.h>

//#define NOVA_DEBUG
GS_LOG_GROUP(gs_gps_log, "gps", GS_LOG_CAT_ADCS, LOG_DEFAULT_MASK);
#define LOG_DEFAULT gs_gps_log

#ifndef GPS_USART_FD
#define GPS_USART_FD 1
#endif
//#define  GNSS_UART 	1

#define CRC32_POLYNOMIAL 0xEDB88320L

static const uint8_t novatel_sync[3] = {0xAA, 0x44, 0x12};

#ifdef  NOVA_DEBUG
static void print_header(novatel_msg_header_t * header)
{
    printf("\r\n----------------- HEADER ----------------- \r\n");
    printf("Header length %d\r\n", header->header_lgth);
    printf("Msg ID %d\r\n", header->msg_id);
    printf("Msg type %d\r\n", header->msg_type);
    printf("Port addr %d\r\n", header->port_addr);
    printf("Msg length %d\r\n", header->msg_lgth);
    printf("Sequence %d\r\n", header->sequence);
    printf("Idle time %d\r\n", header->idle_time);
    printf("Time status %d\r\n", header->time_status);
    printf("Week %d\r\n", header->week);
    printf("ms %" PRIu32 "\r\n", header->ms);
    printf("Receiver status %" PRIu32 "\r\n", header->receiver_status);
    printf("Reserved %d\r\n", header->reserved);
    printf("Receiver SW version %d\r\n", header->receiver_sw_version);
}

static void print_log_request(novatel_log_request_t * request)
{
    printf("\r\n----------------- Log request ----------------- \r\n");
    printf("Port %" PRIu32 "\r\n", request->port);
    printf("Msg ID %d\r\n", request->msg_id);
    printf("Msg type %d\r\n", request->msg_type);
    printf("Trigger %" PRIu32 "\r\n", request->trigger);
    printf("Period %f\r\n", request->period);
    printf("Offset %f\r\n", request->offset);
    printf("Hold %" PRIu32 "\r\n", request->hold);
}
#endif

void novatel_print_bestpos(novatel_bestpos_t * msg)
{
    printf("\r\n----------------- Best Pos ---------------------\r\n");
    printf("Solution status: %" PRIu32"\r\n", msg->sol_stat);
    printf("Pos type: %" PRIu32"\r\n", msg->pos_type);
    printf("Lat: %f\r\n", msg->lat);
    printf("Lon: %f\r\n", msg->lon);
    printf("Hgt: %f\r\n", msg->hgt);
    printf("Satellites tracked: %" PRIu8 "\r\n", msg->SVs);
    printf("Satellites used in solution: %" PRIu8 "\r\n", msg->solnSVs);
}

void novatel_print_bestxyz(novatel_bestxyz_t * msg)
{
    printf("Satellites tracked: %" PRIu8 "\r\n", msg->SVs);
    printf("Satellites used in solution: %" PRIu8 "\r\n", msg->solnSVs);
}

static gs_error_t novatel_error_to_gs_error(int nova)
{
    log_debug("Novatel error code: %d", nova);
    switch (nova)
    {
        case 1: // OK
            return GS_OK;
            break;
        case 2:
        case 18:
            return GS_ERROR_NOT_FOUND;
            break;
        case 3: // no resources
            return GS_ERROR_BUSY;
            break;
        case 4: // data packet not verified
        case 8:
        case 10:
        case 14:
        case 16:
        case 17:
        case 19:
        case 20:
        case 21:
        case 22:
        case 24:
        case 26:
        case 27:
        case 33:
        case 34:
        case 35:
        case 36:
        case 40:
        case 51:
            return GS_ERROR_DATA;
            break;
        case 78:
            return GS_ERROR_OVERFLOW;
            break;
        case 25:
            return GS_ERROR_NOT_SUPPORTED;
            break;
        case 15:
        case 29:
            return GS_ERROR_FULL;
            break;
        case 31:
            return GS_ERROR_TIMEOUT;
            break;
        case 6:
        case 7:
        case 9:
            return GS_ERROR_ARG;
            break;
        case 11:
            return GS_ERROR_RANGE;
            break;
        case 5:
        default:
            return GS_ERROR_UNKNOWN;
            break;
    }
}

static void novatel_msg_header_htole(novatel_msg_header_t * msg)
{
    msg->msg_id = util_htole16(msg->msg_id);
    msg->msg_lgth = util_htole16(msg->msg_lgth);
    msg->sequence = util_htole16(msg->sequence);
    msg->week = util_htole16(msg->week);
    msg->ms = util_htole32(msg->ms);
    msg->receiver_status = util_htole32(msg->receiver_status);
    msg->reserved = util_htole16(msg->reserved);
    msg->receiver_sw_version = util_htole16(msg->receiver_sw_version);
}

static void novatel_msg_header_letoh(novatel_msg_header_t * msg)
{
    msg->msg_id = util_letoh16(msg->msg_id);
    msg->msg_lgth = util_letoh16(msg->msg_lgth);
    msg->sequence = util_letoh16(msg->sequence);
    msg->week = util_letoh16(msg->week);
    msg->ms = util_letoh32(msg->ms);
    msg->receiver_status = util_letoh32(msg->receiver_status);
    msg->reserved = util_letoh16(msg->reserved);
    msg->receiver_sw_version = util_letoh16(msg->receiver_sw_version);
}

static void novatel_log_request_htole(novatel_log_request_t * req)
{
    req->port = util_htole32(req->port);
    req->msg_id = util_htole16(req->msg_id);
    req->trigger = util_htole32(req->trigger);
    req->period = util_htole64(req->period);
    req->offset = util_htole64(req->offset);
    req->hold = util_htole32(req->hold);
}

static void novatel_log_request_letoh(novatel_log_request_t * req)
{
    req->port = util_letoh32(req->port);
    req->msg_id = util_letoh16(req->msg_id);
    req->trigger = util_letoh32(req->trigger);
    req->hold = util_letoh32(req->hold);
}

/* --------------------------------------------------------------------------
 * Calculate a CRC value to be used by CRC calculation functions.
 * -------------------------------------------------------------------------- */
static unsigned long CRC32Value(int i)
{
    int j;
    unsigned long ulCRC;
    ulCRC = i;
    for ( j = 8 ; j > 0; j-- )
    {
        if ( ulCRC & 1 )
            ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
        else
            ulCRC >>= 1;
    }
    return ulCRC;
}
/* --------------------------------------------------------------------------
 * Calculates the CRC-32 of a block of data all at once
 * -------------------------------------------------------------------------- */
static unsigned long CalculateBlockCRC32(unsigned long ulCount,/* Number of bytes in the data block */
                                         unsigned char *ucBuffer ) /* Data block */
{
    unsigned long ulTemp1;
    unsigned long ulTemp2;
    unsigned long ulCRC = 0;
    while ( ulCount-- != 0 )
    {
        ulTemp1 = ( ulCRC >> 8 ) & 0x00FFFFFFL;
        ulTemp2 = CRC32Value( ((int) ulCRC ^ *ucBuffer++ ) & 0xff );
        ulCRC = ulTemp1 ^ ulTemp2;
    }
    return( ulCRC );
}

static gs_error_t send_command(novatel_msg_header_t * header, void * cmd, uint32_t cmd_len)
{
    uint32_t chk_sum;

    /* Move to one buffer and add crc */
    uint16_t buff_size = sizeof(*header) + cmd_len + 4;
    unsigned char buffer[buff_size];

    /* Header to le */
    novatel_msg_header_htole(header);
    memcpy(buffer, header, sizeof(*header));
    memcpy(&buffer[sizeof(*header)], cmd, cmd_len);
    chk_sum = util_htole32(CalculateBlockCRC32(sizeof(*header) + cmd_len , buffer));
    memcpy(&buffer[sizeof(*header) + cmd_len], &chk_sum, 4);

    /* Send command */
    return gs_uart_write_buffer(GPS_USART_FD, -1, buffer, buff_size, NULL);
}

#ifdef NOVATEL_READ_RESPONSE_RAW
static int read_response_raw(char * buff, uint32_t max_read)
{
    uint32_t bytecnt = 0;
    int i;
    do {
        i = usart_messages_waiting(GPS_USART_FD);
        if (i > 0) {
            buff[bytecnt++] = usart_getc(GPS_USART_FD);
        }
    } while (i > 0 && (bytecnt < max_read));
    return bytecnt;
}
#endif

static gs_error_t read_response(char * buff, uint32_t max_read, uint16_t timeout_ms, uint16_t msg_id, int * bytecnt_out)
{
    int bytecnt = 0;
    int bytes_to_read = 0;
    int msg_lgth = 0;
    //int i;
    int state = 0;
    uint32_t crc;
    int8_t crc_ok = 0;
    novatel_msg_header_t * header = (novatel_msg_header_t *) buff;
    log_debug("State %d", state);
    do {
        //i = usart_messages_waiting(GPS_USART_FD);
        uint8_t data;
        gs_error_t error = gs_uart_read(GPS_USART_FD, (int)timeout_ms, &data);
        if (error == GS_OK) {
            buff[bytecnt] = data;
            /* Check for preamble */
            if (state == 0) {
                if (novatel_sync[bytes_to_read] != buff[bytecnt]) {
                } else {
                    bytes_to_read++;
                    bytecnt++;
                }
                if (bytes_to_read == 3) {
                    state = 1;
                    log_debug("State %d", state);
                }

                /* Process header */
            } else if (state == 1) {
                if (bytes_to_read == (sizeof(novatel_msg_header_t) - 1)) {
                    header = (novatel_msg_header_t *) &buff[bytecnt - sizeof(novatel_msg_header_t) + 1];
                    msg_lgth = util_letoh16(header->msg_lgth);
#ifdef NOVA_DEBUG
                    printf("\r\n\r\n -------------- \r\n\r\n");
                    print_header(header);
#endif
                    bytes_to_read = msg_lgth + 4;
                    state = 2;
                    log_debug("State %d", state);
                } else {
                    bytes_to_read++;
                }
                bytecnt++;
                /* Precess data */
            } else if (state == 2) {
                bytecnt++;
                bytes_to_read--;
                if (!bytes_to_read) {
                    /* CRC check */
                    memcpy(&crc, &buff[bytecnt - 4], 4);
                    crc = util_letoh32(crc);
                    if (CalculateBlockCRC32(msg_lgth+28, (unsigned char *)&buff[bytecnt - (msg_lgth+4+28)]) != crc) {
                        crc_ok = -1;
                        log_debug("CRC err");
                    }

                    /* See if we have received the wanted message */
                    if (util_letoh16(header->msg_id) == msg_id) {
                        state = 4;
                        log_debug("State %d", state);
                    } else {
                        state = 0;
                        log_debug("State %d msg_id %d", state, util_letoh16(header->msg_id));
                        bytes_to_read = 0;
                    }
                }
            }

        /* } else { */
        /*     vTaskDelay(10 / portTICK_RATE_MS); */
        /*     timeout_ms -= 10; */
        }else
        {
            return error;
        }
    } while ((bytecnt < (int32_t) max_read) && (state != 4));


    novatel_msg_header_letoh(header);
    if (crc_ok == -1 || !timeout_ms)
    {
        return GS_ERROR_DATA;
    }
    if (bytecnt_out)
    {
        *bytecnt_out = bytecnt;
    }
    return GS_OK;
}

int novatel_ascii_command(const char * cmd, uint16_t cmd_len, unsigned char * response, uint16_t max_res_len, uint16_t timeout_ms)
{
    //int i;
    int bytecnt = 0;
    //int tmp = 0;

    /* Write command to GPS */
    gs_uart_write_buffer(GPS_USART_FD, -1, (const uint8_t *) cmd, cmd_len, NULL);
    gs_time_sleep_ms(10); // vTaskDelay(10 / portTICK_RATE_MS);
    /* Read response */
    gs_error_t error;
    do {
        uint8_t data;
        error = gs_uart_read(GPS_USART_FD, (int) timeout_ms, &data);
        /* i = usart_messages_waiting(GPS_USART_FD); */
        /* if (i == 0) { */
        /*     vTaskDelay(10 / portTICK_RATE_MS); */
        /*     tmp++; */
        /*     continue; */
        /* } */
        if (bytecnt < max_res_len) {
            response[bytecnt++] = data; // usart_getc(GPS_USART_FD);
            //} else {
            //usart_getc(GPS_USART_FD);
        }
    } while (error == GS_OK);

    return bytecnt;
}	

static void gen_header_log_cmd(novatel_msg_header_t * header, uint32_t len)
{
    header->sync[0] = 0xaa;
    header->sync[1] = 0x44;
    header->sync[2] = 0x12;
    header->header_lgth = sizeof(*header);
    header->msg_id = (CMD_LOG);
    header->msg_type = MSG_TYPE_BIN;
    header->port_addr = NOVA_PORT;
    header->msg_lgth = (len);
    header->sequence = 0;
    header->time_status = 0;
}

static gs_error_t parse_log_response(char * response, uint16_t * tot_len, char * res_string, uint8_t res_str_max_len)
{
    novatel_msg_header_t res_header;
    memcpy(&res_header, response, sizeof(res_header));

    novatel_msg_header_letoh(&res_header);

#ifdef NOVA_DEBUG
    print_header(&res_header);
#endif

    /* Check sync. If the sync dosen't match we are not at the start 
     * of a packet */
    if (memcmp(res_header.sync, novatel_sync, sizeof(novatel_sync)) != 0) {
        return -1;
    }

    /* Get response id */
    uint32_t response_id;
    memcpy(&response_id, &response[sizeof(res_header)], 4);

    response_id = util_letoh32(response_id);

    /* Copy the response string */
    if (res_str_max_len < (res_header.msg_lgth-4+1))
        res_header.msg_lgth = res_str_max_len+4-1;

    res_string[res_header.msg_lgth-4] = '\0';
    memcpy(res_string, &response[sizeof(res_header)+4], res_header.msg_lgth-4);

    /* Write total length of the log response to tot_len */
    *tot_len = res_header.header_lgth + res_header.msg_lgth + 4;

    return novatel_error_to_gs_error(response_id);
}

static void init_logparam(novatel_log_request_t * logreq)
{
    logreq->port = (NOVA_PORT);
    logreq->msg_type = MSG_TYPE_BIN;
    logreq->reserved = 0;
    logreq->trigger = (TRIGGER_ONNEXT);
    logreq->period = 0.0;
    logreq->offset = 0.0;
    logreq->hold = (0);
}

static gs_error_t log_command(novatel_msg_header_t * header, novatel_log_request_t * logrequest, void * response_log, uint32_t response_max_len)
{
    char response[1000];
    char response_str[50];
    int bytecnt;
    uint16_t tot_len;
    int tmp;

#ifdef NOVA_DEBUG
    print_header(header);
    hex_dump(header, sizeof(*header));
    hex_dump(logrequest, sizeof(*logrequest));
    print_log_request(logrequest);
#endif
    novatel_log_request_htole(logrequest);
    send_command(header, logrequest, sizeof(*logrequest));

    /* Read response */
    novatel_log_request_letoh(logrequest);
    gs_error_t error = read_response(response, sizeof(response), 2000, logrequest->msg_id, &bytecnt);
    log_debug("Got %d bytes from GPS", bytecnt);
    if (error) 
    {
        return error;
    }

    /* First read log command response. This will tell us if 
     * the request was successful and tot_len gives where 
     * the logresponse starts*/
    error = parse_log_response(response, &tot_len, response_str, sizeof(response_str));

    /* Check that we got response ID ok */
    log_debug("Got response error: %d String: %s", error, response_str);
    if (error)
    {
        return error;
    }

    /* Parse log */
    /* Header */
    novatel_msg_header_t response_header;
    memcpy(&response_header, &response[tot_len], sizeof(response_header));

    //novatel_msg_header_letoh(&response_header);

    if (response_header.msg_lgth <= response_max_len) {
        tmp = response_header.msg_lgth;
    } else {
        tmp = response_max_len;
    }

    /* log */
    memcpy(response_log, &response[tot_len+sizeof(response_header)], tmp);

#ifdef NOVA_DEBUG
    hex_dump(response, bytecnt);
#endif

    return error;
}

gs_error_t novatel_get_bestpos(novatel_bestpos_t * bestpos)
{
    /* Header for log command */
    novatel_msg_header_t header;
    gen_header_log_cmd(&header, sizeof(novatel_log_request_t));

    /* Log parameters */
    novatel_log_request_t log_parameters;
    init_logparam(&log_parameters);
    log_parameters.msg_id = (LOG_BESTPOS);
    //log_parameters.period = 1.0;

    return log_command(&header, &log_parameters, bestpos, sizeof(*bestpos));
}

gs_error_t novatel_get_bestxyz(novatel_bestxyz_t * bestxyz)
{
    /* Header for log command */
    novatel_msg_header_t header;
    gen_header_log_cmd(&header, sizeof(novatel_log_request_t));

    /* Log parameters */
    novatel_log_request_t log_parameters;
    init_logparam(&log_parameters);
    log_parameters.msg_id = LOG_BESTXYZ;

    return log_command(&header, &log_parameters, bestxyz, sizeof(*bestxyz));
}

gs_error_t novatel_get_trackstat(novatel_trackstat_t *trackstat)
{
    /* Header for log command */
    novatel_msg_header_t header;
    gen_header_log_cmd(&header, sizeof(novatel_log_request_t));

    /* Log parameters */
    novatel_log_request_t log_parameters;
    init_logparam(&log_parameters);
    log_parameters.msg_id = LOG_TRACKSTAT;

    return log_command(&header, &log_parameters, trackstat, sizeof(*trackstat));
}

gs_error_t novatel_get_receiver_status(novatel_rxstatus_t * rxstatus)
{
    /* Header for log command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};

    gen_header_log_cmd(&header, sizeof(novatel_log_request_t));

    /* Log parameters */
    novatel_log_request_t log_parameters;
    init_logparam(&log_parameters);
    log_parameters.msg_id = LOG_RXSTATUS; /* RXSTATUS log */

    return log_command(&header, &log_parameters, rxstatus, sizeof(*rxstatus));
}

gs_error_t novatel_get_receiver_time(novatel_time_t * ntime)
{
    /* Header for log command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};

    gen_header_log_cmd(&header, sizeof(novatel_log_request_t));

    /* Log parameters */
    novatel_log_request_t log_parameters;
    init_logparam(&log_parameters);
    log_parameters.msg_id = LOG_TIME; /* TIME log */
    log_parameters.trigger = TRIGGER_ONNEXT; /* Get the next PPS time */

    return log_command(&header, &log_parameters, ntime, sizeof(*ntime));
}

gs_error_t novatel_get_temperature(novatel_hwstatus_t * meas)
{
    /* Header for log command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};

    gen_header_log_cmd(&header, sizeof(novatel_log_request_t));

    /* Log parameters */
    novatel_log_request_t log_parameters;
    init_logparam(&log_parameters);
    log_parameters.msg_id = LOG_HW; /* HWMONITOR log */

    return log_command(&header, &log_parameters, meas, sizeof(*meas));
}

gs_error_t novatel_set_serial_baudrate(uint32_t baudrate, uint8_t parity, uint8_t databits, uint8_t stopbits)
{

    char response[200];
    uint16_t tot_len;
    uint16_t msg_id;
    char response_str[20];
    gs_error_t error;

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = CMD_SERIALCONFIG;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_serial_config_t);
    header.sequence = 0;
    header.time_status = 0;

    msg_id = header.msg_id;

#ifdef NOVA_DEBUG
    print_header(&header);
#endif

    /* Form command */
    novatel_serial_config_t cmd;
    cmd.port = util_htole32(6);
    cmd.baudrate = util_htole32(baudrate);
    cmd.parity = util_htole32(parity);
    cmd.databits = util_htole32(databits);
    cmd.stopbits = util_htole32(stopbits);
    cmd.handshake = util_htole32(0);
    cmd.break_detection = util_htole32(1);

    error = send_command(&header, &cmd, sizeof(cmd));
    if (error)
    {
        return error;
    }

    /* Wait for GPS */
    gs_time_sleep_ms(1000); //vTaskDelay(1000 / portTICK_RATE_MS);

    /* Read response */
    error = read_response(response, sizeof(response), 2000, msg_id, NULL);
    if (error)
    {
        return error;
    }
    

    /* First read header from log command */
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}

gs_error_t novatel_reset_receiver(void)
{
    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = CMD_RESET;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_reset_command_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_reset_command_t cmd;
    cmd.target = util_htole32(0);

    return send_command(&header, &cmd, sizeof(cmd));

}

gs_error_t novatel_set_approxpos(uint64_t lat, uint64_t lon, uint64_t height)
{
    char response[200];
    char response_str[20];
    uint16_t tot_len;
    gs_error_t error;

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = CMD_SETPOS;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_approxpos_command_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_approxpos_command_t cmd;
    cmd.lat = util_htole64(lat);
    cmd.lon = util_htole64(lon);
    cmd.height = util_htole64(height);
    error = send_command(&header, &cmd, sizeof(cmd));
    if (error)
    {
        return error;
    }

    /* Read respons */
    error = read_response(response, sizeof(response), 6000, CMD_SETPOS, NULL);
    if (error)
    {
        return error;
    }
    memcpy(&header, &response[0], 28);

    /* First read header from log command */
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}

gs_error_t novatel_set_tropomodel(novatel_tropo_model_t model)
{
    char response[200];
    char response_str[20];
    gs_error_t error;
    uint16_t tot_len;

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = CMD_SETTROPOMODEL;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_tropomodel_command_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_tropomodel_command_t cmd;
    cmd.model = util_htole32(model);
    error = send_command(&header, &cmd, sizeof(cmd));
    if (error)
    {
        return error;
    }

    /* Read respons */
    error = read_response(response, sizeof(response), 6000, CMD_SETTROPOMODEL, NULL);
    if (error)
    {
        return error;
    }
    
    memcpy(&header, &response[0], 28);

    /* First read header from log command */
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}


gs_error_t novatel_set_dopplerwindow(novatel_assign_system_t assign_system, novatel_doppler_mode_t mode, uint32_t window_hz)
{
    char response[200];
    char response_str[20];
    uint16_t tot_len;

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = CMD_DOPPLERWINDOW;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_dopplerwindow_command_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_dopplerwindow_command_t cmd;
    cmd.system = util_htole32(assign_system);
    cmd.mode = util_htole32(mode);
    cmd.window = util_htole32(window_hz);
    cmd.centerfreq = util_htole32(0); // based on documentation this is reserved for future use, hence not implemented
    send_command(&header, &cmd, sizeof(cmd));

    /* Read respons */
    gs_error_t error = read_response(response, sizeof(response), 6000, CMD_DOPPLERWINDOW, NULL);
    if (error)
    {
        return error;
    }
    
    memcpy(&header, &response[0], 28);

    /* First read header from log command */
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}


gs_error_t novatel_set_approxpos_timeout(uint32_t secs)
{
    char response[200];
    char response_str[20];
    uint16_t tot_len;

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = CMD_APPROXPOSTIMEOUT;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_approxpostimeout_command_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_approxpostimeout_command_t cmd;
    cmd.secs = util_htole32(secs);
    send_command(&header, &cmd, sizeof(cmd));

    /* Read respons */
    gs_error_t error = read_response(response, sizeof(response), 6000, CMD_APPROXPOSTIMEOUT, NULL);
    if (error)
    {
        return error;
    }
    
    memcpy(&header, &response[0], 28);

    /* First read header from log command */
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}

gs_error_t novatel_set_postimeout(uint32_t secs)
{
    char response[200];
    char response_str[20];
    uint16_t tot_len;
    gs_error_t error;

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = CMD_SETPOSTIMEOUT;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_tropomodel_command_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_postimeout_command_t cmd;
    cmd.secs = util_htole32(secs);
    error = send_command(&header, &cmd, sizeof(cmd));
    if (error)
    {
        return error;
    }

    /* Read respons */
    error = read_response(response, sizeof(response), 6000, CMD_SETPOSTIMEOUT, NULL);
    if (error)
    {
        return error;
    }
    
    memcpy(&header, &response[0], 28);

    /* First read header from log command */
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}

gs_error_t novatel_set_approxtime(uint32_t week, uint64_t sec)
{
    char response[200];
    char response_str[20];
    uint16_t tot_len;
    gs_error_t error;

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = CMD_SETTIME;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_approxtime_command_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_approxtime_command_t cmd;
    cmd.week = util_htole32(week);
    cmd.sec = util_htole64(sec);
    error = send_command(&header, &cmd, sizeof(cmd));
    if (error)
    {
        return error;
    }

    /* Read respons */
    error = read_response(response, sizeof(response), 6000, CMD_SETTIME, NULL);
    if (error)
    {
        return error;
    }
    memcpy(&header, &response[0], 28);

    /* First read header from log command */
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}

gs_error_t novatel_set_ecutoff(float angle)
{
    char response[200];
    gs_error_t error;
    uint16_t tot_len;
    uint16_t msg_id = CMD_ECUTOFF;
    char response_str[20];


    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = msg_id;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_ecutoff_command_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_ecutoff_command_t cmd;

    uint32_t tempvalue = 0;
    float tempvalue2 = angle;
    memcpy(&tempvalue, &tempvalue2, sizeof(tempvalue2));

    cmd.angle = util_htole32(tempvalue);
    error = send_command(&header, &cmd, sizeof(cmd));
    if (error)
    {
        return error;
    }

    error = read_response(response, sizeof(response), 2000, CMD_ECUTOFF, NULL);
    if (error)
    {
        return error;
    }
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}

/* Adjust the PPS behaviour */
gs_error_t novatel_set_ppscontrol(void)
{
    char response[200];
    gs_error_t error;
    uint16_t tot_len;
    uint16_t msg_id = CMD_PPSCTRL;
    char response_str[20];

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = msg_id;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_ppscontrol_t);
    header.sequence = 0;
    header.time_status = 0;

    /* Cmd parameters */
    novatel_ppscontrol_t cmd;

    /* Default values */
    uint32_t temp_switch = 2;
    cmd.switch_var = util_htole32( temp_switch );

    uint32_t temp_polarity = 1;
    cmd.polarity = util_htole32( temp_polarity );

    uint32_t temp_pulse_width = 1000;
    cmd.pulse_width = util_htole32( temp_pulse_width );

    uint64_t temp_period = 0;
    double period = 1.0;
    memcpy(&temp_period, &period, sizeof(period));
    cmd.period = util_htole64(temp_period);

    error = send_command(&header, &cmd, sizeof(cmd));
    if (error)
    {
        return error;
    }
    error = read_response(response, sizeof(response), 2000, CMD_PPSCTRL, NULL);
    if (error)
    {
        return error;
    }
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}

/* Sets output pulse train available on VARF */
gs_error_t novatel_set_frequencyout(uint8_t en_varf){
    char response[200];
    gs_error_t error;
    uint16_t tot_len;
    uint16_t msg_id = CMD_FREQUENCYOUT;
    char response_str[20];

    /* Header for command */
    novatel_msg_header_t header = {.sync = {0xaa, 0x44, 0x12}};
    header.header_lgth = sizeof(header);
    header.msg_id = msg_id;
    header.msg_type = MSG_TYPE_BIN;
    header.port_addr = NOVA_PORT;
    header.msg_lgth = sizeof(novatel_frequencyout_t);
    header.sequence = 0;
    header.time_status = 0;

     /* Cmd parameters */
    novatel_frequencyout_t cmd;

    /* Default values */
    uint32_t switch_var; 
    if (en_varf == 1){
        switch_var = 2; // Enable the VARF signal, be synchronize it with the PPS signal
    } else {
        switch_var = 0; // Disable frequency output
    }

    cmd.switch_var = util_htole32( switch_var );
    uint32_t pulse_width = 5; // 10 MHz
    uint32_t period = 10; // 50% Duty Cycle
    cmd.pulse_width = util_htole32( pulse_width );
    cmd.period = util_htole32( period );

    error = send_command(&header, &cmd, sizeof(cmd));
    if (error)
    {
        return error;
    }
    error = read_response(response, sizeof(response), 2000, CMD_FREQUENCYOUT, NULL);
    if (error)
    {
        return error;
    }
    return parse_log_response(response, &tot_len, response_str, sizeof(response_str));
}
