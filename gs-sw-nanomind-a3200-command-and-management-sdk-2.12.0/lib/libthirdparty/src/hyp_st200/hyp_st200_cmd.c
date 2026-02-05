/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
#include <gs/util/gosh/command.h>
#include <gs/thirdparty/hyp_st200/hyp_st200.h>
#include <gs/embed/drivers/uart/uart.h>

static gs_hyp_st200_handle_t handle;

static int cmd_st200_init(gs_command_context_t * context)
{
    uint8_t device;
    gs_error_t error = gs_string_to_uint8(context->argv[1], &device);
    if (error)
    {
        printf("failed to parse string [%s]\n", gs_error_string(error));
        return error;
    }

    error = gs_hyp_st200_init(device, &handle);
    if (error)
    {
        printf("failed to init star tracker [%s]\n", gs_error_string(error));
        return error;
    }

    return GS_OK;
}


static int cmd_st200_sample(gs_command_context_t * context)
{
    gs_hyp_st200_standard_frame_t frame;
    gs_error_t error = gs_hyp_st200_sample(handle, &frame);
    if (error)
    {
        printf("Failed to read sample [%s]\n", gs_error_string(error));
        return error;
    }

    gs_hyp_st200_show_frame(frame);
    return GS_OK;
}

static int cmd_st200_custom(gs_command_context_t * context)
{
    char cmd[200];
    cmd[0] = '\0';

    for (uint8_t i=1; i<context->argc; i++)
    {
        strcat(cmd, context->argv[i]);
        strcat(cmd, " ");
    }
    printf("cmd [%s]", cmd);
    strcat(cmd, "\r");
    
    return gs_hyp_st200_send_command(handle, cmd, false, true);
}


static const gs_command_t st200_commands[] = {
    {
        .name = "init",
        .help = "initialize the camera",
        .usage = "<uart device>",
        .handler = cmd_st200_init,
    },{
        .name = "sample",
        .help = "capture sample from camera",
        .handler = cmd_st200_sample,
    },{
        .name = "custom",
        .help = "send custom command",
        .handler = cmd_st200_custom,
        .optional_args = 20,
    }
};

static const gs_command_t GS_COMMAND_ROOT st200_root_command[] = {
    {
        .name = "st200",
        .help = "Hyperion ST200 Star Tracker",
        .chain = GS_COMMAND_INIT_CHAIN(st200_commands),
    },
};

void gs_hyp_st200_register_commands(void)
{
    GS_COMMAND_REGISTER(st200_root_command);
}
