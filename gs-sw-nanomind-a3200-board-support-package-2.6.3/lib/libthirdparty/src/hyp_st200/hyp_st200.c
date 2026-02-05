/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/*
 * Power up sequence
 *
 * initialize system
 * first acquisition - find events
 * first acquisition - identify stars
 * first acquisition - calculate attitude
 * on success - switch to tracking mode
 *
 * all data big endian
 *
 */

#include <gs/thirdparty/hyp_st200/hyp_st200.h>
#include <gs/util/error.h>
#include <gs/embed/drivers/uart/uart.h>
#include <string.h>
#include <gs/util/byteorder.h>
#include <stdio.h>
#include <gs/util/log.h>
#include <gs/util/hexdump.h>
#include <gs/util/time.h>

static GS_LOG_GROUP(gs_hyp_st200_log, "st200", GS_LOG_CAT_DRIVER, LOG_DEFAULT_MASK);
#define LOG_DEFAULT gs_hyp_st200_log

#define GS_HYP_ST200_BPS 115200
#define GS_HYP_ST200_COM_TIMEOUT 500

gs_error_t gs_hyp_st200_parse_response(uint8_t *input, size_t input_length, gs_hyp_st200_standard_frame_t *frame)
{
    if (input_length != 53)
    {
        return GS_ERROR_IO;
    }

    frame->header = input[0];
    frame->dev_id = input[1];
    frame->status_register = input[2];

    memcpy(&frame->exposure_time, input+3, sizeof(uint16_t));
    frame->exposure_time = util_ntoh16(frame->exposure_time);

    memcpy(&frame->time_tag_s, input+5, sizeof(uint32_t));
    frame->time_tag_s = util_ntoh32(frame->time_tag_s);

    memcpy(&frame->time_tag_sub_s, input+9, sizeof(uint16_t));
    frame->time_tag_sub_s = util_ntoh16(frame->time_tag_sub_s);

    memcpy(&frame->q1, input+11, sizeof(float));
    frame->q1 = util_ntohflt(frame->q1);

    memcpy(&frame->q2, input+15, sizeof(float));
    frame->q2 = util_ntohflt(frame->q2);

    memcpy(&frame->q3, input+19, sizeof(float));
    frame->q3 = util_ntohflt(frame->q3);

    memcpy(&frame->q4, input+23, sizeof(float));
    frame->q4 = util_ntohflt(frame->q4);

    memcpy(&frame->pitch, input+27, sizeof(float));
    frame->pitch = util_ntohflt(frame->pitch);

    memcpy(&frame->yaw, input+31, sizeof(float));
    frame->yaw = util_ntohflt(frame->yaw);

    memcpy(&frame->roll, input+35, sizeof(float));
    frame->roll = util_ntohflt(frame->roll);

    frame->stars_ident = input[51];
    frame->crc = input[52];

    return GS_OK;
}

gs_error_t gs_hyp_st200_destuff(uint8_t *input, size_t input_length, uint8_t *output, size_t * output_length)
{
    uint8_t destuff_mask = 0xf0;
    // never destuff the first byte as that has to be the header
    *output_length = 1;
    output[0] = input[0];
    for (size_t idx = 1; idx < input_length; idx++)
    {
        if (input[idx] != destuff_mask)
        {
            output[*output_length] = input[idx];
        }else{
            output[*output_length] = input[idx] | input[idx + 1];
            idx++;
        }
        (*output_length)++;
    }
    return GS_OK;
}

gs_error_t gs_hyp_st200_init(const uint8_t device, gs_hyp_st200_handle_t * handle)
{
    gs_log_group_register(LOG_DEFAULT);
    log_trace("Initializing device %d", device);
    handle->device = device;
    gs_uart_config_t conf;
    gs_uart_get_default_config(&conf);
    conf.comm.bps = GS_HYP_ST200_BPS;
    conf.tx_queue_size = 0;
    gs_error_t error;

    // The following is done as on a3200 uarts 2 and 4 are initialized during boot
    // So if we are using an uart that has been configured we do not have to configure again.
    // If we blindly try to init the uart we would get error logs from embed

    error = gs_uart_change_comm(device, &conf.comm);
    if (error == GS_ERROR_HANDLE)
    {
        // The uart might not be initialized
        error = gs_uart_init(device, &conf);
        if (error)
        {
            log_error("Failed to init uart [%s]", gs_error_string(error));
            return error;
        }
    }else if(error)
    {
        log_error("Failed to init uart [%s]", gs_error_string(error));
    }

    gs_time_sleep_ms(50);

    error = gs_hyp_st200_send_command(*handle, "ping\r", true);
    if (error)
    {
        return error;
    }


    error = gs_hyp_st200_send_command(*handle, "set frame-type binary standard\r", false);
    if (error)
    {
        return error;
    }

    error = gs_hyp_st200_send_command(*handle, "set keep-alive off\r", false);
    if (error)
    {
        return error;
    }

    return gs_hyp_st200_send_command(*handle, "set dumping-mode polling\r", false);
}


