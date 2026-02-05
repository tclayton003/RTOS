#ifndef GS_EMBED_DRIVERS_UART_UART_H
#define GS_EMBED_DRIVERS_UART_UART_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Cross platform UART API.

   The API has no access locking due to performance.
*/

#include <gs/util/queue.h>
#include <gs/embed/freertos.h>

/**
   Parity check.
*/
typedef enum {
    //! None
    GS_UART_NO_PARITY = 0,
    //! Even
    GS_UART_EVEN_PARITY = 1,
    //! Odd
    GS_UART_ODD_PARITY = 2,
} gs_uart_parity_t;

/**
   Flow control.
*/
typedef enum {
    //! No flow control
    GS_UART_NO_FLOW_CTRL = 0,
} gs_uart_flow_ctrl_t;

/**
   Stop bits.
*/
typedef enum {
    //! 1 stop bit
    GS_UART_1_STOP_BIT    = 0,
    //! 1.5 stop bits
    GS_UART_1_5_STOP_BITS = 1,
    //! 2 stop bits
    GS_UART_2_STOP_BITS   = 2,
} gs_uart_stop_bits_t;

/**
   Default UART speed.
*/
#define GS_UART_DEFAULT_BPS  500000

/**
   Default UART data bits.
*/
#define GS_UART_DEFAULT_DATA_BITS 8

/**
   Communication values.
   @see gs_uart_get_default_comm()
*/
typedef struct {
    //! Bits per second (default: #GS_UART_DEFAULT_BPS)
    uint32_t bps;
    //! Number of data bits (default: #GS_UART_DEFAULT_DATA_BITS)
    uint8_t data_bits;
    //! Parity bit type (default: #GS_UART_NO_PARITY)
    gs_uart_parity_t parity_bit;
    //! Number of stop bits (default: #GS_UART_1_STOP_BIT)
    gs_uart_stop_bits_t stop_bits;
    //! Type of flow control (default: #GS_UART_NO_FLOW_CTRL)
    gs_uart_flow_ctrl_t flow_control;
} gs_uart_comm_t;

/**
   Callback when receiving data.
   @note Called from within ISR.
*/
typedef void (*gs_uart_rx_callback_isr_t)(void * user_data, const uint8_t * data, size_t data_size, gs_context_switch_t * cswitch);

/**
   Configuration values.
   @see gs_uart_get_default_config()
*/
typedef struct {
    //! Communication settings
    gs_uart_comm_t comm;
    //! Tx queue size
    size_t tx_queue_size;
    //! Rx queue size
    size_t rx_queue_size;
    //! Rx callback. If set, \a rx_queue should be zero
    gs_uart_rx_callback_isr_t rx_callback_isr;
    //! Rx callback user data.
    void * rx_callback_isr_user_data;
} gs_uart_config_t;

/**
   Get default uart communication settings.

   @param[out] buffer user supplied buffer for returning default settings.
   @return_gs_error_t
*/
void gs_uart_get_default_comm(gs_uart_comm_t * buffer);

/**
   Get default uart configuration.

   @param[out] buffer user supplied buffer for returning default configuration.
*/
void gs_uart_get_default_config(gs_uart_config_t * buffer);

/**
   Initialize an uart device.

   @param_device
   @param[in] config Configuration of the uart
   @return GS_ERROR_NOT_SUPPORTED if un-supported values specified, e.g. communication settings.
   @return GS_ERROR_IN_USE if device already initialized.
   @return_gs_error_t
*/
gs_error_t gs_uart_init(uint8_t device, const gs_uart_config_t * config);

/**
   Change communication settings.

   @note Changing the callback may cause the uart to be stopped and restarted, e.g. lossing data.
   @param_device
   @param[in] comm new communication settings.
   @return GS_ERROR_NOT_SUPPORTED if un-supported values specified, e.g. communication settings.
   @return_gs_error_t
*/
gs_error_t gs_uart_change_comm(uint8_t device, const gs_uart_comm_t * comm);

/**
   Set callback.

   If set, callback will be called instead of queuing the received data.

   @note Changing the callback may cause the uart to be stopped and restarted, e.g. losing data.
   @param_device
   @param[in] user_func call back function
   @param[in] user_data user data.
   @return_gs_error_t
*/
gs_error_t gs_uart_set_rx_callback(uint8_t device, gs_uart_rx_callback_isr_t user_func, void * user_data);

/**
   UART status bits.

   @see gs_uart_status()
*/
typedef enum {
    //! Set if received data lost, e.g. Rx queue full.
    GS_UART_STATUS_RX_LOST = (1 << 0),
} gs_uart_status_t;

/**
   Return current status of the uart.

   Errors states are cleared, once read.

   @param_device
   @return current status of the uart, see #gs_uart_status_t
*/
uint32_t gs_uart_status(uint8_t device);

/**
   Write single byte to uart.

   @param_device
   @param_int_timeout_ms
   @param[in] data data to write.
   @return_gs_error_timeout
   @return_gs_error_t
*/
gs_error_t gs_uart_write(uint8_t device, int timeout_ms, uint8_t data);

/**
   Write multiple bytes to uart.

   @param_device
   @param_int_timeout_ms
   @param[in] data data to write.
   @param[in] data_size number elements to write.
   @param[out] wrote number of elements written.
   @return_gs_error_timeout
   @return_gs_error_t
*/
gs_error_t gs_uart_write_buffer(uint8_t device, int timeout_ms, const uint8_t * data, size_t data_size, size_t * wrote);

/**
   Write multiple bytes directly to uart.

   By passes any pending characters in the Tx queue.

   @param_device
   @param[in] data data to write.
   @param[in] data_size number elements to write.
   @param[out] wrote number of elements written.
   @return_gs_error_t
*/
gs_error_t gs_uart_write_buffer_direct(uint8_t device, const uint8_t * data, size_t data_size, size_t * wrote);

/**
   Read byte from uart queue.

   @param_device
   @param_int_timeout_ms
   @param[out] buffer data read
   @return_gs_error_timeout
   @return_gs_error_t
*/
gs_error_t gs_uart_read(uint8_t device, int timeout_ms, uint8_t * buffer);

/**
   Read multiple bytes from uart queue.

   @param_device
   @param_int_timeout_ms
   @param[out] buffer data read
   @param[in] buffer_size size of provided \a buffer
   @param[out] read number of elements read.
   @return_gs_error_timeout
   @return_gs_error_t
*/
gs_error_t gs_uart_read_buffer(uint8_t device, int timeout_ms, uint8_t * buffer, size_t buffer_size, size_t * read);

#endif
