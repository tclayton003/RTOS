#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_WATCHDOG_WATCHDOG_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_WATCHDOG_WATCHDOG_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
    @file

    @brief STM32L4 watchdog API

   This API includes initialization of two different watchdogs.
   The independent watchdog is what GS use as the standard. It is capable of of longer timeouts.
   The window watchdog is more precise, and can also be used to just generate a interrupt.
*/

#include <gs/util/drivers/watchdog/device.h>

/**
   Callback from ISR, which the window watchdog can be configured to call instead of a reset generation
 */
typedef void (*gs_watchdog_wwdg_callback_t)(void);

/**
   Get device operations for libutil watchdog API

   @return pointer to device operations
*/
const gs_watchdog_dev_ops_t * gs_stm32l4_watchdog_get_dev_ops(void);

/**
   Initialize independent watchdog with specific timeout (this is the standard watchdog)

   @note Can be configured to call a callback instead of generating reset.
         But if watchdog ISR for some reason is not called, a reset is generated anyway.
   @note The range of of achievable timeouts is dependent on the specific clock setup.
         The watchdog clock is derived from PCLK1. The PLCK1 is derived from the SYSCLK through two dividers.
         Example: If SYSCLK is 80 MHz and no dividers is used to PLCK1, then the span of achievable timeout
         is within the span [51.2 µs: 26.2 ms]

   @param[in] timeout_val_ms Desired timeout in milliseconds
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_watchdog_iwdg_init(uint32_t timeout_val_ms);

/**
   Kicking/resetting the independent watchdog, which prevents reset

   @return_gs_error_t
*/
gs_error_t gs_stm32l4_watchdog_iwdg_kick(void);

/**
   Initialize system window watchdog with specific timeout

   @note Can be configured to call a callback instead of generating reset.
         But if watchdog ISR for some reason is not called, a reset is generated anyway.
   @note The range of of achievable timeouts is dependent on the specific clock setup.
         The watchdog clock is derived from PCLK1. The PLCK1 is derived from the SYSCLK through two dividers.
         Example: If SYSCLK is 80 MHz and no dividers is used to PLCK1, then the span of achievable timeout
         is within the span [51.2 µs: 26.2 ms]

   @param[in] timeout_ms Desired timeout in milliseconds
   @param[in] callback Watchdog callback (can be NULL), if set this callback is called instead of generating a reset
   @param[out] timeout_ms_achieved The actual achieved timeout in milliseconds (can be NULL)
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_watchdog_wwdg_init(float timeout_ms, gs_watchdog_wwdg_callback_t callback, float * timeout_ms_achieved);

/**
   Kicking/resetting the system window watchdog, which prevents reset

   @return_gs_error_t
*/
gs_error_t gs_stm32l4_watchdog_wwdg_kick(void);

#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_WATCHDOG_WATCHDOG_H_ */
