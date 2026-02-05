/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <reset.h>
#include <gs/embed/drivers/sys/reset.h>

void gs_sys_reset(gs_sys_reset_cause_t reset_cause)
{
    system_reset();
}

gs_sys_reset_cause_t gs_sys_get_reset_cause()
{
    /* Not implemented */
    return GS_SYS_RESET_UNKNOWN;
}

gs_sys_boot_cause_t gs_sys_get_boot_cause(uint32_t * platform_boot_cause)
{
    /* Not implemented */
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
