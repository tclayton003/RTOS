/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * Interface for external temperature sensor
 *
 * LM60BIM3 - sensor output 0.174V-1.205V (-40deg to 125deg).
 * Data sheet: http://www.ti.com/lit/ds/symlink/lm60-q1.pdf
 */

/* --- includes ----------------------------------------------------------*/

#include <gs/embed/asf/samc/drivers/tsense_external.h> // first - ensure self contained
#include <gs/embed/asf/samc/drivers/adc.h>
#include <gs/embed/asf/samc/command.h>
#include <gs/util/gosh/command.h>
#include <board.h> // pin configuration from conf/user_board.h
#include <power.h>

/* --- definitions -------------------------------------------------------*/

#define GS_TSENSE_EXTERNAL_ADC_MAXVAL    (4095)
#define GS_TSENSE_EXTERNAL_OFFSET        (0.424F)      // from data sheet
#define GS_TSENSE_EXTERNAL_SLOPE         (6.25e-3)     // from data sheet

// ADC configuration
static struct adc_config config_adc;

/* --- code --------------------------------------------------------------*/

static inline int32_t convert_adc_value_to_temperature(uint16_t val)
{
    const float voltage = ((float)val) * (GS_SAMC_TSENSE_EXTERNAL_ADC_VREF / GS_TSENSE_EXTERNAL_ADC_MAXVAL);

    float temp = (voltage - GS_TSENSE_EXTERNAL_OFFSET) / GS_TSENSE_EXTERNAL_SLOPE;

    return (int32_t)(temp * 10);
}

gs_error_t gs_tsense_external_init(void)
{
    gs_error_t error = gs_adc_init();
    if (error == GS_OK) {
        adc_get_config_defaults(&config_adc);

        config_adc.clock_source      = GS_SAMC_TSENSE_EXTERNAL_ADC_CLOCK_SOURCE;
        config_adc.clock_prescaler   = GS_SAMC_TSENSE_EXTERNAL_ADC_CLOCK_PRESCALER;
        config_adc.sample_length     = ADC_AVGCTRL_SAMPLENUM_4_Val;
        config_adc.resolution        = ADC_RESOLUTION_12BIT;

        config_adc.positive_input    = GS_SAMC_TSENSE_EXTERNAL_ADC_INPUT;
        config_adc.reference         = GS_SAMC_TSENSE_EXTERNAL_ADC_REF;

        int32_t temp;
        error = gs_tsense_external_get(&temp, NULL);
    }

    return error;
}

/* Get/read temperature */
gs_error_t gs_tsense_external_get(int32_t * temperature, uint16_t * adc_value)
{
    uint16_t value;
    gs_error_t error = gs_adc_read(GS_ADC0, &config_adc, &value);
    if (error == GS_OK) {

        if (adc_value) {
            *adc_value = value;
        }

        *temperature = convert_adc_value_to_temperature(value);
    }

    return error;
}

static int cmd_tsense_external_get_common(struct command_context * ctx, bool showADC)
{
    int32_t value = 0;
    uint16_t adc_value = 0;
    int result = gs_tsense_external_get(&value, &adc_value);
    if (result < 0) {
        return result;
    }

    if (showADC) {
    	printf("Temperature: %.1f C, ADC: %u\r\n", ((float)value)/10.0F, adc_value);
    } else {
    	printf("Temperature: %.1f C\r\n", ((float)value)/10.0F);
    }

    return GS_OK;
}

static int cmd_tsense_external_get(struct command_context * ctx)
{
    return cmd_tsense_external_get_common(ctx, false);
}

static int cmd_tsense_external_get2(struct command_context * ctx)
{
    return cmd_tsense_external_get_common(ctx, true);
}

static const gs_command_t GS_COMMAND_SUB subcmd_tsense_ext[] = {
    {
        .name = "get",
        .help = "read external temperature sensor",
        .handler = cmd_tsense_external_get,
    },
    {
        .name = "get2",
        .help = "read external temperature sensor and show ADC value",
        .handler = cmd_tsense_external_get2,
    }
};

static const gs_command_t GS_COMMAND_ROOT cmd_tsense_ext[] = {
    {
        .name = "tsense_external",
        .help = "external temperature sensor commands",
        .chain = GS_COMMAND_INIT_CHAIN(subcmd_tsense_ext),
    }
};

gs_error_t gs_samc_register_tsense_ext_commands(void)
{
    return GS_COMMAND_REGISTER(cmd_tsense_ext);
}
