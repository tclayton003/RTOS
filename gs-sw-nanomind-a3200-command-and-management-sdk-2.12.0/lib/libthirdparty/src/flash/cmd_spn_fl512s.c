/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/flash/spn_fl512s.h>
#include <gs/util/gosh/command.h>
#include <gs/util/hexdump.h>
#include <gs/util/log.h>
#include <gs/util/base16.h>
#include <gs/util/string.h>

static uint8_t parse_partition(const char * spartition)
{
    uint8_t partition;
    if (gs_string_to_uint8(spartition, &partition) != GS_OK) {
        return UINT8_MAX;
    }
    if (partition > 1) {
        return UINT8_MAX;
    }
    return partition;
}

static int cmd_spn_fl512s_read_sr(gs_command_context_t *ctx)
{
    const uint8_t partition = parse_partition(ctx->argv[1]);
    uint8_t sr;
    gs_error_t error = spn_fl512s_read_status_register(partition, &sr);
    if (error == GS_OK) {
        fprintf(ctx->out, "Status register: 0x%"PRIx8"\r\n", sr);
    }
    return error;
}

static int cmd_spn_fl512s_read_id(gs_command_context_t *ctx)
{
    const uint8_t partition = parse_partition(ctx->argv[1]);
    char data[64];
    gs_error_t error = spn_fl512s_read_device_id(partition, data);
    if (error == GS_OK) {
        gs_hexdump_to_stream(data, sizeof(data), 0, ctx->out);
    }
    return error;
}

static int cmd_spn_fl512s_write(gs_command_context_t *ctx)
{
    const uint8_t partition = parse_partition(ctx->argv[1]);
    uint32_t addr;
    if (gs_string_to_uint32(ctx->argv[2], &addr) != GS_OK) {
        return GS_ERROR_ARG;
    }

    uint8_t data[256];
    if (strlen(ctx->argv[3]) > (sizeof(data)*2)) { // two bytes per byte in base16 encoding.
        fprintf(ctx->out, "Max data length is: %u\r\n", (unsigned int)sizeof(data));
        return GS_ERROR_ARG;
    }
    int32_t len = base16_decode(ctx->argv[3], data);
    if (len > (int)sizeof(data) || (len < 0)) {
        return GS_ERROR_ARG;
    }

    return spn_fl512s_write_data(partition, addr, data, len);
}

static int cmd_spn_fl512s_read(gs_command_context_t *ctx)
{
    const uint8_t partition = parse_partition(ctx->argv[1]);
    uint32_t addr;
    if (gs_string_to_uint32(ctx->argv[2], &addr) != GS_OK) {
        return GS_ERROR_ARG;
    }
    uint8_t data[256];
    gs_error_t error = spn_fl512s_read_data(partition, addr, data, sizeof(data));
    if (error == GS_OK) {
        gs_hexdump_to_stream(data, sizeof(data), GS_TYPES_UINT2PTR(addr), ctx->out);
    }
    return error;
}

static int cmd_spn_fl512s_erase(gs_command_context_t *ctx)
{
    const uint8_t partition = parse_partition(ctx->argv[1]);
    uint32_t addr;
    if (gs_string_to_uint32(ctx->argv[2], &addr) != GS_OK) {
        return GS_ERROR_ARG;
    }
    return spn_fl512s_erase_block(partition, addr);
}

static int cmd_spn_fl512s_erase_chip(gs_command_context_t *ctx)
{
    const uint8_t partition = parse_partition(ctx->argv[1]);
    return spn_fl512s_erase_chip(partition);
}

static const gs_command_t spn_fl512s_subcommands[] = {
    {
        .name = "readsr",
        .help = "Read Status Register",
        .usage = "<partition>",
        .handler = cmd_spn_fl512s_read_sr,
        .mandatory_args = 1,
    },{
        .name = "readid",
        .help = "Read CFI/ID",
        .usage = "<partition>",
        .handler = cmd_spn_fl512s_read_id,
        .mandatory_args = 1,
    },{
        .name = "read",
        .help = "Read Data",
        .usage = "<partition> <addr>",
        .handler = cmd_spn_fl512s_read,
        .mandatory_args = 2,
    },{
        .name = "write",
        .help = "Write Data",
        .usage = "<partition> <addr> <hex data>",
        .handler = cmd_spn_fl512s_write,
        .mandatory_args = 3,
    },{
        .name = "erase",
        .help = "Erase Data (block size 256kB)",
        .usage = "<partition> <addr>",
        .handler = cmd_spn_fl512s_erase,
        .mandatory_args = 2,
    },{
        .name = "erase_chip",
        .help = "Erase all data (up to 2 minutes)",
        .usage = "<partition>",
        .handler = cmd_spn_fl512s_erase_chip,
        .mandatory_args = 1,
    },
};

static const gs_command_t GS_COMMAND_ROOT spn_fl512s_commands[] = {
    {
        .name = "spn",
        .help = "Flash 2xS25FL512S (2 partitions/dies)",
        .chain = GS_COMMAND_INIT_CHAIN(spn_fl512s_subcommands),
    },
};

gs_error_t gs_spn_fl512s_register_commands(void)
{
    return GS_COMMAND_REGISTER(spn_fl512s_commands);
}
