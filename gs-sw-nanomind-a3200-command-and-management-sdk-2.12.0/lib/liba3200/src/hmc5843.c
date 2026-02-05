/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/hmc5843.h>
#include "local.h"
#include <math.h>
#include <gs/util/drivers/i2c/master.h>
#include <gs/util/time.h>

/** Default values */
static bool is_initialised;
static int meas = MAG_MEAS_NORM;
static int gain = MAG_GAIN_1_0;
static int rate = MAG_RATE_10;
static int defaultmode = MAG_MODE_IDLE;
static float scale = 1/1.3;


// Write register "reg" with value "val"
static gs_error_t gs_hmc5843_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t txdata[2];
    txdata[0] = reg;
    txdata[1] = val;

    return gs_i2c_master_transaction(2,0x1E, txdata, 2,NULL,0,2);
}

// Read register "reg" into value "val"
// Returns -1 on error and 0 on no-error
static gs_error_t gs_hmc5843_read_reg(uint8_t reg, uint8_t *val)
{
    uint8_t txdata[1];
    txdata[0] = reg;

    return gs_i2c_master_transaction(2,0x1E, txdata, 1,val,1,2);
}

gs_hmc5843_gain_t gs_hmc5843_get_gain()
{
    return gain;
}

gs_hmc5843_rate_t gs_hmc5843_get_rate()
{
    return rate;
}

gs_hmc5843_meas_t gs_hmc5843_get_meas()
{
    return meas;
}

gs_error_t gs_hmc5843_set_conf(gs_hmc5843_rate_t new_rate, gs_hmc5843_meas_t new_meas, gs_hmc5843_gain_t new_gain)
{
    rate = new_rate;
    meas = new_meas;
    gain = new_gain;

    switch(new_gain)
    {
        case MAG_GAIN_0_7:
            scale = 1/1.62;
            break;
        case MAG_GAIN_1_0:
            scale = 1/1.3;
            break;
        case MAG_GAIN_1_5:
            scale = 1/0.97;
            break;

        default:
        case MAG_GAIN_2_0:
            scale = 1/0.78;
            break;

        case MAG_GAIN_3_2:
            scale = 1/0.53;
            break;
        case MAG_GAIN_3_8:
            scale = 1/0.46;
            break;
        case MAG_GAIN_4_5:
            scale = 1/0.39;
            break;
        case MAG_GAIN_6_5:
            scale = 1/0.28;
            break;
    }

    gs_hmc5843_write_reg(0x00, (rate << 2) | meas);

    return gs_hmc5843_write_reg(0x01, (gain << 5));
}

gs_error_t gs_hmc5843_set_mode(gs_hmc5843_mode_t mode)
{
    return gs_hmc5843_write_reg(0x02, mode);
}

// Setup i2c to hmc5843
gs_error_t gs_hmc5843_init(void)
{
    if(is_initialised)
        return GS_OK;

    /* Turn-on-time: 200 us */
    /*UPIO_ECR = 1;
      UPIO_MDDR = 0;
      UPIO_OER = 0x00000001;
      if (UPIO_PDSR & 0x00000001) {
      UPIO_CODR = 0x00000001;
      vTaskDelay(configTICK_RATE_HZ * 0.5);
      }
      UPIO_SODR = 0x00000001;
      vTaskDelay(configTICK_RATE_HZ * 0.5);*/

    /* Setup I2C */
    //i2c_init(2, I2C_MASTER, 0x06, 60, 5, 5, NULL);

    //vTaskDelay(configTICK_RATE_HZ * 0.2);

    is_initialised = true;

    gs_hmc5843_set_conf(rate, meas, gain);

    return gs_hmc5843_set_mode(defaultmode);
}

// Setup i2c to hmc5843
gs_error_t gs_hmc5843_init_force(void)
{
    /* Turn-on-time: 200 us */
    /*UPIO_ECR = 1;
      UPIO_MDDR = 0;
      UPIO_OER = 0x00000001;
      if (UPIO_PDSR & 0x00000001) {
      UPIO_CODR = 0x00000001;
      vTaskDelay(configTICK_RATE_HZ * 0.5);
      }
      UPIO_SODR = 0x00000001;*/
    vTaskDelay(configTICK_RATE_HZ * 0.5);

    /* Setup I2C */
    //i2c_init(2, I2C_MASTER, 0x06, 60, 5, 5, NULL);

    //vTaskDelay(configTICK_RATE_HZ * 0.2);

    is_initialised = true;

    gs_hmc5843_set_conf(rate, meas, gain);

    return gs_hmc5843_set_mode(defaultmode);
}

