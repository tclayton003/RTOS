/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/a3200/pwr_switch.h>
#include <gs/a3200/adc_channels.h>
#include <gs/util/gosh/command.h>

static uint16_t get_current(gs_a3200_pwr_switch_t channel)
{
#define ADC_ARRAY 12
#if (ADC_ARRAY < GS_A3200_ADC_NCHANS)
#error Invalid adc array size
#endif
    int16_t adcval[ADC_ARRAY];
    gs_error_t error = gs_a3200_adc_channels_sample(adcval);
    if (error == GS_OK) {
        uint16_t gssb;
        uint16_t gssb2;
        uint16_t flash;
        uint16_t pwm;
        if (GS_A3200_BOARD_REVISION >= 3) {
            gssb = (uint16_t)(GS_A3200_ADC_TO_MV * adcval[11] / 0.220 / 100);
            gssb2 = (uint16_t)(GS_A3200_ADC_TO_MV * adcval[9] / 0.220 / 100);
            flash = (uint16_t)(GS_A3200_ADC_TO_MV * adcval[10] / 0.220 / 100);
            pwm = (uint16_t)(GS_A3200_ADC_TO_MV * adcval[8] / 0.100 / 100);
        } else {
            gssb = (uint16_t)(GS_A3200_ADC_TO_MV * adcval[2] / 0.025 / 100);
            gssb2 = (uint16_t)(GS_A3200_ADC_TO_MV * adcval[0] / 0.025 / 100);
            flash = (uint16_t)(GS_A3200_ADC_TO_MV * adcval[1] / 0.025 / 100);
        }
        switch (channel) {
            case GS_A3200_PWR_GSSB:
                return gssb;
            case GS_A3200_PWR_SD:
                return flash;
            case GS_A3200_PWR_GSSB2:
                return gssb2;
            case GS_A3200_PWR_PWM:
                if (GS_A3200_BOARD_REVISION >= 3) {
                    return pwm;
                }
        }
    } else {
        log_error("gs_a3200_adc_channels_sample(%i)", error);
    }
    return UINT16_MAX;
}

static gs_error_t cmd_pwr(gs_a3200_pwr_switch_t channel, gs_command_context_t *ctx)
{
    if (ctx->argc == 2) {
        bool state;
        if (gs_string_to_bool(ctx->argv[1], &state)) {
            return GS_ERROR_ARG;
        }
        gs_a3200_pwr_switch_set(channel, state);
        return GS_OK;
    }
    bool on = gs_a3200_pwr_switch_get(channel);


    gs_command_set_output_printf(ctx, "", "state", on ? "ON" : "OFF");
    gs_command_set_output_printf(ctx, "", "current [mA]", "%"PRIu16"", get_current(channel));

    return GS_OK;
}

static gs_error_t cmd_pwr_gssb(gs_command_context_t *ctx)
{
    return cmd_pwr(GS_A3200_PWR_GSSB, ctx);
}

static gs_error_t cmd_pwr_flash(gs_command_context_t *ctx)
{
    return cmd_pwr(GS_A3200_PWR_SD, ctx);
}

static gs_error_t cmd_pwr_gssb2(gs_command_context_t *ctx)
{
    return cmd_pwr(GS_A3200_PWR_GSSB2, ctx);
}

static gs_error_t cmd_pwr_pwm(gs_command_context_t *ctx)
{
    return cmd_pwr(GS_A3200_PWR_PWM, ctx);
}

static const gs_command_t pwr_subcommands[] = {
    {
        .name = "gssb",
        .help = "Set/get power for GSSB (1 = ON, 0 = OFF)",
        .usage = "[state]",
        .handler = cmd_pwr_gssb,
        .mandatory_args = 0,
        .optional_args = 1,
    },
    {
        .name = "flash",
        .help = "Set/get power for external flash (1 = ON, 0 = OFF)",
        .usage = "[state]",
        .handler = cmd_pwr_flash,
        .mandatory_args = 0,
        .optional_args = 1,
    },
    {
        .name = "gssb2",
        .help = "Set/get power for GSSB2 (1 = ON, 0 = OFF)",
        .usage = "[state]",
        .handler = cmd_pwr_gssb2,
        .mandatory_args = 0,
        .optional_args = 1,
    },
    {
        .name = "pwm",
        .help = "Set/get power for PWM outputs (1 = ON, 0 = OFF)",
        .usage = "[state]",
        .handler = cmd_pwr_pwm,
        .mandatory_args = 0,
        .optional_args = 1,
    }
};

static const gs_command_t GS_COMMAND_ROOT pwr_commands[] = {
    {
        .name = "a3200_power",
        .help = "A3200 power switches",
        .chain = GS_COMMAND_INIT_CHAIN(pwr_subcommands),
    },
};

gs_error_t gs_a3200_pwr_register_commands(void)
{
    return GS_COMMAND_REGISTER(pwr_commands);
}
