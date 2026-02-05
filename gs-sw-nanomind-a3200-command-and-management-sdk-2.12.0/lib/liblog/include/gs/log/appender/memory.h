#ifndef GS_LOG_APPENDER_MEMORY_H
#define GS_LOG_APPENDER_MEMORY_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/log/appender/appender.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Memory Log Appender driver configuration
*/
typedef struct gs_log_appender_memory_config {
    /** Pointer to address */
    uint8_t *address;
    /** Size of the log memory */
    uint32_t size;
} gs_log_appender_memory_config_t;

/**
   Memory Log Appender driver
*/
extern const gs_log_appender_driver_t gs_log_appender_memory_driver;

#ifdef __cplusplus
}
#endif
#endif
