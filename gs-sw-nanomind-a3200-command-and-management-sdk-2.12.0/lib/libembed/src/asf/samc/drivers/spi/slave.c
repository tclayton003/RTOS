/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/* --- includes ----------------------------------------------------------*/

#include <gs/embed/asf/samc/drivers/spi/slave.h>
#include <gs/embed/asf/samc/command.h>

// libasf
#include <gs/embed/asf/samc/convert_asf_status_code.h>
#include <gs/embed/freertos.h>
#include <spi.h>
#include <spi_interrupt.h>

#include <gs/util/drivers/spi/common.h>
#include <gs/util/gosh/command.h>
#include <gs/util/check.h>

/* --- definitions -------------------------------------------------------*/

#define LOG_DEFAULT  gs_spi_log

#define GS_SAMC_SPI_SERCOM_MODULE            SERCOM3
#define GS_SAMC_SPI_SERCOM_MUX_SETTING       SPI_SIGNAL_MUX_SETTING_I  // PA22..25: MOSI, SCK, CS, MISO
#define GS_SAMC_SPI_SERCOM_PINMUX_PAD0       PINMUX_PA22C_SERCOM3_PAD0 // MOSI
#define GS_SAMC_SPI_SERCOM_PINMUX_PAD1       PINMUX_PA23C_SERCOM3_PAD1 // SCK
#define GS_SAMC_SPI_SERCOM_PINMUX_PAD2       PINMUX_PA24C_SERCOM3_PAD2 // CS
#define GS_SAMC_SPI_SERCOM_PINMUX_PAD3       PINMUX_PA25C_SERCOM3_PAD3 // MISO

typedef struct spi_stats {
    uint32_t irq;
    uint32_t irq_ssl;
    uint32_t irq_rx;
} spi_stats_t;

typedef struct spi_channel_instance {
    // Hardware / ASF
    struct spi_module module;

    // Rx
    gs_spi_slave_receive_t rx_func;
    uint8_t rx_buffer[50];
    uint8_t rx_index;

    // Tx
    uint8_t tx_index;
    const uint8_t * tx_buffer;
    uint8_t tx_buffer_index_begin;  // tx_buffer[<0>] starts here
    uint8_t tx_buffer_index_end;    // tx_buffer[<end>] ends here

    // Stats
    spi_stats_t stats;
} spi_channel_instance_t;

// Only supports 1 channel
spi_channel_instance_t spi_slave_instance;

/* --- code    -----------------------------------------------------------*/

void gs_samc_spi_slave_get_config_defaults(gs_spi_slave_config_t * config)
{
    if (config) {
        memset(config, 0, sizeof(*config));

        config->data_order_msb = true;
        config->mode = GS_SPI_MODE_CPOL0_CPHA0;
        config->bits = 8;
    }
}

gs_error_t gs_samc_spi_slave_init(uint8_t device, const gs_spi_slave_config_t * config)
{
    GS_CHECK_ARG(device == 0);
    GS_CHECK_ARG(config != NULL);
    GS_CHECK_SUPPORTED(config->bits == 8);

    gs_log_group_register(LOG_DEFAULT);

    memset(&spi_slave_instance, 0, sizeof(spi_slave_instance));

    // Configure hardware
    {
        struct spi_config config_slave;
        spi_get_config_defaults(&config_slave);

        config_slave.mode             = SPI_MODE_SLAVE;
        config_slave.data_order       = config->data_order_msb ? SPI_DATA_ORDER_MSB : SPI_DATA_ORDER_LSB;
        config_slave.transfer_mode    = config->mode; // SPI mode - phase and polarity
        config_slave.character_size   = SPI_CHARACTER_SIZE_8BIT;
        config_slave.select_slave_low_detect_enable = true;
	config_slave.generator_source = GCLK_GENERATOR_0;

        config_slave.mode_specific.slave.preload_enable = false;
        config_slave.mode_specific.slave.frame_format = SPI_FRAME_FORMAT_SPI_FRAME;

        config_slave.mux_setting = GS_SAMC_SPI_SERCOM_MUX_SETTING;
        config_slave.pinmux_pad0 = GS_SAMC_SPI_SERCOM_PINMUX_PAD0;
        config_slave.pinmux_pad1 = GS_SAMC_SPI_SERCOM_PINMUX_PAD1;
        config_slave.pinmux_pad2 = GS_SAMC_SPI_SERCOM_PINMUX_PAD2;
        config_slave.pinmux_pad3 = GS_SAMC_SPI_SERCOM_PINMUX_PAD3;

        enum status_code status = spi_init(&spi_slave_instance.module, GS_SAMC_SPI_SERCOM_MODULE, &config_slave);
        if (status != STATUS_OK) {
            log_error("%s: spi_init() failed, status=%d", __FUNCTION__, status);
            return gs_convert_asf_status_code(status);
        }
    }

    return GS_OK;
}

gs_error_t gs_spi_slave_set_rx(uint8_t spi_channel, gs_spi_slave_receive_t rx)
{
    spi_slave_instance.rx_func = rx;
    return GS_OK;
}

