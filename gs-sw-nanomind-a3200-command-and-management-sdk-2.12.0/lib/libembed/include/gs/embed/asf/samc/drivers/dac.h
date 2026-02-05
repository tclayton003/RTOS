#ifndef _LIB_LIBSAMC_INCLUDE_DRIVERS_DAC_H_
#define _LIB_LIBSAMC_INCLUDE_DRIVERS_DAC_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <util/error.h>

gs_error_t gs_dac_init(int clock_source, uint16_t initialValue);
gs_error_t gs_dac_write(uint16_t value);
uint16_t gs_dac_read(void);

#endif
