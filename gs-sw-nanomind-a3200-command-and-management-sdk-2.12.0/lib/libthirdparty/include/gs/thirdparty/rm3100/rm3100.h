#ifndef GS_THIRDPARTY_RM3100_RM3100_H
#define GS_THIRDPARTY_RM3100_RM3100_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Driver for RM3100 Magnetometer and temperature sensor.

   Datasheet: https://www.terraelectronica.ru/pdf/show?pdf_file=%252Fds%252Fpdf%252FR%252FRM3100.pdf
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Measurement from the rm3100 converted into mGuass
 */
typedef struct {
    //! Field strength in the X direction
    float x;
    //! Field strength in the Y direction
    float y;
    //! Field strength in the Z direction
    float z;
    //! data valid = 1, else 0.
    int valid;
} gs_rm3100_data_t;

/**
   Raw data.
*/
typedef struct {
    //! X
    int32_t x;
    //! Y
    int32_t y;
    //! Z
    int32_t z;
    //! data valid = 1, else 0.
    uint8_t valid;
} gs_rm3100_rawdata_t;

/**
   Update/sample rate.
*/
typedef enum {
    //! 600 Hz
    RM3100_RATE_600 = 0x92,
    //! 300 Hz
    RM3100_RATE_300 = 0x93,
    //! 150 Hz
    RM3100_RATE_150 = 0x94,
    //! 75 Hz
    RM3100_RATE_75 = 0x95,
    //! 37 Hz
    RM3100_RATE_37 = 0x96,
} gs_rm3100_rate_t;

/**
   RM3100 handle.
*/
typedef struct {
    //! I2C device
    uint8_t twi_handler;
    //! I2C address of magnetometer
    uint8_t i2c_addr;
    //! I2C address of temperature sensor
    uint8_t i2c_addr_temp;
} gs_rm3100_handle_t;

/**
 * Setup rm3100 device
 * @param handle preconfigured handle
 * @param rate sample rate
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_init(const gs_rm3100_handle_t handle, gs_rm3100_rate_t rate);

/**
 * Used for setting device address. If this function is not called the address is set to 0x20
 * @param twi_handler Magnetometer twi/i2c handler
 * @param device_i2c_addr Magnetometer device i2c addr.
 * @param device_temp_i2c_addr Temperature device i2c addr.
 * @return handle
 */
gs_rm3100_handle_t gs_rm3100_node(uint8_t twi_handler, uint8_t device_i2c_addr, uint8_t device_temp_i2c_addr);

/**
 * Configure the rm3100 with default settings
 * @return handle
 */
gs_rm3100_handle_t gs_rm3100_node_default();


/**
 * Configure the rm3100 with defautl TWI handler
 * @param device_i2c_addr Magnetometer device i2c addr.
 * @param device_temp_i2c_addr Temperature device i2c addr.
 * @return handle
 */
gs_rm3100_handle_t gs_rm3100_node_setup(uint8_t device_i2c_addr, uint8_t device_temp_i2c_addr);


/**
 * Perform single measurement
 * @param handle handle
 * @param data returns data in mG
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_single_meas(const gs_rm3100_handle_t handle, gs_rm3100_data_t *data);


/**
 * Start a single measurement
 * @param handle handle
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_start_single_meas(const gs_rm3100_handle_t handle);


/**
 * Poll a single measurement previously started
 * @param handle handle
 * @param data: returns data in mGt
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_poll_single_meas(const gs_rm3100_handle_t handle, gs_rm3100_data_t *data);

/**
 * Enabled or disable continuous sampling on selected axis
 * @param handle handle
 * @param enable if set to one the command will enable continuous meas and if set to zero it will disable it.
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_continuous_meas_setup(const gs_rm3100_handle_t handle, uint8_t enable);

/**
 * Read the measurement registers
 * @param handle handle
 * @param rawdata pointer to raw data struct
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_read_meas_raw(const gs_rm3100_handle_t handle, gs_rm3100_rawdata_t *rawdata);

/**
 * Read the measurement registers
 * @param handle handle
 * @param data pointer to data struct (mG)
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_read_meas(const gs_rm3100_handle_t handle, gs_rm3100_data_t *data);

/**
 * Read the status register
 * @param handle handle
 * @param status data ready 0=no, 1=yes
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_drdy(const gs_rm3100_handle_t handle, uint8_t * status);


/**
 * Read sensor temperature
 * @param handle handle
 * @param temp pointer to temperature in degC
 * @return_gs_error_t
 */
gs_error_t gs_rm3100_read_temp(const gs_rm3100_handle_t handle, float *temp);

/**
   Register commands.
   @return_gs_error_t
*/
gs_error_t gs_rm3100_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
