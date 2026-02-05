/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/error.h>

/**
   Configure internal .
   GCLK[0]: 48 MHz (master clock, input DPLL)
   GCLK[1]:  1 MHz (input OSC48M 4MHz clock)
   DPLL   : 48 MHz -> GCLK[0], GCLK[2]
*/
gs_error_t gs_clock_configure_internal_48_mhz(void);
