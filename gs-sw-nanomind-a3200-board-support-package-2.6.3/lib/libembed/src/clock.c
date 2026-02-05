/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/clock.h>
#include <gs/util/rtc.h>

/* Local time offset */
static timestamp_t utc_offset;

void gs_clock_get_time(timestamp_t * time)
{
    /* Get system clock */
    gs_clock_get_monotonic(time);

    if (utc_offset.tv_sec == 0 && utc_offset.tv_nsec == 0) {
        if (gs_rtc_supported() == GS_OK) {
            /* If no utc_offset has been set, try to get it from RTC */
            timestamp_t rtc_time;
            gs_error_t error = gs_rtc_get_time(&rtc_time);
            if (error == GS_OK) {
                /* Offset = RTC - TickTime */
                timestamp_diff(&rtc_time, time);
                timestamp_copy(&rtc_time, &utc_offset);
            }
        }
    }

    /* Add offset */
    timestamp_add(time, &utc_offset);
}

gs_error_t gs_clock_set_time(const timestamp_t * utc_time)
{
    if (!utc_time) {
        return GS_ERROR_ARG;
    }

    gs_error_t error = GS_OK;
    if (gs_rtc_supported() == GS_OK) {
        /* We also store a backup of the UTC time in the RTC, but this is low precision
         * So it's only used once at bootup to give an initial time in case no
         * network time sync node is present. */
	error = gs_rtc_set_time(utc_time);
    }

    timestamp_t tick;
    gs_clock_get_monotonic(&tick);

    // calculate new utc offset
    timestamp_t tmp_offset = *utc_time;
    timestamp_diff(&tmp_offset, &tick);
    timestamp_copy(&tmp_offset, &utc_offset);

    return error;
}

void gs_clock_get_monotonic(timestamp_t * time)
{
    uint64_t nsec = gs_clock_get_nsec();

    /* Return current tick-timer value */
    time->tv_sec = nsec / GS_TIMESTAMP_NSEC_PER_SEC;
    time->tv_nsec = nsec % GS_TIMESTAMP_NSEC_PER_SEC;
}
