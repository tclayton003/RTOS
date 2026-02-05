/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/avr32/drivers/gpio/gpio.h>
#include <gpio.h>

#define GPIO_PORT_MAX   3  // PORT A = 0, B = 1, C = 2, D = 3
#define GPIO_PIN_MAX    (32-1)

gs_error_t gs_gpio_get(gs_gpio_t gpio, bool *value)
{
    if (gpio.port > (GPIO_PORT_MAX) || gpio.pin > GPIO_PIN_MAX)
        return GS_ERROR_ARG;

    *value = gpio_get_pin_value(gpio.port << 5 | gpio.pin);
    return GS_OK;
}

bool gs_gpio_get_nc(gs_gpio_t gpio)
{
    volatile avr32_gpio_port_t *gpio_port = &AVR32_GPIO.port[gpio.port];

    return ((gpio_port->pvr >> (gpio.pin & 0x1F)) & 1);
}

gs_error_t gs_gpio_set(gs_gpio_t gpio, bool value)
{
    if (gpio.port > (GPIO_PORT_MAX) || gpio.pin > GPIO_PIN_MAX) {
        return GS_ERROR_ARG;
    }

    if (value) {
        gpio_set_pin_high(gpio.port << 5 | gpio.pin);
    } else {
        gpio_set_pin_low(gpio.port << 5 | gpio.pin);
    }

    return GS_OK;
}

void gs_gpio_set_nc(gs_gpio_t gpio, bool value)
{
    volatile avr32_gpio_port_t *gpio_port = &AVR32_GPIO.port[gpio.port];

    if (value) {
        /* Value to be driven on the I/O line: 1. */
        gpio_port->ovrs  = 1 << (gpio.pin & 0x1F);
    } else {
        /* Value to be driven on the I/O line: 0. */
        gpio_port->ovrc  = 1 << (gpio.pin & 0x1F);
    }
}
