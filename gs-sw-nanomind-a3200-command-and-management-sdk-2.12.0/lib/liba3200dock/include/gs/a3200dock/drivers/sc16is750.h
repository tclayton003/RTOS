#ifndef GS_A3200DOCK_DRIVERS_SC16IS750_H
#define GS_A3200DOCK_DRIVERS_SC16IS750_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Interface for SC16IS750.

   Single UART with I2C-bus/SPI interface, 64 bytes of transmit and receive FIFOs.

   Mounted on ADCS6 and ADCS8 dock's.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Initialize device.

   @return_gs_error_t
*/
gs_error_t sc16is750_init(void);

/**
   Ping device by writing/reading to/from it's \a scratchpad.

   @return_gs_error_t
*/
gs_error_t sc16is750_scratchpad_ping(void);

/**
   GPIO - set direction (in | out).

   @param[in] gpio_dir direction: 0 = input, 1 = output. Bit 0-7: gpio0 - gpio7.
   @return_gs_error_t
*/
gs_error_t sc16is750_gpio_direction(uint8_t gpio_dir);

/**
   GPIO - set single output.

   @param[in] pin bit: 0 - 7.
   @param[in] state \a true = high, \a false = low.
   @return_gs_error_t
*/
gs_error_t sc16is750_gpio_set(uint8_t pin, bool state);

/**
   GPIO - set all outputs.

   @param[in] state \a true = high, \a false = low.
   @return_gs_error_t
*/
gs_error_t sc16is750_gpio_set_all(bool state);

/**
   UART - initialize UART.

   Initialized to: 8 data bits, no parity, 1 stop-bit.
   FIFO is enabled.

   Use sc16is750_usart_setbaud() to configure speed.

   @return_gs_error_t
*/
gs_error_t sc16is750_usart_conn_conf(void);

/**
   UART - set speed.

   Use sc16is750_usart_conn_conf() first, for basic configuration.

   @param[in] baud speed.
   @return_gs_error_t
*/
gs_error_t sc16is750_usart_setbaud(uint32_t baud);

/**
   UART - reset/clear (and enabled) FIFO.

   @return_gs_error_t
*/
gs_error_t sc16is750_usart_reset_fifo(void);

/**
   UART - read one byte (from FIFO).

   @param[in] databyte read byte.
   @return_gs_error_t
*/
gs_error_t sc16is750_usart_read_byte(uint8_t *databyte);

/**
   UART - write one byte.

   @param[in] databyte byte to write.
   @return_gs_error_t
*/
gs_error_t sc16is750_usart_write_byte(uint8_t databyte);

/**
   UART - reset/clear (and enabled) FIFO.

   @deprecated 2.4, use sc16is750_usart_reset_fifo()
   @return_gs_error_t
*/
static inline gs_error_t sc16is750_reset_fifo(void) {
    return sc16is750_usart_reset_fifo();
}

#ifdef __cplusplus
}
#endif
#endif
