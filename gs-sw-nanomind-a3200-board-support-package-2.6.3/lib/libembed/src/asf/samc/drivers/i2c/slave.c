/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/samc/drivers/i2c/slave.h>
#include <gs/embed/asf/samc/command.h>

#include <i2c_slave.h>
#include <i2c_slave_interrupt.h>

#include <gs/util/gosh/command.h>
#include <gs/util/string.h>

#define LOG_DEFAULT gs_i2c_log

#define RX_DATA_LENGTH 100
static uint8_t i2c_rx_buffer[RX_DATA_LENGTH];
static struct i2c_slave_packet rx_packet;
static struct i2c_slave_packet tx_packet;
static struct i2c_slave_module i2c_slave_instance;
static gs_i2c_slave_receive_t rx_func;

static struct {
    uint32_t callbacks[_I2C_SLAVE_CALLBACK_N];
    uint32_t set_response;
} i2c_stats;

// from slave (me) to master
static void i2c_read_request_callback(struct i2c_slave_module *const module)
{
    ++i2c_stats.callbacks[I2C_SLAVE_CALLBACK_READ_REQUEST];

    if (tx_packet.data) {
        i2c_slave_write_packet_job(module, &tx_packet);
    }
    tx_packet.data = NULL;
}

static void i2c_write_packet_job_complete_callback(struct i2c_slave_module *const module)
{
    ++i2c_stats.callbacks[I2C_SLAVE_CALLBACK_WRITE_COMPLETE];
}

// from master to slave (me)
static void i2c_write_request_callback(struct i2c_slave_module *const module)
{
    ++i2c_stats.callbacks[I2C_SLAVE_CALLBACK_WRITE_REQUEST];

    rx_packet.data        = i2c_rx_buffer;
    rx_packet.data_length = RX_DATA_LENGTH;
    if (i2c_slave_read_packet_job(module, &rx_packet) != STATUS_OK) {
        rx_packet.data = NULL;
    }
}

static void i2c_read_packet_job_complete_callback(struct i2c_slave_module *const module)
{
    ++i2c_stats.callbacks[I2C_SLAVE_CALLBACK_READ_COMPLETE];

    if (rx_packet.data && (module->buffer >= rx_packet.data)) {
        rx_func(0, rx_packet.data, (module->buffer - rx_packet.data), NULL);
    }
}

static void i2c_error_callback(struct i2c_slave_module *const module)
{
    ++i2c_stats.callbacks[I2C_SLAVE_CALLBACK_ERROR];
}

static void i2c_error_last_callback(struct i2c_slave_module *const module)
{
    ++i2c_stats.callbacks[I2C_SLAVE_CALLBACK_ERROR_LAST_TRANSFER];
}

/* Initialize config structure and module instance. */
static void configure_i2c_slave(uint8_t device, uint8_t addr)
{
    struct i2c_slave_config config;
    Sercom * module;
    memset(&config, 0, sizeof(config));
    i2c_slave_get_config_defaults(&config);

    config.address        = addr;
    config.address_mode   = I2C_SLAVE_ADDRESS_MODE_MASK;
    switch (device) {
        case 0:
        default:
            config.pinmux_pad0    = PINMUX_PA08C_SERCOM0_PAD0;
            config.pinmux_pad1    = PINMUX_PA09C_SERCOM0_PAD1;
            module = SERCOM0;
            break;
        case 3:
            config.pinmux_pad0    = PINMUX_PA22C_SERCOM3_PAD0;
            config.pinmux_pad1    = PINMUX_PA23C_SERCOM3_PAD1;
            module = SERCOM3;
            break;
    }

    config.transfer_speed = I2C_SLAVE_SPEED_STANDARD_AND_FAST;
    config.scl_stretch_only_after_ack_bit = false;

    i2c_slave_init(&i2c_slave_instance, module, &config);
    i2c_slave_enable(&i2c_slave_instance);
}

static void register_i2c_slave_callback(i2c_slave_callback_t callback,
                                        enum i2c_slave_callback callback_type)
{
    i2c_slave_register_callback(&i2c_slave_instance, callback, callback_type);
    i2c_slave_enable_callback(&i2c_slave_instance, callback_type);
}

