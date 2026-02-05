/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/a3200/led.h>
#include <gs/a3200/a3200.h>
#include <gs/util/time.h>
#include <gs/util/thread.h>
#include <wdt.h>  // libasf

/**
   Toogle leds on/off every second.
   Touch watchdog.
*/
static void * task_led(void * param)
{
    for (;;) {
        // Touch watchdog to prevent reset.
        // This should be tied into other tasks as well, to ensure everything is running.
        wdt_clear();

        gs_a3200_led_off(GS_A3200_LED_CPU_OK);
        gs_a3200_led_on(GS_A3200_LED_A);
        gs_time_sleep_ms(1000);

        gs_a3200_led_on(GS_A3200_LED_CPU_OK);
        gs_a3200_led_off(GS_A3200_LED_A);
        gs_time_sleep_ms(1000);
    }

    // Will never get here
    gs_thread_exit(NULL);
}

void led_init(void)
{
    // Initialize led (GPO)
    gs_a3200_led_init();

    gs_thread_create("LED", task_led, NULL, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_LOW, 0, NULL);
}
