/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Device driver specific commands
*/

#include <gs/embed/command.h>
#include <gs/embed/drivers/can/can.h>
#include <gs/util/gosh/command.h>
#include <stdlib.h>

static int cmd_can_drv_stats(gs_command_context_t *ctx)
{
    uint8_t device;
    if (gs_string_to_uint8(ctx->argv[1], &device)) {
        return GS_ERROR_ARG;
    }
    return gs_can_driver_print_stats(ctx->out, device);
}

static const gs_command_t driver_subcommands[] = {
    {
        .name = "can",
        .help = "Show CAN driver stats",
        .usage = "<device>",
        .handler = cmd_can_drv_stats,
        .mandatory_args = 1,
    },
};

static const gs_command_t GS_COMMAND_ROOT driver_commands[] = {
    {
        .name = "driver",
        .help = "Driver stats",
        .chain = GS_COMMAND_INIT_CHAIN(driver_subcommands),
    },
};

gs_error_t gs_embed_register_driver_commands(void)
{
    return GS_COMMAND_REGISTER(driver_commands);
}
