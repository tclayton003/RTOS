#ifndef _GS_SAMC_MCU_H_
#define _GS_SAMC_MCU_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   MCU specific commands.
*/

#include <gs/util/error.h>

gs_error_t gs_mcu_check_configuration(void);
gs_error_t gs_mcu_configure(void);

/**
   Get bootcause (GOMspace) or resetcause (Atmel)
*/
gs_error_t gs_mcu_get_boot_cause(uint16_t * cause);

#endif
