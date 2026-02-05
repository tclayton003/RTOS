/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/a3200/param/host/telemetry.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_a3200_telemetry_table[] = {
    {.name = "fs_mounted",    .addr = GS_A3200_TELEMETRY_FS_MOUNTED,                     .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "ram_image",     .addr = GS_A3200_TELEMETRY_RAM_IMAGE,                      .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "temp_mcu",      .addr = GS_A3200_TELEMETRY_TEMP_MCU,                       .type = GS_PARAM_INT16,   .size =   2, .array_size =  1, .flags = 0},
    {.name = "temp_ram",      .addr = GS_A3200_TELEMETRY_TEMP_RAM,                       .type = GS_PARAM_INT16,   .size =   2, .array_size =  1, .flags = 0},
    {.name = "i_GSSB1",       .addr = GS_A3200_TELEMETRY_I_GSSB1,                        .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "i_GSSB2",       .addr = GS_A3200_TELEMETRY_I_GSSB2,                        .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "i_Flash",       .addr = GS_A3200_TELEMETRY_I_FLASH,                        .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "i_PWM",         .addr = GS_A3200_TELEMETRY_I_PWM,                          .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "resetcause",    .addr = GS_A3200_TELEMETRY_RESETCAUSE,                     .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "bootcause",     .addr = GS_A3200_TELEMETRY_BOOTCAUSE,                      .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "bootcount",     .addr = GS_A3200_TELEMETRY_BOOTCOUNT,                      .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = GS_PARAM_F_AUTO_PERSIST},
    {.name = "clock",         .addr = GS_A3200_TELEMETRY_CLOCK,                          .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "uptime",        .addr = GS_A3200_TELEMETRY_UPTIME,                         .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
};

const gs_a3200_telemetry_memory_t gs_a3200_telemetry_default_values GS_PGM_OBJECT = {
    .uptime = 0,
};

