#ifndef GS_EMBED_ASF_SAMC_DRIVERS_I2C_I2C_H
#define GS_EMBED_ASF_SAMC_DRIVERS_I2C_I2C_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/* --- includes ----------------------------------------------------------*/

#include <gs/util/drivers/i2c/slave.h>

/* --- definitions -------------------------------------------------------*/

gs_error_t gs_samc_i2c_slave_init(uint8_t device, const gs_i2c_bus_config_t * config);

#endif
