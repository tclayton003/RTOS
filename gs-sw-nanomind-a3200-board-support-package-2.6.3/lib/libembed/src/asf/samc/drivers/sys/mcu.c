/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/samc/mcu.h>
#include <gs/util/log.h>
#include <gs/util/string.h>

#include <sam0/drivers/nvm/nvm.h>
#include <sam0/drivers/system/reset/reset_sam_c/reset.h>
#include <clock.h>

#include "gs/embed/freertos.h"
#include <gs/embed/asf/samc/convert_asf_status_code.h>

static gs_error_t gs_mcu_get_fuse_bits(struct nvm_fusebits * fusebits, bool update)
{
    enum status_code status = nvm_get_fuses(fusebits);
    if (status != STATUS_OK) {
        log_error("%s: Failed to get bits, status=%d\r\n", __FUNCTION__, status);
        return gs_convert_asf_status_code(status);
    }

    if (update) {
        /** BODVDD Threshold level at power on., level->Vdd, 8->2,86, 9->2,92  */
        fusebits->bodvdd_level = 7;
        /** BODVDD Enable at power on. */
        fusebits->bodvdd_enable = true;
        /** BODVDD Action at power on. */
        fusebits->bodvdd_action = NVM_BOD33_ACTION_RESET;
        /* BODVDD Hysteresis at power on*/
        fusebits->bodvdd_hysteresis = true;
    }

    return GS_OK;
}

gs_error_t gs_mcu_check_configuration(void)
{
    // check fuses
    struct nvm_fusebits fusebits;
    gs_error_t error = gs_mcu_get_fuse_bits(&fusebits, false);
    if (error) {
        log_error("Failed to read fuses!");
    } else {
        if (fusebits.bodvdd_enable != true) {
            log_warning("BOD is not enabled, value: %d", fusebits.bodvdd_enable);
        }
        if (fusebits.bodvdd_action != NVM_BOD33_ACTION_RESET) {
            log_warning("BOD is not configured to reset, value: %d", fusebits.bodvdd_action);
        }
        /* struct nvm_fusebits fusebits_ref; */
        /* error = gs_mcu_get_fuse_bits(&fusebits_ref, true); */
        /* if (error == GS_OK) { */
        /*     if (memcmp(&fusebits, &fusebits_ref, sizeof(fusebits)) != 0) { */
        /*         log_error("WARNING: incorrect FUSE/BOD configuration - run command \"mcu configure\" to update!"); */
        /*         error = GS_ERROR_DATA; */
        /*     } */
        /* } */
    }

    // check clock against FreeRTOS
    const uint32_t real_clock = system_gclk_gen_get_hz(0);
    if (configCPU_CLOCK_HZ != real_clock) {
        log_error("WARNING: mismatch in clock configuration, FreeRTOS: %u, real clock: %" PRIu32, configCPU_CLOCK_HZ, real_clock);
        error = GS_ERROR_DATA;
    }

    return error;
}

gs_error_t gs_mcu_configure(void)
{
    log_error("disabled for now!");
    return GS_ERROR_NOT_SUPPORTED;
    
    /* struct nvm_fusebits fusebits; */
    /* gs_error_t error = gs_mcu_get_fuse_bits(&fusebits, true); */
    /* if (error == GS_OK) { */
    /*     enum status_code status = nvm_set_fuses(&fusebits); */
    /*     if (status != STATUS_OK) { */
    /*         log_error("%s: Failed to set FUSE bits, status=%d\r\n", __FUNCTION__, status); */
    /*         return GS_ERROR_IO; */
    /*     } */
    /* } */

    /*     log_debug("%s: MCU configured successfully - reset to activate", __FUNCTION__); */

    /* return GS_OK; */
}

gs_error_t gs_mcu_get_boot_cause(uint16_t * cause)
{
    if (cause == NULL) {
        return GS_ERROR_ARG;
    }
    
    *cause = system_get_reset_cause();

    return GS_OK;
}
