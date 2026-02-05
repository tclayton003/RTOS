/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/rm3100/rm3100.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gs/embed/stdio.h>
#include <gs/embed/freertos.h>
#include <gs/util/gosh/command.h>
#include <gs/util/gosh/console.h>

static gs_rm3100_rate_t rm3100_rate = RM3100_RATE_37;
static gs_rm3100_handle_t handle;

static int cmd_rm3100_setrate(gs_command_context_t *ctx)
{
    if (ctx->argc > 1) {
        uint8_t rate;
        if (gs_string_to_uint8(ctx->argv[1], &rate)) {
            return GS_ERROR_ARG;
        }

        switch(rate) {
            case 0:
                rm3100_rate = RM3100_RATE_600;
                break;
            case 1:
                rm3100_rate = RM3100_RATE_300;
                break;
            case 2:
                rm3100_rate = RM3100_RATE_150;
                break;
            case 3:
                rm3100_rate = RM3100_RATE_75;
                break;
            case 4:
                rm3100_rate = RM3100_RATE_37;
                break;
            default:
                return GS_ERROR_RANGE;
        }
    }

    fprintf(ctx->out, "Rate %d\r\n", rm3100_rate);
    
    return GS_OK;
}

static int cmd_rm3100_init(gs_command_context_t *ctx)
{
    uint8_t addr;
    uint8_t addr_temp;
    
    if(ctx->argc > 2){

        if (gs_string_to_uint8(ctx->argv[1], &addr) || (addr == 0)) {
            return GS_ERROR_ARG;
        }

        if (gs_string_to_uint8(ctx->argv[2], &addr_temp) || (addr_temp == 0)) {
            return GS_ERROR_ARG;
        }
        
        handle = gs_rm3100_node_setup(addr, addr_temp);
    
    } else {
        handle = gs_rm3100_node_default();
    }

    addr = handle.i2c_addr;
    addr_temp = handle.i2c_addr_temp;
    
    fprintf(ctx->out, "Magnetometer Addr [addr, addr_temp] : [%d, %d]\r\n", addr, addr_temp);
    
    return gs_rm3100_init(handle, rm3100_rate);
}

static int cmd_rm3100_single(gs_command_context_t *ctx)
{
    gs_rm3100_data_t data;
    gs_error_t error;


    error = gs_rm3100_single_meas(handle, &data);
    if (error)
    {
        return error;
    }

    if (data.valid) {
        fprintf(ctx->out, "X: %4.1f mG\r\n", data.x);
        fprintf(ctx->out, "Y: %4.1f mG\r\n", data.y);
        fprintf(ctx->out, "Z: %4.1f mG\r\n", data.z);
        fprintf(ctx->out, "Magnitude: %4.1f mG\r\n", sqrt(powf(data.x,	2.0) + powf(data.y, 2.0) + powf(data.z, 2.0)));
    }
    else
        fprintf(ctx->out, "Magnetometer sample failed\r\n");
    return GS_OK;
}

static int cmd_rm3100_singlenoformat(gs_command_context_t *ctx)
{
    gs_rm3100_data_t data;
    gs_error_t error;


    error = gs_rm3100_single_meas(handle, &data);
    if (error)
    {
        return error;
    }

    if (data.valid) {
        fprintf(ctx->out, "%4.1f %4.1f %4.1f %4.1f \r\n", data.x, data.y, data.z, sqrt(powf(data.x,	2.0) + powf(data.y, 2.0) + powf(data.z, 2.0)));
    }
    else
        fprintf(ctx->out, "Magnetometer sample failed\r\n");
    return GS_OK;
}

// Do loop measurements
static int cmd_rm3100_loop(gs_command_context_t *ctx)
{
    uint32_t period = 100; // ms
    if (ctx->argc > 1) {
        if (gs_string_to_uint32(ctx->argv[1], &period) || (period == 0)) {
            return GS_ERROR_ARG;
        }
    }

    int ret = gs_rm3100_init(handle, rm3100_rate);
    if (ret != GS_OK)
        return ret;

    if ((ret = gs_rm3100_continuous_meas_setup(handle, 1)) != GS_OK)
        return ret;

    portTickType xLastWakeTime = xTaskGetTickCount();

    while (1) {

        if (gs_command_wait_any_key(ctx, 0)) {
            break;
        }

        gs_rm3100_data_t data;
        if (gs_rm3100_read_meas(handle, &data) == GS_OK) {
            gs_console_clear();
            fprintf(ctx->out, "X: %4.1f mG\r\n", data.x);
            fprintf(ctx->out, "Y: %4.1f mG\r\n", data.y);
            fprintf(ctx->out, "Z: %4.1f mG\r\n", data.z);
            fprintf(ctx->out, "Magnitude: %4.1f mG\r\n", sqrt(powf(data.x,	2.0) + powf(data.y, 2.0) + powf(data.z, 2.0)));
        }
        vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ * period / 1000);
    }

    if ((ret = gs_rm3100_continuous_meas_setup(handle, 0)) != GS_OK)
        return ret;

    return GS_OK;
}

