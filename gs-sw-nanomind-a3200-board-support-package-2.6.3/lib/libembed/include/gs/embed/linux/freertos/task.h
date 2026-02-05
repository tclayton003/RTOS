#ifndef GS_EMBED_LINUX_FREERTOS_TASK_H
#define GS_EMBED_LINUX_FREERTOS_TASK_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   FreeRTOS task support on Linux.
*/

#if !(__DOXYGEN__)

#include <gs/util/thread.h>
#include <gs/util/time.h>
#include <gs/util/clock.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Type definitions. */
typedef int BaseType_t;
typedef unsigned int UBaseType_t;

typedef UBaseType_t TickType_t;
typedef UBaseType_t StackType_t;

typedef gs_thread_t TaskHandle_t;
typedef void (*TaskFunction_t)(void * param);

#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  StackType_t
#define portBASE_TYPE   BaseType_t
#define portTickType    TickType_t

#define pdTRUE   true
#define pdFALSE  false
#define pdPASS   pdTRUE

typedef enum
{
    eRunning = 0,	//!< A task is querying the state of itself, so must be running.
    eReady,		//!< The task being queried is in a read or pending ready list.
    eBlocked,		//!< The task being queried is in the Blocked state.
    eSuspended,		//!< The task being queried is in the Suspended state, or is in the Blocked state with an infinite time out.
    eDeleted		//!< The task being queried has been deleted, but its TCB has not yet been freed.
} eTaskState;

/**
   Copy of FreeRTOS task for use on Linux.
   Primarily used for compiling FreeRTOS code on Linux.
*/
typedef struct xTASK_STATUS
{
    TaskHandle_t xHandle;		//!< The handle of the task to which the rest of the information in the structure relates.
    const char *pcTaskName;		//!< A pointer to the task's name.  This value will be invalid if the task was deleted since the structure was populated! */ /*lint !e971 Unqualified char types are allowed for strings and single characters only.
    UBaseType_t xTaskNumber;		//!< A number unique to the task.
    eTaskState eCurrentState;	        //!< The state in which the task existed when the structure was populated.
    UBaseType_t uxCurrentPriority;	//!< The priority at which the task was running (may be inherited) when the structure was populated.
    UBaseType_t uxBasePriority;		//!< The priority to which the task will return if the task's current priority has been inherited to avoid unbounded priority inversion when obtaining a mutex.  Only valid if configUSE_MUTEXES is defined as 1 in FreeRTOSConfig.h.
    uint32_t ulRunTimeCounter;	        //!< The total run time allocated to the task so far, as defined by the run time stats clock.  See http://www.freertos.org/rtos-run-time-stats.html.  Only valid when configGENERATE_RUN_TIME_STATS is defined as 1 in FreeRTOSConfig.h.
    uint16_t usStackHighWaterMark;	//!< The minimum amount of stack space that has remained for the task since the task was created.  The closer this value is to zero the closer the task has come to overflowing its stack.
} TaskStatus_t;
    
#define DISABLE_ALL_INTERRUPTS()

static inline void vTaskDelay( const TickType_t xTicksToDelay )
{
    gs_time_sleep_ms(xTicksToDelay);
}

static inline TickType_t xTaskGetTickCount( void )
{
    return (gs_clock_get_nsec() / (1000 * 1000));
}

TickType_t xTaskGetTickCountFromISR( void );

static inline void vTaskDelayUntil( TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement )
{
    TickType_t xTimeToWake = *pxPreviousWakeTime + xTimeIncrement;  // this will wrap
    const TickType_t xConstTickCount = xTaskGetTickCount();
    if (xTimeToWake > xConstTickCount)
    {
        gs_time_sleep_ms(xTimeToWake - xConstTickCount);
    }
    /* Update the wake time ready for the next call. */
    *pxPreviousWakeTime = xTimeToWake;
}

static inline BaseType_t xTaskCreate( TaskFunction_t pxTaskCode, const char * const pcName, const uint16_t usStackDepth, void * const pvParameters, UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask)
{
    union {
        TaskFunction_t task;
        gs_thread_func_t thread;
    } task = { .task = pxTaskCode};
    return (gs_thread_create(pcName, task.thread, pvParameters,
                             0, GS_THREAD_PRIORITY_NORMAL, 0, pxCreatedTask) == GS_OK) ? pdTRUE : pdFALSE;
}

static inline void vTaskDelete( void * dummy ) // FreeRTOS definition: void vTaskDelete( TaskHandle_t xTaskToDelete );
{
    gs_thread_exit(0);
}

static inline void vTaskStartScheduler( void )
{
    // nothing to do on linux - xTaskCreate -> gs_thread_create() launches thread
    for (;;) {
        gs_time_sleep_ms(10000);
    }
}

static inline void taskYIELD( void )
{
    gs_time_sleep_ms(0);
}

static inline void * pvPortMalloc(size_t size)
{
    return malloc(size);
}

static inline void vPortFree(void * mem)
{
    free(mem);
}

static inline UBaseType_t uxTaskGetNumberOfTasks( void )
{
    return 0; // perhaps count /proc/1746/task/*
}

static inline UBaseType_t uxTaskGetSystemState( TaskStatus_t * const pxTaskStatusArray, const UBaseType_t uxArraySize,
                                                uint32_t * const pulTotalRunTime )
{
    return pdFALSE;
}
    
#ifdef __cplusplus
}
#endif
#endif // __DOXYGEN__
#endif
