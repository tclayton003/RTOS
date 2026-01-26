// The first 3 includes would be files in our project that we get from different places
#include "FreeRTOS.h"				// FreeRTOS functions
#include "task.h"						// Defines task creation functions
#include "nanomind_bsp.h"   // BSP functions
#include <stdio.h>          // Only for PC simulation / debug



// Task prototypes so that we can define the functions below main()
void CommandTask(void *pvParameters);
void TelemetryTask(void *pvParameters);



// Actual entry point
int main(void)
{
    // Initialize hardware via BSP
    bsp_init();  // Sets clocks, peripherals, UART, interrupts, watchdog

    // Create application tasks (Turn our defined tasks into stuff that FreeRTOS can manage)
    xTaskCreate(CommandTask, "CMD", 256, NULL, 2, NULL);
    xTaskCreate(TelemetryTask, "TEL", 256, NULL, 1, NULL);

    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    // If scheduler returns, something went wrong
    while (1)
    {
        // Error trap
    }

    return 0; // Never reached
}



// Task implementations (May be done in a separate file and imported for readability???)
void CommandTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        // Process satellite commands
        bsp_uart_send("CommandTask running\n"); // Example BSP API
        vTaskDelay(pdMS_TO_TICKS(1000));       // Delay 1 second
    }
}

void TelemetryTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        // Send telemetry data
        bsp_uart_send("TelemetryTask running\n"); // Example BSP API
        vTaskDelay(pdMS_TO_TICKS(2000));         // Delay 2 seconds
    }
}

