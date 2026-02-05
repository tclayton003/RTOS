#ifndef GS_A3200_UART_H
#define GS_A3200_UART_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   UART interface (initialization).
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Initialize UART in MCU.

   UARTs:
   - 1: FSI connector (GPS).
   - 2: DEBUG connector.
   - 4: FSI connector.

   @param[in] uart which uart device to initialize.
   @param[in] enable currently not supported - always set to \a true.
   @param[in] bps bitrate.
   @return_gs_error_t
*/
gs_error_t gs_a3200_uart_init(uint8_t uart, bool enable, uint32_t bps);
#ifdef __cplusplus
}
#endif
#endif
