/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/thread.h>
#include <gs/util/minmax.h>
#include <gs/util/log.h>
#include <gs/util/time.h>
#include <gs/util/lock.h>
#include <gs/embed/freertos.h>
#include <gs/embed/drivers/sys/reset.h>

#if (configMAX_PRIORITIES < 5)
#error configMAX_PRIORITIES must be at least 5
#endif

#ifndef GS_EMBED_THREAD_MIN_STACK_SIZE
#define GS_EMBED_THREAD_MIN_STACK_SIZE   0
#endif

// No check if FreeRTOS configMINIMAL_STACK_SIZE < GS_EMBED_THREAD_MIN_STACK_SIZE

gs_error_t gs_thread_create_with_stack(const char * const name,
                                       gs_thread_func_t func,
                                       void * parameter,
                                       size_t stack_size,
                                       gs_stack_type_t *stack_buf,
                                       gs_thread_priority_t priority,
                                       uint32_t flags,
                                       gs_thread_t * return_handle)
{
    gs_time_uptime(); // force initialize of lock in time API
    gs_lock_init(); // initialize global lock.
    
    UBaseType_t uxPriority = 2;

    // no default - compiler will tell if we are missing something
    switch (priority) {
        case GS_THREAD_PRIORITY_IDLE:
            uxPriority = 0;
            break;
        case GS_THREAD_PRIORITY_LOW:
            uxPriority = 1;
            break;
        case GS_THREAD_PRIORITY_NORMAL:
            uxPriority = 2;
            break;
        case GS_THREAD_PRIORITY_HIGH:
            uxPriority = 3;
            break;
        case GS_THREAD_PRIORITY_CRITICAL:
            uxPriority = 4;
            break;
    }

    if ((stack_buf == NULL) && (stack_size <= GS_EMBED_THREAD_MIN_STACK_SIZE)) {
        stack_size = GS_EMBED_THREAD_MIN_STACK_SIZE;
    }

    xTaskHandle ref;
#if (tskKERNEL_VERSION_MAJOR >= 9)

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    portBASE_TYPE ret = pdFALSE;
    if (stack_buf == NULL) {
        ret = xTaskCreate((TaskFunction_t) func,
                          name,
                          (stack_size / sizeof(StackType_t)), // stack_size is number of 'StackType_t'
                          parameter,
                          uxPriority,
                          &ref);
    } else {
        /* Reserve the first portion of stack buffer for tcb/ */
        StaticTask_t * task_buf = (StaticTask_t*)stack_buf;
        StackType_t  * stack_ptr = (StackType_t*)(task_buf++);

        ref = xTaskCreateStatic((TaskFunction_t) func,
                                name,
                                stack_size - sizeof(StaticTask_t),
                                parameter,
                                uxPriority,
                                stack_ptr,
                                task_buf);
        if (ref != NULL) {
            ret = pdTRUE;
        }
    }
#else
    if (stack_buf) {
        log_warning("configSUPPORT_STATIC_ALLOCATION not set, ignoring stack buffer!!!");
    }
    portBASE_TYPE ret = xTaskCreate((TaskFunction_t) func,
                                    name,
                                    (stack_size / sizeof(StackType_t)), // stack_size is number of 'StackType_t'
                                    parameter,
                                    uxPriority,
                                    &ref);
#endif

#else // #if (tskKERNEL_VERSION_MAJOR >= 9)
    portBASE_TYPE ret = xTaskGenericCreate((TaskFunction_t) func,
                                           name,
                                           (stack_size / sizeof(StackType_t)), // stack_size is number of 'StackType_t'
                                           parameter,
                                           uxPriority,
                                           &ref,
                                           (StackType_t * const)stack_buf,
                                           NULL);
#endif
    if (ret != pdTRUE) {
        return GS_ERROR_ALLOC;
    }

    if (return_handle) {
        *return_handle = (gs_thread_t) ref;
    }

    return GS_OK;
}

gs_error_t gs_thread_create(const char * const name,
                            gs_thread_func_t func,
                            void * parameter,
                            size_t stack_size,
                            gs_thread_priority_t priority,
                            uint32_t flags,
                            gs_thread_t * return_handle)
{
    return gs_thread_create_with_stack(name, func, parameter, stack_size, NULL,
                                       priority, flags, return_handle);
}

// no return (e.g. GCC __builtin_unreachable), but with a "known" behaviour
static void __attribute__ ((noreturn)) _gs_unreachable(void)
{
    // we should never ever get here .... but since we are here, best thing is reset
    for (;;) {
        gs_thread_sleep_ms(5000);
        gs_sys_reset(GS_SYS_RESET_UNKNOWN);
    }
}

void gs_thread_exit(void * exit_value)
{
    // no where to put it yet - perhaps FreeRTOS will get a join()
    (void) exit_value;

#if (configUSE_NEWLIB_REENTRANT)
    // Newlib: close files to prevent memory leak
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
#endif

    // FreeRTOS
    vTaskDelete(NULL);

    _gs_unreachable();
}

void gs_thread_sleep_ms(uint32_t time_ms)
{
    if (time_ms) {
        const TickType_t ticks = gs_max(time_ms / portTICK_RATE_MS, 1U);
        vTaskDelay(ticks);
    } else {
        taskYIELD();
    }
}
