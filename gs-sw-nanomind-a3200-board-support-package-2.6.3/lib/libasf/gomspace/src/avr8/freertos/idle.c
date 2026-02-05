#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

/* IDLE hook */
void vApplicationIdleHook(void) {
	void i2c_hw_poll(void) __attribute__((weak));
	if (i2c_hw_poll != NULL)
		i2c_hw_poll();
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}