gs_error_t gs_hyp_st200_send_command(const gs_hyp_st200_handle_t handle, const char* command, bool no_response)
{
    size_t transfered_bytes;
    log_trace("Sending command [%s] to device %d", command, handle.device);
    gs_error_t error = gs_uart_write_buffer(handle.device,
                                            GS_HYP_ST200_COM_TIMEOUT,
                                            (const uint8_t *)command,
                                            strlen(command),
                                            &transfered_bytes);


    if (error)
    {
        log_error("Failed to send command [%s] to st200 [%s]", command, gs_error_string(error));
        return error;
    }

    if (transfered_bytes != strlen(command))
    {
        log_error("Failed to send entire command [%s]. Tried to send [%zu] but did send [%zu]",
                  command, strlen(command), transfered_bytes);
    }
    if (no_response)
    {
        // Command does not reply with READY
        return GS_OK;
    }

    uint8_t buffer[200];
    memset(buffer, 0, sizeof(buffer));

    size_t end_idx = 0;
    int timeout = GS_HYP_ST200_COM_TIMEOUT;
    while ((error == GS_OK) && (end_idx < 200))
    {
        error = gs_uart_read(handle.device, timeout, &buffer[end_idx]);
        if (error == GS_OK)
        {
            end_idx++;
            timeout = 5;  // reduce timeout when communication is started
        }
    }

    log_trace("error was %s end_idx was [%u]", gs_error_string(error), (unsigned int)end_idx);

    uint8_t st200_level_mask = 0;
    gs_log_group_get_level_mask(LOG_DEFAULT->name, &st200_level_mask);
    if (st200_level_mask & LOG_DEBUG)
    {
        gs_hexdump(buffer, 200);
    }

    for (size_t search=0; (search < (end_idx - 5)) && (search < 200); search++)
    {
        if ((buffer[search] == 0x52) &&      // R
            (buffer[search + 1] == 0x45) &&  // E
            (buffer[search + 2] == 0x41) &&  // A
            (buffer[search + 3] == 0x44) &&  // D
            (buffer[search + 4] == 0x59))    // Y
        {
            log_trace("Command sent successfully [%s]", command);
            return GS_OK;
        }
    }
    log_error("Did not receive READY for command [%s]", command);
    return GS_ERROR_DATA;
}


gs_error_t gs_hyp_st200_sample(const gs_hyp_st200_handle_t handle, gs_hyp_st200_standard_frame_t *sample)
{
    gs_error_t error = gs_hyp_st200_send_command(handle, "show record\r", true);
    if (error)
    {
        return error;
    }
    uint8_t buffer[200];
    uint16_t end_idx = 0;
    int timeout = GS_HYP_ST200_COM_TIMEOUT;
    error = GS_OK;
    while ((error == GS_OK) && (end_idx < 200))
    {
        error = gs_uart_read(handle.device, timeout, &buffer[end_idx]);
        if (error == GS_OK)
        {
            end_idx++;
            timeout = 5;  // reduce timeout when communication is started
        }
    } 

    uint8_t st200_level_mask = 0;
    gs_log_group_get_level_mask(LOG_DEFAULT->name, &st200_level_mask);
    if (st200_level_mask & LOG_DEBUG)
    {
        gs_hexdump(buffer, 200);
    }

    if (end_idx < 53)
    {
        log_error("Did not receive enough chars for a full frame. Got only [%d]", end_idx);
        return GS_ERROR_DATA;
    }

    uint16_t start_idx = 0;

    while ((start_idx < 200) && (buffer[start_idx] != 0xf1) && (start_idx < end_idx))
    {
        start_idx++;
    }

    // There has to be at least 53 bytes for a frame (perhaps more)
    if ((start_idx + 53) > end_idx)
    {
        return GS_ERROR_DATA;
    }

    uint8_t destuff[200];
    size_t out_len;

    error = gs_hyp_st200_destuff(buffer + start_idx, (end_idx - start_idx), destuff, &out_len);
    if (error)
    {
        return error;
    }

    return gs_hyp_st200_parse_response(destuff, (size_t)53, sample);
}



// Table and function provided by Hyperion
static const uint8_t crc8_table[256] =
{
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23,0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
};
static unsigned char crc8_calculate(unsigned char *data, unsigned int n)
{
    unsigned char crc = 0; unsigned int i;
    for (i = 0; i < n; i++)
    {
        crc = crc8_table[crc ^ data [i]];
    }
return crc;
}

gs_error_t gs_hyp_st200_crc(uint8_t *input, size_t input_length)
{
    // last byte is crc
    uint8_t expected_crc = input[input_length - 1];
    uint8_t crc = crc8_calculate(input, input_length - 1);
    if (crc == expected_crc)
    {
        return GS_OK;
    }else{
        return GS_ERROR_DATA;
    }
}

void gs_hyp_st200_show_frame(const gs_hyp_st200_standard_frame_t frame)
{
    printf("%-20s: %10d\n", "Header", frame.header);
    printf("%-20s: %10d\n", "Device ID", frame.dev_id);
    printf("%-20s: %10d\n", "Status register", frame.status_register);
    printf("%-20s: %10d\n", "Exposure time", frame.exposure_time);
    printf("%-20s: %10"PRIu32"\n", "Time sec", frame.time_tag_s);
    printf("%-20s: %10d\n", "Time subsec", frame.time_tag_sub_s);
    printf("%-20s: %10d\n", "Stars Identified", frame.stars_ident);
    printf("%-20s: %10d\n", "Crc", frame.crc);

    printf("%-20s: %10.6f\n", "Q1", frame.q1);
    printf("%-20s: %10.6f\n", "Q2", frame.q2);
    printf("%-20s: %10.6f\n", "Q3", frame.q3);
    printf("%-20s: %10.6f\n", "Q4", frame.q4);
    printf("%-20s: %10.6f\n", "Pitch", frame.pitch);
    printf("%-20s: %10.6f\n", "Yaw", frame.yaw);
    printf("%-20s: %10.6f\n", "Roll", frame.roll);
}
