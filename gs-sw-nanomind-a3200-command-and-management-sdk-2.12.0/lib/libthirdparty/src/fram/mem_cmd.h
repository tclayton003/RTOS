/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

// Common memory code for FRAMs

#include <gs/util/gosh/command.h>
#include <gs/util/hexdump.h>
#include <gs/util/base16.h>
#include <gs/util/string.h>
#include <gs/util/minmax.h>

#define FRAM_DEVICE_ID  0

static int cmd_mem_read(gs_command_context_t * ctx)
{
    uint32_t addr;
    if (gs_string_to_uint32(ctx->argv[1], &addr)) {
        return GS_ERROR_ARG;
    }

    uint32_t len;
    if (gs_string_to_uint32(ctx->argv[2], &len)) {
        return GS_ERROR_ARG;
    }

    uint32_t bytes = 0;
    uint8_t data[256];
    while (bytes < len) {
        const uint32_t chunk_sz = gs_min((len - bytes), sizeof(data));

        gs_error_t error = gs_fram_read(FRAM_DEVICE_ID, addr+bytes, data, chunk_sz);
        if (error) {
            return error;
        }
        gs_hexdump_to_stream(data, chunk_sz, GS_TYPES_UINT2PTR(addr+bytes), ctx->out);

        bytes += chunk_sz;
    }

    return GS_OK;
}

static int cmd_mem_write(gs_command_context_t * ctx)
{
    uint32_t addr;
    if (gs_string_to_uint32(ctx->argv[1], &addr)) {
        return GS_ERROR_ARG;
    }

    uint8_t data[256];
    if (strlen(ctx->argv[2]) > (sizeof(data)*2)) { // two bytes per byte in base16 encoding.
        fprintf(ctx->out, "Max data length is: %u\r\n", (unsigned int)sizeof(data));
        return GS_ERROR_ARG;
    }

    int32_t len = base16_decode(ctx->argv[2], data);
    if (len > (int)sizeof(data) || (len < 0)) {
        return GS_ERROR_ARG;
    }

    gs_error_t error = gs_fram_write(FRAM_DEVICE_ID, addr, data, len);
    if (error == GS_OK) {
        // read back
        uint8_t rdata[len];
        error = gs_fram_read(FRAM_DEVICE_ID, addr, rdata, len);
        if ((error == GS_OK) && (memcmp(data, rdata, len) != 0)) {
            error = GS_ERROR_DATA;
        }
    }
    return error;
}

static int cmd_mem_erase(gs_command_context_t * ctx)
{
    uint32_t addr;
    if (gs_string_to_uint32(ctx->argv[1], &addr)) {
        return GS_ERROR_ARG;
    }

    uint32_t len;
    if (gs_string_to_uint32(ctx->argv[2], &len)) {
        return GS_ERROR_ARG;
    }

    fprintf(ctx->out, "Erase/Clear %"PRIu32" bytes from address 0x%"PRIx32"\r\n", len, addr);

    uint32_t bytes = 0;
    uint8_t wdata[100];
    uint8_t rdata[100];
    memset(wdata, 0, sizeof(wdata));
    gs_error_t error = GS_OK;
    while ((bytes < len) && (error == GS_OK)) {
        const uint16_t chunk_sz = gs_min((len - bytes), sizeof(wdata));
        error = gs_fram_write(FRAM_DEVICE_ID, addr+bytes, wdata, chunk_sz);
        if (error == GS_OK) {
            error = gs_fram_read(FRAM_DEVICE_ID, addr+bytes, rdata, chunk_sz);
            if ((error == GS_OK) && (memcmp(wdata, rdata, sizeof(wdata)) != 0)) {
                error = GS_ERROR_DATA;
            }
        }
        bytes += chunk_sz;
    }

    return error;
}

static int cmd_mem_protect(gs_command_context_t * ctx)
{
    int8_t region;
    if (gs_string_to_int8(ctx->argv[1], &region)) {
        return GS_ERROR_ARG;
    }
    return gs_fram_protect_region(FRAM_DEVICE_ID, (gs_fram_region_t)region);
}

static const gs_command_t GS_COMMAND_SUB mem_subcommands[] = {
    {
        .name = "read",
        .help = "Read from memory",
        .usage = "<addr> <bytes>",
        .handler = cmd_mem_read,
        .mandatory_args = 2,
    },{
        .name = "write",
        .help = "Write to memory",
        .usage = "<addr> <hex data>",
        .handler = cmd_mem_write,
        .mandatory_args = 2,
    },{
        .name = "erase",
        .help = "Erase (zero) memory",
        .usage = "<addr> <bytes>",
        .handler = cmd_mem_erase,
        .mandatory_args = 2,
    },{
        .name = "protect",
        .help = "Enable/disable write protection",
        .usage = "<region: None=0, Upper_Quarter=1, Upper_Half=2, All=3>",
        .handler = cmd_mem_protect,
        .mandatory_args = 1,
    },
};
