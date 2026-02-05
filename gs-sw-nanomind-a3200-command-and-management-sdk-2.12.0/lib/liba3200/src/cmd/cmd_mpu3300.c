/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   MPU3300 Gyro commands.
*/

#include <gs/a3200/mpu3300.h>
#include <gs/util/string.h>
#include <gs/util/time.h>
#include <gs/util/gosh/command.h>

static gs_error_t mpu3300_readout_bias(float *xyz_bias)
{
    gs_error_t error = GS_OK;
    gs_mpu3300_gyro_t gyro;

    for (uint8_t i=0; i<30; i++) {
        error = gs_mpu3300_read_gyro(&gyro);
        if (error) {
            return error;
        }
        xyz_bias[0] -= gyro.gyro_x/30.0;
        xyz_bias[1] -= gyro.gyro_y/30.0;
        xyz_bias[2] -= gyro.gyro_z/30.0;

        gs_time_sleep_ms(20);
    }
    return error;
}

static int cmd_mpu3300_read_raw(gs_command_context_t *ctx)
{
    gs_mpu3300_gyro_raw_t gyro_raw;
    gs_error_t error = gs_mpu3300_read_raw(&gyro_raw);
    if (error == GS_OK) {
        printf("X: %"PRId16"\r\n", gyro_raw.gyro_x);
        printf("Y: %"PRId16"\r\n", gyro_raw.gyro_y);
        printf("Z: %"PRId16"\r\n", gyro_raw.gyro_z);
    }
    return error;
}

static int cmd_mpu3300_read_loop(gs_command_context_t *ctx)
{
    gs_mpu3300_gyro_t gyro;
    uint32_t sleep_ref = gs_time_rel_ms();
    gs_mpu3300_gyro_t gyro_nobias;
    gs_mpu3300_gyro_t gyro_angle;
    memset(&gyro_angle, 0, sizeof(gyro_angle));

    // Calculate gyro bias
    float bias[3]={0,0,0};
    gs_error_t bias_error = mpu3300_readout_bias(bias);
    if (bias_error) {
        return bias_error;
    }

    for (; gs_command_wait_any_key(ctx, 0) == false; ) {
        gs_time_sleep_until_ms(&sleep_ref, 100);

        gs_error_t error = gs_mpu3300_read_gyro(&gyro);
        if (error) {
            return error;
        }

        gyro_nobias.gyro_x = gyro.gyro_x + bias[0];
        gyro_nobias.gyro_y = gyro.gyro_y + bias[1];
        gyro_nobias.gyro_z = gyro.gyro_z + bias[2];

        gyro_angle.gyro_x += gyro_nobias.gyro_x * 0.1;
        gyro_angle.gyro_y += gyro_nobias.gyro_y * 0.1;
        gyro_angle.gyro_z += gyro_nobias.gyro_z * 0.1;

        float temp;
        error = gs_mpu3300_read_temp(&temp);
        if (error) {
            return error;
        }

        printf("Rate:  X: %10.5f, Y: %10.5f, Z: %10.5f\r\n",
               gyro_nobias.gyro_x,
               gyro_nobias.gyro_y,
               gyro_nobias.gyro_z);
        printf("Angle: X: %10.5f, Y: %10.5f, Z: %10.5f\r\n",
               gyro_angle.gyro_x,
               gyro_angle.gyro_y,
               gyro_angle.gyro_z);
        printf("Temperature: %f\r\n", temp);
    }
    return GS_OK;
}

static int cmd_mpu3300_log(gs_command_context_t *ctx)
{
    gs_mpu3300_gyro_t gyro;
    uint32_t sleep_ref = gs_time_rel_ms();
    gs_mpu3300_gyro_t gyro_nobias;
    gs_mpu3300_gyro_t gyro_angle;
    memset(&gyro_angle, 0, sizeof(gyro_angle));

    // Calculate gyro bias
    float bias[3]={0,0,0};
    gs_error_t bias_error = mpu3300_readout_bias(bias);
    if (bias_error) {
        return bias_error;
    }

    unsigned long int k = 0;
    for (; gs_command_wait_any_key(ctx, 0) == false; ) {
        gs_time_sleep_until_ms(&sleep_ref, 1000);
        k++;

        gs_error_t error = gs_mpu3300_read_gyro(&gyro);
        if (error) {
            return error;
        }

        gyro_nobias.gyro_x = gyro.gyro_x + bias[0];
        gyro_nobias.gyro_y = gyro.gyro_y + bias[1];
        gyro_nobias.gyro_z = gyro.gyro_z + bias[2];

        gyro_angle.gyro_x += gyro_nobias.gyro_x * 1;
        gyro_angle.gyro_y += gyro_nobias.gyro_y * 1;
        gyro_angle.gyro_z += gyro_nobias.gyro_z * 1;

        float temp;
        error = gs_mpu3300_read_temp(&temp);
        if (error) {
            return error;
        }

        printf("%lu, %10.5f, %10.5f, %10.5f, %10.5f, %10.5f, %10.5f, %f\r\n",
               k,
               gyro_nobias.gyro_x,
               gyro_nobias.gyro_y,
               gyro_nobias.gyro_z,
               gyro_angle.gyro_x,
               gyro_angle.gyro_y,
               gyro_angle.gyro_z,
               temp);
    }

    return GS_OK;
}

