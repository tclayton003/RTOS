/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "isis_ants.h"
#include "local.h"
#include <gs/thirdparty/isis_ants/isis_ants_driver.h>

#include <gs/util/log.h>
#include <gs/util/thread.h>

uint8_t num_ants;
uint8_t max_deploy_time_s;

__attribute__((weak)) // Allows redefinition of function in unittest
void isis_ants_init(uint8_t num_of_ants, uint8_t max_dply_time_s)
{
    num_ants = num_of_ants;
    max_deploy_time_s = max_dply_time_s;
}

void print_status(uint8_t i2c_addr)
{
    gs_isis_ants_status_t ants_status;
    gs_error_t com_status = gs_isis_ants_status(i2c_addr, &ants_status);
    if (com_status != GS_OK) {
        log_error("Antenna print status failed [%x]", i2c_addr);
        return;
    }

    printf("\n");
    printf("I2C address: %x\n", i2c_addr);
    if (ants_status.armed == 1)
        printf("Antennas armed\n");
    else
        printf("Antennas not armed\n");

    if (ants_status.switch_ignore == 1)
        printf("Ignoring switches\n");
    else
        printf("Not ignoring switches\n");

    for (int antenna = 0; antenna < num_ants; antenna++) {
        printf("Antenna %i: not_deployed: %i\ttime_limit_reached: %i\tdeployment_active: %i\tactivation_count: %i\tactivation_time: %i\n", antenna,
                                                                                                                                           ants_status.ant[antenna].not_deployed,
                                                                                                                                           ants_status.ant[antenna].time_limit_reached,
                                                                                                                                           ants_status.ant[antenna].deployment_active,
                                                                                                                                           ants_status.ant[antenna].activation_count,
                                                                                                                                           ants_status.ant[antenna].activation_time);
    }
}

__attribute__((weak)) // Allows redefinition of function in unittest
gs_error_t get_ants_deployed(uint8_t i2c_addr, bool * deployed, int ant)
{
    gs_isis_ants_status_t ants_status;
    gs_error_t com_status = gs_isis_ants_deploy_status(i2c_addr, &ants_status);
    if (com_status != GS_OK) {
        log_error("Antenna status failed [%x]", i2c_addr);
        return com_status;
    }

    // Check deployment status for single antenna
    if (ant >= 0 && ant < num_ants) {
        if (ants_status.ant[ant].not_deployed == 0) {
            log_info("Antenna %u deployed ok [%x]", ant, i2c_addr);
            *deployed = true;
        }
        else {
            *deployed = false;
        }
    }
    else { // Check deployment status for all antennas
        if (ants_status.ant[0].not_deployed == 0 &&
            ants_status.ant[1].not_deployed == 0 &&
            ants_status.ant[2].not_deployed == 0 &&
            ants_status.ant[3].not_deployed == 0) {
            log_info("Antennas deployed ok [%x]", i2c_addr);
            *deployed = true;
        }
        else {
            *deployed = false;
        }
    }

    return GS_OK;
}

__attribute__((weak)) // Allows redefinition of function in unittest
gs_error_t arm_antennas(uint8_t i2c_addr, bool arm)
{
    gs_error_t ret_value = GS_ERROR_TIMEOUT;
    if (arm) {
        log_info("Starting antenna arming [%x]", i2c_addr);
        ret_value = gs_isis_ants_arm(i2c_addr);
        if (ret_value != GS_OK) {
            log_error("Antenna arming failed [%x]", i2c_addr);
        }
    }
    else {
        log_info("Starting antenna disarming [%x]", i2c_addr);
        ret_value = gs_isis_ants_disarm(i2c_addr);
        if (ret_value != GS_OK) {
            log_error("Antenna disarming failed [%x]", i2c_addr);
        }
    }

    return ret_value;
}

__attribute__((weak)) // Allows redefinition of function in unittest
void deployment_active_wait(uint8_t i2c_addr, bool all_ants)
{
    // Wait while deployment is active
    bool active;
    gs_error_t com_status;
    gs_isis_ants_status_t ants_status;
    int count = 0;
    int wait_time_s = max_deploy_time_s + 2;
    if (all_ants)
        wait_time_s *= num_ants;

     do {
        gs_thread_sleep_ms(1000);
        com_status = gs_isis_ants_deploy_status(i2c_addr, &ants_status);
        if (com_status == GS_OK) {
            if (ants_status.ant[0].deployment_active || ants_status.ant[1].deployment_active ||
                ants_status.ant[2].deployment_active || ants_status.ant[3].deployment_active) {
                active = true;
            }
            else {
                active = false;
            }
        }
        else {
            log_error("Could not get deploy status");
            active = false;
        }
    } while (active && count++ < wait_time_s);
}

