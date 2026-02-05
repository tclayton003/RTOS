#ifndef DEPLOY_ISIS_ANTS_H_
#define DEPLOY_ISIS_ANTS_H_
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
    @file

    Functions to handle deployment of the ISIS antenna system.
    Make sure to call gs_deploy_isis_ants_init() first to set variables
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
    Initialize deployment variables

    @param[in] num_of_ants Number of antennas
    @param[in] max_dply_time_s Maximum burn duration for each antenna in seconds
*/
void gs_deploy_isis_ants_init(uint8_t num_of_ants, int max_dply_time_s);

/**
    Handle the deployment of the ISIS antenna system.

    @param[in] goto_next_state Should the deployment statemachine go to the next state

    @return_gs_error_t
*/
gs_error_t gs_deploy_isis_ants(bool goto_next_state);

/**
    Handle the backup deployment of the ISIS antenna system

    @return_gs_error_t
*/
gs_error_t gs_backup_deploy_isis_ants(void);

#ifdef __cplusplus
}
#endif
#endif /* DEPLOY_ISIS_ANTS_H_ */
