/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * Interface for internal temperature sensor in SAMC21
 */

/* --- includes ----------------------------------------------------------*/

#include <gs/embed/asf/samc/drivers/tsense_internal.h> // first - ensure self contained

// libasf
#include <sam0/drivers/tsens/tsens.h>

#include <gs/util/log.h>
#include <gs/util/sem.h>
#include <gs/util/gosh/command.h>
#include <gs/embed/asf/samc/convert_asf_status_code.h>

/* --- definitions -------------------------------------------------------*/

static bool tsense_internal_initialized;
static gs_sem_t tsense_internal_sem;

/* --- code --------------------------------------------------------------*/

gs_error_t gs_tsense_internal_init()
{
    gs_error_t error = gs_sem_create(1, &tsense_internal_sem);
    if (error == GS_OK) {

        if (tsense_internal_initialized == false) {
            struct tsens_config config;
            tsens_get_config_defaults(&config);
            config.clock_source = GCLK_GENERATOR_0;

#if GS_TSENSE_INTERNAL_DEBUG
            printf("tsense internal: nvm bits[0]=%"PRIu32",  bits[1]=%"PRIu32"\n",
                   *((uint32_t *)NVMCTRL_TEMP_LOG),
                   *(((uint32_t *)NVMCTRL_TEMP_LOG) + 1));

            printf("tsense internal: gain=%"PRIu32", offset=%"PRId32"\n",
                   config.calibration.gain,
                   config.calibration.offset);
#endif

            enum status_code status  = tsens_init(&config);
            if (status != STATUS_OK) {
                log_error("%s: tsens_init() failed, returned value: %d", __FUNCTION__, status);
                error = gs_convert_asf_status_code(status);
            } else {
                tsens_enable();

                tsense_internal_initialized = true;

                // Do dummy read - ignore first result
                int32_t temp;
                gs_tsense_internal_get(&temp);
            }
        }
    }
  
    return error;
}

gs_error_t gs_tsense_internal_get(int32_t *dest)
{
    if (tsense_internal_initialized == false) {
        return GS_ERROR_HANDLE;
    }

    gs_error_t error = gs_sem_wait(tsense_internal_sem, 1000);
    if (error == GS_OK) {

        tsens_start_conversion();

        int32_t temp;
        enum status_code status;
        do {
            // Wait for conversion to be done and read out result
            status = tsens_read(&temp);
        } while (status != STATUS_OK);

        if (status == STATUS_OK) {

            // compensate for "milli" degrees when using a 48Mhz clock
            temp /= 10;

            temp *= -1;

            *dest = temp;
        } else {
            log_warning("%s: tsens_read() failed, return value: %d", __FUNCTION__, status);
            error = gs_convert_asf_status_code(status);
        }
    

        gs_sem_post(tsense_internal_sem);
    }

    return error;
}

static int cmd_tsense_internal_get(gs_command_context_t * ctx)
{
    int32_t val;
    gs_error_t error = gs_tsense_internal_get(&val);
    if (error) {
        return error;
    }

    printf("Temperature: %.1f C\r\n", ((float)val)/10.0F);

    return GS_OK;
}

static int cmd_tsense_internal_status(gs_command_context_t * ctx)
{
    printf("Bus syncing %d \r\n", tsens_is_syncing());

    const uint32_t status = tsens_get_status();

    printf("Conversion status: %"PRIu32"\r\n", status);

    if (status & TSENS_STATUS_RESULT_READY) {
        printf("conversion status: TSENS_STATUS_RESULT_READY\r\n");
    }
    if (status & TSENS_STATUS_OVERRUN) {
        printf("conversion status: TSENS_STATUS_OVERRUN\r\n");
    }
    if (status & TSENS_STATUS_WINDOW) {
        printf("conversion status: TSENS_STATUS_WINDOW\r\n");
    }
    if (status & TSENS_STATUS_OVERFLOW) {
        printf("conversion status: TSENS_STATUS_OVERFLOW\r\n");
    }
    
    {
        struct tsens_config config;
        tsens_get_config_defaults(&config);
        printf("tsens calibrate data: gain=%"PRIu32", offset=%"PRIu32"\r\n",
               config.calibration.gain, config.calibration.offset);
    }

    return GS_OK;
}

static const gs_command_t GS_COMMAND_SUB subcmd_tsense_int[] = {
    {
        .name = "get",
        .help = "read internal temperature sensor",
        .handler = cmd_tsense_internal_get,
    },
    {
        .name = "status",
        .help = "show status for internal temperature sensor",
        .handler = cmd_tsense_internal_status,
    }
};

static const gs_command_t GS_COMMAND_ROOT cmd_tsense_int[] = {
    {
        .name = "tsense_internal",
        .help = "internal temperature sensor commands",
        .chain = GS_COMMAND_INIT_CHAIN(subcmd_tsense_int),
    }
};

gs_error_t gs_samc_register_tsense_int_commands(void)
{
    return GS_COMMAND_REGISTER(cmd_tsense_int);
}
