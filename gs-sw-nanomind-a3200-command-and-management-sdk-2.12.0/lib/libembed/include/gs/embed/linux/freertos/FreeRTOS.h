#ifndef GS_EMBED_LINUX_FREERTOS_FREERTOS_H
#define GS_EMBED_LINUX_FREERTOS_FREERTOS_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   FreeRTOS.h (and FreeRTOSConfig.h) on linux.
*/

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   On linux 1 tick = 1 mS (for simplicity)
*/
#define configTICK_RATE_HZ     1000

/**
   On linux 1 tick = 1 mS (for simplicity)
*/
#define portTICK_RATE_MS       (1000 / configTICK_RATE_HZ)

/**
   Not used on Linux, but required by FreeRTOS.
*/
#define configMAX_PRIORITIES   10

/**
   Max task name length.
*/
#define configMAX_TASK_NAME_LEN  16

/**
   Max delay in ticks (= mS).
*/
#define portMAX_DELAY          UINT_MAX

#ifdef __cplusplus
}
#endif
#endif
