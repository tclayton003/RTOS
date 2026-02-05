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
   Setup/configure SPI channel/select.
   For normal chip-select (0-3), this function must be called once to configure chip-select-registers 0-3.
   For decode chip-select, this function will automatically be called from \a gs_spi_select_chip().
   @deprecated 1.4, use gs_spi_asf_master_configure_slave()
   @param[in] chip chip configuration.
   @return_gs_error_t
*/
gs_error_t gs_spi_setup_chip(gs_spi_chip_t * chip) __attribute__ ((deprecated));

/**
   Send a byte/word via SPI
   Depending on the transfer-size specified in spi_setup_chip, 8 to 16 bits will be transferred from the 16 bit data-word
   @deprecated 1.4, use gs_spi_master_transaction()
   @param[in] chip chip select and configuration
   @param[in] data data to chip
   @return_gs_error_t
*/
gs_error_t gs_spi_write(gs_spi_chip_t * chip, uint16_t data) __attribute__ ((deprecated));

/**
   Read a byte/word from SPI
   Depending on the transfer-size specified in spi_setup_chip, 8 to 16 bits will be received from the SPI device.
   @deprecated 1.4, use gs_spi_master_transaction()
   @param[in] chip chip select and configuration
   @param[out] data data read from chip
   @return_gs_error_t
*/
gs_error_t gs_spi_read(gs_spi_chip_t * chip, uint16_t * data) __attribute__ ((deprecated));

/**
   Select chip (and acquire lock).
   If device configured in decode mod, the communication will be re-configured.
   @note if call succeeds, gs_spi_unselect_chip() must be called to release lock.
   @deprecated 1.4, use gs_spi_master_transaction()
   @param[in] chip chip select and configuration
   @return_gs_error_t
*/
gs_error_t __attribute__ ((warn_unused_result)) gs_spi_select_chip(gs_spi_chip_t * chip) __attribute__ ((deprecated));

/**
   Un-select chip (and release lock).
   @deprecated 1.4, use gs_spi_master_transaction()
   @param[in] chip chip select and configuration
   @return_gs_error_t
*/
gs_error_t gs_spi_unselect_chip(gs_spi_chip_t * chip) __attribute__ ((deprecated));

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

/**
   Set the timeout for SPI chip select.
   Default 100ms.

   @param[in] milisec default timeout in mS.
   @return_gs_error_t
*/
gs_error_t gs_spi_set_cs_timeout(uint16_t milisec);

#ifdef __cplusplus
}
#endif
#endif
