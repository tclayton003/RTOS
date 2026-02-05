#ifndef GS_EMBED_DELAY_H
#define GS_EMBED_DELAY_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Delay API - delaying execution a number of micro seconds.

   Some implementations uses busy wait.
   Delay is performed with *interrupts enabled*
*/

#include <gs/util/delay.h>
#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Init delay system.

   Required on platforms: STM32, not reqired on platforms: AVR32, Linux, SAMC.

   @return_gs_error_t
*/
gs_error_t gs_delay_init(void);

#ifdef __cplusplus
}
#endif
#endif