static int cmd_mpu3300_calib(gs_command_context_t *ctx)
{
    // Calculate gyro bias
    float bias[3]={0,0,0};
    gs_error_t bias_error = mpu3300_readout_bias(bias);
    if (bias_error) {
        return bias_error;
    }

    printf("Bias x %f\r\n", bias[0]);
    printf("Bias y %f\r\n", bias[1]);
    printf("Bias z %f\r\n", bias[2]);

    return GS_OK;
}

static int cmd_mpu3300_read(gs_command_context_t *ctx)
{
    gs_mpu3300_gyro_t gyro_reading;
    gs_error_t error = gs_mpu3300_read_gyro(&gyro_reading);
    if (error == GS_OK) {
        printf("X: %10.5f, Y: %10.5f, Z: %10.5f\r\n", 
               gyro_reading.gyro_x, 
               gyro_reading.gyro_y, 
               gyro_reading.gyro_z);
    }
    return error;
}

static int cmd_mpu3300_read_temp(gs_command_context_t *ctx)
{
    float temp;
    gs_error_t error = gs_mpu3300_read_temp(&temp);
    if (error == GS_OK) {
        printf("Temperature: %f\r\n", temp);
    }
    return error;
}

static int cmd_mpu3300_reset(gs_command_context_t *ctx)
{
    return gs_mpu3300_reset();
}

static int cmd_mpu3300_sleep(gs_command_context_t *ctx)
{
    bool sleep;
    if (gs_string_to_bool(ctx->argv[1], &sleep) != GS_OK) {
        return GS_ERROR_ARG;
    }

    return gs_mpu3300_sleep(sleep);
}

static int cmd_mpu3300_init(gs_command_context_t *ctx)
{
    int32_t bw;
    if ((gs_string_to_int32(ctx->argv[1], &bw) != GS_OK) || (bw < 0) || (bw > 6)) {
        return GS_ERROR_ARG;
    }

    int32_t scale;
    if ((gs_string_to_int32(ctx->argv[2], &scale) != GS_OK) || (scale < 0) || (scale > 1)) {
        return GS_ERROR_ARG;
    }

    return gs_mpu3300_init(bw, scale);
}

static int cmd_mpu3300_selftest(gs_command_context_t *ctx)
{
    return gs_mpu3300_selftest();
}

static const gs_command_t mpu3300_subcommands[] = {
    {
        .name = "readraw",
        .help = "Raw gyro readings",
        .handler = cmd_mpu3300_read_raw,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "read",
        .help = "Gyro readings",
        .handler = cmd_mpu3300_read,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "cal",
        .help = "Calibrate gyro",
        .handler = cmd_mpu3300_calib,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "readloop",
        .help = "Gyro reading (loop)",
        .handler = cmd_mpu3300_read_loop,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "log",
        .help = "Gyro logging",
        .handler = cmd_mpu3300_log,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "temp",
        .help = "Read gyro temperature",
        .handler = cmd_mpu3300_read_temp,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "reset",
        .help = "Reset gyro",
        .handler = cmd_mpu3300_reset,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },{
        .name = "sleep",
        .help = "Gyro sleep",
        .usage = "<sleep 1=sleep, 0=wake>",
        .handler = cmd_mpu3300_sleep,
        .mandatory_args = 1,
    },{
        .name = "init",
        .help = "Init gyro",
        .usage = "<bw> <full_scale> (bw: 0=256, 1=188, 2=98, 3=42, 4=20, 5=10, 5=6, scale: 0=255, 1=450) ",
        .handler = cmd_mpu3300_init,
        .mandatory_args = 2,
    },{
        .name = "selftest",
        .help = "Gyro self test",
        .handler = cmd_mpu3300_selftest,
        .mandatory_args = GS_COMMAND_NO_ARGS,
    },
};

static const gs_command_t GS_COMMAND_ROOT mpu3300_commands[] = {
    {
        .name = "gyro",
        .help = "MPU3300 Gyroscope",
        .chain = GS_COMMAND_INIT_CHAIN(mpu3300_subcommands),
    },
};

gs_error_t gs_mpu3300_register_commands(void)
{
    return GS_COMMAND_REGISTER(mpu3300_commands);
}
