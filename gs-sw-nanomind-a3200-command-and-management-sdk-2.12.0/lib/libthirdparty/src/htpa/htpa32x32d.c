/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
#include "gs/thirdparty/htpa/htpa32x32d.h"
#include <gs/util/drivers/i2c/master.h>
#include <gs/util/check.h>
#include <gs/util/time.h>
#include <gs/util/byteorder.h>
#include <math.h>

#define SPI_TIMEOUT 100
#define WAKEUP 1
#define START 8
#define HTPA_CONFIG_REG 0x01
#define HTPA_STATUS_REG 0x02
#define HTPA_TRIM_1 0x03
#define HTPA_TRIM_2 0x04
#define HTPA_TRIM_3 0x05
#define HTPA_TRIM_4 0x06
#define HTPA_TRIM_5 0x07
#define HTPA_TRIM_6 0x08
#define HTPA_TRIM_7 0x09
#define HTPA_BLOCK_UPPER 0x0A
#define HTPA_BLOCK_LOWER 0x0B
#define HTPA_READY_WAIT_ATTEMPTS 0xFFF

#define HTPA_CONFIG_START 0x8
#define HTPA_CONFIG_WAKEUP 0x1
#define HTPA_CONFIG_BLIND 0x2

#define HTPA_STATUS_EOC 0x1
#define HTPA_STATUS_BLIND 0x02
#define HTPA_STATUS_VDD_MEAS 0x4
#define HTPA_STATUS_BLOCK_MASK 0x30

LOG_GROUP_EXTERN(log_htpa);
#define LOG_DEFAULT log_htpa

gs_error_t gs_htpa_configure(gs_htpa_handle_t *handle, uint8_t bus, uint8_t address, uint8_t eeprom_address)
{
    handle->htpa_bus = bus;
    handle->htpa_address = address;
    handle->htpa_eeprom_address = eeprom_address;
    return GS_OK;
}

gs_error_t gs_htpa_mbit_ref_cal(const gs_htpa_handle_t handle, uint8_t mbit_trim, uint8_t ref_cal)
{
    GS_CHECK_ARG(ref_cal < 4);
    GS_CHECK_ARG(mbit_trim <= 12);
    GS_CHECK_ARG(mbit_trim >= 4);
    uint8_t cmd[2];
    cmd[0] = HTPA_TRIM_1;
    cmd[1] = (mbit_trim) | (ref_cal << 4);
    return gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);
}

gs_error_t gs_htpa_bias_trim(const gs_htpa_handle_t handle, uint8_t bias_trim_top, uint8_t bias_trim_bottom)
{
    GS_CHECK_ARG(bias_trim_top <= 31);
    GS_CHECK_ARG(bias_trim_bottom <= 31);
    uint8_t cmd[2];
    cmd[0] = HTPA_TRIM_2;
    cmd[1] = bias_trim_top;
    gs_error_t error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);
    
    if (error) {
        return error;
    }
    
    cmd[0] = HTPA_TRIM_3;
    cmd[1] = bias_trim_bottom;
    return gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);
}

gs_error_t gs_htpa_clock_trim(const gs_htpa_handle_t handle, uint8_t clockfreq)
{
    GS_CHECK_ARG(clockfreq <= 63);
    uint8_t cmd[2];
    cmd[0] = HTPA_TRIM_4;
    cmd[1] = clockfreq;
    return gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);
}

gs_error_t gs_htpa_bpa_trim(const gs_htpa_handle_t handle, uint8_t bpa_top, uint8_t bpa_bottom)
{
    GS_CHECK_ARG(bpa_top <= 31);
    GS_CHECK_ARG(bpa_bottom <= 31);

    uint8_t cmd[2];
    cmd[0] = HTPA_TRIM_5;
    cmd[1] = bpa_top;
    gs_error_t error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);
    
    if (error) {
        return error;
    }

    cmd[0] = HTPA_TRIM_6;
    cmd[1] = bpa_bottom;
    return gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);
}

