/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <drivers/uart/uart_platform.h> // ensure self-contained

#include <gs/embed/freertos.h>
#include <gs/embed/drivers/uart/uart.h>
#include <gs/util/error.h>
#include <gs/util/check.h>

void gs_uart_get_default_comm(gs_uart_comm_t * buffer)
{
    if (buffer) {
        memset(buffer, 0, sizeof(*buffer));

        buffer->bps = GS_UART_DEFAULT_BPS;
        buffer->data_bits = GS_UART_DEFAULT_DATA_BITS;
        buffer->parity_bit = GS_UART_NO_PARITY;
        buffer->stop_bits = GS_UART_1_STOP_BIT;
        buffer->flow_control = GS_UART_NO_FLOW_CTRL;
    }
}

void gs_uart_get_default_config(gs_uart_config_t * buffer)
{
    if (buffer) {
        memset(buffer, 0, sizeof(*buffer));
        gs_uart_get_default_comm(&buffer->comm);
        buffer->tx_queue_size = buffer->rx_queue_size = GS_UART_QUEUE_SIZE;
    }
}

gs_error_t gs_uart_init_common(gs_uart_common_t * handle, uint8_t device, const gs_uart_config_t * config)
{
    GS_CHECK_ARG(config != NULL);

    handle->device = device;
    handle->rx_callback_isr = config->rx_callback_isr;
    handle->rx_callback_isr_user_data = config->rx_callback_isr_user_data;

    if (config->rx_queue_size) {
        gs_error_t retval = gs_queue_create(config->rx_queue_size, sizeof(uint8_t), &handle->rx_queue);
        if (retval != GS_OK) {
            return retval;
        }
    }

    if (config->tx_queue_size) {
        gs_error_t retval = gs_queue_create(config->tx_queue_size, sizeof(uint8_t), &handle->tx_queue);
        if (retval != GS_OK) {
            return retval;
        }
    }
    return GS_OK;
}

uint32_t gs_uart_status(uint8_t device)
{
    gs_uart_common_t * handle = gs_uart_get_common(device);
    if (handle) {
        uint32_t status = 0;
        if (handle->status.rx_lost) {
            handle->status.rx_lost = false; // we need a atomic swap function
            status |= GS_UART_STATUS_RX_LOST;
        }
        return status;
    }
    return 0;
}

gs_error_t gs_uart_set_rx_callback(uint8_t device, gs_uart_rx_callback_isr_t user_func, void * user_data)
{
    gs_uart_common_t * handle = gs_uart_get_common(device);
    if (handle) {
        gs_uart_disable(handle->platform);
        handle->rx_callback_isr = user_func;
        handle->rx_callback_isr_user_data = user_data;
        return gs_uart_enable(handle->platform, NULL);
    }
    return GS_ERROR_HANDLE;
}

gs_error_t gs_uart_change_comm(uint8_t device, const gs_uart_comm_t * comm)
{
    gs_uart_common_t * handle = gs_uart_get_common(device);
    if (handle) {
        gs_uart_disable(handle->platform);
        return gs_uart_enable(handle->platform, comm);
    }
    return GS_ERROR_HANDLE;
}

gs_error_t gs_uart_read(uint8_t device, int timeout_ms, uint8_t * buffer)
{
    gs_uart_common_t * handle = gs_uart_get_common(device);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    return gs_queue_dequeue(handle->rx_queue, timeout_ms, buffer);
}

gs_error_t gs_uart_write(uint8_t device, int timeout_ms, uint8_t data)
{
    gs_uart_common_t * handle = gs_uart_get_common(device);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    if (handle->tx_queue) {
        gs_error_t retval = gs_queue_enqueue(handle->tx_queue, &data, timeout_ms);
        if (retval != GS_OK) {
            return retval;
        }
        gs_uart_tx_start_if_idle(handle);
    } else {
        return gs_uart_write_buffer_direct(device, &data, sizeof(data), NULL);
    }

    return GS_OK;
}

gs_error_t gs_uart_write_buffer(uint8_t device, int timeout_ms, const uint8_t * data, size_t data_size, size_t * return_wrote)
{
    gs_uart_common_t * handle = gs_uart_get_common(device);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    if (handle->tx_queue) {
        gs_error_t error = GS_OK;
        size_t wrote = 0;
        for (size_t i = 0; (i < data_size) && (error == GS_OK); ++i, ++data) {
            error = gs_queue_enqueue(handle->tx_queue, data, timeout_ms);
            if (error == GS_OK) {
                ++wrote;
                gs_uart_tx_start_if_idle(handle);
            }
        }

        if (return_wrote) {
            *return_wrote = wrote;
        }

        return error;

    } else {
        return gs_uart_write_buffer_direct(device, data, data_size, return_wrote);
    }
}

gs_error_t gs_uart_write_buffer_direct(uint8_t device, const uint8_t * data, size_t data_size, size_t * return_wrote)
{
    gs_uart_common_t * handle = gs_uart_get_common(device);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    for (unsigned int i = 0; i < data_size; ++i, ++data) {
        gs_uart_write_direct(handle->platform, *data);
    }

    if (return_wrote) {
        *return_wrote = data_size;
    }

    return GS_OK;
}
