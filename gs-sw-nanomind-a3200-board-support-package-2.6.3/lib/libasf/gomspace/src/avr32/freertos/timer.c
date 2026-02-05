#include <gs/util/clock.h>
#include <gs/asf/avr32/asf.h>
#include <FreeRTOS.h>
#include <task.h>
#include <tc.h>

static unsigned long last = 0;
static uint64_t xtime = 0;

/**
 * Inspiration:
 * From: http://sr71.net/~jstultz/tod/ols-presentation-final.pdf
 * timer_interrupt:
 *    now = read_timesource()
 *    delta = cycles_to_ns(now – last, ntp_adj)
 *    xtime += delta
 *    ntp_adj = ntp_advance(delta)
 *    last = now
 * gettimeofday:
 *    now = read_timesource()
 *    delta = cycles_to_ns(now – last, ntp_adj)
 *    return (xtime + delta)
 */

static inline unsigned int port_get_compare(void) {
	return ((sysclk_get_peripheral_bus_hz(&AVR32_TC0) / 32) / configTICK_RATE_HZ);
}

static inline unsigned int port_get_counter(void) {
	return tc_read_tc(&AVR32_TC0, configTICK_TC_CHANNEL);
}

static inline uint32_t counter_to_ns(unsigned long counter) {
	return (counter * (1000000000.0 / configTICK_RATE_HZ)) / port_get_compare();
}

void vApplicationTickHook(void) {
	unsigned long now = port_get_counter();
	unsigned long increment = counter_to_ns((now - last) + port_get_compare());
	xtime += increment;
	last = now;
}

uint64_t gs_clock_get_nsec(void) {

	portENTER_CRITICAL();
	unsigned long now = port_get_counter();
	unsigned long last_copy = last;
	uint64_t xtime_copy = xtime;
	portEXIT_CRITICAL();

	if (now < last_copy)
		now += port_get_compare();

	return xtime_copy + counter_to_ns(now - last_copy);

}

/* This function is used by FreeRTOS process-list */
int vGetRuntimeCounterValue(void) {
	timestamp_t clock;
	gs_clock_get_monotonic(&clock);
	return clock.tv_sec * 100 + (clock.tv_nsec / 10000000) % 100;
}
