#ifndef INCLUDE_GS_THIRDPARTY_MG3XX_MG3XX_H_
#define INCLUDE_GS_THIRDPARTY_MG3XX_MG3XX_H_
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   IMU M-G3xx driver (Gyroscope, accelerometer and temperature sensor)
*/

#include <gs/util/error.h>
#include <gs/util/drivers/spi/master.h>
#include <gs/thirdparty/gyro.h>

/**
   Digital filter.

   This filter type both controls the digital low pass filter and down sampling,
   as these must have a valid combination.
 */
typedef enum gs_mg3xx_filter {
    /**
       No filter and down sampled to 1000 samples per second.
     */
    NO_FILTER_1000SPS = (0x00 << 8) | (0x01),
    /**
       FIR kaiser filter with 32 taps and cutoff frequency at 50 hz and down sampled to 125 samples per second.
     */
    KAISER_FC50_TAP_32_125SPS = (0x08 << 8) | (0x04),
} gs_mg3xx_filter_t;

/**
   M-G3xx configuration.
 */
typedef struct gs_mg3xx_conf {
    /**
       SPI slave (preconfigured).
     */
    uint8_t slave;
    /**
       Digital filter type, @see gs_mg3xx_filter_t.
     */
    gs_mg3xx_filter_t filter;
} gs_mg3xx_conf_t;

/**
   Register M-G3xx GOSH commands.

   @return_gs_error_t
 */
gs_error_t gs_mg3xx_register_commands(void);

/**
   Get the default SPI config for a M-G3xx device.

   @param[out] config Cross platform config struct.
   @param[in]  driver_data Specific driver data - not used.
   @return_gs_error_t
 */
gs_error_t gs_mg3xx_spi_default_config(gs_spi_master_slave_config_t * config, void * driver_data);

/**
   Initialize M-G3xx device.

   @param[in]  handle Handle to be initialized.
   @param[in]  config M-G3xx specific configuration, see gs_mg3xx_conf_t.
   @param[in]  driver_data Specific driver data - not used.
   @return_gs_error_t
 */
gs_error_t gs_mg3xx_init(uint8_t handle, const gs_mg3xx_conf_t * config, void * driver_data);

/**
   Self test.
   Perform internal self test.

   @param[in]  handle Handle for M-G3xx device.
   @param[in]  driver_data Specific driver data - not used.
   @return_gs_error_t
 */
gs_error_t gs_mg3xx_self_test(uint8_t handle, void * driver_data);

/**
   Get product ID.
   Product ID is the product type, ex. "G364PDCA".

   @param[in]  handle Handle for M-G3xx device.
   @param[out] id_buffer Buffer for product ID.
   @param[in]  buffer_len Length of buffer, min length is 9 bytes.
   @param[in]  driver_data Specific driver data - not used.
   @return_gs_error_t
 */
gs_error_t gs_mg3xx_get_product_id(uint8_t handle, char * id_buffer, size_t buffer_len, void * driver_data);

/**
   Get serial number.
   Get the unique device serial number, ex. "00000060".

   @param[in]  handle Handle for M-G3xx device.
   @param[out] serial_buffer Buffer for serial number.
   @param[in]  buffer_len Length of buffer, min length is 9 bytes.
   @param[in]  driver_data Specific driver data - not used.
   @return_gs_error_t
 */
gs_error_t gs_mg3xx_get_serial(uint8_t handle, char * serial_buffer, size_t buffer_len, void * driver_data);

/**
   Get firmware version.
   Get the firmware version of the device, ex. 9488.

   @param[in]  handle Handle for M-G3xx device.
   @param[out] version Firmware version.
   @param[in]  driver_data Specific driver data - not used.
   @return_gs_error_t
 */
gs_error_t gs_mg3xx_get_version(uint8_t handle, uint16_t * version, void * driver_data);

/**
   Start sampling.

   Starts the sampling procedure in the chosen mode.
   @see gs_gyro_get_sample_t to get sample.
   @note GS_GYRO_SINGLE_SAMPLE mode uses ~30% less power than GS_GYRO_FREE_RUNNING mode.

   @param[in]  handle Gyroscope handle.
   @param[in]  mode Sample mode, @see gs_gyro_sample_mode_t
   @param[in]  driver_data data to specific driver.
   @return_gs_error_t
*/
gs_error_t gs_mg3xx_start_sampling(uint8_t handle, gs_gyro_sample_mode_t mode, void * driver_data);

/**
   Get sample.

   Get the latest sample if any (not all ready sampled) sample is ready.
   @note If no sample is ready it returns error, @see gs_gyro_max_norm_sample_time_t.

   @param[in]  handle Gyroscope handle.
   @param[in]  sample Gyroscope measurement, @see gs_gyro_sample_t.
   @param[in]  driver_data data to specific driver.
   @return_gs_error_t
*/
gs_error_t gs_mg3xx_get_sample(uint8_t handle, gs_gyro_sample_t * sample, void * driver_data);

/**
   Get max normal sampling time.

   Get the normal maximum time at takes the device to get a sample ready.
   @note This only applies to GS_GYRO_SINGLE_SAMPLE mode.

   @param[in]  handle Gyroscope handle.
   @param[out] max_sample_time_ms maximum normal sample time.
   @param[in]  driver_data data to specific driver.
   @return_gs_error_t
*/
gs_error_t gs_mg3xx_max_norm_sample_time(uint8_t handle, uint32_t * max_sample_time_ms, void * driver_data);

#endif /* INCLUDE_GS_THIRDPARTY_MG3XX_MG3XX_H_ */
