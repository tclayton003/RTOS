/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <limits.h>
#include <gs/util/sem.h>
#include <gs/embed/freertos.h>

gs_error_t gs_sem_create(unsigned int initialValue, gs_sem_t * sem)
{
    if (sem == NULL) {
        return GS_ERROR_ARG;
    }
    *sem = (gs_sem_t) xSemaphoreCreateCounting((UBaseType_t)-1, initialValue);
    if (*sem == NULL) {
        return GS_ERROR_ALLOC;
    }

    return GS_OK;
}

gs_error_t gs_sem_destroy(gs_sem_t sem)
{
    if (sem) {
        vSemaphoreDelete(sem);
    }
    return GS_OK;
}

gs_error_t gs_sem_wait(gs_sem_t sem, int timeout_ms)
{
    TickType_t ticks;
    if (timeout_ms < 0) {
        ticks = portMAX_DELAY;
    } else {
        ticks = timeout_ms / portTICK_RATE_MS;
    }

    BaseType_t res = xSemaphoreTake((SemaphoreHandle_t) sem, ticks);
    if (res == pdPASS) {
        return GS_OK;
    }
    return GS_ERROR_TIMEOUT;
}

gs_error_t gs_sem_post(gs_sem_t sem)
{
    BaseType_t res = xSemaphoreGive(sem);
    if (res == pdPASS) {
        return GS_OK;
    }
    return GS_ERROR_FULL;
}

gs_error_t gs_sem_post_isr(gs_sem_t sem, gs_context_switch_t * cswitch)
{
    BaseType_t res = xSemaphoreGiveFromISR(sem, &cswitch->task_woken);
    if (res == pdPASS) {
        return GS_OK;
    }
    return GS_ERROR_FULL;
}
