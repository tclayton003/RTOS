/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/pwm.h>
#include <gs/util/gosh/command.h>

static int cmd_pwm_init(gs_command_context_t *ctx)
{
    gs_a3200_pwm_init();

    return GS_OK;
}

static int cmd_pwm_set(gs_command_context_t *ctx)
{
    uint8_t channel;
    if ((gs_string_to_uint8(ctx->argv[1], &channel) != GS_OK) || (channel > 2)) {
        return GS_ERROR_ARG;
    }
    float duty;
    if (gs_string_to_float(ctx->argv[2], &duty) != GS_OK) {
        return GS_ERROR_ARG;
    }
    float freq;
    if (gs_string_to_float(ctx->argv[3], &freq) != GS_OK) {
        return GS_ERROR_ARG;
    }

    gs_a3200_pwm_enable(channel);
    float freq_achieved = gs_a3200_pwm_set_freq(channel, freq);
    gs_a3200_pwm_set_duty(channel, duty);

    fprintf(ctx->out, "Freq requested: %f Hz, achieved: %f Hz\r\n", freq, freq_achieved);

    return GS_OK;
}

static const gs_command_t pwm_subcommands[] = {
    {
        .name = "init",
        .help = "Init PWM",
        .handler = cmd_pwm_init,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "set",
        .help = "Set PWM",
        .usage = "<channel 0,1,2> <dutycycle (-100 to +100)> <frequency (Hz)>",
        .handler = cmd_pwm_set,
        .mandatory_args = 3,
    }
};

static const gs_command_t GS_COMMAND_ROOT pwm_commands[] = {
    {
        .name = "pwm",
        .help = "Pulse-Width Modulation",
        .chain = GS_COMMAND_INIT_CHAIN(pwm_subcommands),
    },
};

gs_error_t gs_a3200_pwm_register_commands(void)
{
    return GS_COMMAND_REGISTER(pwm_commands);
}
