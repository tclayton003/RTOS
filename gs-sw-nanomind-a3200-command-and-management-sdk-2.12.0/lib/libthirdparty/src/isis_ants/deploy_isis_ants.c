/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/isis_ants/deploy_isis_ants.h>
#include <gs/thirdparty/isis_ants/isis_ants_driver.h>

#include <gs/util/log.h>
#include <gs/util/thread.h>

#include "isis_ants.h"
#include "local.h"

GS_LOG_GROUP(gs_isis_ants_log, "isis_ants", GS_LOG_CAT_DRIVER, LOG_DEFAULT_MASK);

enum gs_deploy_state {
    GS_AUTO_A,
    GS_AUTO_B,
    GS_SINGLE_A,
    GS_SINGLE_B
} deploy_state;

gs_error_t gs_deploy_isis_ants(bool goto_next_state)
{
    gs_error_t antenna_deploy_status = GS_ERROR_STATE;

    switch (deploy_state) {
        case GS_AUTO_A:
            antenna_deploy_status = auto_deploy(GS_ISIS_ANTS_DFL_ADDR_A);
            if (goto_next_state)
                deploy_state = GS_AUTO_B;
            break;
        case GS_AUTO_B:
            antenna_deploy_status = auto_deploy(GS_ISIS_ANTS_DFL_ADDR_B);
            if (goto_next_state)
                deploy_state = GS_SINGLE_A;
            break;
        case GS_SINGLE_A:
            antenna_deploy_status = single_deploy(GS_ISIS_ANTS_DFL_ADDR_A);
            if (goto_next_state)
                deploy_state = GS_SINGLE_B;
            break;
        case GS_SINGLE_B:
            antenna_deploy_status = single_deploy(GS_ISIS_ANTS_DFL_ADDR_B);
            if (goto_next_state)
                deploy_state = GS_AUTO_A;
            break;
    }

    return antenna_deploy_status;
}

// Start with microcontroller A for override deployment
uint8_t i2c_addr_override = GS_ISIS_ANTS_DFL_ADDR_A;
gs_error_t gs_backup_deploy_isis_ants()
{
    gs_error_t ret_value = single_deploy_override(i2c_addr_override);

    if (i2c_addr_override == GS_ISIS_ANTS_DFL_ADDR_A)
        i2c_addr_override = GS_ISIS_ANTS_DFL_ADDR_B;
    else
        i2c_addr_override = GS_ISIS_ANTS_DFL_ADDR_A;

    return ret_value;
}

void gs_deploy_isis_ants_init(uint8_t num_of_ants, int max_dply_time_s)
{
    gs_log_group_register(gs_isis_ants_log);

    isis_ants_init(num_of_ants, max_dply_time_s);

    deploy_state = GS_AUTO_A;
}

