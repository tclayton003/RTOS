// !! ONLY FOR COMPILE CHECK - NOT FOR PRODUCTION

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION		1
#define configUSE_IDLE_HOOK		0
#define configUSE_TICK_HOOK		0
#define configCPU_CLOCK_HZ		( 48000000 )
#define configTICK_RATE_HZ		( 500 )
#define configMAX_PRIORITIES		5
#define configMINIMAL_STACK_SIZE	80
#define configMAX_TASK_NAME_LEN		5
#define configUSE_TRACE_FACILITY	1
#define configUSE_16_BIT_TICKS		0
#define configIDLE_SHOULD_YIELD		1
#define configUSE_MUTEXES		1
#define configQUEUE_REGISTRY_SIZE	8
#define configCHECK_FOR_STACK_OVERFLOW	2
#define configUSE_RECURSIVE_MUTEXES	1
//#define configUSE_MALLOC_FAILED_HOOK	1
#define configUSE_APPLICATION_TASK_TAG	0
#define configUSE_COUNTING_SEMAPHORES	1
#define configUSE_QUEUE_SETS		1

/* Run time stats related definitions. */
#define configGENERATE_RUN_TIME_STATS	     0
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configUSE_TRACE_FACILITY             1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                0
#define configMAX_CO_ROUTINE_PRIORITIES      ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS		     0
#define configTIMER_TASK_PRIORITY	     4
#define configTIMER_QUEUE_LENGTH	     5
#define configTIMER_TASK_STACK_DEPTH	     ( 80 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete			1
#define INCLUDE_vTaskCleanUpResources	        1
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay			1
#define INCLUDE_eTaskGetState			1

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names - or at least those used in the unmodified vector table. */
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* The size of the global output buffer that is available for use when there
are multiple command interpreters running at once (for example, one on a UART
and one on TCP/IP).  This is done to prevent an output buffer being defined by
each implementation - which would waste RAM.  In this case, there is only one
command interpreter running. */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE 2048

#endif /* FREERTOS_CONFIG_H */

