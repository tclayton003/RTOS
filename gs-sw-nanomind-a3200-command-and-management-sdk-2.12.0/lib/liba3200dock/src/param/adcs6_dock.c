/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/a3200dock/param/host/adcs6_dock.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_a3200dock_adcs6_dock_table[] = {
    {.name = "gps_en",        .addr = GS_A3200DOCK_ADCS6_DOCK_GPS_EN,                    .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "rw0_en",        .addr = GS_A3200DOCK_ADCS6_DOCK_RW0_EN,                    .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "rw1_en",        .addr = GS_A3200DOCK_ADCS6_DOCK_RW1_EN,                    .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "rw2_en",        .addr = GS_A3200DOCK_ADCS6_DOCK_RW2_EN,                    .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "rw3_en",        .addr = GS_A3200DOCK_ADCS6_DOCK_RW3_EN,                    .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "gssb_c_en",     .addr = GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN,                 .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "gssb_d_en",     .addr = GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN,                 .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "gps_i",         .addr = GS_A3200DOCK_ADCS6_DOCK_GPS_I,                     .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "gssb_c_i",      .addr = GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I,                  .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "gssb_d_i",      .addr = GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I,                  .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
};

const gs_a3200dock_adcs6_dock_memory_t gs_a3200dock_adcs6_dock_default_values GS_PGM_OBJECT = {
    .gps_en = false,
    .rw0_en = true,
    .rw1_en = true,
    .rw2_en = true,
    .rw3_en = true,
    .gssb_c_en = true,
    .gssb_d_en = true,
};

