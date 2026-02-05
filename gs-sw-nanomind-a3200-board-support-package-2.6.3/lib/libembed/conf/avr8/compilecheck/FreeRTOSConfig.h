/**
 * NanoPower firmware
 *
 */

#ifndef GOMSPACE_CONFIG_AVR8_H_
#define GOMSPACE_CONFIG_AVR8_H_

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *----------------------------------------------------------*/

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

#define configCPU_CLOCK_HZ				8000000
#define configTICK_RATE_HZ				100
#define configMAX_PRIORITIES				5 // TODO changed from 3 //possible task priorities (high number uses more RAM)
#define configMAX_TASK_NAME_LEN				8 //Max chars as name
#define configIDLE_SHOULD_YIELD				1 //0 = If we have tasks that share IDLE priority, they will share timeslices with idle_task.
#define configQUEUE_REGISTRY_SIZE			0 //Allows a textual name to be associated with a queue for easy queue identification within a debugging GUI.
#define configUSE_TRACE_FACILITY			1 //debug facility
#define configUSE_16_BIT_TICKS				0 //Max 65 seconds delay on tasks. If set to 0 a 32 bit value is used, but decreases performance.
#define configUSE_MUTEXES				1
#define configUSE_PREEMPTION				1
#define configUSE_IDLE_HOOK				1
#define configUSE_TICK_HOOK				1
#define configUSE_MALLOC_FAILED_HOOK 		        1
#define configCHECK_FOR_STACK_OVERFLOW		        2
#define configENABLE_BACKWARD_COMPATIBILITY             1
#define configMINIMAL_STACK_SIZE                        250
#define configUSE_TRACE_FACILITY			1
#define configUSE_STATS_FORMATTING_FUNCTIONS 	        1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 				0

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet			0
#define INCLUDE_uxTaskPriorityGet			0
#define INCLUDE_vTaskDelete				0
#define INCLUDE_vTaskCleanUpResources		        0
#define INCLUDE_vTaskSuspend				1
#define INCLUDE_vTaskDelayUntil				1
#define INCLUDE_vTaskDelay				1
#define INCLUDE_uxTaskGetStackHighWaterMark             0
#define INCLUDE_xTaskGetCurrentTaskHandle 	        1
#define INCLUDE_vTaskList				1

/* Run-time statistics */
#define configGENERATE_RUN_TIME_STATS 		        1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portLU_PRINTF_SPECIFIER_REQUIRED 	        1

unsigned long xGetRuntimeCounterValue(void);
#define portGET_RUN_TIME_COUNTER_VALUE()                xGetRuntimeCounterValue()

/* Port config */
#define portUSE_TIMER3					1
#define portLU_PRINTF_SPECIFIER_REQUIRED	        1

#define portPOINTER_SIZE_TYPE	                        uintptr_t

#endif /* GOMSPACE_CONFIG_AVR8_H_ */
