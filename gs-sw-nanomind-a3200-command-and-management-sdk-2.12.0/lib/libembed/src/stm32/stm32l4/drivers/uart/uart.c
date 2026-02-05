/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/uart/uart.h> // first, ensure self contained

#include <drivers/uart/uart_platform.h>
#include <stm32l4xx_hal.h>
#include <gs/embed/freertos.h>
#include <gs/embed/stm32/stm32l4/error.h>
#include <gs/util/check.h>
#include <gs/util/thread.h>
#include "../common.h"

#ifdef UART4
#define GS_EMBED_UART_NUMBER_OF_HANDLES 4
#else
#define GS_EMBED_UART_NUMBER_OF_HANDLES 3
#endif

struct gs_uart_platform {
    //! STM32 uart_handle
    UART_HandleTypeDef stm32_uart_handle;

    //! Platform common
    gs_uart_common_t common;

    //! Receive buffer
    uint8_t rx_buffer[1];

    //! True if uart device has been initialized
    bool in_use;

    //! True if uart device is enabled
    volatile bool enabled;
};

static gs_uart_platform_t uart_devices[GS_EMBED_UART_NUMBER_OF_HANDLES + 1];

static gs_error_t init_communication(UART_HandleTypeDef * stm32_uart_handle, const  gs_uart_comm_t * comm)
{
    if (comm) {
        stm32_uart_handle->Init.BaudRate = comm->bps;
        switch (comm->data_bits) {
            case 8:
                stm32_uart_handle->Init.WordLength = UART_WORDLENGTH_8B;
                break;
            default:
                return GS_ERROR_ARG;
        }

        switch (comm->stop_bits) {
            case GS_UART_1_STOP_BIT:
                stm32_uart_handle->Init.StopBits = UART_STOPBITS_1;
                break;
            case GS_UART_2_STOP_BITS:
                stm32_uart_handle->Init.StopBits = UART_STOPBITS_2;
                break;
            default:
                return GS_ERROR_ARG;
        }

        switch (comm->parity_bit) {
            case GS_UART_NO_PARITY:
                stm32_uart_handle->Init.Parity = UART_PARITY_NONE;
                break;
            default:
                return GS_ERROR_ARG;
        }

        switch (comm->flow_control) {
            case GS_UART_NO_FLOW_CTRL:
                stm32_uart_handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
                break;
            default:
                return GS_ERROR_ARG;
        }
    }

    HAL_StatusTypeDef HAL_res = HAL_UART_Init(stm32_uart_handle);
    if (HAL_res != HAL_OK) {
        return gs_hal_status_to_error(HAL_res);
    }

    return GS_OK;
}

