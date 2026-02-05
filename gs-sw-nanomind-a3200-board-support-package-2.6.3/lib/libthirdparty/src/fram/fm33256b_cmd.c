/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/fram/fm33256b.h>
#include <gs/util/clock.h>

#define gs_fram_read            gs_fm33256b_fram_read
#define gs_fram_write           gs_fm33256b_fram_write
#define gs_fram_region_t        gs_fm33256b_fram_region_t
#define gs_fram_protect_region  gs_fm33256b_fram_protect_region

#include "mem_cmd.h" // insert common code

static int cmd_rtc_get(gs_command_context_t * ctx)
{
    gs_timestamp_t tm;
    gs_error_t ret = gs_fm33256b_rtc_driver.get_time(NULL, &tm);
    if (ret == GS_OK) {
        char buf[50];
        gs_clock_to_iso8601_string(&tm, buf, sizeof(buf));
        fprintf(ctx->out, "%s\r\n", buf);
    }
    return ret;
}

static int cmd_rtc_set(gs_command_context_t * ctx)
{
    gs_timestamp_t ts;
    if (gs_clock_from_string(ctx->argv[1], &ts)) {
        return GS_ERROR_ARG;
    }

    return gs_fm33256b_rtc_driver.set_time(NULL, &ts);
}

static const gs_command_t GS_COMMAND_SUB fm33256b_rtc_subcommands[] = {
    {
        .name = "get",
        .help = "Read the RTC clock",
        .handler = cmd_rtc_get,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "set",
        .help = "Set the RTC clock",
        .usage = "[<sec.nsec> | <YYYY-MM-DDTHH:MM:SSZ>]",
        .handler = cmd_rtc_set,
        .mandatory_args = 1,
    },
};

static const gs_command_t GS_COMMAND_SUB gs_fm33256b_subcommands[] = {
    {
        .name = "mem",
        .help = "Memory",
        .chain = GS_COMMAND_INIT_CHAIN(mem_subcommands),
    },{
        .name = "rtc",
        .help = "Real Time Clock",
        .chain = GS_COMMAND_INIT_CHAIN(fm33256b_rtc_subcommands),
    },
};

static const gs_command_t GS_COMMAND_ROOT gs_fm33256b_commands[] = {
    {
        .name = "fm33256b",
        .help = "FRAM FM33256B",
        .chain = GS_COMMAND_INIT_CHAIN(gs_fm33256b_subcommands),
    }
};

gs_error_t gs_fm33256b_register_commands(void)
{
    return GS_COMMAND_REGISTER(gs_fm33256b_commands);
}