gs_error_t gs_htpa_i2c_pull_up(const gs_htpa_handle_t handle, gs_htpa_pullup_t sda, gs_htpa_pullup_t scl)
{
    uint8_t cmd[2];
    cmd[0] = HTPA_TRIM_7;
    cmd[1] = (sda << 4) | scl;
    return gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);
}


gs_error_t gs_htpa_wakeup(const gs_htpa_handle_t handle)
{
    uint8_t wake_up[2];
    wake_up[0] = HTPA_CONFIG_REG;
    wake_up[1] = 0x01;
    return gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, wake_up, 2, NULL, 0, SPI_TIMEOUT);
}

static gs_error_t gs_htpa_is_expected_status(const gs_htpa_handle_t handle,
        uint8_t block, bool eoc, bool blind, bool* ok)
{
    uint8_t control = HTPA_STATUS_REG;
    uint8_t value;
    *ok = 0;
    gs_error_t error = gs_i2c_master_transaction(handle.htpa_bus,
            handle.htpa_address, &control, 1, &value, 1, SPI_TIMEOUT);
    
    if (error) {
        return error;
    }

    uint8_t expected = 0;
    if (blind)
    {
        expected |= HTPA_STATUS_BLIND;
    }
    if (eoc)
    {
        expected |= HTPA_STATUS_EOC;
    }
    if (block > 0)
    {
        expected |= (block << 4);
    }
    if (value == expected)
    {
        *ok = 1;
    }

    return GS_OK;
}

gs_error_t gs_htpa_read_status(const gs_htpa_handle_t handle, gs_htpa_status_t *status)
{
    uint8_t control = HTPA_STATUS_REG;
    uint8_t value;
    gs_error_t error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address,
            &control, 1, &value, 1, SPI_TIMEOUT);
    status->eoc = value & HTPA_STATUS_EOC;
    status->blind = value & HTPA_STATUS_BLIND;
    status->vdd_meas = value & HTPA_STATUS_VDD_MEAS;
    status->block = (value & HTPA_STATUS_BLOCK_MASK) >> 4;
    
    return error;
}

gs_error_t gs_htpa_start(const gs_htpa_handle_t handle)
{
    uint8_t start[2];
    start[0] = HTPA_CONFIG_REG;
    start[1] = 0x09;
    return gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, start, 2, NULL, 0, SPI_TIMEOUT);
}

gs_error_t gs_htpa_sleep(const gs_htpa_handle_t handle)
{
    uint8_t sleep[2];
    sleep[0] = HTPA_CONFIG_REG;
    sleep[1] = 0x00;
    return gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, sleep, 2, NULL, 0, SPI_TIMEOUT);
}

static uint16_t th_idx(uint16_t pixel)
{
    if (pixel < 512)
    {
        return pixel;
    }else
    {
        uint16_t th_row = 47 - (pixel/32);
        return (th_row * 32) + (pixel % 32);
    }
}

static uint16_t blind_idx(uint16_t pixel)
{
    uint16_t row = (pixel/32) % 4;
    if (pixel > 511)
    {
        row = row + 4;
    }
    return (pixel % 32) + (row*32);
}

void gs_htpa_adjust_image(gs_htpa_image_t* image,
        gs_htpa_blind_image_t *blind,
        gs_htpa_th_data_t* th_data,
        uint16_t m_ptat,
        gs_htpa_adjusted_image_t* result)
{
    for (uint16_t i=0; i<1024; i++) {
        result->image[i] = (int16_t)image->image[i] - th_data->offset[th_idx(i)];
        result->image[i] = result->image[i] - (th_data->gradient[th_idx(i)] * m_ptat) / th_data->gradscalepow - blind->image[blind_idx(i)];
    }
}

