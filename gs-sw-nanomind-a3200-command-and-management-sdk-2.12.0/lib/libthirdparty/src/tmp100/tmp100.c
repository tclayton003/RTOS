/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/tmp100/tmp100.h>
#include <gs/util/drivers/i2c/master.h>


gs_tmp100_handle_t gs_tmp100_node(uint8_t twi_handler, uint8_t device_temp_i2c_addr)
{
    gs_tmp100_handle_t handle;
    handle.i2c_addr_temp = device_temp_i2c_addr;
    handle.twi_handler = twi_handler;
    return handle;
}

gs_tmp100_handle_t gs_tmp100_node_default()
{
    return gs_tmp100_node(DEFAULT_TMP100_TWI_HANDLER, DEFAULT_TMP100_ADDRESS);
}

gs_error_t gs_tmp100_init(const gs_tmp100_handle_t handle)
{
    uint8_t cmd[2];
    gs_error_t error;

    cmd[0] = 0x01;  // Configure
    cmd[1] = 0x60;  // Setup temp sensor 12 bit resolution. Continuous read.
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr_temp, cmd, 2, NULL, 0, 100);

    return error;
}

gs_error_t gs_tmp100_read_temp(const gs_tmp100_handle_t handle, float *temp)
{
    uint8_t cmd[1];
    uint8_t data[2] = {0,0};
    int16_t t;
    gs_error_t error;

    cmd[0] = 0;
    error = gs_i2c_master_transaction(handle.twi_handler, handle.i2c_addr_temp, cmd, 1, data, 2, 100);
    if (error) {
        return error;
    }
    t = (int) data[0];
    t = t << 8;
    t += data[1];
    *temp = ((float) t) * 0.0625 / 16;
    return GS_OK;
}

