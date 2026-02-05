/* Copyright (c) 2013-2020 GomSpace A/S. All rights reserved. */

#include "mission_init.h"

#include <gs/a3200/a3200.h>
#include <gs/a3200/timesync.h>
#include <gs/adcs/adcs_command.h>
#include <gs/csp/port.h>
#include <gs/csp/service_dispatcher.h>
#include <gs/fp/fp.h>
#include <gs/fp/fp_client.h>
#include <gs/ftp/server.h>
#include <gs/gosh/gscript/gscript.h>
#include <gs/gssb/gssb_command.h>
#include <gs/gssb/gssb_service.h>
#include <gs/gssb/rgssb.h>
#include <gs/hk/beacon_service.h>
#include <gs/hk/client.h>
#include <gs/hk/collector.h>
#include <gs/hk/command.h>
#include <gs/hk/config.h>
#include <gs/hk/persister.h>
#include <gs/param/csp_service_handler.h>
#include <gs/rgosh/rgosh.h>
#include <gs/rgosh/rgosh_exec.h>
#include <gs/rgosh/service_handler.h>
#include <gs/util/log.h>
#include <gs/util/thread.h>
#include <gs/util/time.h>

#include "scheduler.h"

// Setup Flight Planner (libfp)
static void mission_fp_init(void)
{
    fp_init();
    fp_server_start(GS_CSP_PORT_FP);
    fp_timer_start();
    gs_fp_register_commands();
}

// Setup House Keeping (libhk)
static void mission_hk_init(void)
{
    gs_hk_config_init();

    /* sleep a grace-period before loading configuration to
     * allow an operator to disable the hk system completely
     * in the event of a critical error in uploaded beacon
     * configurations.
     */
    gs_time_sleep_ms(5000);

    gs_hk_create_config_init_thread();
    gs_hk_create_collector_threads();
    gs_hk_create_beacon_service_thread();
    gs_hk_create_persister_thread();
    gs_hk_srv_register_commands();
    gs_hk_register_commands();
}

// Setup remote gosh/command services
static void mission_rgosh_init(void)
{
    gs_gscript_init();
    gs_gscript_register_commands();
    gs_rgosh_register_commands();

    // Create thread(s) for executing commands
    gs_rgosh_exec_engine_start(1, gs_a3200_get_default_stack_size());

    static const gs_csp_service_handler_t handlers[30] = {
        [GS_CSP_PORT_GSCRIPT] = gs_gscript_csp_service_handler,
        [GS_CSP_PORT_RGOSH] = gs_rgosh_service_handler,
    };

    // Configuration for service handler - must remain valid as long service handler is running
    static const gs_csp_service_dispatcher_conf_t conf = {
        .name = "RGOSH",
        .handler_array = handlers,
        .handler_array_size = GS_ARRAY_SIZE(handlers),
    };

    gs_csp_service_dispatcher_create(&conf, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_NORMAL, NULL);
}

// Setup server task
static void mission_server_init(void)
{
    static const gs_csp_service_handler_t handlers[30] = {
        GS_CSP_BASIC_SERVICE_HANDLERS,
        [GS_CSP_PORT_FTP] = gs_ftp_server_handle_connection,
        [GS_CSP_PORT_RPARAM] = gs_rparam_csp_service_handler,
        [GS_CSP_PORT_GSSB] = gs_rgssb_csp_service_handler,
    };

    // Configuration for service handler - must remain valid as long service handler is running
    static const gs_csp_service_dispatcher_conf_t conf = {
        .name = "A3200SDKSRV",
        .bind_any = true,  // bind to any un-bounded ports, used for logging unexpected connections
        .handler_array = handlers,
        .handler_array_size = GS_ARRAY_SIZE(handlers),
    };

    gs_csp_service_dispatcher_create(&conf, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_NORMAL, NULL);
}

// Basic initialization completed (not file system)
void mission_init(void)
{
    log_notice("Welcome to nanomind, model: %s, revision: %s", csp_get_model(), csp_get_revision());

    // Launch server task for handling CSP service requests
    mission_server_init();

    // Start time synchronization service
    gs_a3200_timesync();

    // Register command "only" clients
    gs_adcs_register_commands();
    extern void cmd_ax100_setup(void);
    cmd_ax100_setup();
    extern void cmd_p60pdu_setup(void);
    cmd_p60pdu_setup();
    extern void cmd_p60dock_setup(void);
    cmd_p60dock_setup();
    extern void cmd_p60pdu_setup(void);
    cmd_p60pdu_setup();
    extern void cmd_p60acu_setup(void);
    cmd_p60acu_setup();
    extern void cmd_eps_setup(void);
    cmd_eps_setup();
    extern void cmd_bpx_setup(void);
    cmd_bpx_setup();
    extern void gatoss_cmd_setup(void);
    gatoss_cmd_setup();
    extern void nanocam_register_commands(void);
    nanocam_register_commands();
    extern void soft_register_commands(void);
    soft_register_commands();
    extern void cmd_power_if_setup(void);
    cmd_power_if_setup();
    gs_rgssb_register_commands();
    gs_gssb_register_commands();

    // Start scheduler
    scheduler_init(10);
}

// File system is now mounted
void mission_init_fs_mounted(void)
{
    // Initialize Flight Planner (requires missionlib)
    mission_fp_init();

    // Initialize House Keeping (requires missionlib)
    mission_hk_init();

    // Initialize gscript/rgosh services
    mission_rgosh_init();
}

/**
   Called by the scheduler task, when re-schedule is needed, e.g. changes to the scheduler table.

   Functions can call scheduler_modify() to schedule or de-schedule functions.

   @note This function MUST only be called by the scheduler task, because the scheduling API isn't thread-safe.
*/
void mission_schedule(void)
{
    extern void adsb_hk_schedule(void);
    adsb_hk_schedule();
    extern void bpx_hk_schedule(void);
    bpx_hk_schedule();
    extern void eps_hk_schedule(void);
    eps_hk_schedule();
}
