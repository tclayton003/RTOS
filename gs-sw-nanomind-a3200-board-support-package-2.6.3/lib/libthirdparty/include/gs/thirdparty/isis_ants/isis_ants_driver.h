#ifndef ISIS_ANTS_DRIVER_H_
#define ISIS_ANTS_DRIVER_H_
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Driver for the ISIS antenna system.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
    I2C address of microcontroller A
*/
#define GS_ISIS_ANTS_DFL_ADDR_A            0x31
/**
    I2C address of microcontroller B
*/
#define GS_ISIS_ANTS_DFL_ADDR_B            0x32
/**
    Reset command
*/
#define ISIS_ANTS_CMD_RESET         0xAA
/**
    Arm command
*/
#define ISIS_ANTS_CMD_ARM           0xAD
/**
    Disarm command
*/
#define ISIS_ANTS_CMD_DISARM        0xAC
/**
    Deploy antenna 1 command
*/
#define ISIS_ANTS_CMD_DEPLOY_1      0xA1
/**
    Deploy antenna 2 command
*/
#define ISIS_ANTS_CMD_DEPLOY_2      0xA2
/**
    Deploy antenna 3 command
*/
#define ISIS_ANTS_CMD_DEPLOY_3      0xA3
/**
    Deploy antenna 4 command
*/
#define ISIS_ANTS_CMD_DEPLOY_4      0xA4
/**
    Auto deploy antennas command
*/
#define ISIS_ANTS_CMD_DEPLOY_AUTO   0xA5
/**
    Cancel deployment command
*/
#define ISIS_ANTS_CMD_DEPLOY_CANCEL 0xA9
/**
    Deploy antenna 1 with override command
*/
#define ISIS_ANTS_CMD_O_DEPLOY_1    0xBA
/**
    Deploy antenna 2 with override command
*/
#define ISIS_ANTS_CMD_O_DEPLOY_2    0xBB
/**
    Deploy antenna 3 with override command
*/
#define ISIS_ANTS_CMD_O_DEPLOY_3    0xBC
/**
    Deploy antenna 4 with override command
*/
#define ISIS_ANTS_CMD_O_DEPLOY_4    0xBD
/**
    Get temperature command
*/
#define ISIS_ANTS_CMD_TEMP          0xC0
/**
    Get deployment status command
*/
#define ISIS_ANTS_CMD_STATUS_DEPLOY 0xC3
/**
    Get activation count for antenna 1 command
*/
#define ISIS_ANTS_CMD_COUNT_1       0xB0
/**
    Get activation count for antenna 2 command
*/
#define ISIS_ANTS_CMD_COUNT_2       0xB1
/**
    Get activation count for antenna 3 command
*/
#define ISIS_ANTS_CMD_COUNT_3       0xB2
/**
    Get activation count for antenna 4 command
*/
#define ISIS_ANTS_CMD_COUNT_4       0xB3
/**
    Get activation time for antenna 1 command
*/
#define ISIS_ANTS_CMD_TIME_1        0xB4
/**
    Get activation time for antenna 2 command
*/
#define ISIS_ANTS_CMD_TIME_2        0xB5
/**
    Get activation time for antenna 3 command
*/
#define ISIS_ANTS_CMD_TIME_3        0xB6
/**
    Get activation time for antenna 4 command
*/
#define ISIS_ANTS_CMD_TIME_4        0xB7

/**
 * Status return for each antenna
 */
typedef struct gs_isis_ant_status_s {
    /**
        Value of deployment switch
    */
    uint8_t not_deployed;
    /**
        Set to 1 if deployment was stopped due to a time limit
    */
    uint8_t time_limit_reached;
    /**
        Set to 1 while burning
    */
    uint8_t deployment_active;
    /**
        Counts the number of activations since antenna reset
    */
    uint8_t activation_count;
    /**
        Counts the number of 1/20 seconds since antenna reset
    */
    uint16_t activation_time;
} gs_isis_ant_status_t;

/**
 * Status return for entire ants system
 */
typedef struct gs_isis_ants_status_s {
    /**
        Global system arm status
    */
    uint8_t armed;
    /**
        Set to 1 if using override deploy
    */
    uint8_t switch_ignore;
    /**
        Set to 1 if independent burn is cuurently active
    */
    uint8_t independent_burn;
    /**
        Detailed data structure for each antenna
    */
    gs_isis_ant_status_t ant[4];
} gs_isis_ants_status_t;

/**
   Get deploy status parameters
   @param[in] i2c_addr i2c address
   @param[out] status structure containing parameters
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_deploy_status(uint8_t i2c_addr, gs_isis_ants_status_t * status);

/**
   Get all status parameters
   @param[in] i2c_addr i2c address
   @param[out] status structure containing parameters
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_status(uint8_t i2c_addr, gs_isis_ants_status_t * status);

/**
   Get temperature
   @param[in] i2c_addr i2c address
   @param[out] v_out Vout value from the microcontroller. Look in convcersion table to convert to Celcius (ISIS Antenna System User Manual)
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_temp(uint8_t i2c_addr, uint16_t * v_out);

/**
   Automatically deploy all antennas
   @param[in] i2c_addr i2c address
   @param[in] time_sec maximum allowed burn time (beware too high a value may damage burn resistor)
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_deploy_auto(uint8_t i2c_addr, uint8_t time_sec);

/**
   Try to deploy single antenna
   @param[in] i2c_addr i2c address
   @param[in] isis_ant_nr antenna number [1-4]
   @param[in] time_sec maximum allowed burn time (beware too high a value may damage burn resistor)
   @param[in] override false = do not use override, use override otherwise.
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_deploy_single(uint8_t i2c_addr, int isis_ant_nr, uint8_t time_sec, bool override);

/**
   Cancel deployment
   @param[in] i2c_addr i2c address
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_deploy_cancel(uint8_t i2c_addr);

/**
   Disarm
   @param[in] i2c_addr i2c address
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_disarm(uint8_t i2c_addr);

/**
   Arm
   @param[in] i2c_addr i2c address
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_arm(uint8_t i2c_addr);

/**
   Reset
   @param[in] i2c_addr i2c address
   @return_gs_error_t
 */
gs_error_t gs_isis_ants_reset(uint8_t i2c_addr);

#ifdef __cplusplus
}
#endif
#endif /* ISIS_ANTS_DRIVER_H_ */
