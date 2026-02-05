/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include "hk_srv.h"

static const gs_command_t GS_COMMAND_SUB options_commands[] = {
    {
        .name="show",
        .help="Show current option values.",
        .mandatory_args=GS_COMMAND_NO_ARGS,
        .handler=cmd_handler_hk_srv_options_show,
    },
};

static const gs_command_t GS_COMMAND_SUB beacon_commands[] = {
    {
        .name="show",
        .help="Show information on one or all loaded beacon specifications.",
        .usage="[type] [verbose]",
        .optional_args=2,
        .handler=cmd_handler_hk_srv_beacon_show,
    },
    {
        .name="samplerate",
        .help="Temporarily set the samplerate of a beacon (highest, high, medium, low, lowest), will revert on next reboot or reload.",
        .usage="<type> <samplerate>",
        .mandatory_args=2,
        .handler=cmd_handler_hk_srv_beacon_samplerate,
    },
    {
        .name="policy",
        .help="Temporarily set the autobeacon policy (always, complete, off), will revert on next reboot or reload.",
        .usage="<type> <policy>",
        .mandatory_args=2,
        .handler=cmd_handler_hk_srv_beacon_policy,
    },
};

static const gs_command_t GS_COMMAND_SUB table_commands[] = {
    {
        .name="show",
        .help="Show information on one or all sampled tables.",
        .usage="[node] [id] [verbose]",
        .optional_args=3,
        .handler=cmd_handler_hk_srv_table_show,
    },
};

static const gs_command_t GS_COMMAND_SUB hk_srv_commands[] = {
    {
        .name="reload",
        .help="Reload configuration from files.",
        .mandatory_args=GS_COMMAND_NO_ARGS,
        .handler=cmd_handler_hk_srv_reload,
    },
    {
        .name="options",
        .help="Housekeeping system option commands",
        .chain=GS_COMMAND_INIT_CHAIN(options_commands),
    },
    {
        .name="beacon",
        .help="Beacon commands",
        .chain=GS_COMMAND_INIT_CHAIN(beacon_commands),
    },
    {
        .name="table",
        .help="Table configuration commands",
        .chain=GS_COMMAND_INIT_CHAIN(table_commands),
    },
};

static const gs_command_t GS_COMMAND_ROOT root_commands[] = {
    {
        .name="hk_srv",
        .help="Housekeeping (local)",
        .chain=GS_COMMAND_INIT_CHAIN(hk_srv_commands),
    },
};

gs_error_t gs_hk_srv_register_commands(void)
{
    return GS_COMMAND_REGISTER(root_commands);
}
