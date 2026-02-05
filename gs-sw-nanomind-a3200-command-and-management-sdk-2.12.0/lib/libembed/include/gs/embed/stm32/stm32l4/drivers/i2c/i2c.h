#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_I2C_I2C_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_I2C_I2C_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
    @file

    @brief STM32L4 I2C API

   This API includes initialization routine of I2C.
   The cross platform I2C API holds functions for send/receive.
*/

#include <gs/util/drivers/i2c/master.h>

/**
   Initialize I2C device

   @note The speed must be in span [100000 bps : 400000 bps]

   @param[in] device STM32 I2C device (ex. I2C1: device = 1)
   @param[in] config Configuration for I2C device
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_i2c_init(uint8_t device, const gs_i2c_bus_config_t * config);

#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_I2C_I2C_H_ */
