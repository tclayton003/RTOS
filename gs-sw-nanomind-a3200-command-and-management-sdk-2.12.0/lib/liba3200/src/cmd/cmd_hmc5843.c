/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/hmc5843.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gs/embed/freertos.h>
#include <gs/embed/drivers/uart/uart.h>
#include <gs/embed/stdio.h>
#include <gs/util/drivers/i2c/master.h>
#include <gs/util/gosh/command.h>
#include <gs/util/gosh/console.h>

typedef struct hmc5843_reg_data_s {
    uint8_t id1;
    uint8_t id2;
    uint8_t id3;
    uint8_t conf_a;
    uint8_t conf_b;
    uint8_t mode;
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint8_t status;
} hmc5843_reg_data_t;

/** Default values */
static float scale = 1/1.3;

static int cmd_hmc5843_setgain(gs_command_context_t *ctx)
{
    const char * args = gs_command_args(ctx);
    unsigned int gain;
    if (sscanf(args, "%u", &gain ) != 1)
        return GS_ERROR_ARG;

    if (gain > 7)
        return GS_ERROR_ARG;

    return gs_hmc5843_set_conf(gs_hmc5843_get_rate(), gs_hmc5843_get_meas(), (gs_hmc5843_gain_t)gain);
}

static int cmd_hmc5843_setrate(gs_command_context_t *ctx)
{

    const char * args = gs_command_args(ctx);
    unsigned int rate;
    if (sscanf(args, "%u", &rate ) != 1)
        return GS_ERROR_ARG;

    if (rate > 6)
        return GS_ERROR_ARG;

    return gs_hmc5843_set_conf((gs_hmc5843_rate_t)rate, gs_hmc5843_get_meas(), gs_hmc5843_get_gain());
}

static int cmd_hmc5843_init(gs_command_context_t *ctx)
{
    return gs_hmc5843_init_force();
}

// Read in entire status of hmc5843
static int hmc5843_get_info(gs_command_context_t *ctx)
{
    gs_error_t error = gs_hmc5843_init();
    if (error)
        return error;

    uint8_t txdata[1];
    uint8_t rxdata[13];
    txdata[0] = 0x0A;
    error = gs_i2c_master_transaction(2, 0x1E, txdata, 1, rxdata, 13, 500);
    if (error)
        return error;

    hmc5843_reg_data_t * data = (void *) rxdata;

    printf("ID: %c%c%c\r\n", data->id1, data->id2, data->id3);

    uint8_t dorate = (data->conf_a >> 2) & 0x07;
    float dorate_f = 0;
    switch (dorate) {
	case 0: dorate_f = 0.5; break;
	case 1: dorate_f = 1; break;
	case 2: dorate_f = 2; break;
	case 3: dorate_f = 5; break;
	case 4: dorate_f = 10; break;
	case 5: dorate_f = 20; break;
	case 6: dorate_f = 50; break;
	case 7: dorate_f = 0; break;
    }
    printf("Data output rate is %f [Hz]\r\n", dorate_f);

    uint8_t mmode = (data->conf_a & 0x3);
    switch (mmode) {
	case 0: printf("Normal Measurement Mode\r\n"); break;
	case 1: printf("Positive Bias Mode\r\n"); break;
	case 2: printf("Negative Bias Mode\r\n"); break;
	case 3: printf("Invalid mode\r\n"); break;
    }

    uint8_t gain = ((data->conf_b >> 5) & 0x7);
    switch (gain) {
	case 0: printf("Field Range: +- 0.7 [Ga] Gain 1620\r\n"); break;
	case 1: printf("Field Range: +- 1.0 [Ga] Gain 1300\r\n"); break;
	case 2: printf("Field Range: +- 1.5 [Ga] Gain 970\r\n"); break;
	case 3: printf("Field Range: +- 2.0 [Ga] Gain 780\r\n"); break;
	case 4: printf("Field Range: +- 3.2 [Ga] Gain 530\r\n"); break;
	case 5: printf("Field Range: +- 3.8 [Ga] Gain 460\r\n"); break;
	case 6: printf("Field Range: +- 4.5 [Ga] Gain 390\r\n"); break;
	case 7: printf("Field Range: +- 6.5 [Ga] Gain 280 (Not recommended!)\r\n"); break;
    }
    printf("Scale %f\r\n",scale);

    uint8_t mode = data->mode & 3;
    switch (mode) {
	case 0: printf("Continuous-Conversion Mode\r\n"); break;
	case 1: printf("Single-Conversion Mode\r\n"); break;
	case 2: printf("Idle Mode\r\n"); break;
	case 3: printf("Sleep Mode\r\n"); break;
    }

    return GS_OK;
}

