/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/embed/stdio.h>
#include <gs/embed/drivers/uart/uart.h>

static int stdio_uart_console = GS_EMBED_UART_CONSOLE;

gs_error_t gs_stdio_set_uart(int uart)
{
    if (uart < 0) {
        stdio_uart_console = -1;
    } else {
        stdio_uart_console = uart;
    }
    return GS_OK;
}

int gs_stdio_get_uart(void)
{
    return stdio_uart_console;
}

int gs_stdio_get_default_uart(void)
{
    return GS_EMBED_UART_CONSOLE;
}

gs_error_t gs_stdio_putchar(int ch)
{
    if (stdio_uart_console >= 0) {
        gs_uart_write(stdio_uart_console, -1, (uint8_t)ch);
        return GS_OK;
    }
    return GS_ERROR_HANDLE;
}

gs_error_t gs_stdio_getchar_timed(int timeout_ms, int * ch)
{
    if (stdio_uart_console >= 0) {
        uint8_t tmp_ch;
        gs_error_t error = gs_uart_read(stdio_uart_console, timeout_ms, &tmp_ch);
        if ((error == GS_OK) && ch) {
            *ch = tmp_ch;
        }
        return error;
    }
    return GS_ERROR_HANDLE;
}

gs_error_t gs_stdio_put_direct(const char * buf, size_t len)
{
    if (stdio_uart_console >= 0) {
        return gs_uart_write_buffer_direct(stdio_uart_console, (uint8_t *) buf, len, NULL);
    }
    return GS_ERROR_HANDLE;
}
