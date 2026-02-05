/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/sys/reset.h>
#include <gs/util/log.h>

const char * gs_sys_boot_cause_to_string(gs_sys_boot_cause_t boot_cause)
{
    switch (boot_cause) {
        case GS_SYS_BOOT_UNKNOWN: return "unknown";
        case GS_SYS_BOOT_BROWN_OUT: return "brownout";
        case GS_SYS_BOOT_POWER_ON: return "power on";
        case GS_SYS_BOOT_WATCHDOG: return "watchdog";
        case GS_SYS_BOOT_SW: return "software";
        case GS_SYS_BOOT_PIN_EXTERNAL: return "external reset pin";
        case GS_SYS_BOOT_SLEEP_WAKE: return "sleep wake";
        case GS_SYS_BOOT_ERROR: return "error";
        case GS_SYS_BOOT_JTAG: return "JTAG";
    }
    return "INVALID CAUSE";
}

const char * gs_sys_reset_cause_to_string(gs_sys_reset_cause_t reset_cause)
{
    switch (reset_cause) {
        case GS_SYS_RESET_UNKNOWN: return "unknown";
        case GS_SYS_RESET_CSP_WATCHDOG: return "CSP watchdog";
        case GS_SYS_RESET_GROUND_WATCHDOG: return "ground watchdog";
        case GS_SYS_RESET_STACK_OVERFLOW: return "stack overflow";
        case GS_SYS_RESET_EXCEPTION: return "exception";
        case GS_SYS_RESET_FILE_SYSTEM: return "file system";
        case GS_SYS_RESET_GOSH: return "GOSH";
        case GS_SYS_RESET_CSP: return "CSP";
        default: return gs_sys_app_reset_cause_to_string(reset_cause);
    }
}

__attribute__ ((__weak__)) const char * gs_sys_app_reset_cause_to_string(gs_sys_reset_cause_t reset_cause)
{
    switch (reset_cause) {
        default: return "INVALID CAUSE";
    }
}

void gs_sys_log_causes(gs_sys_boot_cause_t * boot_cause, gs_sys_reset_cause_t * reset_cause, uint32_t * crash_addr)
{
    uint32_t platform_boot_cause;
    gs_sys_boot_cause_t boot_cause_int = gs_sys_get_boot_cause(&platform_boot_cause);
    gs_sys_reset_cause_t reset_cause_int = gs_sys_get_reset_cause();
    log_notice("boot cause: %s (%"PRIu32"), platform boot cause: %"PRIu32", reset cause: %s (%"PRIu32")",
               gs_sys_boot_cause_to_string(boot_cause_int),
               (uint32_t)boot_cause_int,
               platform_boot_cause,
               gs_sys_reset_cause_to_string(reset_cause_int),
               reset_cause_int);
    uint32_t crash_addr_int = gs_sys_get_crash_address();
    if (crash_addr_int != UINT32_MAX) {
        log_warning("program crashed at addr: %p ", GS_TYPES_INT2PTR(crash_addr_int));
    }

    if (boot_cause) {
        *boot_cause = boot_cause_int;
    }
    if (reset_cause) {
        *reset_cause = reset_cause_int;
    }
    if (crash_addr) {
        *crash_addr = crash_addr_int;
    }
}
