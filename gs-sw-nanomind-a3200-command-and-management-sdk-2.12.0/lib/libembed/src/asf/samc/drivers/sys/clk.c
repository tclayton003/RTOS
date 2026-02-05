/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/sys/clk.h>
#include <clock.h>

unsigned long gs_sys_clk_get_freq(void)
{
    return system_cpu_clock_get_hz();
}
