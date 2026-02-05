#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_SYS_CLOCK_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_SYS_CLOCK_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   @brief STM32L4 clock configuration API
*/
#include <gs/embed/stm32/stm32l4/error.h>

/**
   Configure internal clock at 80 MHz

   @return_gs_error_t
 */
gs_error_t gs_stm32l4_clock_config_internal_80_mhz(void);

#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_SYS_CLOCK_H_ */
