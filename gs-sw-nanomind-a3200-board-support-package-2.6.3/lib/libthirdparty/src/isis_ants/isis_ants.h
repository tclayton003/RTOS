#ifndef ISIS_ANTS_H_
#define ISIS_ANTS_H_
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
    Number of antennas
*/
extern uint8_t num_ants;

/**
    Maximum activation time of deployment system in seconds
*/
extern uint8_t max_deploy_time_s;

void isis_ants_init(uint8_t num_of_ants, uint8_t max_dply_time_s);
gs_error_t get_ants_deployed(uint8_t i2c_addr, bool * deployed, int ant);
void deployment_active_wait(uint8_t i2c_addr, bool all_ants);
gs_error_t arm_antennas(uint8_t i2c_addr, bool arm);
gs_error_t single_deploy(uint8_t i2c_addr);
gs_error_t single_deploy_override(uint8_t i2c_addr);
gs_error_t auto_deploy(uint8_t i2c_addr);
void print_status(uint8_t i2c_addr);

#ifdef __cplusplus
}
#endif

#endif /* DEPLOY_ISIS_ANTS_H_ */
