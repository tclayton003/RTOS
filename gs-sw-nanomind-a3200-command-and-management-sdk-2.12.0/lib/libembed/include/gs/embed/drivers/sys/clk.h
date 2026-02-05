/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
#ifndef GS_EMBED_DRIVERS_SYS_CLK_H
#define GS_EMBED_DRIVERS_SYS_CLK_H
/**
   @file

   System clock.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
   Get system (MCU) frequency.

   @return system clock frequency.
*/
unsigned long gs_sys_clk_get_freq(void);

#ifdef __cplusplus
}
#endif
#endif
