#ifndef GS_A3200_PWM_H
#define GS_A3200_PWM_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Pulse-Width Modulation driver
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   GPIO pin mapping
   @{
*/
#define PWM_PIN_L0			AVR32_PIN_PB08				//!< GPIO 40	(GPIO)
#define PWM_PIN_H0			AVR32_PWM_PWMH_0_1_PIN			//!< GPIO 41	(PWMH0)
#define PWM_PIN_H0_FUNCTION		AVR32_PWM_PWMH_0_1_FUNCTION
#define PWM_PIN_L1			AVR32_PIN_PA26				//!< GPIO 26	(GPIO)
#define PWM_PIN_H1			AVR32_PWM_PWMH_3_PIN			//!< GPIO 75	(PWMH3)
#define PWM_PIN_H1_FUNCTION		AVR32_PWM_PWMH_3_FUNCTION
#define PWM_PIN_L2			AVR32_PIN_PA19				//!< GPIO 19	(GPIO)
#define PWM_PIN_H2			AVR32_PWM_PWMH_2_PIN			//!< GPIO 77	(PWMH2)
#define PWM_PIN_H2_FUNCTION		AVR32_PWM_PWMH_2_FUNCTION
/**@}*/

/**
   Channel period.
*/
#define CPRD_MAX 65535

/**
   Run this before the other functions
*/
void gs_a3200_pwm_init(void);

/**
   Set changing frequency for PWM channel.

   @param channel Channel number
   @param freq Requested frequency in Hz
   @return Actual achieved PWM frequency
*/
float gs_a3200_pwm_set_freq(int channel, float freq);

/**
   Set duty cycle for channel.

   @param channel Channel number
   @param duty Duty cycle (float) in percent (-100 to 100 sign=direction)
*/
void gs_a3200_pwm_set_duty(int channel, float duty);

/**
   Set polarity for PWM channel.
   
   @param channel number
   @param dir Polarity, 0=positive, 1=negative
*/
void gs_a3200_pwm_set_dir(int channel, uint8_t dir);

/**
   Enable PWM output.

   @param channel Channel number
*/
void gs_a3200_pwm_enable(int channel);

/**
   Disable PWM output.

   @param channel Channel number
*/
void gs_a3200_pwm_disable(int channel);

/**
   Register commands.
   @return_gs_error_t
*/
gs_error_t gs_a3200_pwm_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
