#ifndef INCLUDE_GS_THIRDPARTY_GYRO_H_
#define INCLUDE_GS_THIRDPARTY_GYRO_H_
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Generic GomSpace gyroscope API.
*/

#include <gs/util/error.h>

/**
   Gyroscope measurement.
 */
typedef struct gs_gyro_sample {
    union {
        struct {
            /**
               Rotation around x axis in degrees per second.
             */
            float x;
            /**
               Rotation around y axis in degrees per second.
             */
            float y;
            /**
               Rotation around z axis in degrees per second.
             */
            float z;
        };
        /**
           Rotation around x,y,z axes in degrees per second.
         */
        float axes[3];

    };
    /**
       Gyroscope temperature in deci degrees C
     */
    int16_t temp;
} gs_gyro_sample_t;

/**
   Gyroscope sample mode.
 */
typedef enum gs_gyro_sample_mode {
    /**
       Start sampling just single sample.
     */
    GS_GYRO_SINGLE_SAMPLE   = 0,
    /**
       Start sampling continuously .
     */
    GS_GYRO_FREE_RUNNING    = 1,
} gs_gyro_sample_mode_t;

/**
   Get max normal sampling time.

   Get the normal maximum time at takes the sensor to get a sample ready.
   @note This only applies to GS_GYRO_SINGLE_SAMPLE mode.

   @param[in] handle Gyroscope handle.
   @param[out] max_sample_time maximum normal sample time.
   @param[in] driver_data data to specific driver.
   @return_gs_error_t
*/
typedef gs_error_t (* gs_gyro_max_norm_sample_time_t)(uint8_t handle, uint32_t * max_sample_time_ms, void * driver_data);

/**
   Gyro start sampling.

   Starts the sampling procedure in the chosen mode.
   @see gs_gyro_get_sample_t to get sample.

   @param[in] handle Gyroscope handle.
   @param[in] mode Sample mode, @see gs_gyro_sample_mode_t.
   @param[in] driver_data data to specific driver.
   @return_gs_error_t
*/
typedef gs_error_t (* gs_gyro_start_sampling_t)(uint8_t handle, gs_gyro_sample_mode_t mode, void * driver_data);


/**
   Gyro get sample.

   Get the latest sample if any (not all ready sampled) sample is ready.
   @note If no sample is ready it returns error, @see gs_gyro_max_norm_sample_time_t.

   @param[in] handle Gyroscope handle.
   @param[in] sample Gyroscope measurement, @see gs_gyro_sample_t.
   @param[in] driver_data data to specific driver.
   @return_gs_error_t
*/
typedef gs_error_t (* gs_gyro_get_sample_t)(uint8_t handle, gs_gyro_sample_t * sample, void * driver_data);

#endif /* INCLUDE_GS_THIRDPARTY_GYRO_H_ */
