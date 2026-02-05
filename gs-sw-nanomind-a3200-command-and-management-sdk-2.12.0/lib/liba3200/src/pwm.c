/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   PWM driver

   Driver configuration:
   Using the DRV8838.

   PIN:  PWML (GPIO) used for IN1/Phase
         PWMH (PWM) used for IN2/Enable

   Operations:
       One side of the coil (OUT1 or OUT2 depending on direction) will be grounded.
       The other side will switch (PWM) between GDN/Vcc. Coast mode is not used.
       Shorting the coil is used to get a faster responce from the coil.
*/

#include <gs/a3200/pwm.h>
#include <gs/a3200/pwr_switch.h>
#include "local.h"
#include <pwm4.h>


#define PWM_NCHANS 3

static struct {
    int channel;
    int pinl;
    int pinh;
    int pinh_function;
} pwm_map[PWM_NCHANS] = {
    {0, PWM_PIN_L0, PWM_PIN_H0, PWM_PIN_H0_FUNCTION},
    {3, PWM_PIN_L1, PWM_PIN_H1, PWM_PIN_H1_FUNCTION},
    {2, PWM_PIN_L2, PWM_PIN_H2, PWM_PIN_H2_FUNCTION}
};

/* Global config structs */
static pwm_opt_t pwm_opt;
static avr32_pwm_channel_t pwm_channel;

void gs_a3200_pwm_init(void)
{
    /* Enable PWML as GPIO */
    gpio_configure_pin(pwm_map[0].pinl, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
    gpio_configure_pin(pwm_map[1].pinl, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
    gpio_configure_pin(pwm_map[2].pinl, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);

    /* Enable PWMH as PWM module pins */
    gpio_enable_module_pin(pwm_map[0].pinh, pwm_map[0].pinh_function);
    gpio_enable_module_pin(pwm_map[1].pinh, pwm_map[1].pinh_function);
    gpio_enable_module_pin(pwm_map[2].pinh, pwm_map[2].pinh_function);

    /* PWM controller config */
    pwm_opt.diva = 10;
    pwm_opt.prea = AVR32_PWM_PREA_CCK;
    pwm_opt.cksel = PWM_CKSEL_MCK; // Selecting master clock instead of generic clock
    pwm_opt.fault_detection_activated = false;
    pwm_opt.sync_channel_activated    = false;
    
    pwm_init(&pwm_opt);

    /* Channel config */
    pwm_channel.CMR.dte   = 0; // Disable deadtime
    pwm_channel.CMR.dthi  = 0; // Signal not inverted on PWMH
    pwm_channel.CMR.dtli  = 0; // Signal not inverted on PWML
    pwm_channel.CMR.ces   = 0; // End of period event (not used)
    pwm_channel.CMR.calg  = PWM_MODE_LEFT_ALIGNED; // Channel mode.
    pwm_channel.CMR.cpol  = PWM_POLARITY_LOW; // Channel polarity.
    pwm_channel.CMR.cpre  = AVR32_PWM_CPRE_CLKA; // Channel prescaler.
    pwm_channel.cdty      = 0; // Channel duty cycle, startup as zero
    pwm_channel.cprd      = CPRD_MAX; // Channel period

    /* Apply settings */
    pwm_channel_init(pwm_map[0].channel, &pwm_channel);
    pwm_channel_init(pwm_map[1].channel, &pwm_channel);
    pwm_channel_init(pwm_map[2].channel, &pwm_channel);

    /* Turn on power channel */
    gs_a3200_pwr_switch_enable(GS_A3200_PWR_PWM);
}

void gs_a3200_pwm_enable(int id)
{
    if (id < 0 || id > PWM_NCHANS)
        return;

    pwm_start_channels(1 << pwm_map[id].channel);
}

void gs_a3200_pwm_disable(int id)
{
    if (id < 0 || id > PWM_NCHANS)
        return;

    pwm_stop_channels(1 << pwm_map[id].channel);
}

void gs_a3200_pwm_set_dir(int id, uint8_t dir)
{
    if (id < 0 || id > PWM_NCHANS)
        return;

    if (dir == 1) {
    	gpio_set_pin_high(pwm_map[id].pinl);
    } else {
    	gpio_set_pin_low(pwm_map[id].pinl);
    }

}

/* Atmel datasheet page 997 + 998. Divider 8 bit, Prescaler 4 bit (0-10 value).
 * CPRD_ MAX fixed to make the resolution become 16 bit */
/* The pwm cant handle frequencies above 433 Hz or belov 0.1 Hz */
float gs_a3200_pwm_set_freq(int id, float freq)
{
    if (id < 0 || id > PWM_NCHANS)
        return -1;
    pwm_opt.prea = (AVR32_PWM_PREA_CCK); // Set prescaler to 0
    float base = (sysclk_get_peripheral_bus_hz(&AVR32_PWM) * 2. / CPRD_MAX);
    float result = base;
    int divi = 1;

    while (result > (freq * 2)) {
        result = base / (float)(divi*(pwm_opt.prea +1));
        divi++;	// Increment divider until frequency is possible.
        if (divi > 255){
            pwm_opt.prea++; // Increment prescaler if divider becomes to large
            divi = 1;
            if (pwm_opt.prea >= AVR32_PWM_PREA_CCK_DIV_1024){
                break;
            }
        }
    }
    //	printf("Divider, prescaler: %d %d\r\n",divi, pwm_opt.prea);
    //    divi--;

    pwm_opt.diva = divi;
    //    pwm_opt.prea = (AVR32_PWM_PREA_CCK);
    pwm_init(&pwm_opt);

    return result / 2;
}

void gs_a3200_pwm_set_duty(int id, float duty)
{
    int cdty;

    /* Error handling */
    if (id < 0 || id > PWM_NCHANS)
        return;
    
    if (duty < -100)
        duty = -100;
    if (duty > 100)
        duty = 100;

    /* Setting duty cycle and direction */
    if (duty < 0) {
        cdty = (int)((CPRD_MAX / 100.) * (-1. * duty));
        pwm_channel.cdty = CPRD_MAX - cdty;
        pwm_channel_init(pwm_map[id].channel, &pwm_channel);
        gs_a3200_pwm_set_dir(id, 0);
    } else {
        cdty = (int)((CPRD_MAX / 100.) * duty);
        pwm_channel.cdty = CPRD_MAX - cdty;
        pwm_channel_init(pwm_map[id].channel, &pwm_channel);
        gs_a3200_pwm_set_dir(id, 1);
    }
}
