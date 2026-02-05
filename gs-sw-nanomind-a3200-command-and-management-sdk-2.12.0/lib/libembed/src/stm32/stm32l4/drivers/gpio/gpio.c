/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
    @file

    @brief GPIO Implementation for STM32L4 of the GPIO API in libutil

    GPIO driver provides a simple interface toward driving HW GPIO's.
*/

#include <gs/util/check.h>
#include <gs/util/drivers/gpio/gpio.h>
#include <gs/embed/stm32/stm32l4/drivers/gpio/gpio.h>
#include <stm32l4xx_hal.h>
#include "../common.h"

typedef struct gs_stm32l4_ext_isr_handle {
    //! Bool true if active
    bool inuse;
    //! Callback to application isr
    gs_gpio_isr_t isr_callback;
} gs_stm32l4_ext_isr_handle_t;

static gs_stm32l4_ext_isr_handle_t ext_isr_handle[15];

static GPIO_TypeDef* const g_port_map[6] = {GPIOA,GPIOB,GPIOC,GPIOD,GPIOE,GPIOH};
static const uint8_t GPIO_MAX_PIN_NO = 15;

gs_error_t gs_gpio_get(gs_gpio_t gpio, bool *value)
{
    if (gpio.port > (sizeof(g_port_map)-1) || gpio.pin > GPIO_MAX_PIN_NO)
        return GS_ERROR_ARG;

    *value = (bool)HAL_GPIO_ReadPin(g_port_map[gpio.port], (1 << gpio.pin));
    return GS_OK;
}

bool gs_gpio_get_nc(gs_gpio_t gpio)
{
    return ((g_port_map[gpio.port]->IDR & (1 << gpio.pin)) > 0);
}

gs_error_t gs_gpio_set(gs_gpio_t gpio, bool value)
{
    if (gpio.port > (sizeof(g_port_map)-1) || gpio.pin > GPIO_MAX_PIN_NO)
        return GS_ERROR_ARG;

    HAL_GPIO_WritePin(g_port_map[gpio.port], (1 << gpio.pin), (GPIO_PinState)value);
    return GS_OK;
}

void gs_gpio_set_nc(gs_gpio_t gpio, bool value)
{
    if(value)
    {
        g_port_map[gpio.port]->BSRR = (uint32_t)(1 << gpio.pin);
    }
    else
    {
        g_port_map[gpio.port]->BRR = (uint32_t)(1 << gpio.pin);
    }
}

gs_error_t gs_stm32l4_gpio_init(gs_gpio_t gpio, const gs_stm32l4_gpio_conf_t * conf)
{
    if (gpio.port > (sizeof(g_port_map)-1) || gpio.pin > GPIO_MAX_PIN_NO) {
        return GS_ERROR_ARG;
    }

    GPIO_InitTypeDef GPIO_InitStruct;
    static bool first = true;
    if (first) {
        /* First time init is called we set all GPIO's at analog in to save power,
           if pin does not get configured. We exclude the ones used for programming */
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;

        GPIO_InitStruct.Pin = GPIO_PIN_All & ~GPIO_PIN_13 & ~GPIO_PIN_14; // PA13 and PA14 excluded
        HAL_GPIO_Init(g_port_map[PORT_A], &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_All & ~GPIO_PIN_4; // PB4 excluded
        HAL_GPIO_Init(g_port_map[PORT_B], &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_All;
        HAL_GPIO_Init(g_port_map[PORT_C], &GPIO_InitStruct);
        HAL_GPIO_Init(g_port_map[PORT_D], &GPIO_InitStruct);
        HAL_GPIO_Init(g_port_map[PORT_E], &GPIO_InitStruct);
        HAL_GPIO_Init(g_port_map[PORT_H], &GPIO_InitStruct);
        first = false;
    }
    GPIO_InitStruct.Pin = (1 << gpio.pin);
    GPIO_InitStruct.Mode = conf->mode;
    GPIO_InitStruct.Pull = conf->pull;
    GPIO_InitStruct.Speed = conf->speed;
    GPIO_InitStruct.Alternate = conf->alternate;
    HAL_GPIO_Init(g_port_map[gpio.port], &GPIO_InitStruct);

    return GS_OK;
}

gs_error_t gs_gpio_init_as_interrupt(gs_gpio_t gpio, const gs_interrupt_conf_t * conf)
{
    GS_CHECK_ARG(gpio.pin <= 15);

    GPIO_InitTypeDef GPIO_InitStruct;

    if (conf->falling_edge && conf->rising_edge) {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    } else if (!conf->falling_edge && conf->rising_edge) {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    } else if (conf->falling_edge && !conf->rising_edge) {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    } else {
        return GS_ERROR_ARG;
    }

    if (!ext_isr_handle[gpio.pin].inuse){
        ext_isr_handle[gpio.pin].inuse = true;
    } else {
        return GS_ERROR_IN_USE;
    }

    ext_isr_handle[gpio.pin].isr_callback = conf->isr;

    GPIO_InitStruct.Pin = (1 << gpio.pin);
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(g_port_map[gpio.port], &GPIO_InitStruct);

    IRQn_Type isr;
    switch (gpio.pin) {
        case 0:
            isr = EXTI0_IRQn;
            break;
        case 1:
            isr = EXTI1_IRQn;
            break;
        case 2:
            isr = EXTI2_IRQn;
            break;
        case 3:
            isr = EXTI3_IRQn;
            break;
        case 4:
            isr = EXTI4_IRQn;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            isr = EXTI9_5_IRQn;
            break;
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            isr = EXTI15_10_IRQn;
            break;
    }

    uint32_t priority = GS_STM32L4_DEFAULT_ISR_PRIORITY;
    if(conf->high_priority) {
        priority--; // Lower number equals higher priority in NVIC
    }
    HAL_NVIC_SetPriority(isr, priority, 0);
    HAL_NVIC_EnableIRQ(isr);
    return GS_OK;
}

static inline void gs_gpio_exti_callback(uint8_t pin)
{
    gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;
    ext_isr_handle[pin].isr_callback(&cswitch);
    portEND_SWITCHING_ISR(cswitch.task_woken);
}

static void gs_gpio_exti_handler(uint8_t pin)
{
    uint32_t stm_pin_no = 1 << pin;

    if(__HAL_GPIO_EXTI_GET_IT(stm_pin_no) != RESET)
    {
      __HAL_GPIO_EXTI_CLEAR_IT(stm_pin_no);
      gs_gpio_exti_callback(pin);
    }
}

void EXTI0_IRQHandler(void)
{
    gs_gpio_exti_handler(0);
}

void EXTI1_IRQHandler(void)
{
    gs_gpio_exti_handler(1);
}

void EXTI2_IRQHandler(void)
{
    gs_gpio_exti_handler(2);
}

void EXTI3_IRQHandler(void)
{
    gs_gpio_exti_handler(3);
}

void EXTI4_IRQHandler(void)
{
    gs_gpio_exti_handler(4);
}

void EXTI9_5_IRQHandler(void)
{
    for (int i = 5; i <= 9; i++) {
        gs_gpio_exti_handler(i);
    }
}

void EXTI15_10_IRQHandler(void)
{
    for (int i = 10; i <= 15; i++) {
        gs_gpio_exti_handler(i);
    }
}