gs_error_t gs_htpa_read_block(const gs_htpa_handle_t handle, uint8_t block, uint16_t *upper_data, uint16_t *lower_data)
{
    GS_CHECK_ARG(block < 4);
    GS_CHECK_ARG(upper_data != NULL);
    GS_CHECK_ARG(lower_data != NULL);
    uint8_t cmd[2];

    cmd[0] = HTPA_CONFIG_REG;
    cmd[1] = (block << 4) | START | WAKEUP;
    // send the command to read the block

    gs_error_t error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);

    if (error) {
        return error;
    }

    gs_time_sleep_ms(25);
    uint16_t attempts = 0x0;
    while (1)
    {
        bool status_ok;
        error = gs_htpa_is_expected_status(handle, block, 1, 0, &status_ok);
        
        if (error) {
            return error;
        }
        
        attempts++;
        
        if (status_ok) {
            break;
        }

        if (attempts > HTPA_READY_WAIT_ATTEMPTS) {
            return GS_ERROR_TIMEOUT;
        }

        gs_time_sleep_ms(2);
        
    }

    uint8_t block_upper = HTPA_BLOCK_UPPER;
    uint8_t block_lower = HTPA_BLOCK_LOWER;
    
    // Read the the ptat and block (1 uint16 + 128 * uint16) from the sensor
    error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address,
            &block_upper, 1, (uint8_t *)upper_data, 258, SPI_TIMEOUT);
    
    if (error) {
        return error;
    }

    // read the lower block (1 * uint16 mptat and 128 * uint16)
    error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address,
            &block_lower, 1, (uint8_t *)lower_data, 258, SPI_TIMEOUT);

    return error;
}

gs_error_t gs_htpa_read_grad_scale_pow(const gs_htpa_handle_t handle, gs_htpa_th_data_t* th_data)
{
    uint8_t cmd[2];
    cmd[0] = 0;
    cmd[1] = 0x08;
    uint8_t gradScale;
    gs_error_t error =  gs_i2c_master_transaction(
            handle.htpa_bus,
            handle.htpa_eeprom_address,
            cmd,
            2,
            &gradScale,
            1,
            SPI_TIMEOUT);
    th_data->gradscalepow = pow(2, gradScale);
    
    return error;
}

gs_error_t gs_htpa_read_factory_calibration(const gs_htpa_handle_t handle,
                                            uint8_t *mbit,
                                            uint8_t *ref_cal,
                                            uint8_t *bias,
                                            uint8_t *clock,
                                            uint8_t *bpa,
                                            uint8_t *pu)
{
    uint8_t data[16];
    uint8_t cmd[2];
    cmd[0] = 0x00;
    cmd[1] = 0x10;
    gs_error_t error = gs_i2c_master_transaction(
            handle.htpa_bus,
            handle.htpa_eeprom_address,
            cmd,
            2,
            data,
            16,
            SPI_TIMEOUT);
    
    if (error) {
        return error;
    }

    *mbit = data[0x0a] & 0xf;
    *ref_cal = (data[0x0a] & 0b110000) >> 4;
    *bias = data[0x0b];
    *clock = data[0x0c];
    *bpa = data[0x0d];
    *pu = data[0x0e];
    
    return GS_OK;
}

gs_error_t gs_htpa_read_th_offset(const gs_htpa_handle_t handle, gs_htpa_th_data_t *th_data)
{
    // The data is read in chunks of 128 bytes
    uint8_t cmd[2];
    // Start address of the th offset data in the eeprom
    uint16_t addr = 0x0F40;
    gs_error_t error;
    // memset(th_data, 0, 2048);
    for (uint8_t i = 0; i < 16; i++) {
        cmd[0] = addr >> 8;
        cmd[1] = addr & 0xff;
        error =  gs_i2c_master_transaction(
                handle.htpa_bus,
                handle.htpa_eeprom_address,
                cmd,
                2,
                ((uint8_t*)(&th_data->offset)) + (i*128),
                128,
                SPI_TIMEOUT);
        addr = addr + 128;
        
        if (error) {
            return error;
        }
    }

    for (uint16_t i=0; i<1024; i++) {
        th_data->offset[i] = util_letoh16(th_data->offset[i]);
    }
    
    return GS_OK;
}

