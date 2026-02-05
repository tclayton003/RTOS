#ifndef GS_PARAM_PP_SPI_SLAVE_DISPATCH_HANDLER_H
#define GS_PARAM_PP_SPI_SLAVE_DISPATCH_HANDLER_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   SPI dispatch handler.
*/

#include <gs/util/drivers/spi/slave.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Dispatch handler - processes a request.

   Register using gs_spi_slave_set_rx().

   @param[in] device SPI device (handle).
   @param[in] rx_buffer Pointer to start of rx buffer.
   @param[in] rx number of bytes received so far.
   @param[in] new_request \a true on the first callback of new data, \a false on receiving additional data during same \a chip-select. Can be used to bring receiver back in sync with new request.
   @param_cswitch
   @return total number of bytes to receive before next call back. Return 0 to ignore rest of data - no additional call backs will be done for current SPI transaction.
*/
uint8_t gs_pp_spi_slave_rx(uint8_t device, const uint8_t * rx_buffer, size_t rx, bool new_request, gs_context_switch_t * cswitch);

#ifdef __cplusplus
}
#endif
#endif
