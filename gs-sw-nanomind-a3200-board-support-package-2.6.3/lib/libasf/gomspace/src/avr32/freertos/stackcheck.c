#include <gs/util/stdio.h>
#include <gs/asf/avr32/asf.h>
#include <FreeRTOS.h>
#include <task.h>
#include <wdt.h>

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
    printf("STACK OVERFLOW!\r\n");
    printf("In task %p name: %s\r\n", pxTask, pcTaskName);
    wdt_clear();
    reset_do_soft_reset();
}
