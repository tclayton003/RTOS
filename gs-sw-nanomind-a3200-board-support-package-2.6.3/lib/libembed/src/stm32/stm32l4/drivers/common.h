/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
#ifndef LIB_LIBEMBED_SRC_STM32_STM32L4_DRIVERS_COMMON_H_
#define LIB_LIBEMBED_SRC_STM32_STM32L4_DRIVERS_COMMON_H_

#include <gs/embed/freertos.h>

/**
   The ARM Cortex M4 core has a nested interrupt vector scheme
   Around these drivers all the internal communication ISR's get the same priority
   so they will not interrupt each other
   The value is set to second highest priority (high priority = low numerical value)
   which still allow call to FreeRTOS API functions
   This allows for user to set an external ISR with one higher priority
 */
#define GS_STM32L4_DEFAULT_ISR_PRIORITY     (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

#endif /* LIB_LIBEMBED_SRC_STM32_STM32L4_DRIVERS_COMMON_H_ */
