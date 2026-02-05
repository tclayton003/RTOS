#ifndef INCLUDE_GS_THIRDPARTY_TMP100_TMP100_H_
#define INCLUDE_GS_THIRDPARTY_TMP100_TMP100_H_

/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

/**
   @file

   @brief Driver for the TI TMP100 temperature sensor.

   This is the driver for the Texas Instruments TMP100 temperature sensor.
   It is an I2C connected sensor and the driver will use the sensor at its
   highest resolution.
*/


#include <gs/util/types.h>
#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Default TWI address */
#define DEFAULT_TMP100_TWI_HANDLER 2
/** Default TMP100 I2C address */
#define DEFAULT_TMP100_ADDRESS 0x48

/**
 * Handle for a TMP100 instance.
 */
typedef struct {
    /** TWI handler address. Default value is 2. */
    uint8_t twi_handler;
    /** I2C address of the sensor. Default value is 0x48. Refer the sensor for the actual address. */
    uint8_t i2c_addr_temp;
} gs_tmp100_handle_t;

/**
 * Initialize a tmp100 handle with specific parameters.
 * @param[in] twi_handler Temperature sensor twi/i2c handler.
 * @param[in] device_temp_i2c_addr I2C address of the sensor.
 * @return handle to a TMP100 device.
 */
gs_tmp100_handle_t gs_tmp100_node(uint8_t twi_handler, uint8_t device_temp_i2c_addr);

/**
 * Initialize the tmp100 with default settings.
 */
gs_tmp100_handle_t gs_tmp100_node_default();

/**
 * Setup tmp100 device for maximum resolution.
 * @param[in] handle pre-configured handle.
 * @return result of I2C transaction. GS_OK if successful.
 */
gs_error_t gs_tmp100_init(const gs_tmp100_handle_t handle);

/**
 * Read the temperature sensor.
 * @param[in] handle pre-configured handle.
 * @param[out] temp pointer to temperature in degC.
 * @return GS_OK if successful.
 */
gs_error_t gs_tmp100_read_temp(const gs_tmp100_handle_t handle, float *temp);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_GS_THIRDPARTY_TMP100_TMP100_H_ */
