/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/embed/delay.h>
#include <gs/asf/avr32/asf.h>

gs_error_t gs_delay_init(void)
{
    return GS_OK;
}

void gs_delay_us(uint32_t us)
{
    cpu_delay_us(us, sysclk_get_cpu_hz());
}