// Do single conversion and print it
static int hmc5843_test_single(gs_command_context_t *ctx)
{
    gs_error_t error = gs_hmc5843_init();
    if (error)
        return error;

    gs_hmc5843_data_t data;
    error = gs_hmc5843_read_test(&data);
    if (error == GS_OK) {
        printf("X: %4.1f mG\n\r", data.x);
        printf("Y: %4.1f mG\n\r", data.y);
        printf("Z: %4.1f mG\n\r", data.z);
        printf("Magnitude: %4.1f mG\r\n", sqrt(powf(data.x,	2.0) + powf(data.y, 2.0) + powf(data.z, 2.0)));
    }

    return error;
}

// Do bias testing
static int hmc5843_test_bias(gs_command_context_t *ctx)
{
    gs_error_t error = gs_hmc5843_init();
    if (error)
        return error;

    gs_hmc5843_set_mode(MAG_MODE_SINGLE);
    gs_hmc5843_set_conf(gs_hmc5843_get_rate(), MAG_MEAS_POS, gs_hmc5843_get_gain() );

    printf("Positive bias mode\r\n");
    vTaskDelay(configTICK_RATE_HZ * 0.1);

    hmc5843_test_single(NULL);
    hmc5843_test_single(NULL);
    hmc5843_test_single(NULL);

    gs_hmc5843_set_conf(gs_hmc5843_get_rate(), MAG_MEAS_NEG, gs_hmc5843_get_gain() );

    printf("Negative bias mode\r\n");
    vTaskDelay(configTICK_RATE_HZ * 0.1);

    hmc5843_test_single(NULL);
    hmc5843_test_single(NULL);
    hmc5843_test_single(NULL);

    gs_hmc5843_set_conf(gs_hmc5843_get_rate(), MAG_MEAS_NORM, gs_hmc5843_get_gain() );

    printf("Normal bias mode\r\n");
    vTaskDelay(configTICK_RATE_HZ * 0.1);

    hmc5843_test_single(NULL);
    hmc5843_test_single(NULL);
    hmc5843_test_single(NULL);

    gs_hmc5843_set_mode(MAG_MODE_IDLE);

    return GS_OK;
}

//Perform HMC5843 selftest
static int cmd_hmc5843_selftest(gs_command_context_t *ctx)
{
    gs_hmc5843_data_t data;
    gs_error_t error = gs_hmc5843_init();

    if (error){
        return error;
    }

    error = gs_hmc5843_selftest(&data);


    return error;

} 

// Do loop measurements
static int hmc5843_loop(gs_command_context_t *ctx)
{
    gs_error_t error = gs_hmc5843_init();
    if (error)
        return error;

    gs_hmc5843_set_mode(MAG_MODE_CONTINUOUS);

    while (1) {

        if (gs_stdio_getchar_timed(0, NULL) != GS_ERROR_TIMEOUT)
            break;

        gs_hmc5843_data_t data;
        if (gs_hmc5843_read(&data) == GS_OK) {
            gs_console_clear();
            printf("X: %4.1f mG\n\r", data.x);
            printf("Y: %4.1f mG\n\r", data.y);
            printf("Z: %4.1f mG\n\r", data.z);
            printf("Magnitude: %4.1f mG\n\r", sqrt(powf(data.x,	2.0) + powf(data.y, 2.0) + powf(data.z, 2.0)));
        }

        vTaskDelay(configTICK_RATE_HZ * 0.100);
    }

    gs_hmc5843_set_mode(MAG_MODE_IDLE);

    return GS_OK;
}

