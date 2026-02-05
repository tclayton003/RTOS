#ifndef GS_A3200_LED_H
#define GS_A3200_LED_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   LED interface.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
   LED definitions.
*/
typedef enum {
    GS_A3200_LED_CPU_OK = 0,
    GS_A3200_LED_A = 1,
} gs_a3200_led_t ;

/**
   Initialize GPIO controlling the LEDs.
*/
void gs_a3200_led_init(void);

/**
   Turn LED on.
   @param led led.
*/
void gs_a3200_led_on(gs_a3200_led_t led);

/**
   Turn LED off.
   @param led led.
*/
void gs_a3200_led_off(gs_a3200_led_t led);

/**
   Toggle LED
   @param led led.
*/
void gs_a3200_led_toggle(gs_a3200_led_t led);

#ifdef __cplusplus
}
#endif
#endif
