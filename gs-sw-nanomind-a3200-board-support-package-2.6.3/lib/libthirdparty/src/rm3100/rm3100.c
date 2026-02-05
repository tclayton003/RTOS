/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/rm3100/rm3100.h>

#include <gs/embed/freertos.h>
#include <gs/util/drivers/i2c/master.h>


/* Register Map */
#define POLL 0x00 // Polls for a Single Measurement
#define CMM 0x01 // Initiates Continuous Measurement Mode
#define CCX 0x04 // Cycle Count Register - X Axis
#define CCY 0x06 // Cycle Count Register - Y Axis
#define CCZ 0x08 // Cycle Count Register - Z Axis
#define TMRC 0x0B // Sets continuous meas mode data rate
#define ALLX 0x0C // Alarm setup
#define AULX 0x0F
#define ALLY 0x12
#define AULY 0x15
#define ALLZ 0x18
#define AULZ 0x1B
#define ADLX 0x1E
#define ADLY 0x20
#define ADLZ 0x22
#define MX 0x24 // Measurement registers
#define MY 0x27
#define MZ 0x2A
#define BIST 0x33 // Build-In Self Test
#define STATUS 0x34 // Status of DRDY
#define HSHAKE 0x35 // Handshake reg
#define REVID 0x36 // MagI2C Revision Identification

/* Masks */
#define CMM_START 0x00
#define CMM_DRDM0 0x02
#define CMM_DRDM1 0x03
#define CMM_CMX 0x04
#define CMM_CMY 0x05
#define CMM_CMZ 0x06
#define CMM_LDM 0x07


#define POLL_CMX 0x04
#define POLL_CMY 0x05
#define POLL_CMZ 0x06


#define DEFAULT_I2C_ADDR 0x20
#define DEFAULT_I2C_ADDR_TEMP 0x48
#define DEFAULT_GS_RM3100_TWI_HANDLER 2

gs_rm3100_handle_t gs_rm3100_node_default()
{
    return gs_rm3100_node(DEFAULT_GS_RM3100_TWI_HANDLER, DEFAULT_I2C_ADDR, DEFAULT_I2C_ADDR_TEMP);
}

gs_rm3100_handle_t gs_rm3100_node_setup(uint8_t device_i2c_addr, uint8_t device_temp_i2c_addr)
{
    return gs_rm3100_node(DEFAULT_GS_RM3100_TWI_HANDLER, device_i2c_addr, device_temp_i2c_addr);
}

gs_rm3100_handle_t gs_rm3100_node(uint8_t twi_handler, uint8_t device_i2c_addr, uint8_t device_temp_i2c_addr)
{
    gs_rm3100_handle_t handle;
    handle.i2c_addr = device_i2c_addr;
    handle.i2c_addr_temp = device_temp_i2c_addr;
    handle.twi_handler = twi_handler;
    return handle;
}

gs_error_t gs_rm3100_init(const gs_rm3100_handle_t handle, gs_rm3100_rate_t rate)
{
    // setup for max speed (~146Hz for all 3 axis) at best resolution (200 cycle count)
    uint8_t cmd[2];
    gs_error_t error;

    cmd[0] = TMRC;
    cmd[1] = rate;
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 2, NULL, 0, 100);
    if (error)
    {
        return error;
    }

    // Set DRC1 and clean DRC0 = DRDY cleared on data-reading
    cmd[0] = HSHAKE;
    cmd[1] = 0x02;
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 2, NULL, 0, 100);
    if (error)
    {
        return error;
    }

    // Setup temp sensor 12 bit resolution
    cmd[0] = 0x01;
    cmd[1] = 0x60; // Now shutdown
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr_temp, cmd, 2, NULL, 0, 100);

    return error;
}

gs_error_t gs_rm3100_continuous_meas_setup(const gs_rm3100_handle_t handle,  uint8_t enabled)
{
    uint8_t cmd[2];
    gs_error_t error;

    cmd[0] = CMM;
    cmd[1] = (0x02 << CMM_DRDM0) | ( 0x07 << CMM_CMX) | ((enabled & 0x01) << CMM_START);

    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 2, NULL, 0, 100);

    return error;
}


gs_error_t gs_rm3100_start_single_meas(const gs_rm3100_handle_t handle)
{
    uint8_t cmd[2];
    gs_error_t error;
    // start single measurement
    cmd[0] = POLL;
    cmd[1] = ((0x07) << CMM_CMX);
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 2, NULL, 0, 100);

    return error;
}