// Do loop measurements
static int cmd_rm3100_loop_no_format(gs_command_context_t *ctx)
{
    uint32_t period = 100; // ms
    if (ctx->argc > 1) {
        if (gs_string_to_uint32(ctx->argv[1], &period) || (period == 0)) {
            return GS_ERROR_ARG;
        }
    }

    int ret = gs_rm3100_init(handle, rm3100_rate);
    if (ret != GS_OK)
        return ret;

    if ((ret = gs_rm3100_continuous_meas_setup(handle, 1)) != GS_OK)
        return ret;

    portTickType xLastWakeTime = xTaskGetTickCount();

    while (1) {

        if (gs_command_wait_any_key(ctx, 0)) {
            break;
        }

        gs_rm3100_data_t data;
        if (gs_rm3100_read_meas(handle, &data) == GS_OK) {
            fprintf(ctx->out, "%4.1f %4.1f %4.1f %4.1f\r\n", 
                    data.x, data.y, data.z, sqrt(powf(data.x,	2.0) + powf(data.y, 2.0) + powf(data.z, 2.0)));
        }
        vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ * period / 1000);
    }

    if ((ret = gs_rm3100_continuous_meas_setup(handle, 0)) != GS_OK)
        return ret;

    return GS_OK;
}

// Do loop measurements
static int cmd_rm3100_loop_raw(gs_command_context_t *ctx)
{
    uint32_t period = 100; // ms
    if (ctx->argc > 1) {
        if (gs_string_to_uint32(ctx->argv[1], &period) || (period == 0)) {
            return GS_ERROR_ARG;
        }
    }

    int ret = gs_rm3100_init(handle, rm3100_rate);
    if (ret != GS_OK)
        return ret;

    if ((ret = gs_rm3100_continuous_meas_setup(handle, 1)) != GS_OK)
        return ret;

    portTickType xLastWakeTime = xTaskGetTickCount();

    while (1) {

        if (gs_command_wait_any_key(ctx, 0)) {
            break;
        }

        gs_rm3100_rawdata_t data;
        if (gs_rm3100_read_meas_raw(handle, &data) == GS_OK) {
            fprintf(ctx->out, "%"PRId32" %"PRId32" %"PRId32" \r\n", data.x, data.y, data.z);
        }

        vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ * period / 1000);
    }

    if ((ret = gs_rm3100_continuous_meas_setup(handle, 0)) != GS_OK)
        return ret;

    return GS_OK;
}

static int cmd_rm3100_temp(gs_command_context_t *ctx)
{
    float temp = 0;

    gs_rm3100_init(handle, rm3100_rate);

    if (gs_rm3100_read_temp(handle, &temp) != GS_OK)
        return GS_ERROR_IO;

    fprintf(ctx->out, "Temp: %4.1f degC\r\n",temp);
    return GS_OK;
}

static const gs_command_t GS_COMMAND_SUB rm3100_subcommands[] = {
    {
        .name = "init",
        .usage = "[addr, addr_temp]",
        .help = "Init RM3100 (default addr: 0x20, addr_temp: 0x48)",
        .handler = cmd_rm3100_init,
        .optional_args = 2,
    },{
        .name = "loop",
        .usage = "[intv. mS]",
        .help = "read loop meas",
        .handler = cmd_rm3100_loop,
        .optional_args = 1,
    },{
        .name = "loopnoformat",
        .usage = "[intv. mS]",
        .help = "read loop meas (no-format)",
        .handler = cmd_rm3100_loop_no_format,
        .optional_args = 1,
    },{
        .name = "loopraw",
        .usage = "[intv. mS]",
        .help = "read loop meas (raw)",
        .handler = cmd_rm3100_loop_raw,
        .optional_args = 1,
    },{
        .name = "single",
        .help = "Read single meas",
        .handler = cmd_rm3100_single,
    },{
        .name = "singlenorformat",
        .help = "Read single meas (no-format)",
        .handler = cmd_rm3100_singlenoformat,
    },{
        .name = "temp",
        .help = "Read temperature",
        .handler = cmd_rm3100_temp,
    },{
        .name = "setrate",
        .usage = "[0...4]",
        .help = "Set rate",
        .handler = cmd_rm3100_setrate,
        .optional_args = 1,
    },
};

static const gs_command_t GS_COMMAND_ROOT rm3100_commands[] = {
    {
        .name = "rm3100",
        .help = "Magnetometer RM3100",
        .chain = GS_COMMAND_INIT_CHAIN(rm3100_subcommands),
    }
};

gs_error_t gs_rm3100_register_commands(void)
{
    return GS_COMMAND_REGISTER(rm3100_commands);
}
