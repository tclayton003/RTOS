/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   Delay routines based on libasf.
*/

#include <gs/embed/delay.h>

// libasf
#include <common2/services/delay/delay.h>

gs_error_t gs_delay_init(void)
{
    return GS_OK;
}

void gs_delay_us(uint32_t time_us)
{
    delay_us(time_us);
}
