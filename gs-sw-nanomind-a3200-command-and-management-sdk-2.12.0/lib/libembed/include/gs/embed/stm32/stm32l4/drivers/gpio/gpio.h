#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_GPIO_GPIO_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_GPIO_GPIO_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   @brief STM32L4 GPIO API

   This API includes initialization routine of GPIO.
   The cross platform GPIO API holds functions for get/set GPIO.
*/

#include <gs/util/error.h>
#include <gs/util/drivers/gpio/gpio.h>

/**
   Port numbering for gs_gpio_t
 */
typedef enum {
    //! PORT A
    PORT_A,
    //! PORT B
    PORT_B,
    //! PORT C
    PORT_C,
    //! PORT D
    PORT_D,
    //! PORT E
    PORT_E,
    //! PORT H
    PORT_H
} gs_stm32l4_gpio_port;

/**
   Pin configuration

   Values for struct are found in stm32l4xx_hal_gpio.h and stm32l4xx_hal_gpio_ex.h
 */
typedef struct gs_stm32l4_gpio_init{
    //! Mode of pin, push/pull, analog, etc. (Must be of GPIO_mode)
    uint32_t mode;
    //! Internal pull, pullup, pulldown, no pull (Must be of GPIO_pull)
    uint32_t pull;
    //! Speed/slewrate of pin (Must be of GPIO_pull). High speed draws more current
    uint32_t speed;
    //! Peripheral to be connected to the selected pins (Must be of GPIOEx_Alternate_function_selection)
    uint32_t alternate;
} gs_stm32l4_gpio_conf_t;

/**
   Initialize a GPIO

   @param[in] gpio Pin to configure
   @param[in] conf Configuration for pin
   @return_gs_error_t
 */
gs_error_t gs_stm32l4_gpio_init(gs_gpio_t gpio, const gs_stm32l4_gpio_conf_t * conf);

#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_GPIO_GPIO_H_ */
