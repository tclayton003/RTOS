/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <math.h>
#include <inttypes.h>
#include <gs/embed/stm32/stm32l4/drivers/watchdog/watchdog.h>
#include <gs/embed/stm32/stm32l4/error.h>
#include <stm32l4xx_hal.h>
#include <gs/embed/freertos.h>
#include "../common.h"

/***************** Implementation of watchdog (IWDG) *****************************/

static IWDG_HandleTypeDef stm32_wdg_handle;

gs_error_t gs_stm32l4_watchdog_iwdg_init(uint32_t timeout_val_ms)
{
    const float clock_freq_khz = 32;
    float reload_val = 1;
    uint32_t prescaler = 4;
    for (int i = 0; i < 7; i++) {
        reload_val = (clock_freq_khz / prescaler) * timeout_val_ms;
        if (reload_val > 0xFFF) {
            prescaler *= 2;
        } else {
            break;
        }
    }
    switch (prescaler) {
        case 4:
            stm32_wdg_handle.Init.Prescaler = IWDG_PRESCALER_4;
            break;
        case 8:
            stm32_wdg_handle.Init.Prescaler = IWDG_PRESCALER_8;
            break;
        case 16:
            stm32_wdg_handle.Init.Prescaler = IWDG_PRESCALER_16;
            break;
        case 32:
            stm32_wdg_handle.Init.Prescaler = IWDG_PRESCALER_32;
            break;
        case 64:
            stm32_wdg_handle.Init.Prescaler = IWDG_PRESCALER_64;
            break;
        case 128:
            stm32_wdg_handle.Init.Prescaler = IWDG_PRESCALER_128;
            break;
        case 256:
            stm32_wdg_handle.Init.Prescaler = IWDG_PRESCALER_256;
            break;
        default:
            return GS_ERROR_RANGE;
    }
    uint32_t reload_reg_val = (uint32_t)reload_val;
    if (reload_reg_val == 0) {
        return GS_ERROR_RANGE;
    }
    stm32_wdg_handle.Instance = IWDG;
    stm32_wdg_handle.Init.Reload = reload_reg_val;
    stm32_wdg_handle.Init.Window = 0xFFF;
    return gs_hal_status_to_error(HAL_IWDG_Init(&stm32_wdg_handle));
}

gs_error_t gs_stm32l4_watchdog_iwdg_kick(void)
{
    return gs_hal_status_to_error(HAL_IWDG_Refresh(&stm32_wdg_handle));
}

/***************** Hooks for implementation of watchdog in libutil *****************/

static gs_error_t gs_watchdog_start(gs_watchdog_device_t * watchdog)
{
    return gs_stm32l4_watchdog_iwdg_init(watchdog->timeout * 1000);
}

static gs_error_t gs_watchdog_ping(gs_watchdog_device_t * watchdog)
{
    return gs_stm32l4_watchdog_iwdg_kick();
}

static gs_error_t gs_watchdog_stop(gs_watchdog_device_t * watchdog)
{
    return GS_ERROR_NOT_SUPPORTED;
}

static const gs_watchdog_dev_ops_t dev_ops = {
    .start = gs_watchdog_start,
    .ping = gs_watchdog_ping,
    .stop = gs_watchdog_stop
};

const gs_watchdog_dev_ops_t * gs_stm32l4_watchdog_get_dev_ops(void)
{
    return &dev_ops;
}

/***************** Implementation of watchdog (WWDG) *****************************/

static WWDG_HandleTypeDef watchdog_handle;
static gs_watchdog_wwdg_callback_t watchdog_callback = NULL;

gs_error_t gs_stm32l4_watchdog_wwdg_init(float timeout_ms, gs_watchdog_wwdg_callback_t callback, float * timeout_ms_achieved)
{
    float clock_freq_mhz = (float)HAL_RCC_GetPCLK1Freq() / (4096 * 1000);
    float counter_val = 0xFF;
    uint32_t prescaler = 1;
    for (int i = 0; i < 4; i++) {
        counter_val = floor((clock_freq_mhz/prescaler) * timeout_ms);
        if (counter_val > 0x3F) {
            prescaler *= 2;
        } else {
            break;
        }
    }
    switch (prescaler) {
        case 1:
            watchdog_handle.Init.Prescaler = WWDG_PRESCALER_1;
            break;
        case 2:
            watchdog_handle.Init.Prescaler = WWDG_PRESCALER_2;
            break;
        case 4:
            watchdog_handle.Init.Prescaler = WWDG_PRESCALER_4;
            break;
        case 8:
            watchdog_handle.Init.Prescaler = WWDG_PRESCALER_8;
            break;
        default:
            return GS_ERROR_RANGE;
    }
    if (counter_val < 1) {
        return GS_ERROR_RANGE;
    }
    if (timeout_ms_achieved) {
        *timeout_ms_achieved = counter_val * prescaler / clock_freq_mhz ;
    }

    watchdog_handle.Instance = WWDG;
    watchdog_handle.Init.Counter =  ((uint32_t)counter_val) | 0x40;
    watchdog_handle.Init.Window = 0x7F;
    if (callback) {
        watchdog_handle.Init.EWIMode = WWDG_CFR_EWI;
        watchdog_callback = callback;
        HAL_NVIC_SetPriority(WWDG_IRQn, GS_STM32L4_DEFAULT_ISR_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(WWDG_IRQn);
    } else {
        watchdog_handle.Init.EWIMode = WWDG_EWI_DISABLE;
    }

    __HAL_RCC_WWDG_CLK_ENABLE();
    HAL_StatusTypeDef HAL_res = HAL_WWDG_Init(&watchdog_handle);

    return gs_hal_status_to_error(HAL_res);
}

gs_error_t gs_stm32l4_watchdog_wwdg_kick(void)
{
    return gs_hal_status_to_error(HAL_WWDG_Refresh(&watchdog_handle));
}

void WWDG_IRQHandler(void)
{
    HAL_WWDG_IRQHandler(&watchdog_handle);
}

void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef* hwwdg)
{
    HAL_WWDG_Refresh(hwwdg);
    if (watchdog_callback) {
        watchdog_callback();
    }
}