gs_error_t gs_uart_init(uint8_t device, const gs_uart_config_t * config)
{
    GS_CHECK_HANDLE((device > 0) && (device <= GS_EMBED_UART_NUMBER_OF_HANDLES));

    IRQn_Type USARTx_IRQn;
    gs_uart_platform_t * platform = &uart_devices[device];
    memset(platform, 0, sizeof(*platform));
    switch (device) {
        case 1:
            /* Enable USARTx clock */
            __HAL_RCC_USART1_CLK_ENABLE();
            USARTx_IRQn = USART1_IRQn;
            platform->stm32_uart_handle.Instance = USART1;
            break;
        case 2:
            /* Enable USARTx clock */
            __HAL_RCC_USART2_CLK_ENABLE();
            USARTx_IRQn = USART2_IRQn;
            platform->stm32_uart_handle.Instance = USART2;
            break;
        case 3:
            /* Enable USARTx clock */
            __HAL_RCC_USART3_CLK_ENABLE();
            USARTx_IRQn = USART3_IRQn;
            platform->stm32_uart_handle.Instance = USART3;
            break;
#if(GS_EMBED_UART_NUMBER_OF_HANDLES == 4)
        case 4:
            /* Enable USARTx clock */
            __HAL_RCC_UART4_CLK_ENABLE();
            USARTx_IRQn = UART4_IRQn;
            platform->stm32_uart_handle.Instance = UART4;
            break;
#endif
        default:
            return GS_ERROR_ARG;
    }

    platform->stm32_uart_handle.Init.Mode = UART_MODE_TX_RX;

    gs_error_t retval = gs_uart_init_common(&platform->common, device, config);
    if (retval != GS_OK) {
        return retval;
    }

    retval = init_communication(&platform->stm32_uart_handle, &config->comm);
    if (retval != GS_OK) {
        return retval;
    }

    platform->common.platform = platform; // configured ok

    /* NVIC for UART */
    HAL_NVIC_SetPriority(USARTx_IRQn, GS_STM32L4_DEFAULT_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(USARTx_IRQn);

    platform->in_use = true;
    platform->enabled = true;

    HAL_StatusTypeDef HAL_res = HAL_UART_Receive_IT(&platform->stm32_uart_handle, platform->rx_buffer, sizeof(platform->rx_buffer));
    if (HAL_res != HAL_OK) {
        return gs_hal_status_to_error(HAL_res);
    }

    return GS_OK;
}

static inline gs_uart_platform_t * find_platform(UART_HandleTypeDef *UartHandle)
{
    return (gs_uart_platform_t*)UartHandle;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    gs_uart_platform_t * platform = find_platform(UartHandle);
    if (platform) {
        gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;
        gs_uart_rx_isr(&platform->common, platform->rx_buffer, sizeof(platform->rx_buffer), &cswitch);
        HAL_UART_Receive_IT(&platform->stm32_uart_handle, platform->rx_buffer, sizeof(platform->rx_buffer));
        portEND_SWITCHING_ISR(cswitch.task_woken);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
}

gs_uart_common_t * gs_uart_get_common(uint8_t device)
{
    if ((device <= GS_EMBED_UART_NUMBER_OF_HANDLES) & (device != 0)) {
        if (uart_devices[device].common.platform) {
            return &uart_devices[device].common;
        }
    }
    return NULL;
}

void gs_uart_tx_start_if_idle(gs_uart_common_t * handle)
{
    if (handle->platform->enabled) {
        /* Enable the UART Transmit buffer empty Interrupt */
        __HAL_UART_ENABLE_IT(&handle->platform->stm32_uart_handle, UART_IT_TXE);
    }
}


void gs_uart_write_direct(gs_uart_platform_t * platform, uint8_t data)
{
    platform->stm32_uart_handle.gState = HAL_UART_STATE_READY;
    HAL_UART_Transmit(&platform->stm32_uart_handle, &data, 1, HAL_MAX_DELAY);
}

gs_error_t gs_uart_enable(gs_uart_platform_t * platform, const gs_uart_comm_t * comm)
{
    if (platform) {
        if (platform->in_use && !platform->enabled) {
            gs_error_t retval = init_communication(&platform->stm32_uart_handle, comm);
            if (retval != GS_OK) {
                return retval;
            }
            platform->enabled = true;
            return gs_hal_status_to_error(HAL_UART_Receive_IT(&platform->stm32_uart_handle, platform->rx_buffer, sizeof(platform->rx_buffer)));
        }
    }
    return GS_ERROR_HANDLE;
}

void gs_uart_disable(gs_uart_platform_t * platform)
{
    if (platform) {
        if (platform->in_use && platform->enabled) {
            HAL_UART_Abort_IT(&platform->stm32_uart_handle);
            for(int i = 0; i < 100; i++) {
                if (!platform->enabled) {
                    break;
                }
                gs_thread_sleep_ms(1);
            }
            HAL_UART_DeInit(&platform->stm32_uart_handle);
        }
    }
}

void HAL_UART_AbortCpltCallback (UART_HandleTypeDef *huart)
{
    gs_uart_platform_t * platform = find_platform(huart);
    platform->enabled = false;
}

void UART_IRQHandler(UART_HandleTypeDef *UartHandle)
{
    if((__HAL_UART_GET_IT(UartHandle, UART_IT_TXE) != RESET) &&
       (__HAL_UART_GET_IT_SOURCE(UartHandle, UART_IT_TXE) != RESET))
    {
        gs_uart_platform_t * platform = find_platform(UartHandle);
        if (platform) {
            uint8_t ch;
            gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;
            if (gs_queue_dequeue_isr(platform->common.tx_queue, &cswitch, &ch) == GS_OK) {
                UartHandle->Instance->TDR = ch & 0xFF;
            } else {
                __HAL_UART_DISABLE_IT(UartHandle, UART_IT_TXE);
            }
            portEND_SWITCHING_ISR(cswitch.task_woken);
        }
    }
}

/* Interrupt handlers for all 4 uarts */
void USART1_IRQHandler(void)
{
    UART_IRQHandler(&uart_devices[1].stm32_uart_handle);
    HAL_UART_IRQHandler(&uart_devices[1].stm32_uart_handle);
}

void USART2_IRQHandler(void)
{
    UART_IRQHandler(&uart_devices[2].stm32_uart_handle);
    HAL_UART_IRQHandler(&uart_devices[2].stm32_uart_handle);
}

void USART3_IRQHandler(void)
{
    UART_IRQHandler(&uart_devices[3].stm32_uart_handle);
    HAL_UART_IRQHandler(&uart_devices[3].stm32_uart_handle);
}

void UART4_IRQHandler(void)
{
    UART_IRQHandler(&uart_devices[4].stm32_uart_handle);
    HAL_UART_IRQHandler(&uart_devices[4].stm32_uart_handle);
}
