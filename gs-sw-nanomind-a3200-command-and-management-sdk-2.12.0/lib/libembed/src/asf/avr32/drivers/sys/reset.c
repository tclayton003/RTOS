/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/crc32.h>
#include <gs/asf/avr32/asf.h>
#include <gs/embed/drivers/sys/reset.h>
#include "wdt.h"


volatile gs_reset_cause_store_t __attribute__ ((section(".reset_cause_store"))) gs_reset_cause;


void gs_sys_reset(gs_sys_reset_cause_t reset_cause)
{
    gs_reset_cause.reset_cause = reset_cause;
    gs_reset_cause.reset_cause_crc = gs_crc32((void*)&gs_reset_cause.reset_cause, sizeof(gs_reset_cause.reset_cause));
    wdt_clear();
    reset_do_soft_reset();
}

static gs_sys_reset_cause_t local_reset_cause = UINT32_MAX;
gs_sys_reset_cause_t gs_sys_get_reset_cause()
{
    if (local_reset_cause == UINT32_MAX)
    {
        local_reset_cause = GS_SYS_RESET_UNKNOWN;
        if (gs_crc32((void*)&gs_reset_cause.reset_cause, sizeof(gs_reset_cause.reset_cause)) ==
            gs_reset_cause.reset_cause_crc)
        {
            local_reset_cause = gs_reset_cause.reset_cause;
        }
    }
    return local_reset_cause;
}

gs_sys_boot_cause_t gs_sys_get_boot_cause(uint32_t * platform_boot_cause)
{
    gs_sys_boot_cause_t boot_cause = GS_SYS_BOOT_UNKNOWN;
    reset_cause_t asf_cause = reset_cause_get_causes();

    if (asf_cause & CHIP_RESET_CAUSE_EXTRST) {
        boot_cause = GS_SYS_BOOT_PIN_EXTERNAL;
    }

    if (asf_cause & AVR32_PM_RCAUSE_BOD_MASK) {
        boot_cause = GS_SYS_BOOT_BROWN_OUT;
    }

    if (asf_cause & AVR32_PM_RCAUSE_BOD33_MASK) {
        boot_cause = GS_SYS_BOOT_BROWN_OUT;
    }

    if (asf_cause & AVR32_PM_RCAUSE_OCDRST_MASK) {
        boot_cause = GS_SYS_BOOT_SW;
    }

    if (asf_cause & AVR32_PM_RCAUSE_POR_MASK) {
        boot_cause = GS_SYS_BOOT_POWER_ON;
    }

    if (asf_cause & CHIP_RESET_CAUSE_JTAG) {
        boot_cause = GS_SYS_BOOT_JTAG;
    }

    if (asf_cause & AVR32_PM_RCAUSE_WDT_MASK) {
        boot_cause = GS_SYS_BOOT_WATCHDOG;
    }

    if (asf_cause & AVR32_PM_RCAUSE_CPUERR_MASK) {
        boot_cause = GS_SYS_BOOT_ERROR;
    }

    if (asf_cause & AVR32_PM_RCAUSE_SLEEP_MASK) {
        boot_cause = GS_SYS_BOOT_SLEEP_WAKE;
    }

    if (platform_boot_cause) {
        *platform_boot_cause = asf_cause;
    }

    return boot_cause;
}

static uint32_t local_crash_addr = UINT32_MAX;
uint32_t gs_sys_get_crash_address()
{
    if (local_crash_addr == UINT32_MAX)
    {
        if (gs_crc32((void*)&gs_reset_cause.crash_addr, sizeof(gs_reset_cause.crash_addr)) ==
                gs_reset_cause.crash_addr_crc)
        {
            local_crash_addr = gs_reset_cause.crash_addr;
        }
    }
    return local_crash_addr;
}

void gs_sys_clear_reset_cause(void)
{
    /* Clear content and invalidate CRC's */
    gs_reset_cause.crash_addr = UINT32_MAX;
    gs_reset_cause.crash_addr_crc = 0xDEADBEEF;
    gs_reset_cause.reset_cause = GS_SYS_RESET_UNKNOWN;
    gs_reset_cause.reset_cause_crc = 0xDEADAFFE;
    return;
}
