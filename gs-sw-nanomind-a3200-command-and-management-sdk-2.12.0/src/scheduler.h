/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/util/function_scheduler.h>
#include <gs/util/log/log.h>
#include <gs/a3200_sdk/param/host/scheduler.h>
#include <gs/param/host_table.h>

/**
   This log group is used by the scheduler task and can also be used by scheduled functions.
*/
GS_LOG_GROUP_EXTERN(scheduler_log);

/**
   Default timeout for getting house-keeping data
*/
#define GS_CSP_HK_TIMEOUT             300

/**
   Initialize scheduler.
*/
gs_error_t scheduler_init(unsigned int max_entries);

/**
   Update (register or remove) function to be executed at intervals.

   @note This function should only be called from the scheduler task.

   @see gs_function_scheduler_modify_by_name() for details.
*/
gs_error_t scheduler_modify(const char * name, uint32_t interval_ms, gs_function_scheduler_function_t func, void * user_data);

/**
   Trigger re-scheduling.
*/
void scheduler_reload(void);

/**
   Function called from the scheduler task to re-schedule functions.

   @note This function MUST only be called by the scheduler_task, because the scheduling API isn't thread-safe.
*/
void mission_schedule(void);
