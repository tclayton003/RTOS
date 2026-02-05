/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/mpu3300.h>
#include "local.h"
#include <math.h>
#include <gs/util/drivers/i2c/master.h>
#include <gs/util/time.h>

#define MPU3300_I2C_ADDR		0x68

/* Registers */
#define MPUREG_SELF_TEST_X		0x0D
#define MPUREG_SELF_TEST_Y		0x0E
#define MPUREG_SELF_TEST_Z		0x0F
#define MPUREG_SMPLRT_DIV		0x19
#define MPUREG_CONFIG			0x1A
#define MPUREG_GYRO_CONFIG		0x1B
#define MPUREG_TEMP_OUT_H		0x41
#define MPUREG_TEMP_OUT_L		0x42
#define MPUREG_GYRO_XOUT_H		0x43
#define MPUREG_GYRO_XOUT_L		0x44
#define MPUREG_GYRO_YOUT_H		0x45
#define MPUREG_GYRO_YOUT_L		0x46
#define MPUREG_GYRO_ZOUT_H		0x47
#define MPUREG_GYRO_ZOUT_L		0x48
#define MPUREG_SIGNAL_PATH_RESET 	0x68
#define MPUREG_USER_CTRL		0x6A
#define MPUREG_PWR_MGMT_1		0x6B
#define MPUREG_PWR_MGMT_2		0x6C
#define MPUREG_WHO_A_I			0x75

static int mpu3300_full_scale_reading = 0;

gs_error_t gs_mpu3300_read_raw(gs_mpu3300_gyro_raw_t * gyro_reading)
{
    uint8_t cmd[1];
    uint8_t res[6];

    cmd[0] =  MPUREG_GYRO_XOUT_H;
    gs_error_t result = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 1, res, 6, 20);
    gyro_reading->gyro_x = (int16_t) ((res[0] << 8) + res[1]);
    gyro_reading->gyro_y = (int16_t) ((res[2] << 8) + res[3]);
    gyro_reading->gyro_z = (int16_t) ((res[4] << 8) + res[5]);

    return result;
}

gs_error_t gs_mpu3300_selftest(void)
{
    uint8_t cmd[2];
    uint8_t res[6];
    int i;

    uint8_t factory_trim[3];
	
    gs_mpu3300_gyro_raw_t gyro_with_self_test;
    gs_mpu3300_gyro_raw_t gyro_no_self_test;

    int32_t gyro_self_test_res[3];

    float ft_x;
    float ft_y;
    float ft_z;

    float tmp;


    /* First read register to get current values */
    cmd[0] = MPUREG_GYRO_CONFIG;
    gs_error_t error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 1, res, 1, 1000);
    if (error) {
        return error;
    }

    /* Set the selftest bits */
    cmd[1] = res[0] | 0b11100000;
    error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 2, NULL, 0, 1000);
    if (error) {
        return error;
    }

    /* Get factory trim values */
    cmd[0] =  MPUREG_SELF_TEST_X;
    error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 1, factory_trim, 3, 1000);
    if (error) {
        return error;
    }

    /* Wait for data and then read gyro, 200 ms required to allow gyro to settle */
    gs_time_sleep_ms(200);

    error = gs_mpu3300_read_raw(&gyro_with_self_test);
    if (error) {
        return error;
    }

    /* Now read gyro without selftest */

    /* Clear the selftest bits */
    cmd[0] = MPUREG_GYRO_CONFIG;
    cmd[1] = res[0] & 0b00011111;
    error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 2, NULL, 0, 1000);
    if (error) {
        return error;
    }

    /* Wait for data and then read gyro, 200 ms required to allow gyro to settle */
    gs_time_sleep_ms(200);

    error = gs_mpu3300_read_raw(&gyro_no_self_test);
    if (error) {
        return error;
    }

    for (i = 0; i < 3; i++) {
        factory_trim[i] = factory_trim[i] & 0x1f;
        
        if (factory_trim[i] == 0) {
            log_error("Factory Trim value is 0, should have been set at the manufacturer of the MPU3300, failed selftest");
            return GS_ERROR_DATA;
        }
    }

    /* Selftest calculations */
    ft_x = 25 * 145.6 * powf(1.046, (float) factory_trim[0] - 1);
    ft_y = -25 * 145.6 * powf(1.046, (float) factory_trim[1] - 1);
    ft_z = 25 * 145.6 * powf(1.046, (float) factory_trim[2] - 1);

    gyro_self_test_res[0] = gyro_with_self_test.gyro_x - gyro_no_self_test.gyro_x;
    gyro_self_test_res[1] = gyro_with_self_test.gyro_y - gyro_no_self_test.gyro_y;
    gyro_self_test_res[2] = gyro_with_self_test.gyro_z - gyro_no_self_test.gyro_z;

    tmp = (gyro_self_test_res[0] - ft_x) / ft_x;
    if (tmp > 0.14 || tmp < -0.14) {
        log_error("Gyro_x faild selftest: %f", tmp);
        return GS_ERROR_DATA;
    }

    tmp = (gyro_self_test_res[1] - ft_y) / ft_y;
    if (tmp > 0.14 || tmp < -0.14) {
        log_error("Gyro_y faild selftest: %f", tmp);
        return GS_ERROR_DATA;
    }

    tmp = (gyro_self_test_res[2] - ft_z) / ft_z;
    if (tmp > 0.14 || tmp < -0.14) {
        log_error("Gyro_z faild selftest: %f", tmp);
        return GS_ERROR_DATA;
    }

    return GS_OK;
}

