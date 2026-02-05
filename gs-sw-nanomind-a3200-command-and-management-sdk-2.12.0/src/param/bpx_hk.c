/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/a3200_sdk/param/host/bpx_hk.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_a3200_sdk_bpx_hk_table[] = {
    {.name = "c_charge",      .addr = GS_A3200_SDK_BPX_HK_C_CHARGE,                      .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "c_dcharge",     .addr = GS_A3200_SDK_BPX_HK_C_DCHARGE,                     .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "c_heater",      .addr = GS_A3200_SDK_BPX_HK_C_HEATER,                      .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "vbatt",         .addr = GS_A3200_SDK_BPX_HK_VBATT,                         .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "bat_temp1",     .addr = GS_A3200_SDK_BPX_HK_BAT_TEMP1,                     .type = GS_PARAM_INT16,   .size =   2, .array_size =  1, .flags = 0},
    {.name = "bat_temp2",     .addr = GS_A3200_SDK_BPX_HK_BAT_TEMP2,                     .type = GS_PARAM_INT16,   .size =   2, .array_size =  1, .flags = 0},
    {.name = "bat_temp3",     .addr = GS_A3200_SDK_BPX_HK_BAT_TEMP3,                     .type = GS_PARAM_INT16,   .size =   2, .array_size =  1, .flags = 0},
    {.name = "bat_temp4",     .addr = GS_A3200_SDK_BPX_HK_BAT_TEMP4,                     .type = GS_PARAM_INT16,   .size =   2, .array_size =  1, .flags = 0},
    {.name = "bootcount",     .addr = GS_A3200_SDK_BPX_HK_BOOTCOUNT,                     .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "bootcause",     .addr = GS_A3200_SDK_BPX_HK_BOOTCAUSE,                     .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
};

const gs_a3200_sdk_bpx_hk_memory_t gs_a3200_sdk_bpx_hk_default_values GS_PGM_OBJECT = {
    .c_charge = GS_A3200_SDK_BPX_HK_C_CHARGE_INVALID,
    .c_dcharge = GS_A3200_SDK_BPX_HK_C_DCHARGE_INVALID,
    .c_heater = GS_A3200_SDK_BPX_HK_C_HEATER_INVALID,
    .vbatt = GS_A3200_SDK_BPX_HK_VBATT_INVALID,
    .bat_temp1 = GS_A3200_SDK_BPX_HK_BAT_TEMP1_INVALID,
    .bat_temp2 = GS_A3200_SDK_BPX_HK_BAT_TEMP2_INVALID,
    .bat_temp3 = GS_A3200_SDK_BPX_HK_BAT_TEMP3_INVALID,
    .bat_temp4 = GS_A3200_SDK_BPX_HK_BAT_TEMP4_INVALID,
    .bootcount = GS_A3200_SDK_BPX_HK_BOOTCOUNT_INVALID,
    .bootcause = GS_A3200_SDK_BPX_HK_BOOTCAUSE_INVALID,
};

