/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <drivers/uart/uart_platform.h>
#include <gs/embed/freertos.h>
#include <gs/util/minmax.h>
#include <gs/util/string.h>
#include <gs/util/check.h>
#include <gs/asf/avr32/asf.h>

#include <avr32/io.h>
#include <usart.h>
#include <gpio.h>
#include <pdca.h>
#include <intc.h>

#define USART_RX_PDC_BUF_SIZE  GS_UART_QUEUE_SIZE
#define USART_NO_PDC           0

// UART device instance
struct gs_uart_platform {

    // Common layer
    gs_uart_common_t common;

    // ASF/device driver
    volatile avr32_usart_t * addr;
    int pdc_channel_rx;
    int pdc_pid_rx;
    int rx_size;

    // DMA receive buffers
    uint8_t usart_rx_buf[2][USART_RX_PDC_BUF_SIZE];
    volatile uint8_t current_buf;
};

static gs_uart_platform_t * usart[AVR32_USART_NUM];

__attribute__((__noinline__)) portBASE_TYPE usart_DSR(gs_uart_platform_t * usartp)
{
    gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;
    if (usartp->addr->CSR.usart_mode.rxrdy) {
        uint8_t data = usartp->addr->rhr;
        gs_uart_rx_isr(&usartp->common, &data, 1, &cswitch);
    }
    return cswitch.task_woken;
}

__attribute__((__naked__)) void usart0_ISR()
{
    portENTER_SWITCHING_ISR();
    usart_DSR(usart[0]);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__)) void usart1_ISR()
{
    portENTER_SWITCHING_ISR();
    usart_DSR(usart[1]);
    portEXIT_SWITCHING_ISR();
}

#ifdef AVR32_USART2_ADDRESS
__attribute__((__naked__)) void usart2_ISR()
{
    portENTER_SWITCHING_ISR();
    usart_DSR(usart[2]);
    portEXIT_SWITCHING_ISR();
}
#endif

#ifdef AVR32_USART3_ADDRESS
__attribute__((__naked__)) void usart3_ISR()
{
    portENTER_SWITCHING_ISR();
    usart_DSR(usart[3]);
    portEXIT_SWITCHING_ISR();
}
#endif

#ifdef AVR32_USART4_ADDRESS
__attribute__((__naked__)) void usart4_ISR()
{
    portENTER_SWITCHING_ISR();
    usart_DSR(usart[4]);
    portEXIT_SWITCHING_ISR();
}
#endif

__attribute__((__noinline__)) portBASE_TYPE usart_pdc_DSR(gs_uart_platform_t * usartp)
{
    gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;

    /* If no previous RX size was set, we received one byte */
    if (usartp->rx_size == 0) {
        usartp->rx_size = 1;
    }

    const uint8_t * data = usartp->usart_rx_buf[usartp->current_buf & 1];
    gs_uart_rx_isr(&usartp->common, data, usartp->rx_size, &cswitch);

    /* We now set the reload size to full */
    AVR32_PDCA.channel[usartp->pdc_channel_rx].marr = (unsigned long) usartp->usart_rx_buf[usartp->current_buf & 1];
    AVR32_PDCA.channel[usartp->pdc_channel_rx].tcrr = USART_RX_PDC_BUF_SIZE;
    usartp->current_buf++;

    /* During the processing the main-buffer might be used, so readout the size and save it */
    pdca_disable(usartp->pdc_channel_rx);
    usartp->rx_size = USART_RX_PDC_BUF_SIZE - AVR32_PDCA.channel[usartp->pdc_channel_rx].tcr;

    if (usartp->rx_size > 0) {
        /* If it was used, we would like to return to the interrupt */
        AVR32_PDCA.channel[usartp->pdc_channel_rx].tcr = 0;
    } else {
	/* If it wasn't used, we would like to interrupt after next byte */
        AVR32_PDCA.channel[usartp->pdc_channel_rx].tcr = 1;
    }
    pdca_enable(usartp->pdc_channel_rx);

    return cswitch.task_woken;
}

