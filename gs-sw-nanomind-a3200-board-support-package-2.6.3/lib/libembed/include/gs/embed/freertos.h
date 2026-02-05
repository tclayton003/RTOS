#ifndef GS_EMBED_FREERTOS_H
#define GS_EMBED_FREERTOS_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   FreeRTOS interface - includes all FreeRTOS headers.
*/

#include <gs/util/types.h>

# include <FreeRTOS.h>
# include <task.h>
# include <semphr.h>
# include <queue.h>

/**
   Context switch state.
   Forward declared in libutil.
*/
struct gs_context_switch {
    /**
       Signal task switch when returning out of the ISR.
    */
    BaseType_t task_woken;
};

/**
   Static initialization of #gs_context_switch structure.
*/ 
#define GS_CONTEXT_SWITCH_INIT {.task_woken = 0}

/**
   Convert timeout to ticks.

   @param[in] timeout_ms timeout in milli-seconds.
   @return converted ticks.
*/
static inline TickType_t gs_freertos_convert_ms_to_ticks(int timeout_ms)
{
    return (timeout_ms < 0) ? portMAX_DELAY : ((TickType_t)(timeout_ms / portTICK_RATE_MS));
}
        
#endif
