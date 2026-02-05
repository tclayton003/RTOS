/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/a3200/a3200.h>
#include <gs/a3200/lm71.h>
//#include <gs/util/time.h>
#include <gs/util/gosh/command.h>

static int cmd_lm71(gs_command_context_t * ctx)
{
    // Read board temperature sensors
    int16_t sensor1;
    gs_error_t error = gs_lm71_read_temp(GS_A3200_SPI_SLAVE_LM71_0, 100, &sensor1);
    if (error) {
        return error;
    }

    int16_t sensor2;
    error = gs_lm71_read_temp(GS_A3200_SPI_SLAVE_LM71_1, 100, &sensor2);
    if (error) {
        return error;
    }

    // Print values to stdout
    fprintf(ctx->out, "Temp1: %.1f, Temp2 %.1f\r\n",
            sensor1/10.0, sensor2/10.0);

    return GS_OK;
}

static const gs_command_t GS_COMMAND_ROOT commands[] = {
    {
        .name = "lm71",
        .help = "A3200 LM71 temperature sensors.",
        .handler = cmd_lm71,
    },
};

gs_error_t gs_a3200_lm71_register_commands(void)
{
    return GS_COMMAND_REGISTER(commands);
}
