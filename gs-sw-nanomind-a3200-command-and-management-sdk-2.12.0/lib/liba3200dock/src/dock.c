/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "local.h"
#include <gs/util/string.h>

GS_LOG_GROUP(gs_a3200dock_log, "a3200dock", GS_LOG_CAT_DEFAULT, LOG_DEFAULT_MASK | LOG_INFO_MASK);

static const dock_t * g_dock;

static const dock_t * docks [] = {
    &dmc3_dock,
    &adcs3_dock,
    &adcs6_dock,
    &adcs8_dock,
};

static const dock_t * find_dock_by_name(const char *name)
{
    for (unsigned int i = 0; i < GS_ARRAY_SIZE(docks); i++) {
        if (strcasecmp(name, docks[i]->name) == 0) {
            return docks[i];
        }
    }
    return NULL;
};

gs_error_t gs_a3200dock_init(const char * dock)
{
    gs_log_group_register(gs_a3200dock_log);
    
    g_dock = find_dock_by_name(dock);
    if (g_dock == NULL) {
        if (gs_string_empty(dock)) {
            log_info("No dock configured");
            return GS_OK;
        }

        log_error("Invalid/unknown dock configured: [%s]", dock);
        return GS_ERROR_NOT_SUPPORTED;
    }

    log_debug("Initializing dock: %s ...", g_dock->name);
    if (g_dock->init) {
        gs_error_t error = g_dock->init();
        if (error) {
            log_error("Failed to initialize dock: %s (%d), error: %s", g_dock->name, g_dock->type, gs_error_string(error));
            return error;
        }
    }

    log_info("Initialized dock: %s (%d)", g_dock->name, g_dock->type);
    return GS_OK;
}

gs_a3200dock_type_t gs_a3200dock_get_dock_type(void)
{
    if (g_dock) {
        return g_dock->type;
    }
    return GS_A3200DOCK_NONE;
}

gs_error_t gs_a3200dock_update_adc(int16_t adc[], uint8_t arr_size)
{
    /* Only possible in current version of A3200 to update all channels */
    if (arr_size != 12) {
        return GS_ERROR_NOT_SUPPORTED;
    }

    if (g_dock && g_dock->update_adc) {
        return g_dock->update_adc(adc);
    }

    return GS_ERROR_NOT_SUPPORTED;
}

gs_error_t gs_a3200dock_get_bool(gs_a3200dock_parameter_t param, bool *data)
{
    gs_error_t error = GS_ERROR_NOT_SUPPORTED;
    if (g_dock && g_dock->get_bool) {
        error = g_dock->get_bool(param, data);
    }
    if (error) {
        *data = 0;
    }
    return error;
}

gs_error_t gs_a3200dock_set_bool(gs_a3200dock_parameter_t param, bool data)
{
    if (g_dock && g_dock->set_bool) {
        return g_dock->set_bool(param, data);
    }
    return GS_ERROR_NOT_SUPPORTED;
}

gs_error_t gs_a3200dock_get_uint8(gs_a3200dock_parameter_t param, uint8_t *data)

{
    gs_error_t error = GS_ERROR_NOT_SUPPORTED;
    if (g_dock && g_dock->get_uint8) {
        error = g_dock->get_uint8(param, data);
    }
    if (error) {
        *data = 0;
    }
    return error;
}

gs_error_t gs_a3200dock_set_uint8(gs_a3200dock_parameter_t param, uint8_t data)
{
    if (g_dock && g_dock->set_uint8) {
        return g_dock->set_uint8(param, data);
    }
    return GS_ERROR_NOT_SUPPORTED;
}

gs_error_t gs_a3200dock_get_uint16(gs_a3200dock_parameter_t param, uint16_t *data)
{
    gs_error_t error = GS_ERROR_NOT_SUPPORTED;
    if (g_dock && g_dock->get_uint16) {
        error = g_dock->get_uint16(param, data);
    }
    if (error) {
        *data = 0;
    }
    return error;
}

gs_error_t gs_a3200dock_set_uint16(gs_a3200dock_parameter_t param, uint16_t data)
{
    if (g_dock && g_dock->set_uint16) {
        return g_dock->set_uint16(param, data);
    }
    return GS_ERROR_NOT_SUPPORTED;
}
