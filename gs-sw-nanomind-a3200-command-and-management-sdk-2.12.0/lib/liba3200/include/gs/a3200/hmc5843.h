#ifndef GS_A3200_HMC5843_H
#define GS_A3200_HMC5843_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file
   Driver interface for hmc5843
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Rates
*/
typedef enum {
    MAG_RATE_0_5 = 0,//!< MAG_RATE_0_5
    MAG_RATE_1   = 1,//!< MAG_RATE_1
    MAG_RATE_2   = 2,//!< MAG_RATE_2
    MAG_RATE_5   = 3,//!< MAG_RATE_5
    MAG_RATE_10  = 4,//!< MAG_RATE_10
    MAG_RATE_20  = 5,//!< MAG_RATE_20
    MAG_RATE_50  = 6,//!< MAG_RATE_50
} gs_hmc5843_rate_t;

/**
   Measurement modes
*/
typedef enum {
    MAG_MEAS_NORM = 0,//!< MAG_MEAS_NORM
    MAG_MEAS_POS  = 1,//!< MAG_MEAS_POS
    MAG_MEAS_NEG  = 2 //!< MAG_MEAS_NEG
} gs_hmc5843_meas_t;

/**
   Gain settings
*/
typedef enum {
    MAG_GAIN_0_7  = 0,//!< MAG_GAIN_0_7 (max value is 0.7 G)
    MAG_GAIN_1_0  = 1,//!< MAG_GAIN_1_0 (max value is 1.0 G)
    MAG_GAIN_1_5  = 2,//!< MAG_GAIN_1_5 (max value is 1.5 G)
    MAG_GAIN_2_0  = 3,//!< MAG_GAIN_2_0 (max value is 2.0 G)
    MAG_GAIN_3_2  = 4,//!< MAG_GAIN_3_2 (max value is 3.2 G)
    MAG_GAIN_3_8  = 5,//!< MAG_GAIN_3_8 (max value is 3.8 G)
    MAG_GAIN_4_5  = 6,//!< MAG_GAIN_4_5 (max value is 4.5 G)
    MAG_GAIN_6_5  = 7 //!< MAG_GAIN_6_5 (max value is 6.5 G)
} gs_hmc5843_gain_t;

/**
   Mode Settings
*/
typedef enum {
    MAG_MODE_CONTINUOUS = 0,
    MAG_MODE_SINGLE     = 1,
    MAG_MODE_IDLE       = 2,
    MAG_MODE_SLEEP      = 3
} gs_hmc5843_mode_t;

/**
   Set rate of magnetometer.

   @param[in] new_rate rate
   @param[in] new_meas measurement
   @param[in] new_gain gain
   @return_gs_error_t
*/
gs_error_t gs_hmc5843_set_conf(gs_hmc5843_rate_t new_rate, gs_hmc5843_meas_t new_meas, gs_hmc5843_gain_t new_gain);

/**
   Return gain setting.
*/
gs_hmc5843_gain_t gs_hmc5843_get_gain();

/**
   Return rate setting.
*/
gs_hmc5843_rate_t gs_hmc5843_get_rate();

/**
   Return gain setting.
*/
gs_hmc5843_meas_t gs_hmc5843_get_meas();

/**
   Set mode of magnetometer

   @param[in] new_mode mode
   @return_gs_error_t
*/
gs_error_t gs_hmc5843_set_mode(gs_hmc5843_mode_t new_mode);

/**
   Magnetometer readings.
*/
typedef struct {
    float x; //!< X (milli Gauss)
    float y; //!< Y (milli Gauss)
    float z; //!< Z (milli Gauss)
} gs_hmc5843_data_t;

/**
   Initialize device only if it has not been initialized before.

   @return_gs_error_t
*/
gs_error_t gs_hmc5843_init(void);

/**
   Force initialize of device

   @return_gs_error_t
*/
gs_error_t gs_hmc5843_init_force(void);

/**
   Request a single reading.

   Perform a sampling of magnetometer.

   @param data read values
   @return_gs_error_t
*/
gs_error_t gs_hmc5843_read_single(gs_hmc5843_data_t * data);

/**
   Request a single reading.

   Perform read of data-registers from hmc5843.

   @param data read values
   @return_gs_error_t
*/
gs_error_t gs_hmc5843_read(gs_hmc5843_data_t * data);

/**
   Request a single reading.

   Perform read of data-registers from hmc5843: output raw data.

   @param data read values
   @return_gs_error_t
*/
gs_error_t gs_hmc5843_read_raw(gs_hmc5843_data_t * data);

/**
   Request a single reading.

   Perform a sampling of magnetometer (only for testing purposes).

   @param data read values
   @return_gs_error_t
*/
gs_error_t gs_hmc5843_read_test(gs_hmc5843_data_t * data);

/**
   Perform hmc5843 selftest.
  
   Performs selftest operation, verifying correct device operation.
  
   @param data Read values from selftest
   @return_gs_error_t
 */
gs_error_t gs_hmc5843_selftest(gs_hmc5843_data_t * data);

/**
   Register commands.
   @return_gs_error_t
*/
gs_error_t gs_hmc5843_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
