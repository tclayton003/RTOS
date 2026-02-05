/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Power switch driver.
*/

#include <gs/a3200/pwr_switch.h>
#include "local.h"

#if !GS_A3200_SDK
// no parameter system
#define GS_A3200_CONFIGURATION_GSSB1_PWR_EN_SET_NO_CALLBACK(state)
#define GS_A3200_CONFIGURATION_FLASH_PWR_EN_SET_NO_CALLBACK(state)
#define GS_A3200_CONFIGURATION_GSSB2_PWR_EN_SET_NO_CALLBACK(state)
#define GS_A3200_CONFIGURATION_PWM_PWR_EN_SET_NO_CALLBACK(state)
#endif

void gs_a3200_pwr_switch_init(void)
{
    gpio_configure_pin(PWR_GSSB_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
    gpio_configure_pin(PWR_SD_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
    gpio_configure_pin(PWR_GSSB2_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
    if (GS_A3200_BOARD_REVISION >= 3) {
        gpio_configure_pin(PWR_PWM_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
    }
}

static inline void my_gpio_set_pin(uint32_t pin, bool state)
{
    if (state) {
        gpio_set_pin_high(pin);
    } else {
        gpio_set_pin_low(pin);
    }
}

void gs_a3200_pwr_switch_set(gs_a3200_pwr_switch_t pwr_switch, bool state)
{
    switch (pwr_switch) {
        case GS_A3200_PWR_GSSB:
            my_gpio_set_pin(PWR_GSSB_PIN, state);
            GS_A3200_CONFIGURATION_GSSB1_PWR_EN_SET_NO_CALLBACK(state);
            break;
        case GS_A3200_PWR_SD:
            my_gpio_set_pin(PWR_SD_PIN, state);
            GS_A3200_CONFIGURATION_FLASH_PWR_EN_SET_NO_CALLBACK(state);
            break;
        case GS_A3200_PWR_GSSB2:
            my_gpio_set_pin(PWR_GSSB2_PIN, state);
            GS_A3200_CONFIGURATION_GSSB2_PWR_EN_SET_NO_CALLBACK(state);
            break;
        case GS_A3200_PWR_PWM:
            if (GS_A3200_BOARD_REVISION >= 3) {
                my_gpio_set_pin(PWR_PWM_PIN, state);
                GS_A3200_CONFIGURATION_PWM_PWR_EN_SET_NO_CALLBACK(state);
            }
            break;
    }
}

bool gs_a3200_pwr_switch_get(gs_a3200_pwr_switch_t sw)
{
    switch (sw) {
        case GS_A3200_PWR_GSSB:
            return gpio_get_pin_value(PWR_GSSB_PIN);
        case GS_A3200_PWR_SD:
            return gpio_get_pin_value(PWR_SD_PIN);
        case GS_A3200_PWR_GSSB2:
            return gpio_get_pin_value(PWR_GSSB2_PIN);
        case GS_A3200_PWR_PWM:
            if (GS_A3200_BOARD_REVISION >= 3) {
                return gpio_get_pin_value(PWR_PWM_PIN);
            }
            return false;
        default:
            return false;
    }
}
