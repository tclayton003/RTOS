/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/sys/clk.h>
#include <gs/asf/avr32/asf.h>

unsigned long gs_sys_clk_get_freq(void)
{
    return sysclk_get_cpu_hz();
}