gs_error_t gs_htpa_read_th_grad(const gs_htpa_handle_t handle, gs_htpa_th_data_t *th_data)
{
    // The data is read in chunks of 128 bytes
    uint8_t cmd[2];
    // The address of the th gradient data
    uint16_t addr = 0x0740;
    gs_error_t error;
    // memset(th_data, 0, 2048);
    for (uint8_t i = 0; i < 16; i++) {
        cmd[0] = addr >> 8;
        cmd[1] = addr & 0xff;
        error =  gs_i2c_master_transaction(
                handle.htpa_bus,
                handle.htpa_eeprom_address,
                cmd,
                2,
                ((uint8_t*)(&th_data->gradient)) + (i*128),
                128,
                SPI_TIMEOUT);
        addr = addr + 128;
        
        if (error) {
            return error;
        }
    }
    for (uint16_t i=0; i<1024; i++) {
        th_data->gradient[i] = util_letoh16(th_data->gradient[i]);
    }

    return GS_OK;
}

gs_error_t gs_htpa_read_blind_image(const gs_htpa_handle_t handle, gs_htpa_blind_image_t* image)
{
    gs_error_t error;
    uint8_t cmd[2];
    uint16_t attempts = 0;
    
    cmd[0] = HTPA_CONFIG_REG;
    cmd[1] = HTPA_CONFIG_START | HTPA_CONFIG_WAKEUP | HTPA_CONFIG_BLIND;
    
    error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address, cmd, 2, NULL, 0, SPI_TIMEOUT);
    
    if (error) {
        return error;
    }

    gs_time_sleep_ms(25);
    
    while (1) {
        bool status_ok;
        error = gs_htpa_is_expected_status(handle, 0, 1, 1, &status_ok);
        
        if (error) {
            return error;
        }

        attempts++;
        
        if (status_ok) {
            break;
        }

        if (attempts > HTPA_READY_WAIT_ATTEMPTS) {
            return GS_ERROR_TIMEOUT;
        }

        gs_time_sleep_ms(2);
    }

    uint8_t block_upper = HTPA_BLOCK_UPPER;
    uint8_t block_lower = HTPA_BLOCK_LOWER;
    uint16_t upper_data[129];
    uint16_t lower_data[129];
    
    error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address,
            &block_upper, 1, (uint8_t *)upper_data, 258, SPI_TIMEOUT);
    
    if (error) {
        return error;
    }

    error = gs_i2c_master_transaction(handle.htpa_bus, handle.htpa_address,
            &block_lower, 1, (uint8_t *)lower_data, 258, SPI_TIMEOUT);
    
    if (error) {
        return error;
    }

    for (uint16_t i=0; i<128; i++) {
        image->image[i] = util_ntohs(upper_data[i+1]);
        // The data is aligned in rows read from the end
        uint16_t row_nr = 7-(i/32);
        uint16_t row_start = row_nr*32;
        image->image[row_start+(i%32)] = util_ntohs(lower_data[i+1]);
    }
    
    return GS_OK;
}

gs_error_t gs_htpa_read_image(const gs_htpa_handle_t handle, gs_htpa_image_t *image, uint16_t *mean_ptat)
{

    uint16_t upper_data[129];
    uint16_t lower_data[129];
    gs_error_t error;
    *mean_ptat = 0;

    for (uint8_t block = 0; block < 4; block++)
    {
        error = gs_htpa_read_block(handle, block, upper_data, lower_data);
        if (error) {
            return error;
        }

        *mean_ptat += util_ntohs(upper_data[0]);
        *mean_ptat += util_ntohs(lower_data[0]);

        for (uint8_t i=0; i<128; i++) {
            image->image[(128*block)+i] = util_ntohs(upper_data[i+1]);
            // The data is aligned in rows read from the end
            uint16_t row = 31-((i/32)+(block*4));
            image->image[(row*32)+(i%32)] = util_ntohs(lower_data[i+1]);
        }
    }

    *mean_ptat = *mean_ptat/8;

    return GS_OK;
}
