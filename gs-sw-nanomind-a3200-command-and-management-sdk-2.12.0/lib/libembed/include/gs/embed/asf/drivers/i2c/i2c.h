#ifndef _GS_ASF_DRIVERS_I2C_I2C_H_
#define _GS_ASF_DRIVERS_I2C_I2C_H_
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   ASF: I2C interface (initialization)
*/

#include <gs/util/drivers/i2c/common.h>

/**
   Maximum transfer length on I2C
*/
#define I2C_MTU 	256

/**
   I2C bus mode.
*/
typedef enum {
    GS_AVR_I2C_MASTER = GS_I2C_MASTER,
    GS_AVR_I2C_MULTIMASTER = GS_I2C_MULTI_MASTER,
    GS_AVR_I2C_SLAVE = GS_I2C_SLAVE,
} gs_avr32_i2c_mode_t;

/**
   Initialise the I2C driver

   @param[in] handle Which I2C bus (if more than one exists)
   @param[in] mode I2C device mode. Must be either GS_AVR32_I2C_MASTER or GS_AVR32_I2C_MULTIMASTER
   @param[in] addr Own slave address
   @param[in] bps Bits per second
   @return_gs_error_t
*/
gs_error_t gs_avr_i2c_init(uint8_t handle, gs_avr32_i2c_mode_t mode, uint8_t addr, uint32_t bps);

/**
   Reset and disable TWIS hardware, requires reinitialization through twis_init

   @param[in] handle Handle to the device
*/
void twis_reset(uint8_t handle);

#endif
