/* Copyright (c) 2013-2020 GomSpace A/S. All rights reserved. */

#include "scheduler.h"
#include "mission_init.h"
#include "param/a3200_sdk_store.h"
#include <gs/a3200_sdk/param/host/scheduler.h>
#include <gs/a3200/a3200.h>
#include <gs/util/sem.h>
#include <gs/util/thread.h>

static gs_function_scheduler_t * scheduler = NULL;
static gs_sem_t scheduler_sem = NULL;

GS_LOG_GROUP(scheduler_log, "scheduler", GS_LOG_CAT_DEFAULT, GS_LOG_DEFAULT_MASK | GS_LOG_INFO_MASK);
#define LOG_DEFAULT scheduler_log

// Called when the parameter table 'scheduler' is modifed - this will trigger the scheduler to re-schedule.
static void scheduler_table_changed(uint16_t addr, gs_param_table_instance_t *tinst)
{
    scheduler_reload();
}

// Scheduler task
static void* scheduler_task(void* parameters)
{
    int timeout_ms = -1;
    bool scheduling_enabled = false;
    for (;;) {

        gs_error_t error = gs_sem_wait(scheduler_sem, timeout_ms);
        if (error == GS_OK) {
            // Someone signaled (table has most likely changed), re-schedule

            // Clear pending reload events
            while (gs_sem_wait(scheduler_sem, 0) == GS_OK);

            const uint16_t delayed_start_seconds = GS_A3200_SDK_SCHEDULER_DELAYED_START_GET_NC();
            scheduling_enabled = GS_A3200_SDK_SCHEDULER_ENABLED_GET_NC();
            timeout_ms = scheduling_enabled ? (delayed_start_seconds * 1000) : -1;
            log_debug("changed scheduling to %s, delayed start: %d mS", scheduling_enabled ? "enabled" : "disabled", timeout_ms);

            if (scheduling_enabled) {
                // (re)schedule all functions
                mission_schedule();
            }

            continue;
        }

        if (scheduling_enabled) {
            timeout_ms = gs_function_scheduler_execute_ms(scheduler);
        } else {
            timeout_ms = -1;
        }
    }

    gs_thread_exit(NULL);
}

// Initialize scheduler, create/load scheduler table and launch a task for executing scheduled functions.
gs_error_t scheduler_init(unsigned int max_entries)
{
    gs_log_group_register(LOG_DEFAULT);
    
    const gs_param_table_create_t create = {.name = GS_A3200_SDK_SCHEDULER_TABLE_NAME,
                                            .id = GS_A3200_SDK_SCHEDULER_TABLE_ID,
                                            .rows = gs_a3200_sdk_scheduler_table,
                                            .row_count = GS_A3200_SDK_SCHEDULER_TABLE_ROWS,
                                            .memory_size = GS_A3200_SDK_SCHEDULER_TABLE_SIZE,
                                            .default_values = &gs_a3200_sdk_scheduler_default_values,
                                            .callback = scheduler_table_changed,
                                            .stores = GS_A3200_SDK_PARAM_STORE "," GS_A3200_SDK_PARAM_STORE_PROTECTED,
                                            .flags = GS_PARAM_TABLE_F_ALLOC_MEMORY};
    gs_param_table_instance_t* tinst = NULL;
    gs_error_t error = gs_param_table_create(&create, &tinst);
    if (error) {
        log_error("gs_param_table_create() failed, scheduler not created!");
        return error;
    }
    gs_param_table_load_fallback(tinst, GS_PARAM_F_NO_CALLBACK);

    gs_sem_create(1, &scheduler_sem);

    gs_function_scheduler_create(10000, max_entries, &scheduler);

    return gs_thread_create("SCHEDULER", scheduler_task, NULL, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_LOW, 0, NULL);
}

gs_error_t scheduler_modify(const char * name, uint32_t interval_ms, gs_function_scheduler_function_t func, void * user_data)
{
    return gs_function_scheduler_modify_by_name(scheduler, name, interval_ms, func, user_data);
}

void scheduler_reload(void)
{
    if (scheduler_sem) {
        gs_sem_post(scheduler_sem);
    }
}
