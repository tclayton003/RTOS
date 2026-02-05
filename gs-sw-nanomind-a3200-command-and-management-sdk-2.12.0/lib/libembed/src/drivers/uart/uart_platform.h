#ifndef LIB_LIBEMBED_SRC_DRIVERS_UART_UART_PLATFORM_H_
#define LIB_LIBEMBED_SRC_DRIVERS_UART_UART_PLATFORM_H_
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Internal common platform function.
*/

#include <gs/embed/drivers/uart/uart.h>

/**
   Platform specific data.
*/
typedef struct gs_uart_platform gs_uart_platform_t;

/**
   Size/length of rx/tx queue
*/
#define GS_UART_QUEUE_SIZE   512

/**
   Common platform instance data.
*/
typedef struct {
    //! Device handle.
    uint8_t device;
    //! Reference to platform instance data.
    gs_uart_platform_t * platform;
    //! Queue for transmit
    gs_queue_t tx_queue;
    //! Queue for receive
    gs_queue_t rx_queue;
    struct {
        //! Status, see gs_uart_status()
        bool rx_lost;
    } status;
    //! Rx callback
    gs_uart_rx_callback_isr_t rx_callback_isr;
    //! Rx callback user data.
    void * rx_callback_isr_user_data;
} gs_uart_common_t;

/**
   Initialize common layer.

   @param[in] handle
*/
gs_error_t gs_uart_init_common(gs_uart_common_t * handle, uint8_t device, const gs_uart_config_t * config);

/**
   Return common instance data.
   All intance data are placed in the platform specific uart implementetion.
   This generic function gets the queue handle

   @param_device
*/
gs_uart_common_t * gs_uart_get_common(uint8_t device);

/**
   Start transmit transaction if transmitting not ongoing
   This function is called, to set of a tx interrupt

   @param[in] handle common uart instance
   @param[in] data received data
   @param[in] data_size size of received data
   @param[out] cswitch context switch control.
*/
void gs_uart_tx_start_if_idle(gs_uart_common_t * handle);

/**
   Call from platform layer, when data received.
*/
static inline void gs_uart_rx_isr(gs_uart_common_t * handle, const uint8_t * data, size_t data_size, gs_context_switch_t * cswitch)
{
    if (handle->rx_callback_isr) {
        (handle->rx_callback_isr)(handle->rx_callback_isr_user_data, data, data_size, cswitch);
    } else if (handle->rx_queue) {
        for (size_t i = 0; i < data_size; ++i, ++data) {
            if (gs_queue_enqueue_isr(handle->rx_queue, data, cswitch) != GS_OK) {
                handle->status.rx_lost = true;
                return;
            }
        }
    }
}

void gs_uart_write_direct(gs_uart_platform_t * platform, uint8_t data);

/**
   Disable uart, e.g. disable interrupt.
   @param[in] platform platform layer
*/
void gs_uart_disable(gs_uart_platform_t * platform);

/**
   Enable uart, e.g. enable interrupt.

   Change/set communiction 
   Only enable Rx interrupt if rx_queue or callback set.
   @param[in] platform platform layer
   @param[in] comm communication settings. If none NULL, change/set uart.
*/
gs_error_t gs_uart_enable(gs_uart_platform_t * platform, const gs_uart_comm_t * comm);


#endif