__attribute__((__naked__)) void usart_pdc0_ISR() {
    portENTER_SWITCHING_ISR();
    usart_pdc_DSR(usart[0]);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__)) void usart_pdc1_ISR() {
    portENTER_SWITCHING_ISR();
    usart_pdc_DSR(usart[1]);
    portEXIT_SWITCHING_ISR();
}

#ifdef AVR32_USART2_ADDRESS
__attribute__((__naked__)) void usart_pdc2_ISR() {
    portENTER_SWITCHING_ISR();
    usart_pdc_DSR(usart[2]);
    portEXIT_SWITCHING_ISR();
}
#endif

#ifdef AVR32_USART3_ADDRESS
__attribute__((__naked__)) void usart_pdc3_ISR() {
    portENTER_SWITCHING_ISR();
    usart_pdc_DSR(usart[3]);
    portEXIT_SWITCHING_ISR();
}
#endif

#ifdef AVR32_USART4_ADDRESS
__attribute__((__naked__)) void usart_pdc4_ISR() {
    portENTER_SWITCHING_ISR();
    usart_pdc_DSR(usart[4]);
    portEXIT_SWITCHING_ISR();
}
#endif

static gs_error_t gs_uart_set_comm(gs_uart_platform_t * platform, const gs_uart_comm_t * comm)
{
    // map settings gs -> avr32
    usart_options_t options;
    memset(&options, 0, sizeof(options));
    options.baudrate = comm->bps;
    switch (comm->data_bits) {
        case 7:
        case 8: options.charlength = comm->data_bits; break;
        default: return GS_ERROR_NOT_SUPPORTED;
    }
    switch (comm->parity_bit) {
        case GS_UART_NO_PARITY:   options.paritytype = USART_NO_PARITY; break;
        case GS_UART_EVEN_PARITY: options.paritytype = USART_EVEN_PARITY; break;
        case GS_UART_ODD_PARITY:  options.paritytype = USART_ODD_PARITY; break;
        default: return GS_ERROR_NOT_SUPPORTED;
    }
    switch (comm->stop_bits) {
        case GS_UART_1_STOP_BIT:    options.stopbits = USART_1_STOPBIT; break;
        case GS_UART_1_5_STOP_BITS: options.stopbits = USART_1_5_STOPBITS; break;
        case GS_UART_2_STOP_BITS:   options.stopbits = USART_2_STOPBITS; break;
        default: return GS_ERROR_NOT_SUPPORTED;
    }
    options.channelmode = USART_NORMAL_CHMODE;

    int res = usart_init_rs232(platform->addr, &options, sysclk_get_peripheral_bus_hz(platform->addr));
    if (res != USART_SUCCESS) {
        return GS_ERROR_ARG;
    }

    return GS_OK;
}

gs_error_t gs_uart_init(uint8_t device, const gs_uart_config_t * config)
{
    GS_CHECK_HANDLE(device < AVR32_USART_NUM);
    GS_CHECK_ARG(config != NULL);
    GS_CHECK_SUPPORTED(config->tx_queue_size == 0);

    gs_uart_platform_t * platform = usart[device];
    if (platform) {
        log_error("%s: device %u already initialized", __FUNCTION__, device);
        return GS_ERROR_IN_USE;
    }

    platform = calloc(1, sizeof(*platform));
    if (platform == NULL) {
        return GS_ERROR_ALLOC;
    }

    gs_error_t error = gs_uart_init_common(&platform->common, device, config);
    if (error) {
        // ignore clean-up
        return error;
    }

    usart[device] = platform->common.platform = platform;

    return gs_uart_enable(platform, &config->comm);
}

void gs_uart_disable(gs_uart_platform_t * platform)
{
    /* Disable PDC/ISR */
    pdca_disable(platform->pdc_channel_rx);
    pdca_disable_interrupt_reload_counter_zero(platform->pdc_channel_rx);
}