static void gs_spi_interrupt_handler(uint8_t instance)
{
    /* Get device instance from the look-up table */
    struct spi_module *module = (struct spi_module *)_sercom_instances[instance];

    /* Pointer to the hardware module instance */
    SercomSpi *const spi_hw = &(module->hw->SPI);

    /* Read and mask interrupt flag register */
    const uint16_t interrupt_status = spi_hw->INTFLAG.reg;

    gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;

    ++spi_slave_instance.stats.irq;
    /**
       This flag is set when slave select low.
    */
    if (interrupt_status & SPI_INTERRUPT_FLAG_SLAVE_SELECT_LOW) {
        ++spi_slave_instance.stats.irq_ssl;
        spi_slave_instance.rx_index = 0;
        spi_slave_instance.tx_index = 2; // when getting first Rx after Select, will load byte 3, i.e. offset 2
        spi_slave_instance.tx_buffer = NULL;
    }

    /**
       This flag is set when data has been shifted into the data register.
    */
    if (interrupt_status & SPI_INTERRUPT_FLAG_RX_COMPLETE) {
        ++spi_slave_instance.stats.irq_rx;

        const bool new_request = (spi_slave_instance.rx_index == 0);
        spi_slave_instance.rx_buffer[spi_slave_instance.rx_index++] = spi_hw->DATA.reg;
        spi_slave_instance.rx_func(0, spi_slave_instance.rx_buffer, spi_slave_instance.rx_index, new_request, &cswitch);

        // queue next byte to send
        if (spi_slave_instance.tx_buffer &&
            (spi_slave_instance.tx_index >= spi_slave_instance.tx_buffer_index_begin) &&
            (spi_slave_instance.tx_index <= spi_slave_instance.tx_buffer_index_end)) {

            module->hw->SPI.DATA.reg = spi_slave_instance.tx_buffer[spi_slave_instance.tx_index - spi_slave_instance.tx_buffer_index_begin];
        } else {
            module->hw->SPI.DATA.reg = 0;
        }

        ++spi_slave_instance.tx_index;
    }

    spi_hw->INTFLAG.reg = interrupt_status;

    portEND_SWITCHING_ISR(cswitch.task_woken);
}

gs_error_t gs_spi_slave_start(uint8_t device)
{
    uint8_t instance_index = _sercom_get_sercom_inst_index(spi_slave_instance.module.hw);
    _sercom_set_handler(instance_index, gs_spi_interrupt_handler);
    _sercom_instances[instance_index] = &spi_slave_instance.module;

    spi_enable(&spi_slave_instance.module);

    spi_slave_instance.module.hw->SPI.INTENSET.reg |= SPI_INTERRUPT_FLAG_SLAVE_SELECT_LOW | SPI_INTERRUPT_FLAG_RX_COMPLETE;

    return GS_OK;
}

gs_error_t gs_spi_slave_set_response(uint8_t device, size_t offset, const uint8_t * tx, size_t size)
{
    spi_slave_instance.tx_buffer_index_begin  = offset;
    spi_slave_instance.tx_buffer_index_end    = offset + size - 1;
    spi_slave_instance.tx_buffer              = tx;

    return GS_OK;
}

static int cmd_show_stats(gs_command_context_t * ctx)
{
    {
        SERCOM_SPI_INTFLAG_Type value;
        value.reg = spi_slave_instance.module.hw->SPI.INTFLAG.reg;
        printf("INTFLAG Interrupt Flag Status and Clear: 0x%"PRIx8", DRE: %u, TXC: %u, RXC: %u, SSL: %u, ERROR: %u\r\n",
               value.reg, value.bit.DRE, value.bit.TXC, value.bit.RXC, value.bit.SSL, value.bit.ERROR);
    }
    printf("IRQs: total %"PRIu32", ssl: %"PRIu32", rx: %"PRIu32"\r\n",
           spi_slave_instance.stats.irq, spi_slave_instance.stats.irq_ssl, spi_slave_instance.stats.irq_rx);
    printf("Rx buffer: %p, index: %u\r\n",
           spi_slave_instance.rx_buffer, spi_slave_instance.rx_index);
    printf("Tx buffer: %p, index: %u, begin: %u, end: %u\r\n",
           spi_slave_instance.tx_buffer, spi_slave_instance.tx_index, spi_slave_instance.tx_buffer_index_begin, spi_slave_instance.tx_buffer_index_end);

    return GS_OK;
}

static int cmd_clear_stats(gs_command_context_t * ctx)
{
    memset(&spi_slave_instance.stats, 0, sizeof(spi_slave_instance.stats));
    cmd_show_stats(ctx);
    return GS_OK;
}

static const gs_command_t GS_COMMAND_SUB subcmd_spi_int[] = {
    {
        .name = "stats",
        .help = "Show overall SPI stats",
        .handler = cmd_show_stats,
    },
    {
        .name = "clear_stats",
        .help = "Clear stats",
        .handler = cmd_clear_stats,
    },
};

static const gs_command_t GS_COMMAND_ROOT cmd_spi_int[] = {
    {
        .name = "spi",
        .help = "SPI debug commands",
        .chain = GS_COMMAND_INIT_CHAIN(subcmd_spi_int),
    }
};

gs_error_t gs_samc_register_spi_commands(void)
{
    return GS_COMMAND_REGISTER(cmd_spi_int);
}