gs_error_t gs_hmc5843_read(gs_hmc5843_data_t * data)
{
    uint8_t txdata[1];
    uint8_t rxdata[6];
    txdata[0] = 0x03;
    gs_error_t retval = gs_i2c_master_transaction(2,0x1E, txdata, 1,rxdata,6,5);
    if (retval == GS_OK) {

        /* Data is returned in a slave-frame structure */
        int16_t tmpx = rxdata[0] << 8 | rxdata[1];
        int16_t tmpy = rxdata[2] << 8 | rxdata[3];
        int16_t tmpz = rxdata[4] << 8 | rxdata[5];

        data->x = (float) tmpx * scale;
        data->y = (float) tmpy * scale;
        data->z = (float) tmpz * scale;
    }
    return retval;
}

gs_error_t gs_hmc5843_read_raw(gs_hmc5843_data_t * data)
{
    uint8_t txdata[1];
    uint8_t rxdata[6];
    txdata[0] = 0x03;
    gs_error_t retval = gs_i2c_master_transaction(2,0x1E, txdata, 1,rxdata,6,5);
    if (retval == GS_OK) {

        /* Data is returned in a slave-frame structure */
        int16_t tmpx = rxdata[0] << 8 | rxdata[1];
        int16_t tmpy = rxdata[2] << 8 | rxdata[3];
        int16_t tmpz = rxdata[4] << 8 | rxdata[5];

        data->x = (float) tmpx;
        data->y = (float) tmpy;
        data->z = (float) tmpz;
    }
    return retval;
}

gs_error_t gs_hmc5843_read_test(gs_hmc5843_data_t * data)
{
    gs_error_t error = gs_hmc5843_set_mode(MAG_MODE_SINGLE);
    if (error == GS_OK) {
        // Wait
        vTaskDelay(configTICK_RATE_HZ * 0.016);

        error = gs_hmc5843_read(data);
    }
    return error;
}

gs_error_t gs_hmc5843_selftest(gs_hmc5843_data_t * data)
{
    const uint16_t tolmin = 550 - (0.25 * 550);
    const uint16_t tolmax = 550 + (0.25 * 550);

    gs_error_t res = gs_hmc5843_set_conf(gs_hmc5843_get_rate(), MAG_MEAS_POS, gs_hmc5843_get_gain());
    if (res != GS_OK) {
        log_error("gs_hmc5843_set_conf(%i)", res);
    }

    res = gs_hmc5843_set_mode(MAG_MODE_SINGLE);
    if (res != GS_OK) {
        log_error("gs_hmc5843_set_mode(%i)", res);
    }
    res = gs_hmc5843_read_test(data);
    if (res != GS_OK) {
        log_error("gs_hmc5843_read_test(%i)", res);
    }

    //Check results
    if (data->x < tolmin || data->x > tolmax){
        log_error("Mag_x failed selftest: %f, expected value %d < x < %d", data->x,tolmin,tolmax);
        res = GS_ERROR_DATA;
    }

    if (data->y < tolmin || data->y > tolmax){
       log_error("Mag_y failed selftest: %f, expected value %d < x < %d", data->y,tolmin,tolmax);
        res = GS_ERROR_DATA;
    }

    if (data->z < tolmin || data->z > tolmax){
       log_error("Mag_z failed selftest: %f, expected value %d < x < %d", data->z,tolmin,tolmax);
        res = GS_ERROR_DATA;
    }

    gs_error_t res2 = gs_hmc5843_set_conf(gs_hmc5843_get_rate(), MAG_MEAS_NORM, gs_hmc5843_get_gain());
    if (res2 != GS_OK) {
        log_error("gs_hmc5843_set_conf(%i)", res);
        return res2;
    }

    return res;
}

gs_error_t gs_hmc5843_read_single(gs_hmc5843_data_t * data)
{
    gs_error_t error = gs_hmc5843_set_mode(MAG_MODE_CONTINUOUS);
    if (error == GS_OK) {
        int timeout=13;
        uint8_t status;

        while(timeout--) {
            vTaskDelay(1);
            gs_hmc5843_read_reg(0x09, &status);
            if ( (status&0x01) == 1)
                break;
        }
        gs_hmc5843_set_mode(MAG_MODE_IDLE);

        if (timeout == 0)
            return GS_ERROR_TIMEOUT;

        error = gs_hmc5843_read(data);
    }
    return error;
}
