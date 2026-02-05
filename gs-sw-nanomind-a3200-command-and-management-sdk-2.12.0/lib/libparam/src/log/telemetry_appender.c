/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/log/log.h>
#include <gs/util/time.h>
#include <gs/util/clock.h>
#include <gs/util/log/appender/appender.h>
#include <gs/param/table.h>
#include <gs/param/host_table.h>
#include <gs/param/log/appender/telemetry.h>

#include <string.h>
#include <stdlib.h>

typedef enum {
    TELEM_TYPE_ERR = 0,
    TELEM_TYPE_WARN = 1,
    TELEM_TYPE_ERR_ISR = 2,
    TELEM_TYPE_WARN_ISR = 3,
} telemetry_type_t;

typedef struct telemetry_data_set {
    uint32_t mask;
    uint32_t count;
} telemetry_data_set_t;

typedef struct telemetry_drv_data {
    gs_mutex_t lock;
    gs_param_table_instance_t *tinst;
    telemetry_data_set_t data[4];
    bool dirty;
} telemetry_drv_data_t;

static gs_error_t gs_log_appender_telem_init(gs_log_appender_t *appender)
{
    const gs_log_appender_telemetry_drv_config_t *config = appender->drv_config;

    if (config == NULL || config->table_name == NULL) {
        return GS_ERROR_ARG;
    }

    telemetry_drv_data_t *drv_data = appender->drv_data;
    if (drv_data == NULL) {
        drv_data = calloc(1, sizeof(*drv_data));
        if (drv_data == NULL) {
            return GS_ERROR_ALLOC;
        }
    }

    drv_data->tinst = gs_param_table_by_name(config->table_name);
    if (drv_data->tinst == NULL) {
        free(drv_data);
        return GS_ERROR_NOT_FOUND;
    }
    gs_mutex_create(&drv_data->lock);

    appender->drv_data = drv_data;
    return GS_OK;
}

static gs_error_t gs_log_appender_telem_flush(gs_log_appender_t *appender)
{
    if ((appender == NULL) || (appender->drv_data == NULL) || (appender->drv_config == NULL)) {
        return GS_OK;
    }

    telemetry_drv_data_t *drv_data = appender->drv_data;
    const gs_log_appender_telemetry_drv_config_t *config = appender->drv_config;

    if (drv_data->dirty) {
        uint32_t value;
        gs_mutex_lock(drv_data->lock);
        value = drv_data->data[TELEM_TYPE_ERR].mask | drv_data->data[TELEM_TYPE_ERR_ISR].mask;
        gs_param_set_uint32(drv_data->tinst, config->param_error_addr, value, 0);
        value = drv_data->data[TELEM_TYPE_ERR].count + drv_data->data[TELEM_TYPE_ERR_ISR].count;
        gs_param_set_uint32(drv_data->tinst, config->param_error_cnt_addr, value, 0);
        value = drv_data->data[TELEM_TYPE_WARN].mask | drv_data->data[TELEM_TYPE_WARN_ISR].mask;
        gs_param_set_uint32(drv_data->tinst, config->param_warning_addr, value, 0);
        value = drv_data->data[TELEM_TYPE_WARN].count + drv_data->data[TELEM_TYPE_WARN_ISR].count;
        gs_param_set_uint32(drv_data->tinst, config->param_warning_cnt_addr, value, 0);
        drv_data->dirty = false;
        gs_mutex_unlock(drv_data->lock);
    }

    return GS_OK;
}

static inline void gs_log_appender_telem_append_common(telemetry_type_t type, telemetry_drv_data_t *drv_data, const gs_log_group_t *group)
{
    drv_data->data[type].mask |= group->category;
    drv_data->data[type].count++;
    drv_data->dirty = true;
}

