/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/can/can.h>
#include <gs/embed/asf/avr32/drivers/can/can.h>

gs_error_t gs_can_driver_print_stats(FILE * out, uint8_t device)
{
    return GS_ERROR_NOT_SUPPORTED;
}

// avr32: not used on Linux
gs_error_t gs_can_init_w_fallback(uint8_t device, uint32_t bps, uint32_t fallback_bps)
{
    return GS_OK;
}

// avr32: not used on Linux
gs_error_t gs_can_init(uint8_t device, uint32_t brate)
{
    return GS_OK;
}