static void configure_i2c_slave_callbacks(void)
{
    register_i2c_slave_callback(i2c_read_request_callback, I2C_SLAVE_CALLBACK_READ_REQUEST);
    register_i2c_slave_callback(i2c_write_request_callback, I2C_SLAVE_CALLBACK_WRITE_REQUEST);

    register_i2c_slave_callback(i2c_read_packet_job_complete_callback, I2C_SLAVE_CALLBACK_READ_COMPLETE);
    register_i2c_slave_callback(i2c_write_packet_job_complete_callback, I2C_SLAVE_CALLBACK_WRITE_COMPLETE);

    register_i2c_slave_callback(i2c_error_callback, I2C_SLAVE_CALLBACK_ERROR);
    register_i2c_slave_callback(i2c_error_last_callback, I2C_SLAVE_CALLBACK_ERROR_LAST_TRANSFER);
}

gs_error_t gs_samc_i2c_slave_init(uint8_t device, const gs_i2c_bus_config_t * config)
{
    GS_CHECK_HANDLE((device == 0) || (device == 3));
    GS_CHECK_ARG(config != NULL);
    GS_CHECK_ARG(config->mode == GS_I2C_SLAVE);

    gs_log_group_register(LOG_DEFAULT);

    configure_i2c_slave(device, config->addr);
    configure_i2c_slave_callbacks();

    return GS_OK;
}

gs_error_t gs_i2c_slave_set_rx(uint8_t device, gs_i2c_slave_receive_t rx)
{
    rx_func = rx;
    return GS_OK;
}

gs_error_t gs_i2c_slave_set_response(uint8_t bus, const uint8_t * tx, size_t size)
{
    tx_packet.data = (void*) tx;
    tx_packet.data_length = size;
    ++i2c_stats.set_response;
    return GS_OK;
}

static int cmd_i2c_stats(gs_command_context_t * ctx)
{
    printf("CB WRITE_COMPLETE:      %"PRIu32"\r\n", i2c_stats.callbacks[I2C_SLAVE_CALLBACK_WRITE_COMPLETE]);
    printf("CB READ_COMPLETE:       %"PRIu32"\r\n", i2c_stats.callbacks[I2C_SLAVE_CALLBACK_READ_COMPLETE]);
    printf("CB READ_REQUEST:        %"PRIu32"\r\n", i2c_stats.callbacks[I2C_SLAVE_CALLBACK_READ_REQUEST]);
    printf("CB WRITE_REQUEST:       %"PRIu32"\r\n", i2c_stats.callbacks[I2C_SLAVE_CALLBACK_WRITE_REQUEST]);
    printf("CB ERROR:               %"PRIu32"\r\n", i2c_stats.callbacks[I2C_SLAVE_CALLBACK_ERROR]);
    printf("CB ERROR_LAST_TRANSFER: %"PRIu32"\r\n", i2c_stats.callbacks[I2C_SLAVE_CALLBACK_ERROR_LAST_TRANSFER]);
    printf("set response:           %"PRIu32"\r\n", i2c_stats.set_response);

    printf("module:\r\n");
    printf(" buffer length: %u\r\n", i2c_slave_instance.buffer_length);
    printf(" buffer remain: %u\r\n", i2c_slave_instance.buffer_remaining);
    printf(" buffer       : %p\r\n", i2c_slave_instance.buffer);
    printf(" direction    : %d\r\n", i2c_slave_instance.transfer_direction);
    printf(" status       : %d\r\n", i2c_slave_instance.status);

    return GS_OK;
}

static const gs_command_t GS_COMMAND_SUB subcmd_i2c[] = {
    {
        .name = "stats",
        .help = "I2C stats",
        .handler = cmd_i2c_stats,
    }
};

static const gs_command_t GS_COMMAND_ROOT cmd_i2c[] = {
    {
        .name = "i2c",
        .help = "I2C commands",
        .chain = GS_COMMAND_INIT_CHAIN(subcmd_i2c),
    }
};

gs_error_t gs_samc_register_i2c_commands(void)
{
    return GS_COMMAND_REGISTER(cmd_i2c);
}
