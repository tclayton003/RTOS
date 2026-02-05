/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <drivers/uart/uart_platform.h>
#include <gs/embed/stdio.h>
#include <gs/util/check.h>

// ASF
#include <usart.h>
#include <usart_interrupt.h>

#define GS_EMBED_UART_NUMBER_OF_HANDLES 1

struct gs_uart_platform {
    //! Platform common
    gs_uart_common_t common;

    struct usart_module usart;
    //! Receive buffer
    uint8_t rx_buffer[1];
};

static gs_uart_platform_t uart_devices[GS_EMBED_UART_NUMBER_OF_HANDLES];

static inline gs_uart_platform_t * find_platform(struct usart_module *const module)
{
    for (unsigned int i = 0; i < GS_EMBED_UART_NUMBER_OF_HANDLES; i++) {
        if (&uart_devices[i].usart == module) {
            return &uart_devices[i];
        }
    }
    return NULL;
}

static void gs_usart_data_received_cb(struct usart_module *const module)
{
    gs_uart_platform_t * platform = find_platform(module);

    if (platform) {
        gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;
        gs_uart_rx_isr(&platform->common, platform->rx_buffer, sizeof(platform->rx_buffer), &cswitch);
        usart_read_buffer_job(&platform->usart, platform->rx_buffer, sizeof(platform->rx_buffer));
        portEND_SWITCHING_ISR(cswitch.task_woken);
    }
}

gs_error_t gs_uart_init(uint8_t device, const gs_uart_config_t * config)
{
    GS_CHECK_HANDLE(device < GS_EMBED_UART_NUMBER_OF_HANDLES);

    struct usart_config config_usart;
    usart_get_config_defaults(&config_usart);

    config_usart.baudrate    	= config->comm.bps;

    config_usart.mux_setting 	= USART_RX_1_TX_0_XCK_1;
    config_usart.pinmux_pad0 	= PINMUX_PA00D_SERCOM1_PAD0;  // TX
    config_usart.pinmux_pad1 	= PINMUX_PA01D_SERCOM1_PAD1;  // RX
    config_usart.pinmux_pad2 	= PINMUX_UNUSED;
    config_usart.pinmux_pad3 	= PINMUX_UNUSED;

    config_usart.generator_source = GCLK_GENERATOR_0;

    gs_uart_platform_t *platform = &uart_devices[device];
    usart_init(&platform->usart, SERCOM1, &config_usart);

    gs_error_t retval = gs_uart_init_common(&platform->common, device, config);
    if (retval != GS_OK) {
        return retval;
    }

    usart_enable(&platform->usart);

    usart_register_callback(&platform->usart, gs_usart_data_received_cb, USART_CALLBACK_BUFFER_RECEIVED);
    usart_enable_callback(&platform->usart, USART_CALLBACK_BUFFER_RECEIVED);

    platform->common.platform = platform; // configured ok

    usart_read_buffer_job(&platform->usart, platform->rx_buffer, sizeof(platform->rx_buffer));
    return GS_OK;
}

gs_uart_common_t * gs_uart_get_common(uint8_t device)
{
    if (device < GS_EMBED_UART_NUMBER_OF_HANDLES) {
        if (uart_devices[device].common.platform) {
            return &uart_devices[device].common;
        }
    }
    return NULL;
}

void gs_uart_write_direct(gs_uart_platform_t * platform, uint8_t data)
{
    usart_write_wait(&platform->usart, data);
}


void gs_uart_tx_start_if_idle(gs_uart_common_t * handle)
{
    // implement when changing to queued Tx
}
