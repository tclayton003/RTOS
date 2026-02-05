/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/adc_channels.h>
#include <gs/util/gosh/command.h>

static int adc_read(gs_command_context_t *ctx)
{
    int16_t adcval[GS_A3200_ADC_NCHANS];
    gs_error_t error = gs_a3200_adc_channels_sample(adcval);
    if (error == GS_OK) {
        for (unsigned int i = 0; i < GS_A3200_ADC_NCHANS; i++) {
            fprintf(ctx->out, "ADC[%02u] = %d\r\n", i, adcval[i]);
        }
    }
    return error;
}

static const gs_command_t adc_subcommands[] = {
    {
        .name = "read",
        .help = "Read",
        .handler = adc_read,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    }
};

static const gs_command_t GS_COMMAND_ROOT adc_commands[] = {
    {
        .name = "adc",
        .help = "A/D converter",
        .chain = GS_COMMAND_INIT_CHAIN(adc_subcommands),
    },
};

gs_error_t gs_a3200_adc_register_commands(void)
{
    return GS_COMMAND_REGISTER(adc_commands);
}
