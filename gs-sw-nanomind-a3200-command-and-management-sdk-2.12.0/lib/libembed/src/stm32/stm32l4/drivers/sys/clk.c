/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/sys/clk.h>
#include <stm32l4xx_hal.h>

unsigned long gs_sys_clk_get_freq(void)
{
    return SystemCoreClock;
}