gs_error_t gs_uart_enable(gs_uart_platform_t * platform, const gs_uart_comm_t * comm)
{
    volatile avr32_usart_t * addr;
    int pdc_channel_rx;
    int pdc_pid_rx;
    __int_handler pdc_isr;
    __int_handler isr;
    int irq;

    switch(platform->common.device) {
        case 0:
            addr = &AVR32_USART0;
            pdc_channel_rx = 5;
            pdc_pid_rx = AVR32_PDCA_PID_USART0_RX;
            pdc_isr = &usart_pdc0_ISR;
            isr = &usart0_ISR;
            irq = AVR32_USART0_IRQ;
            break;
        case 1:
            addr = &AVR32_USART1;
            pdc_channel_rx = 6;
            pdc_pid_rx = AVR32_PDCA_PID_USART1_RX;
            pdc_isr = &usart_pdc1_ISR;
            isr = &usart1_ISR;
            irq = AVR32_USART1_IRQ;
            break;
#ifdef AVR32_USART2_ADDRESS
        case 2:
            addr = &AVR32_USART2;
            pdc_channel_rx = 2;
            pdc_pid_rx = AVR32_PDCA_PID_USART2_RX;
            pdc_isr = &usart_pdc2_ISR;
            isr = &usart2_ISR;
            irq = AVR32_USART2_IRQ;
            break;
#endif
#ifdef AVR32_USART3_ADDRESS
        case 3:
            addr = &AVR32_USART3;
            pdc_channel_rx = 8;
            pdc_pid_rx = AVR32_PDCA_PID_USART3_RX;
            pdc_isr = &usart_pdc3_ISR;
            isr = &usart3_ISR;
            irq = AVR32_USART3_IRQ;
            break;
#endif
#ifdef AVR32_USART4_ADDRESS
        case 4:
            addr = &AVR32_USART4;
            pdc_channel_rx = 3;
            pdc_pid_rx = AVR32_PDCA_PID_USART4_RX;
            pdc_isr = &usart_pdc4_ISR;
            isr = &usart4_ISR;
            irq = AVR32_USART4_IRQ;
            break;
#endif
        default:
            return GS_ERROR_HANDLE;
    }

    platform->addr = addr;
    platform->pdc_channel_rx = pdc_channel_rx;
    platform->pdc_pid_rx = pdc_pid_rx;

    if (comm) {
        gs_error_t error = gs_uart_set_comm(platform, comm);
        if (error) {
            return error;
        }
    }

    if (platform->common.rx_queue || platform->common.rx_callback_isr) {
        if (USART_NO_PDC) {

            /* Enable Interrupt */
            INTC_register_interrupt(isr, irq, AVR32_INTC_INT0);
            platform->addr->IER.usart_mode.rxrdy = 1;

        } else {

            /* Enable PDC */
            pdca_channel_options_t pdc_rx;
            memset(&pdc_rx, 0, sizeof(pdc_rx));
            pdc_rx.addr = platform->usart_rx_buf[platform->current_buf];
            pdc_rx.size = 1;
            pdc_rx.r_addr = platform->usart_rx_buf[(platform->current_buf+1) & 0x01];
            pdc_rx.r_size = USART_RX_PDC_BUF_SIZE;
            pdc_rx.pid = platform->pdc_pid_rx;
            pdc_rx.transfer_size = PDCA_TRANSFER_SIZE_BYTE;
            pdca_init_channel(platform->pdc_channel_rx, &pdc_rx);

            /* Enable ISR */
            INTC_register_interrupt(pdc_isr, AVR32_PDCA_IRQ_0 + platform->pdc_channel_rx, AVR32_INTC_INT0);
            pdca_enable_interrupt_reload_counter_zero(platform->pdc_channel_rx);
            pdca_enable(platform->pdc_channel_rx);
        }
    }

    return GS_OK;
}

void gs_uart_write_direct(gs_uart_platform_t * platform, uint8_t data)
{
    usart_putchar(platform->addr, data);
}

gs_uart_common_t * gs_uart_get_common(uint8_t device)
{
    if (device < AVR32_USART_NUM) {
        if (usart[device]) {
            return &usart[device]->common;
        }
    }
    return NULL;
}

void gs_uart_tx_start_if_idle(gs_uart_common_t * handle)
{
    // implement when changing to queued Tx
}
