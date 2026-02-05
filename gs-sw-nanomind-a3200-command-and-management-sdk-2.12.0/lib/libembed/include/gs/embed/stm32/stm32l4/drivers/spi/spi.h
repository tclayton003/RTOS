#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_SPI_SPI_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_SPI_SPI_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
    @file

    @brief STM32L4 SPI API

   This API includes initialization routine of SPI and configuration of multiple slaves.
   The cross platform SPI API holds functions for send/receive.
*/

#include <gs/util/drivers/gpio/gpio.h>
#include <gs/util/drivers/spi/master.h>
#include <stm32l4xx_hal.h>

/**
   Slave/Chip select callback, called when slave is selected and deselected.
   @note Active if not NULL in gs_stm32l4_spi_slave_config_t, otherwise slave_select_pin is used.
   @param slave SPI slave identifier
   @param true if slave selected, false if slave deselected.
 */
typedef void (*slave_select_cb_t)(uint8_t, bool);

/**
   SPI slave configuration.
*/
typedef struct gs_stm32l4_spi_slave_config {
    //! spi_device
    uint8_t spi_device;
    //! Slave specific configuration
    gs_spi_master_slave_config_t config;
    //! Slave/Chip select pin, not used if slave_select_cb is not NULL
    gs_gpio_t slave_select_pin;
    //! Slave/Chip select callback, not used if NULL. When used slave_select_pin is disabled.
    slave_select_cb_t slave_select_cb;
} gs_stm32l4_spi_slave_config_t;

/**
   Flags for SPI initialization.
 */
typedef struct {
    //! Initialize in polling mode
    bool polling_mode;
}gs_stm32_spi_config_t;

/**
   Initialize a SPI device to standard configuration
   Specific configuration is done when configuring a slave

   @param device SPI hardware device (ex. SPI1: device = 1 )
   @param config Pointer to configuration structure for this device
   @return_gs_error_t
 */
gs_error_t gs_stm32l4_spi_init(uint8_t device, const gs_stm32_spi_config_t * config);

/**
   Initialize/configure a slave to a specific SPI device
   @note The device must be initialized prior to this call by gs_stm32l4_spi_init()
   @param slave SPI slave (driver allows for 5 slaves)
   @param slave_conf SPI slave configuration
   @return_gs_error_t
 */
gs_error_t gs_stm32l4_spi_master_configure_slave(uint8_t slave, const gs_stm32l4_spi_slave_config_t * slave_conf);

#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_SPI_SPI_H_ */