// Do loop measurements
static int hmc5843_loop_noformat(gs_command_context_t *ctx)
{
    gs_error_t error = gs_hmc5843_init();
    if (error)
        return error;

    error = gs_hmc5843_set_mode(MAG_MODE_CONTINUOUS);
    if (error)
        return error;

    while (1) {

        if (gs_stdio_getchar_timed(0, NULL) != GS_ERROR_TIMEOUT)
            break;

        gs_hmc5843_data_t data;
        if (gs_hmc5843_read(&data) == GS_OK) {
            printf(" %4.1f, ", data.x);
            printf(" %4.1f, ", data.y);
            printf(" %4.1f, ", data.z);
            printf(" %4.1f \n\r", sqrt(powf(data.x,	2.0) + powf(data.y, 2.0) + powf(data.z, 2.0)));
        }

        vTaskDelay(configTICK_RATE_HZ * 0.100);
    }

    gs_hmc5843_set_mode(MAG_MODE_IDLE);

    return GS_OK;
}

// Do loop measurements (raw values)
static int hmc5843_loop_raw(gs_command_context_t *ctx)
{
    gs_error_t error = gs_hmc5843_init();
    if (error)
        return error;

    gs_hmc5843_set_mode(MAG_MODE_CONTINUOUS);

    while (1) {

        if (gs_stdio_getchar_timed(0, NULL) != GS_ERROR_TIMEOUT)
            break;

        gs_hmc5843_data_t data;
        if (gs_hmc5843_read_raw(&data) == GS_OK) {
            printf(" %4.1f, ", data.x);
            printf(" %4.1f, ", data.y);
            printf(" %4.1f, ", data.z);
            printf(" %4.1f \n\r", sqrt(powf(data.x,	2.0) + powf(data.y, 2.0) + powf(data.z, 2.0)));
        }

        vTaskDelay(configTICK_RATE_HZ * 0.100);
    }

    gs_hmc5843_set_mode(MAG_MODE_IDLE);

    return GS_OK;
}

static const gs_command_t hmc5843_subcommands[] = {
    {
        .name = "read",
        .help = "Magnetometer read",
        .handler = hmc5843_test_single,
    },{
        .name = "loop",
        .help = "Magnetometer read in a loop",
        .handler = hmc5843_loop,
    },{
        .name = "init",
        .help = "Magnetometer init",
        .handler = cmd_hmc5843_init,
    },{
        .name = "loop_noformat",
        .help = "Magnetometer read in a loop (not formatted)",
        .handler = hmc5843_loop_noformat,
    },{
        .name = "loop_raw",
        .help = "Magnetometer read in a loop (raw output)",
        .handler = hmc5843_loop_raw,
    },{
        .name = "status",
        .help = "Read magnetometer status registers",
        .handler = hmc5843_get_info,
    },{
        .name = "bias",
        .help = "Test magnetometer bias",
        .handler = hmc5843_test_bias,
    },{
        .name = "setgain",
        .help = "Set magnetometer gain",
        .usage = "GAIN0.7 = 0  ... GAIN6.5 = 7",
        .handler = cmd_hmc5843_setgain,
    },{
        .name = "setrate",
        .help = "Set magnetometer rate",
        .usage = "RATE0.5 = 0  ... RATE50 = 6",
        .handler = cmd_hmc5843_setrate,
    },{
        .name = "selftest",
        .help = "Perform hmc5843_selftest",
        .handler = cmd_hmc5843_selftest,
    }
};

static const gs_command_t GS_COMMAND_ROOT hmc5843_commands[] = {
    {
        .name = "hmc5843",
        .help = "HMC5843 Magnetometer",
        .chain = GS_COMMAND_INIT_CHAIN(hmc5843_subcommands),
    }
};

gs_error_t gs_hmc5843_register_commands(void)
{
    return GS_COMMAND_REGISTER(hmc5843_commands);
}
