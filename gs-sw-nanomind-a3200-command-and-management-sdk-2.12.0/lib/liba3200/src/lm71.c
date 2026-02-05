/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/lm71.h>
#include "local.h"
#include <gs/embed/asf/drivers/spi/master.h>

void gs_a3200_lm71_init(void)
{
    // The 2 sensors uses same SPI chip-select, and a GPIO for actual chip-select
    gs_spi_chip_t spi = {
        .handle = 1,
        .baudrate = 16000000,
        .bits = 8,
        .chipselect = LM71_SPI_CS,
        .spi_mode = 0,
        .stay_act = 1,
        .trans_delay = 0,
        .use_gpio_cs = true
    };

    spi.gpio_chipselect.port = GS_AVR32_GPIO_PORT_PIN_2_PORT(LM71_SPI_CS_TEMP1_PIN);
    spi.gpio_chipselect.pin  = GS_AVR32_GPIO_PORT_PIN_2_PIN(LM71_SPI_CS_TEMP1_PIN);
    gs_spi_asf_master_configure_slave(GS_A3200_SPI_SLAVE_LM71_0, &spi);

    spi.gpio_chipselect.port = GS_AVR32_GPIO_PORT_PIN_2_PORT(LM71_SPI_CS_TEMP2_PIN);
    spi.gpio_chipselect.pin  = GS_AVR32_GPIO_PORT_PIN_2_PIN(LM71_SPI_CS_TEMP2_PIN);
    gs_spi_asf_master_configure_slave(GS_A3200_SPI_SLAVE_LM71_1, &spi);
}