gs_error_t gs_rm3100_poll_single_meas(const gs_rm3100_handle_t handle, gs_rm3100_data_t *data)
{
    int timeout=8;

    while (timeout--)
    {
        uint8_t status;
        gs_error_t error = gs_rm3100_drdy(handle, &status);
        if (error)
        {
            return error;
        }
        if (status > 0)
        {
            return gs_rm3100_read_meas(handle, data);
        }
        vTaskDelay(1);
    }
    data->x = 0;
    data->y = 0;
    data->z = 0;
    data->valid = 0;
    return GS_ERROR_TIMEOUT;
}


gs_error_t gs_rm3100_single_meas(const gs_rm3100_handle_t handle, gs_rm3100_data_t *data)
{
    uint8_t cmd[2];
    gs_error_t error;
    int timeout=8;
    // start single measurement
    cmd[0] = POLL;
    cmd[1] = ((0x07) << CMM_CMX);
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 2, NULL, 0, 100);

    if (error)
    {
        return error;
    }
    vTaskDelay(10);

    while (timeout--)
    {
        uint8_t status;
        error = gs_rm3100_drdy(handle, &status);
        if (error)
        {
            return error;
        }
        if (status > 0)
        {
            return gs_rm3100_read_meas(handle, data);
        }
        vTaskDelay(1);
    }
    data->x = 0;
    data->y = 0;
    data->z = 0;
    data->valid = 0;
    return GS_ERROR_TIMEOUT;
}

gs_error_t gs_rm3100_drdy(const gs_rm3100_handle_t handle, uint8_t *status)
{
    uint8_t cmd[1] = {STATUS};
    // Poll DRDY
    uint8_t value = 0;
    gs_error_t error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 1, &value, 1, 100);
    if (error)
    {
        return error;
    }
    if (value & 0x80)
    {
        *status = 1;
    }else
    {
        *status = 0;
    }
    return GS_OK;
}

gs_error_t gs_rm3100_read_meas_raw(const gs_rm3100_handle_t handle, gs_rm3100_rawdata_t *rawdata)
{
    uint8_t cmd[1] = {MX}; /* Start reading from the X Axis register */
    uint8_t res[9];
    gs_error_t error;
    int32_t b;
    uint32_t a;

    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 1, res, 9, 100);

    if (error == GS_OK)
    {
        //printf("%x %x %x   %x %x %x   %x %x %x \r\n",res[0],res[1],res[2],res[3],res[4],res[5],res[6],res[7],res[8]);
        a = (res[0] << 24) + (res[1] << 16) + (res[2] << 8);
        b = (int32_t) a;
        rawdata->x = b;
        a = (res[3] << 24) + (res[4] << 16) + (res[5] << 8);
        b = (int32_t) a;
        rawdata->y = b;
        a = (res[6] << 24) + (res[7] << 16) + (res[8] << 8);
        b = (int32_t) a;
        rawdata->z = b;
        rawdata->valid = 1;
    }
    else
    {
        rawdata->x = 0;
        rawdata->y = 0;
        rawdata->z = 0;
        rawdata->valid = 0;
    }
    return error;
}

gs_error_t gs_rm3100_read_meas(const gs_rm3100_handle_t handle, gs_rm3100_data_t *data)
{
    gs_rm3100_rawdata_t rawdata;
    gs_error_t error;

    error = gs_rm3100_read_meas_raw(handle, &rawdata);
    if (error == GS_OK)
    {
        data->x = -((float)rawdata.x)/(256*75)*10;
        data->y = -((float)rawdata.y)/(256*75)*10;
        data->z = ((float)rawdata.z)/(256*75)*10;
        data->valid = 1;
    }
    else
    {
        data->x = 0;
        data->y = 0;
        data->z = 0;
        data->valid = 0;
    }
    return error;
}


gs_error_t gs_rm3100_read_temp(const gs_rm3100_handle_t handle, float *temp)
{
    uint8_t cmd[1];
    uint8_t data[2];
    int16_t t;
    gs_error_t error;

    cmd[0] = 0;
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr_temp, cmd, 1, data, 2, 100);
    if (error)
    {
        return error;
    }
    t = (int)data[0];
    t = t << 8;
    t += data[1];
    *temp = ((float)t)*0.0625/16;
    return GS_OK;
}


gs_error_t gs_rm3100_self_test(const gs_rm3100_handle_t handle)
{
    uint8_t cmd[2];
    gs_error_t error;

    cmd[0] = BIST;
    cmd[1] = 0x88;

    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 2, NULL, 0, 100);
    if (error)
    {
        return error;
    }

    // start single measurement
    cmd[0] = POLL;
    cmd[1] = ((0x07) << CMM_CMX);
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr, cmd, 2, NULL, 0, 100);

    return error;
}
