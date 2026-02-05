#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
 * FreeRTOS configuration file for liba3200 based projects.
 */

#include <assert.h>

#define configCPU_CLOCK_HZ                    32000000
#define configPBA_CLOCK_HZ                    32000000
#define configASSERT                          assert
#define configUSE_PREEMPTION                  1
#define configUSE_IDLE_HOOK                   1
#define configUSE_TICK_HOOK                   1
#define configTICK_RATE_HZ                    1000
#define configMAX_PRIORITIES                  5
#define configMINIMAL_STACK_SIZE              400 // words - idle task size
#define configMAX_TASK_NAME_LEN               16
#define configUSE_TRACE_FACILITY              1
#define configUSE_16_BIT_TICKS                0
#define configIDLE_SHOULD_YIELD               1
#define configQUEUE_REGISTRY_SIZE             0
#define configUSE_STATS_FORMATTING_FUNCTIONS  1
#define configENABLE_BACKWARD_COMPATIBILITY   1

#define configUSE_COUNTING_SEMAPHORES         1
#define configUSE_MUTEXES                     1
#define configUSE_RECURSIVE_MUTEXES           1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                 0
#define configMAX_CO_ROUTINE_PRIORITIES       0

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet              1
#define INCLUDE_uxTaskPriorityGet             1
#define INCLUDE_vTaskDelete                   1
#define INCLUDE_vTaskCleanUpResources         0
#define INCLUDE_vTaskSuspend                  1
#define INCLUDE_vTaskDelayUntil               1
#define INCLUDE_vTaskDelay                    1
#define INCLUDE_xTaskGetCurrentTaskHandle     1
#define INCLUDE_xTaskGetSchedulerState        1
#define INCLUDE_pcTaskGetTaskName             1

/* configTICK_USE_TC is a boolean indicating whether to use a Timer Counter or
   the CPU Cycle Counter for the tick generation.
   Both methods will generate an accurate tick.
   0: Use of the CPU Cycle Counter.
   1: Use of the Timer Counter (configTICK_TC_CHANNEL is the TC channel). */
#define configTICK_USE_TC                     1
#define configTICK_TC_CHANNEL                 2

#define configGENERATE_RUN_TIME_STATS         0
extern int vGetRuntimeCounterValue(void);
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE()	vGetRuntimeCounterValue()

/* configHEAP_INIT is a boolean indicating whether to initialize the heap with
   0xA5 in order to be able to determine the maximal heap consumption. */
#define configHEAP_INIT                       0
#define configCHECK_FOR_STACK_OVERFLOW        2

#define configUSE_NEWLIB_REENTRANT            1

#endif /* FREERTOS_CONFIG_H */