__attribute__((weak)) // Allows redefinition of function in unittest
gs_error_t single_deploy(uint8_t i2c_addr)
{
    // Get status of deployment
    bool ants_deployed = false;
    get_ants_deployed(i2c_addr, &ants_deployed, -1);

    // Antennas are deployed so return
    if (ants_deployed) {
        return GS_OK;
    }

    // Arm antenna system
    gs_error_t ret_value = arm_antennas(i2c_addr, true);
    if (ret_value != GS_OK) {
        log_error("Antenna arming failed for single deployment[%x]", i2c_addr);
        return ret_value;
    }

    // Send commands to start single antenna deployment
    bool success;
    ret_value = GS_OK;
    for (int ant = 0; ant < num_ants; ant++) {
        if (gs_isis_ants_deploy_single(i2c_addr, ant, max_deploy_time_s, false) == GS_OK) {
            log_info("Sent antenna single deployment command to [%x] for antenna [%x]", i2c_addr, ant);
            success = true;
        }
        else {
            log_error("Antenna %u single deployment failed [%x]", ant, i2c_addr);
            success = false;
        }

        ants_deployed = false;
        // If deployment cmd successfully sent then check if it deployed
        if (success) {
            // Wait while deployment is active
            deployment_active_wait(i2c_addr, false);

            // Deployment no longer active, so check if it was successfull
            get_ants_deployed(i2c_addr, &ants_deployed, ant);
        }

        if (!ants_deployed) {
            log_error("Antenna [%i] did not deploy", ant);
            ret_value = GS_ERROR_STATE;
        }
    }

    if (ret_value == GS_OK) {
        log_info("All antennas deployed");
    }

    // Disarm antenna system
    arm_antennas(i2c_addr, false);

    return ret_value;
}

__attribute__((weak)) // Allows redefinition of function in unittest
gs_error_t single_deploy_override(uint8_t i2c_addr)
{
    // Arm antenna system
    arm_antennas(i2c_addr, true);

    // Send commands to start single antenna deployment with override
    bool success;
    gs_error_t ret_value = GS_OK;
    for (int ant = 0; ant < num_ants; ant++) {
        success = false;
        if (gs_isis_ants_deploy_single(i2c_addr, ant, max_deploy_time_s, true) == GS_OK) {
            log_info("Sent antenna single override deployment command to [%x] for antenna [%x]", i2c_addr, ant);
            success = true;
        }
        else {
            log_error("Antenna %u single override deployment failed [%x]", ant, i2c_addr);
            success = false;
        }

        bool ants_deployed = false;
        // If deployment cmd successfully sent then check if it deployed
        if (success) {
            // Wait while deployment is active
            deployment_active_wait(i2c_addr, false);

            // Deployment no longer active, so check if it was successfull
            get_ants_deployed(i2c_addr, &ants_deployed, ant);
        }

        if (!ants_deployed) {
            log_error("Antenna [%i] did not deploy", ant);
            ret_value = GS_ERROR_STATE;
        }
    }

    if (ret_value == GS_OK) {
        log_info("All antennas deployed");
    }

    // Disarm antenna system
    arm_antennas(i2c_addr, false);

    return ret_value;
}

__attribute__((weak)) // Allows redefinition of function in unittest
gs_error_t auto_deploy(uint8_t i2c_addr)
{
    // Get status of deployment
    bool ants_deployed = false;
    get_ants_deployed(i2c_addr, &ants_deployed, -1);

    // Antennas are deployed so exit the while loop
    if (ants_deployed) {
        return GS_OK;
    }

    // Arm antenna system
    gs_error_t ret_value = arm_antennas(i2c_addr, true);
    if (ret_value != GS_OK) {
        log_error("Antenna arming failed for auto deployment [%x]", i2c_addr);
        return ret_value;
    }

    // Send command to start automatic antenna deployment
    bool success;
    if (gs_isis_ants_deploy_auto(i2c_addr, max_deploy_time_s) == GS_OK) {
        log_info("Sent antenna auto deployment command to [%x]", i2c_addr);
        success = true;
    }
    else {
        log_error("Antenna auto deployment failed [%x]", i2c_addr);
        ret_value = GS_ERROR_STATE;
        success = false;
    }

    ants_deployed = false;
    // If deployment cmd successfully sent then check if it deployed
    if (success) {
        // Wait while deployment is active
        deployment_active_wait(i2c_addr, true);

        // Deployment no longer active, so check if it was successfull
        get_ants_deployed(i2c_addr, &ants_deployed, -1);
    }

    if (ants_deployed) {
        log_info("All antennas deployed");
        ret_value = GS_OK;
    }
    else {
        log_error("Antennas are not deployed");
        ret_value = GS_ERROR_STATE;
    }

    // Disarm antenna system
    arm_antennas(i2c_addr, false);

    return ret_value;
}