static void gs_log_appender_telem_append(gs_log_appender_t *appender, gs_log_level_t level, const gs_log_group_t *group, const gs_timestamp_t * ts, const char * format, va_list va)
{
    if (level < LOG_WARNING)
        return;

    telemetry_drv_data_t *drv_data = appender->drv_data;
    if (drv_data != NULL)
    {
        gs_mutex_lock(drv_data->lock);
        gs_log_appender_telem_append_common((level == LOG_ERROR) ? TELEM_TYPE_ERR : TELEM_TYPE_WARN, drv_data, group);
        gs_mutex_unlock(drv_data->lock);
    }

    gs_log_appender_telem_flush(appender);
}

static void gs_log_appender_telem_append_isr(gs_log_appender_t *appender, gs_log_level_t level, const gs_log_group_t *group, const gs_timestamp_t * ts, const char * format, va_list va)
{
    if (level < LOG_WARNING)
        return;

    telemetry_drv_data_t *drv_data = appender->drv_data;
    if (drv_data != NULL)
    {
        gs_log_appender_telem_append_common((level == LOG_ERROR) ? TELEM_TYPE_ERR_ISR : TELEM_TYPE_WARN_ISR, drv_data, group);
    }
}

static gs_error_t gs_log_appender_telem_info(gs_log_appender_t *appender, char * info_str, uint8_t str_size)
{
    const gs_log_appender_telemetry_drv_config_t *config = appender->drv_config;

    if (!info_str) {
        return GS_ERROR_ARG;
    }

    snprintf(info_str, str_size, "Logs to parameter system. Table=<%s>", config->table_name);

    return GS_OK;
}

static gs_error_t gs_log_appender_telem_hist(gs_log_appender_t *appender, void * ctx, gs_log_record_iterator_t iter)
{
    telemetry_drv_data_t *drv_data = appender->drv_data;

    if (drv_data == NULL) {
        return GS_ERROR_ARG;
    }

    gs_timestamp_t ts;
    gs_clock_get_time(&ts);

    char log_str[128];
    /* Error hist */
    uint32_t mask = drv_data->data[TELEM_TYPE_ERR].mask | drv_data->data[TELEM_TYPE_ERR_ISR].mask;
    uint32_t count = drv_data->data[TELEM_TYPE_ERR].count + drv_data->data[TELEM_TYPE_ERR_ISR].count;
    snprintf(log_str, sizeof(log_str), "Error Mask=<0x%08"PRIx32">, Count=<%"PRIu32">", mask, count);
    iter(ctx, LOG_ERROR, &ts, "telemetry", log_str);

    /* Warn hist */
    mask = drv_data->data[TELEM_TYPE_WARN].mask | drv_data->data[TELEM_TYPE_WARN_ISR].mask;
    count = drv_data->data[TELEM_TYPE_WARN].count + drv_data->data[TELEM_TYPE_WARN_ISR].count;
    snprintf(log_str, sizeof(log_str), "Warning Mask=<0x%08"PRIx32">, Count=<%"PRIu32">", mask, count);
    iter(ctx, LOG_WARNING, &ts, "telemetry", log_str);

    return GS_OK;
}

static gs_error_t gs_log_appender_telem_clear(gs_log_appender_t *appender)
{
    telemetry_drv_data_t *drv_data = appender->drv_data;

    if (drv_data == NULL) {
        return GS_ERROR_ARG;
    }

    gs_mutex_lock(drv_data->lock);
    memset(drv_data->data, 0, sizeof(drv_data->data));
    drv_data->dirty = true;
    gs_mutex_unlock(drv_data->lock);

    gs_log_appender_telem_flush(appender);

    return GS_OK;
}

const gs_log_appender_driver_t gs_log_appender_telemetry_drv = {
    .init = gs_log_appender_telem_init,
    .append = gs_log_appender_telem_append,
    .append_isr = gs_log_appender_telem_append_isr,
    .info = gs_log_appender_telem_info,
    .hist = gs_log_appender_telem_hist,
    .clear = gs_log_appender_telem_clear,
    .flush = gs_log_appender_telem_flush,
};
