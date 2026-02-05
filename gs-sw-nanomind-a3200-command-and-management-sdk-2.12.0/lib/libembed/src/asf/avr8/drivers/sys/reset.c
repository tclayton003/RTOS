/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/freertos.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <gs/util/delay.h>
#include <gs/embed/drivers/sys/reset.h>

void gs_sys_reset(gs_sys_reset_cause_t reset_cause)
{
    wdt_enable(WDTO_15MS);
    cli();
    for (;;) {
        gs_delay_us(1000);
    }
}

// Function Pototype
void sys_init(void) __attribute__((naked, used)) __attribute__((section(".init3")));

// Function Implementation
void sys_init(void)
{
    wdt_disable();
}

gs_sys_reset_cause_t gs_sys_get_reset_cause()
{
    return GS_SYS_RESET_UNKNOWN;
}

gs_sys_boot_cause_t gs_sys_get_boot_cause(uint32_t * platform_boot_cause)
{
    return GS_SYS_BOOT_UNKNOWN;
}

uint32_t gs_sys_get_crash_address()
{
    /* Not implemented */
    return UINT32_MAX;
}

void gs_sys_clear_reset_cause(void)
{
    /* Not implemented */
}
