#ifndef GS_THIRDPARTY_LM71_LM71_H
#define GS_THIRDPARTY_LM71_LM71_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Driver for LM71 temperature sensor.

   Datasheet: http://www.ti.com/lit/ds/symlink/lm71.pdf.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Read temperature.

   @note SPI slave and device has to be initialized prior.

   @param[in] spi_slave Configured SPI slave
   @param[in] spi_timeout_ms Timeout for the SPI transaction in mili seconds
   @param[out] temperature Temperature in deci degrees Celcius
   @return_gs_error_t
*/
gs_error_t gs_lm71_read_temp(uint8_t spi_slave, int spi_timeout_ms, int16_t * temperature);

#ifdef __cplusplus
}
#endif
#endif
