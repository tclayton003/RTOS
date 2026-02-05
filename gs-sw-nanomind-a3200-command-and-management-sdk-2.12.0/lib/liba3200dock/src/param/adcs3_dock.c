/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/a3200dock/param/host/adcs3_dock.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_a3200dock_adcs3_dock_table[] = {
    {.name = "gps_en",        .addr = GS_A3200DOCK_ADCS3_DOCK_GPS_EN,                    .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "wde_en",        .addr = GS_A3200DOCK_ADCS3_DOCK_WDE_EN,                    .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "gps_i",         .addr = GS_A3200DOCK_ADCS3_DOCK_GPS_I,                     .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "wde_i",         .addr = GS_A3200DOCK_ADCS3_DOCK_WDE_I,                     .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
};

const gs_a3200dock_adcs3_dock_memory_t gs_a3200dock_adcs3_dock_default_values GS_PGM_OBJECT = {
    .gps_en = true,
    .wde_en = true,
};

