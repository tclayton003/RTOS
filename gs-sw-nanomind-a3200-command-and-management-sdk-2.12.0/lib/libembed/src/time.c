/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/time.h>
#include <gs/util/thread.h>
#include <gs/util/mutex.h>
#include <gs/util/check.h>
#include <gs/embed/freertos.h>

/**
   Compile check TickType_t is assumed size
  */
GS_CHECK_STATIC_ASSERT((sizeof(TickType_t) == 4), gs_time_tick_t_must_be_32_bits);
GS_CHECK_STATIC_ASSERT(configTICK_RATE_HZ <= 1000, configTICK_RATE_HZ_must_be_less_or_equal_1000);

/*
  Other time functions expects gs_time_rel_ms to wrap a 32 bits. In freeRTOS, xTaskGetTickCount
  returns the value of a 32 bit variable that is incremented on each RTOS tick, therefore this
  expectation is satisfied.
*/
uint32_t gs_time_rel_ms(void)
{
    return (xTaskGetTickCount() * (1000 / configTICK_RATE_HZ));
}

uint32_t gs_time_rel_ms_isr(void)
{
    return (xTaskGetTickCountFromISR() * (1000 / configTICK_RATE_HZ));
}

GS_STATIC uint32_t old_time_ms = 0;
GS_STATIC uint32_t uptime_offset_s = 0;
GS_STATIC gs_mutex_t uptime_lock;
uint32_t gs_time_uptime(void)
{
    gs_error_t res;

    if (!uptime_lock) {
        res = gs_mutex_create(&uptime_lock);
        if (res != GS_OK) {
            return 0;
        }
    }
    res = gs_mutex_lock(uptime_lock);
    if (res != GS_OK) {
        return 0;
    }

    uint32_t time_now_ms = gs_time_rel_ms();
    if (time_now_ms < old_time_ms) {
        uptime_offset_s += UINT32_MAX / 1000;
    }
    old_time_ms = time_now_ms;
    uint32_t uptime = ((time_now_ms / 1000) + uptime_offset_s);

    gs_mutex_unlock(uptime_lock);

    return uptime;
}

void gs_time_sleep_ms(uint32_t timeout_ms)
{
    gs_thread_sleep_ms(timeout_ms);
}
