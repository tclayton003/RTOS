/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/queue.h>
#include <gs/embed/freertos.h>

/**
   Dummy handle.
   FreeRTOS type is a void * - so we make our own type
*/
struct gs_freertos_queue_t {
    // cppcheck-suppress unusedStructMember
    bool unused;
};

gs_error_t gs_queue_create(size_t items, size_t item_size, gs_queue_t * queue)
{
    if (queue == NULL) {
        return GS_ERROR_ARG;
    }
    gs_queue_t q = (gs_queue_t) xQueueCreate(items, item_size);
    if (q == NULL) {
        return GS_ERROR_ALLOC;
    }
    *queue = q;
    return GS_OK;
}

gs_error_t gs_queue_destroy(gs_queue_t queue)
{
    if (queue) {
        vQueueDelete((QueueHandle_t) queue);
    }
    return GS_OK;
}

gs_error_t gs_queue_enqueue(gs_queue_t queue, const void * value, int timeout_ms)
{
    BaseType_t res = xQueueSendToBack((QueueHandle_t) queue, value, gs_freertos_convert_ms_to_ticks(timeout_ms));
    if (res == pdPASS) {
        return GS_OK;
    }
    return GS_ERROR_TIMEOUT;
}

gs_error_t gs_queue_enqueue_isr(gs_queue_t queue, const void * value, gs_context_switch_t * cswitch)
{
    BaseType_t res = xQueueSendToBackFromISR((QueueHandle_t) queue, value, &cswitch->task_woken);
    if (res == pdPASS) {
        return GS_OK;
    }
    return GS_ERROR_FULL;
}

gs_error_t gs_queue_dequeue(gs_queue_t queue, int timeout_ms, void * buf)
{
    BaseType_t res = xQueueReceive((QueueHandle_t) queue, buf, gs_freertos_convert_ms_to_ticks(timeout_ms));
    if (res == pdPASS) {
        return GS_OK;
    }
    return GS_ERROR_TIMEOUT;
}

gs_error_t gs_queue_dequeue_isr(gs_queue_t queue, gs_context_switch_t * cswitch, void * buf)
{
    BaseType_t res = xQueueReceiveFromISR((QueueHandle_t) queue, buf, &cswitch->task_woken);
    if (res == pdPASS) {
        return GS_OK;
    }
    return GS_ERROR_NOT_FOUND;
}

unsigned int gs_queue_size(gs_queue_t queue)
{
    if (queue) {
        return uxQueueMessagesWaiting((QueueHandle_t) queue);
    }
    return 0;
}

unsigned int gs_queue_size_isr(gs_queue_t queue)
{
    if (queue) {
        return uxQueueMessagesWaitingFromISR((QueueHandle_t) queue);
    }
    return 0;
}
