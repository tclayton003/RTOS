/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/timesync.h>
#include "../local.h"
#include <csp/csp.h>
#include <csp/csp_cmp.h>
#include <csp/csp_endian.h>
#include <gs/util/thread.h>

static int do_csp_timesync(uint8_t tsync_node, uint32_t timeout)
{
    struct csp_cmp_message msg;
    memset(&msg, 0, sizeof(msg));
    if (csp_cmp_clock(tsync_node, timeout, &msg) == CSP_ERR_NONE) {
        csp_timestamp_t now;
        now.tv_sec = csp_ntoh32(msg.clock.tv_sec);
        now.tv_nsec = csp_ntoh32(msg.clock.tv_nsec);
        if (now.tv_sec > 0) {
            /* Set clock and ticks */
            csp_clock_set_time(&now);
            return 1;
        }
    }

    return 0;
}

static void * timesync_task(void * pvParameters)
{
    csp_timestamp_t now;
    TickType_t start = 0;
    while (1) {
        vTaskDelayUntil(&start, 1000);

        /* Get current time */
        csp_clock_get_time(&now);

        /* Time sync if enabled */

        uint8_t tsync_node = GS_A3200_CONFIGURATION_TSYNC_NODE_GET_NC();
        uint16_t tsync_intv = GS_A3200_CONFIGURATION_TSYNC_INTV_GET_NC();
        if ((tsync_node > 0) && (tsync_intv > 0)) {
            if ((now.tv_sec % tsync_intv) == 0) {
                do_csp_timesync(tsync_node, 1000);
            }
        }
    }

    gs_thread_exit(NULL);
}

gs_error_t gs_a3200_timesync(void)
{
    return gs_thread_create("TSYNC", timesync_task, NULL, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_LOW, 0, NULL);
}
