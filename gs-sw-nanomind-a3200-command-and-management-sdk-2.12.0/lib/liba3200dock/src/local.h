/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/a3200dock/dock.h>
#include <gs/a3200dock/param.h>
#include <gs/a3200/param/names.h>
#include <gs/util/log.h>

// All docks uses the same table id.
#define GS_A3200_DOCK_TABLE_ID  5

GS_LOG_GROUP_EXTERN(gs_a3200dock_log);
#define LOG_DEFAULT gs_a3200dock_log

// Struct defining name, type and interface for a dock.
typedef struct {
    const char * const name;
    gs_a3200dock_type_t type;

    gs_error_t (*init)(void);
    gs_error_t (*update_adc)(int16_t adc[]);

    gs_error_t (*get_uint8)(gs_a3200dock_parameter_t param, uint8_t *data);
    gs_error_t (*set_uint8)(gs_a3200dock_parameter_t param, uint8_t data);
    gs_error_t (*get_bool)(gs_a3200dock_parameter_t param, bool *data);
    gs_error_t (*set_bool)(gs_a3200dock_parameter_t param, bool data);
    gs_error_t (*get_uint16)(gs_a3200dock_parameter_t param, uint16_t *data);
    gs_error_t (*set_uint16)(gs_a3200dock_parameter_t param, uint16_t data);
} dock_t;

extern const dock_t dmc3_dock;
extern const dock_t adcs3_dock;
extern const dock_t adcs6_dock;
extern const dock_t adcs8_dock;
