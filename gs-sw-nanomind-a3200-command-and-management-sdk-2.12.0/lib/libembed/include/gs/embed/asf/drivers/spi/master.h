#ifndef GS_ASF_DRIVERS_SPI_MASTER_H
#define GS_ASF_DRIVERS_SPI_MASTER_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   ASF: Master SPI interface.
*/

#include <gs/util/drivers/spi/master.h>
#include <gs/util/drivers/gpio/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Legacy return OK from SPI APIs
*/
#define GS_SPI_OK GS_OK // same value (= 0)

/**
   Parameters for a single chip-select line.
*/
typedef struct {
    uint8_t       handle;           //!< Hardware device
    uint8_t       chipselect;       //!< Chip Select index [0-15] (decode) or [0-3] (normal)
    uint32_t      baudrate;         //!< Baudrate for the SPI
    uint8_t       bits;             //!< Number of bits in each character (8 - 16)
    uint8_t       spck_delay;       //!< Delay before first clock pulse after selecting slave (in MCK)
    uint8_t       trans_delay;      //!< Delay between each transfer/character (in MCK * 32)
    bool          stay_act;         //!< Set this chip to stay active after last transfer to it
    gs_spi_mode_t spi_mode;         //!< Which of the four SPI-modes to use when transmitting
    bool          use_gpio_cs;      //!< True if gpio_chipselect should be used instead of dedicated CS
    gs_gpio_t     gpio_chipselect;  //!< Use this, instead of the dedicated HW chipselect (only used is use_gpio_cs is true)
} gs_spi_chip_t;

/**
   Initialize hardware SPI device.
   @param[in] device Hardware number (0 for SPI0, 1 for SPI1)
   @param[in] decode configure device for decode or normal chip-select.
   @return_gs_error_t
*/
gs_error_t gs_spi_init_device(uint8_t device, bool decode);

/**
   Configure a SPI slave connection.
   Some cross-platform SPI master interfaces (e.g. libparam SPI) relies on a \a slave being configured.
   @param[in] slave handle
   @param[in] chip chip select and configuration
   @return_gs_error_t
*/
gs_error_t gs_spi_asf_master_configure_slave(uint8_t slave, const gs_spi_chip_t * chip);

/**
   Check if a SPI slave is initialized.
   Some cross-platform SPI master interfaces (e.g. libparam SPI) relies on a \a slave being configured.
   @param[in] slave handle
   @return \a true if initialized, else false.
*/
bool gs_spi_master_is_slave_configured(uint8_t slave);

#ifdef __cplusplus
}
#endif
#endif
