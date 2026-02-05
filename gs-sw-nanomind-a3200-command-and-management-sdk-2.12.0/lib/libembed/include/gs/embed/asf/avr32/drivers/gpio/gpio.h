#ifndef GS_EMBED_ASF_AVR32_DRIVERS_GPIO_GPIO_H
#define GS_EMBED_ASF_AVR32_DRIVERS_GPIO_GPIO_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   AVR32: GPIO interface.
*/

#include <gs/util/drivers/gpio/gpio.h>
#include <gpio.h> // libasf

#ifdef __cplusplus
extern "C" {
#endif

/**
   Extract \a port from standard AVR32 GPIO definition.
*/
#define GS_AVR32_GPIO_PORT_PIN_2_PORT(port_pin) ((port_pin) >> 5)

/**
   Extract \a pin from standard AVR32 GPIO definition.
*/
#define GS_AVR32_GPIO_PORT_PIN_2_PIN(port_pin)  ((port_pin) & 0x1f)

/**
   AVR32 PORT A.
*/
#define GS_AVR32_GPIO_PORT_A  GS_AVR32_GPIO_PORT_PIN_2_PORT(AVR32_PIN_PA00)

/**
   AVR32 PORT B.
*/
#define GS_AVR32_GPIO_PORT_B  GS_AVR32_GPIO_PORT_PIN_2_PORT(AVR32_PIN_PB00)

/**
   AVR32 PORT C.
*/
#define GS_AVR32_GPIO_PORT_C  GS_AVR32_GPIO_PORT_PIN_2_PORT(AVR32_PIN_PC02)

/**
   AVR32 PORT D.
*/
#define GS_AVR32_GPIO_PORT_D  GS_AVR32_GPIO_PORT_PIN_2_PORT(AVR32_PIN_PD00)

/**
   AVR32 PORT B.
   @deprecated use #GS_AVR32_GPIO_PORT_B
*/
#define PORT_B GS_AVR32_GPIO_PORT_B

/**
   AVR32 PORT C.
   @deprecated use #GS_AVR32_GPIO_PORT_C
*/
#define PORT_C GS_AVR32_GPIO_PORT_C

/**
   AVR32 PORT D.
   @deprecated use #GS_AVR32_GPIO_PORT_D
*/
#define PORT_D GS_AVR32_GPIO_PORT_D
    
#ifdef __cplusplus
}
#endif
#endif
