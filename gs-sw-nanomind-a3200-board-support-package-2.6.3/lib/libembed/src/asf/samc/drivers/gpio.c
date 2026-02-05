/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   Interface for controlling General Purpose In/Out (GPIO)
*/

/* --- includes ----------------------------------------------------------*/

#include <gs/embed/asf/samc/drivers/gpio.h> // first - ensure self contained

gs_error_t gs_gpio_configure_output(uint32_t gpio_pin, bool initial_state)
{
    struct port_config config;
    port_get_config_defaults(&config);

    config.direction = PORT_PIN_DIR_OUTPUT;
    config.input_pull = PORT_PIN_PULL_NONE;
    port_pin_set_config(gpio_pin, &config);

    port_pin_set_output_level(gpio_pin, initial_state);

    return GS_OK;
}

gs_error_t gs_gpio_configure_input(uint32_t gpio_pin, enum port_pin_pull pull)
{
    struct port_config config;
    port_get_config_defaults(&config);

    config.direction = PORT_PIN_DIR_INPUT;
    config.input_pull = pull;
    port_pin_set_config(gpio_pin, &config);

    return GS_OK;
}
