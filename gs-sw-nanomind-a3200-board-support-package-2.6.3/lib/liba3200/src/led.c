/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/led.h>
#include "local.h"

/* GPIO pin mapping for LED's */
#define LED_PIN_0	74
#define LED_PIN_1	76

void gs_a3200_led_init(void)
{
    gpio_configure_pin(LED_PIN_0, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
    gpio_configure_pin(LED_PIN_1, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
}

void gs_a3200_led_on(gs_a3200_led_t led)
{
    switch(led) {
        case GS_A3200_LED_CPU_OK:
            gpio_set_pin_high(LED_PIN_0);
            break;
        case GS_A3200_LED_A:
            gpio_set_pin_high(LED_PIN_1);
            break;
    }
}

void gs_a3200_led_off(gs_a3200_led_t led)
{
    switch(led) {
        case GS_A3200_LED_CPU_OK:
            gpio_set_pin_low(LED_PIN_0);
            break;
        case GS_A3200_LED_A:
            gpio_set_pin_low(LED_PIN_1);
            break;
    }
}

void gs_a3200_led_toggle(gs_a3200_led_t led)
{
    switch(led) {
        case GS_A3200_LED_CPU_OK:
            gpio_toggle_pin(LED_PIN_0);
            break;
        case GS_A3200_LED_A:
            gpio_toggle_pin(LED_PIN_1);
            break;
    }
}
