/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
#ifndef GS_INCLUDE_THIRDPARTY_INA226_INA226_H
#define GS_INCLUDE_THIRDPARTY_INA226_INA226_H
/**
   @file

   INA226 driver.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  Initialize ina226 device.

  @param[in] bus Id of the I2C bus device is attached to
  @param[in] addr I2C addr of the device
  @param[out] handle Pointer to a handle to be used in future calls to the driver

  @return_gs_error_t
*/
gs_error_t gs_ina226_init(const uint8_t bus, const uint8_t addr, uint8_t * const handle);

/**
  Sample bus and shunt voltages

  @param[in] handle Handle returned by @ref gs_ina226_init

  @return_gs_error_t
*/
gs_error_t gs_ina226_sample(const uint8_t handle);

/**
   return last sampled values of bus and shunt voltages

   @param[in]  handle Handle returned by @ref gs_ina226_init
   @param[out] bus_volt Bus voltage in mV
   @param[out] shunt_volt Shunt voltage in uV

   @return_gs_error_t
*/
gs_error_t gs_ina226_read(const uint8_t handle, uint16_t * const bus_volt, int32_t * const shunt_volt);

/**
   Returns bus and shunt voltages. If the \p cached flag is false, a new value is sampled from the device.
   If \p cached is true, the latest sample is returned without reading from the device.

   @param[in]  handle Handle returned by @ref gs_ina226_init
   @param[out] bus_volt Bus voltage in mV
   @param[out] shunt_volt Shunt voltage in uV
   @param[in]  cached Read cached voltages

   @return_gs_error_t
*/
gs_error_t gs_ina226_read_voltage(const uint8_t handle, uint16_t * const bus_volt, int32_t * const shunt_volt, bool cached);

/**
   Returns current. If the \p cached flag is false, a new value is sampled from the device.
   If \p cached is true, the latest sample is returned without reading from the device.

   Current measurements are only available if a calibration has been written using @ref gs_ina226_write_calibration

   @param[in]  handle Handle returned by @ref gs_ina226_init
   @param[out] current Shunt current in uA
   @param[in]  cached Read cached current

   @return_gs_error_t
*/
gs_error_t gs_ina226_read_current(const uint8_t handle, int32_t * const current, bool cached);

/**
   Returns power. If the \p cached flag is false, a new value is sampled from the device.
   If \p cached is true, the latest sample is returned without reading from the device.

   Power measurements are only available if a calibration has been written using @ref gs_ina226_write_calibration

   @param[in]  handle Handle returned by @ref gs_ina226_init
   @param[out] power Shunt power in uW
   @param[in]  cached Read cached power

   @return_gs_error_t
*/
gs_error_t gs_ina226_read_power(const uint8_t handle, int32_t * const power, bool cached);

/**
   Returns all parameters. If the \p cached flag is false, a new value is sampled from the device.
   If \p cached is true, the latest sample is returned without reading from the device.

   Current and power measurements are only available if a calibration has been written using @ref gs_ina226_write_calibration

   @param[in]  handle     Handle returned by @ref gs_ina226_init
   @param[out] bus_volt   Bus voltage in mV - can be NULL
   @param[out] shunt_volt Shunt voltage in uV - can be NULL
   @param[out] current    Shunt current in uA - can be NULL
   @param[out] power      Shunt power in uW - can be NULL
   @param[in]  cached     Read cached values

   @return_gs_error_t
*/
gs_error_t gs_ina226_read_parameters(const uint8_t handle, uint16_t * const bus_volt, int32_t * const shunt_volt,
                                     int32_t * const current, int32_t * const power, bool cached);

/**
 Set the calibration value for current and power measurements

 @param[in] handle Handle returned @ref gs_ina226_init
 @param[in] calibration Calibration word
 @param[in] current_lsb Current LSB in uA corresponding to the calibration

 @return_gs_error_t
*/
gs_error_t gs_ina226_write_calibration(const uint8_t handle, uint16_t calibration, uint16_t current_lsb);

#ifdef __cplusplus
}
#endif
#endif
