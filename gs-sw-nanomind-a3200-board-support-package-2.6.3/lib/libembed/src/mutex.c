/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/mutex.h>
#include <gs/embed/freertos.h>

gs_error_t gs_mutex_create(gs_mutex_t * mutex)
{
    if (mutex == NULL) {
        return GS_ERROR_ARG;
    }

    *mutex = (gs_mutex_t) xSemaphoreCreateRecursiveMutex();
    if (*mutex == NULL) {
        return GS_ERROR_ALLOC;
    }

    return GS_OK;
}

gs_error_t gs_mutex_destroy(gs_mutex_t mutex)
{
    if (mutex) {
        vSemaphoreDelete(mutex);
    }
    return GS_OK;
}

gs_error_t gs_mutex_lock(gs_mutex_t mutex)
{
    if (mutex) {
        BaseType_t res = xSemaphoreTakeRecursive((SemaphoreHandle_t) mutex, portMAX_DELAY);
        if (res == pdPASS) {
            return GS_OK;
        }
        return GS_ERROR_TIMEOUT;
    }
    return GS_ERROR_HANDLE;
}

gs_error_t gs_mutex_unlock(gs_mutex_t mutex)
{
    if (mutex) {
        BaseType_t res = xSemaphoreGiveRecursive((SemaphoreHandle_t) mutex);
        if (res == pdPASS) {
            return GS_OK;
        }
        return GS_ERROR_PERM;
    }
    return GS_ERROR_HANDLE;
}
