#ifndef _GS_SAMC_DRIVERS_GPIO_H_
#define _GS_SAMC_DRIVERS_GPIO_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/error.h>
#include <port.h>  // libasf

/**
   Configure pin as output and set inital state.
   @param[in] gpio_pin Which pin to configure.
   @param[in] initial_state Initial pin state
*/
gs_error_t gs_gpio_configure_output(uint32_t gpio_pin, bool initial_state);

/**
   Configure pin as input.
   @param[in] gpio_pin Which pin to configure.
   @param[in] pull pull PORT_PIN_PULL_UP, PORT_PIN_PULL_DOWN or PORT_PIN_PULL_NONE.
*/
gs_error_t gs_gpio_configure_input(uint32_t gpio_pin, enum port_pin_pull pull);

#endif