gs_error_t gs_mpu3300_init(gs_mpu3300_bandwidth_t bandwidth, gs_mpu3300_full_scale_reading_t full_scale)
{
    uint8_t cmd[2];

    /* Set full scale range */
    cmd[0] = MPUREG_GYRO_CONFIG;
    if (full_scale == GS_MPU3300_FSR_225) {
        cmd[1] = 0;
    } else {
        cmd[1] = 1 << 3;
    }
    gs_error_t error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 2, NULL, 0, 100);
    if (error) {
        return error;
    }

    mpu3300_full_scale_reading = full_scale;

    /* set bandwidth */
    cmd[0] = MPUREG_CONFIG;
    cmd[1] = bandwidth;
    error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 2, NULL, 0, 100);
    if (error) {
        return error;
    }

    /* Set clock source to be X axis gyro pll. The gyro can not be in low power 
     * mode when using pll so switch to internal clock before low power mode */
    cmd[0] = MPUREG_PWR_MGMT_1;
    cmd[1] = 1;
    error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 2, NULL, 0, 100);
    if (error) {
        return error;
    }

    return GS_OK;
}

gs_error_t gs_mpu3300_reset(void)
{
    uint8_t cmd[2];
    cmd[0] =  MPUREG_PWR_MGMT_1;
    cmd[1] = 1 << 7;
    return gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 2, NULL, 0, 100);
}

gs_error_t gs_mpu3300_sleep(bool sleep)
{
    uint8_t cmd[2];
    uint8_t res[1];

    cmd[0] =  MPUREG_PWR_MGMT_1;
    gs_error_t error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 1, res, 1, 100);
    if (error) {
        return error;
    }

    cmd[1] = res[0];

    if (sleep == false) {
        cmd[1] &= ~(1 << 6);
    } else {
        cmd[1] |= (1 << 6);
    }

    return gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 2, NULL, 0, 100);
}

gs_error_t gs_mpu3300_read_gyro(gs_mpu3300_gyro_t * gyro_readings)
{
    float scale;	// LSB/degrees/s
    if (mpu3300_full_scale_reading == GS_MPU3300_FSR_225) {
        scale = 145.6;
    } else {
        scale = 72.8;
    }

    gs_mpu3300_gyro_raw_t gyro;
    gs_error_t error = gs_mpu3300_read_raw(&gyro);
    if (error == GS_OK) {
        gyro_readings->gyro_x = gyro.gyro_x / scale;
        gyro_readings->gyro_y = gyro.gyro_y / scale;
        gyro_readings->gyro_z = gyro.gyro_z / scale;
    }

    return error;
}

gs_error_t gs_mpu3300_read_temp(float * temp)
{
    uint8_t cmd[1];
    uint8_t res[2];

    cmd[0] =  MPUREG_TEMP_OUT_H;
    gs_error_t error = gs_i2c_master_transaction(2, MPU3300_I2C_ADDR, cmd, 1, res, 2, 20);
    if (error == GS_OK) {
        *temp = ((int16_t) ((res[0] << 8) + res[1])) / 340. + 36.53;
    }

    return error;
}
