#ifndef GS_EMBED_ASF_SAMC_DRIVERS_SPI_SPI_H
#define GS_EMBED_ASF_SAMC_DRIVERS_SPI_SPI_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/drivers/spi/slave.h>

/**
   SPI slave default configuration.

   @param[out] config user supplied buffer for returning default configuration.
*/
void gs_samc_spi_slave_get_config_defaults(gs_spi_slave_config_t * config);

/**
   Initialize SPI slave.
   
   @param[in] device SPI device
   @param[in] config configuration.
*/
gs_error_t gs_samc_spi_slave_init(uint8_t device, const gs_spi_slave_config_t * config);

#endif
