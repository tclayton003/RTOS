/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/clock.h>
#include <gs/util/time.h>

uint64_t gs_clock_get_nsec(void)
{
    uint64_t now = gs_time_rel_ms();
    now *= (GS_TIMESTAMP_NSEC_PER_SEC / 1000);
    return now;
}
