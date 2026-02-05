#ifndef HTPD32X32D_H
#define HTPD32X32D_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * @file
 *
 * Driver for the Heimann Thermophile Array HTPA32x32dR1L2
*/


#include <gs/util/error.h>


#define HTPA_DEFAULT_ADDRESS 0x1A
#define HTPA_DEFAULT_EEPROM_ADDRESS 0x50

/**
 * Configuration settings of the i2c pull-ups
 */
typedef enum {
    GS_HTPA_100K = 8,
    GS_HTPA_50K = 4,
    GS_HTPA_10K = 2,
    GS_HTPA_1K = 1
} gs_htpa_pullup_t;

typedef struct {
    uint8_t htpa_address;
    uint8_t htpa_bus;
    uint8_t htpa_eeprom_address;
} gs_htpa_handle_t;

/**
 * The 32x32 image captured by the sensor
 */
typedef struct {
    uint16_t image[1024];
} gs_htpa_image_t;

/**
 * The adjusted output image
 */
typedef struct {
    int16_t image[1024];
} gs_htpa_adjusted_image_t;

/**
 * status struct representing the status register of the sensor
 */
typedef struct {
    bool eoc;
    bool blind;
    bool vdd_meas;
    uint8_t block;
} gs_htpa_status_t;

/**
 * The blind image captured by the sensor.
 * Should be captured for every 10 frames or so.
 */
typedef struct {
    uint16_t image[256];
} gs_htpa_blind_image_t;

/**
 * Struct containing the thermal offset and gradient compensation values from the eeprom
 */
typedef struct {
    uint8_t id;
    uint32_t gradscalepow;
    int16_t offset[1024];
    int16_t gradient[1024];
} gs_htpa_th_data_t;

/**
 * Configure the communication with the sensor
 *
 * @param[in] bus the i2c bus used by gs_i2c_master_transaction
 * @param[in] address the address of the htpa
 * @param[in] address of the internal eeprom in the sensor
 */
gs_error_t gs_htpa_configure(gs_htpa_handle_t *handle, uint8_t bus, uint8_t address, uint8_t eeprom_address);

/**
 * Read the thermal gradient data from the eeprom
 *
 * @param[in] handle
 * @param[out] th_data
 */
gs_error_t gs_htpa_read_th_grad(const gs_htpa_handle_t handle, gs_htpa_th_data_t* th_data);

/**
 * Read the thermal offset data from the eeprom
 *
 * @param[in] handle
 * @param[out] th_data
 */
gs_error_t gs_htpa_read_th_offset(const gs_htpa_handle_t handle, gs_htpa_th_data_t* th_data);

/**
 * Read the calibration values from the sensor eeprom
 * @param[in] handle
 * @param[out] mbit
 * @param[out] ref_cal
 * @param[out] bias
 * @param[out] clock
 * @param[out] bpa
 * @param[out] pu
 */
gs_error_t gs_htpa_read_factory_calibration(const gs_htpa_handle_t handle, uint8_t *mbit, uint8_t *ref_cal,
                                            uint8_t *bias, uint8_t *clock, uint8_t *bpa, uint8_t *pu);
/**
 * Adjust the received image using the blind frame and the values read from EEPROM
 * @param[in] image
 * @param[in] blind
 * @param[in] th_grad
 * @param[in] th_offset
 * @param[in] gradscalepow
 * @param[in] m_ptat
 * @param[out] result
 */
void gs_htpa_adjust_image(
        gs_htpa_image_t* image,
        gs_htpa_blind_image_t *blind,
        gs_htpa_th_data_t* th_data,
        uint16_t m_ptat,
        gs_htpa_adjusted_image_t* result);

/**
 * Read the gradient scale from the eeprom of the sensor
 * The value returned is 2^gradscale
 * @param[in] handle
 * @param[out] gradScalepow
 */
gs_error_t gs_htpa_read_grad_scale_pow(const gs_htpa_handle_t handle, gs_htpa_th_data_t* th_data);

/**
 * Wake up the sensor from sleep state
 * @param[in] handle
 */
gs_error_t gs_htpa_wakeup(const gs_htpa_handle_t handle);

/**
 * Read the status of the sensor
 * @param[in] handle
 * @param[out] status
 */
gs_error_t gs_htpa_read_status(const gs_htpa_handle_t handle, gs_htpa_status_t *status);

/**
 * Read the blind image of the sensor
 * @param[in] handle
 * @param[out] blind
 */
gs_error_t gs_htpa_read_blind_image(const gs_htpa_handle_t handle, gs_htpa_blind_image_t* blind);

/**
 * Read the image of the sensor
 * @param[in] handle
 * @param[out] image
 * @param[out] m_ptat
 */
gs_error_t gs_htpa_read_image(const gs_htpa_handle_t handle, gs_htpa_image_t* image, uint16_t* mean_ptat);

/**
 * Set the mbit/ref cal register
 * @param[in] handle
 * @param[in] mbit_trim
 * @param[in] ref_cal
 */

gs_error_t gs_htpa_mbit_ref_cal(const gs_htpa_handle_t handle, uint8_t mbit_trim, uint8_t ref_cal);

/**
 * Configure the bias trim register
 * @param[in] handle
 * @param[in] bias_trim_top
 * @param[in] bias_trim_bottom
 */
gs_error_t gs_htpa_bias_trim(const gs_htpa_handle_t handle, uint8_t bias_trim_top, uint8_t bias_trim_bottom);

/**
 * Configure the clock trim register
 * @param[in] handle
 * @param[in] clockfreq
 */
gs_error_t gs_htpa_clock_trim(const gs_htpa_handle_t handle, uint8_t clockfreq);

/**
 * Configure the bpa trim register
 * @param[in] handle
 * @param[in] bpa_top
 * @param[in] bpa_bottom
 */
gs_error_t gs_htpa_bpa_trim(const gs_htpa_handle_t handle, uint8_t bpa_top, uint8_t bpa_bottom);

/**
 * Configure the i2c pull up register
 * @param[in] handle
 * @param[in] sda
 * @param[in] scl
 */
gs_error_t gs_htpa_i2c_pull_up(const gs_htpa_handle_t handle, gs_htpa_pullup_t sda, gs_htpa_pullup_t scl);

/**
 * Wake-up and start the sensor
 * @param[in] handle
 */
gs_error_t gs_htpa_start(const gs_htpa_handle_t handle);

/**
 * Tell the sensor to go into sleep mode
 * @param[in] handle
 */
gs_error_t gs_htpa_sleep(const gs_htpa_handle_t handle);


#endif /* HTPD32X32D_H */
