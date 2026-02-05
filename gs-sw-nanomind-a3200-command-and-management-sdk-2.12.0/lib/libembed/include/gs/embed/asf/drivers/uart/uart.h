#ifndef GS_EMBED_DRIVERS_USART_USART_H
#define GS_EMBED_DRIVERS_USART_USART_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   ASF: USART interface.
*/

#include <gs/util/error.h>

/**
 * Initialise UART
 *
 * @param handle usart[0,1,2,3]
 * @param fcpu CPU speed in Hz
 * @param usart_baud Baud rate in bps
 */
void usart_init(int handle, uint32_t fcpu, uint32_t usart_baud);

/**
 * In order to catch incoming chars use the callback.
 * Only one callback per interface.
 * @param handle usart[0,1,2,3]
 * @param callback function pointer
 */
typedef void (*usart_callback_t) (uint8_t * buf, int len, void * pxTaskWoken);

/**
   Set usart callback.
   
   @param[in] handle usart[0,1,2,3]
   @param[in] callback callback function.
*/
void usart_set_callback(int handle, usart_callback_t callback);

/**
 * Insert a character to the RX buffer of a usart
 * @param[in] handle usart[0,1,2,3]
 * @param[in] c Character to insert
 * @param[out] pxTaskWoken context switch
 */
void usart_insert(int handle, char c, void * pxTaskWoken);

/**
 * Polling putchar
 *
 * @param handle usart[0,1,2,3]
 * @param c Character to transmit
 */
void usart_putc(int handle, char c);

/**
 * Send char buffer on UART
 *
 * @param handle usart[0,1,2,3]
 * @param buf Pointer to data
 * @param len Length of data
 */
void usart_putstr(int handle, const char *buf, int len);

/**
 * Buffered getchar
 *
 * @param handle usart[0,1,2,3]
 * @return Character received
 */
char usart_getc(int handle);

/**
   Get character with timeout.
   @param[in] handle device handle
   @param_int_timeout_ms
   @param[out] ch character
   @return_gs_error_timeout
   @return_gs_error_t
*/
gs_error_t usart_getc_timed(int handle, int timeout_ms, char *ch);

/**
 * Buffered getchar (not blocking)
 *
 * @param handle usart[0,1,2,3]
 * @return Character received
 */
char usart_getc_nblock(int handle);

/**
 * Report number of messages waiting on the RX queue of a handle
 * @param handle usart[0,1,2,3]
 * @return Number of char's in rx buffer
 */
int usart_messages_waiting(int handle);

#endif
