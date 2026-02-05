#ifndef GS_A3200_MPU3300_H
#define GS_A3200_MPU3300_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   MPU3300 gyro interface.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GS_MPU3300_FSR_225 = 0,
    GS_MPU3300_FSR_450 = 1,
} gs_mpu3300_full_scale_reading_t;

typedef enum {
    GS_MPU3300_BW_256 = 0,
    GS_MPU3300_BW_188 = 1,
    GS_MPU3300_BW_98 = 2,
    GS_MPU3300_BW_42 = 3,
    GS_MPU3300_BW_20 = 4,
    GS_MPU3300_BW_10 = 5,
    GS_MPU3300_BW_5 = 6,
} gs_mpu3300_bandwidth_t;

typedef struct {
    float gyro_x;
    float gyro_y;
    float gyro_z;
} gs_mpu3300_gyro_t;

typedef struct {
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} gs_mpu3300_gyro_raw_t;

/**
   Run gyro self test.

   @return GS_ERROR_DATA if selftest failed.
   @return_gs_error_t
*/
gs_error_t gs_mpu3300_selftest(void);

/**
   Initialize gyro registers.
   @param bandwidth sets the gyro bandwidth
   @param full_scale sets full scale reading range
   @return_gs_error_t
*/
gs_error_t gs_mpu3300_init(gs_mpu3300_bandwidth_t bandwidth, gs_mpu3300_full_scale_reading_t full_scale);

/**
   Reset all internal registers to default values.
   @return_gs_error_t
*/
gs_error_t gs_mpu3300_reset(void);

/** 
    Read gyro.
    @param gyro_readings gyro x,y,z rates as degrees/s
    @return_gs_error_t
*/
gs_error_t gs_mpu3300_read_gyro(gs_mpu3300_gyro_t * gyro_readings);

/**
   Read gyro temperature.
   @param temp the temperature read
   @return_gs_error_t
*/
gs_error_t gs_mpu3300_read_temp(float * temp);

/**
   Read gyro and do not scale the result.
   @param gyro_reading gyro x,y,z rates
   @return_gs_error_t
*/
gs_error_t gs_mpu3300_read_raw(gs_mpu3300_gyro_raw_t * gyro_reading);

/**
   Gyro sleep mode control.
   @param sleep true for sleep, false for on.
   @return_gs_error_t
*/
gs_error_t gs_mpu3300_sleep(bool sleep);

/**
   Register commands.
   @return_gs_error_t
*/
gs_error_t gs_mpu3300_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
