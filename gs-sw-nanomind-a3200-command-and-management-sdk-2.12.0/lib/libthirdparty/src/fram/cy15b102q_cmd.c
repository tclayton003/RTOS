/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/fram/cy15b102q.h>

#define gs_fram_read            gs_cy15b102q_fram_read
#define gs_fram_write           gs_cy15b102q_fram_write
#define gs_fram_region_t        gs_cy15b102q_fram_region_t
#define gs_fram_protect_region  gs_cy15b102q_fram_protect_region

#include "mem_cmd.h" // insert common code

static const gs_command_t GS_COMMAND_SUB cy15b102q_subcommands[] = {
    {
        .name = "mem",
        .help = "Memory",
        .chain = GS_COMMAND_INIT_CHAIN(mem_subcommands),
    },
};

static const gs_command_t GS_COMMAND_ROOT gs_cy15b102q_commands[] = {
    {
        .name = "cy15b102q",
        .help = "FRAM CY15B102Q",
        .chain = GS_COMMAND_INIT_CHAIN(cy15b102q_subcommands),
    }
};

gs_error_t gs_cy15b102q_register_commands(void)
{
    return GS_COMMAND_REGISTER(gs_cy15b102q_commands);
}
