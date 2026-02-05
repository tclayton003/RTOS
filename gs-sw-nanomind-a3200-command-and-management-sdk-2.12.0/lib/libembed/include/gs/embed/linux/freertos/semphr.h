#ifndef GS_EMBED_LINUX_FREERTOS_SEMPHR_H
#define GS_EMBED_LINUX_FREERTOS_SEMPHR_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   FreeRTOS semaphore support on Linux.
*/

#if !(__DOXYGEN__)

#include <gs/util/sem.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef gs_sem_t SemaphoreHandle_t;
typedef gs_sem_t xSemaphoreHandle;

#define vSemaphoreCreateBinary( xSemaphore )  gs_sem_create(1, &xSemaphore);

static inline SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    SemaphoreHandle_t sem;
    return (gs_sem_create(1, &sem) == GS_OK) ? sem : NULL;
}

static inline SemaphoreHandle_t xSemaphoreCreateBinary(void)
{
    SemaphoreHandle_t sem;
    return (gs_sem_create(0, &sem) == GS_OK) ? sem : NULL;
}

static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t sem, TickType_t xTicksToWait)
{
    return (gs_sem_wait(sem, xTicksToWait) == GS_OK) ? pdTRUE : pdFALSE;
}

static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t sem)
{
    return (gs_sem_post(sem) == GS_OK) ? pdTRUE : pdFALSE;
}

#ifdef __cplusplus
}
#endif
#endif // __DOXYGEN__
#endif
