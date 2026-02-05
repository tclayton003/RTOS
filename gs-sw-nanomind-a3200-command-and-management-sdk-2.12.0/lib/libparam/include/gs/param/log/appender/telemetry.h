#ifndef GS_PARAM_LOG_APPENDER_STATUS_H
#define GS_PARAM_LOG_APPENDER_STATUS_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/log/appender/appender.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Telemetry Log Appender driver configuration
*/
typedef struct gs_log_appender_telemetry_drv_config {
    /** Name of the param table where log stats should be saved */
    const char *table_name;
    /** Row address of the log error bitmask (since boot) */
    uint16_t param_error_addr;
    /** Row address of the log error total count (since boot) */
    uint16_t param_error_cnt_addr;
    /** Row address of the log warning bitmask (since boot) */
    uint16_t param_warning_addr;
    /** Row address of the log warning total count (since boot) */
    uint16_t param_warning_cnt_addr;
} gs_log_appender_telemetry_drv_config_t;

/**
   Telemetry Log appender driver

   This log appender gathers error & warning statistics about the log events on the system.
   The appender should be registered to the root group, in order to ensure that all
   error and warning logs are captured.
*/
extern const gs_log_appender_driver_t gs_log_appender_telemetry_drv;

#ifdef __cplusplus
}
#endif
#endif
