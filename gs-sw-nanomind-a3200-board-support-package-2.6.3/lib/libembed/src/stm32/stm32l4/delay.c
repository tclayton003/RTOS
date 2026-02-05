/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   Delay routines based on hardware timer.
*/

#include <gs/embed/delay.h>
#include <stm32l4xx_hal.h>
#include <stdlib.h>

#define DELAY_TIM_FREQUENCY_US 1000000
static TIM_HandleTypeDef    handle;

gs_error_t gs_delay_init(void)
{
    __HAL_RCC_TIM6_CLK_ENABLE();

    handle.Instance               = TIM6;
    handle.Init.Period            = UINT16_MAX;
    handle.Init.Prescaler         = (SystemCoreClock/DELAY_TIM_FREQUENCY_US)-1;
    handle.Init.ClockDivision     = 0;
    handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    handle.Init.RepetitionCounter = 0;
    HAL_TIM_Base_Init(&handle);
    HAL_TIM_Base_Start(&handle);
    return GS_OK;
}

uint16_t gs_delay_ts_get(void)
{
    return __HAL_TIM_GET_COUNTER(&handle);
}

void gs_delay_from_ts(uint16_t ts, uint16_t delay)
{
    uint16_t end   = ts + delay;
    while ((int16_t)(end - (uint16_t)__HAL_TIM_GET_COUNTER(&handle)) > 0);
}

void gs_delay_us(uint32_t time_us)
{
    gs_delay_from_ts(__HAL_TIM_GET_COUNTER(&handle), time_us);
}
