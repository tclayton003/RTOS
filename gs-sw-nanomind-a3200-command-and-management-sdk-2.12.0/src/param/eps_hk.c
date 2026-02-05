/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/a3200_sdk/param/host/eps_hk.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_a3200_sdk_eps_hk_table[] = {
    {.name = "vboost",        .addr = GS_A3200_SDK_EPS_HK_VBOOST(0),                     .type = GS_PARAM_UINT16,  .size =   2, .array_size =  3, .flags = 0},
    {.name = "vbatt",         .addr = GS_A3200_SDK_EPS_HK_VBATT,                         .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "curout",        .addr = GS_A3200_SDK_EPS_HK_CUROUT(0),                     .type = GS_PARAM_UINT16,  .size =   2, .array_size =  6, .flags = 0},
    {.name = "curin",         .addr = GS_A3200_SDK_EPS_HK_CURIN(0),                      .type = GS_PARAM_UINT16,  .size =   2, .array_size =  3, .flags = 0},
    {.name = "cursun",        .addr = GS_A3200_SDK_EPS_HK_CURSUN,                        .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "cursys",        .addr = GS_A3200_SDK_EPS_HK_CURSYS,                        .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "temp",          .addr = GS_A3200_SDK_EPS_HK_TEMP(0),                       .type = GS_PARAM_INT16,   .size =   2, .array_size =  6, .flags = 0},
    {.name = "out_val",       .addr = GS_A3200_SDK_EPS_HK_OUT_VAL(0),                    .type = GS_PARAM_UINT8,   .size =   1, .array_size =  8, .flags = 0},
    {.name = "battmode",      .addr = GS_A3200_SDK_EPS_HK_BATTMODE,                      .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "pptmode",       .addr = GS_A3200_SDK_EPS_HK_PPTMODE,                       .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "wdtI2cS",       .addr = GS_A3200_SDK_EPS_HK_WDTI2CS,                       .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "wdtGndS",       .addr = GS_A3200_SDK_EPS_HK_WDTGNDS,                       .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "bootcount",     .addr = GS_A3200_SDK_EPS_HK_BOOTCOUNT,                     .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "cntWdtI2c",     .addr = GS_A3200_SDK_EPS_HK_CNTWDTI2C,                     .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "cntWdtGnd",     .addr = GS_A3200_SDK_EPS_HK_CNTWDTGND,                     .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "cntWdtCsp",     .addr = GS_A3200_SDK_EPS_HK_CNTWDTCSP(0),                  .type = GS_PARAM_UINT32,  .size =   4, .array_size =  2, .flags = 0},
    {.name = "wdtCspC",       .addr = GS_A3200_SDK_EPS_HK_WDTCSPC(0),                    .type = GS_PARAM_UINT32,  .size =   4, .array_size =  2, .flags = 0},
    {.name = "latchups",      .addr = GS_A3200_SDK_EPS_HK_LATCHUPS(0),                   .type = GS_PARAM_UINT16,  .size =   2, .array_size =  6, .flags = 0},
    {.name = "bootcause",     .addr = GS_A3200_SDK_EPS_HK_BOOTCAUSE,                     .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
};

const gs_a3200_sdk_eps_hk_memory_t gs_a3200_sdk_eps_hk_default_values GS_PGM_OBJECT = {
    .vboost = {GS_A3200_SDK_EPS_HK_VBOOST_INVALID, GS_A3200_SDK_EPS_HK_VBOOST_INVALID, GS_A3200_SDK_EPS_HK_VBOOST_INVALID},
    .vbatt = GS_A3200_SDK_EPS_HK_VBATT_INVALID,
    .curout = {GS_A3200_SDK_EPS_HK_CUROUT_INVALID, GS_A3200_SDK_EPS_HK_CUROUT_INVALID, GS_A3200_SDK_EPS_HK_CUROUT_INVALID, GS_A3200_SDK_EPS_HK_CUROUT_INVALID, GS_A3200_SDK_EPS_HK_CUROUT_INVALID, GS_A3200_SDK_EPS_HK_CUROUT_INVALID},
    .curin = {GS_A3200_SDK_EPS_HK_CURIN_INVALID, GS_A3200_SDK_EPS_HK_CURIN_INVALID, GS_A3200_SDK_EPS_HK_CURIN_INVALID},
    .cursun = GS_A3200_SDK_EPS_HK_CURSUN_INVALID,
    .cursys = GS_A3200_SDK_EPS_HK_CURSYS_INVALID,
    .temp = {GS_A3200_SDK_EPS_HK_TEMP_INVALID, GS_A3200_SDK_EPS_HK_TEMP_INVALID, GS_A3200_SDK_EPS_HK_TEMP_INVALID, GS_A3200_SDK_EPS_HK_TEMP_INVALID, GS_A3200_SDK_EPS_HK_TEMP_INVALID, GS_A3200_SDK_EPS_HK_TEMP_INVALID},
    .out_val = {GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID, GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID, GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID, GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID, GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID, GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID, GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID, GS_A3200_SDK_EPS_HK_OUT_VAL_INVALID},
    .battmode = GS_A3200_SDK_EPS_HK_BATTMODE_INVALID,
    .pptmode = GS_A3200_SDK_EPS_HK_PPTMODE_INVALID,
    .wdtI2cS = GS_A3200_SDK_EPS_HK_WDTI2CS_INVALID,
    .wdtGndS = GS_A3200_SDK_EPS_HK_WDTGNDS_INVALID,
    .bootcount = GS_A3200_SDK_EPS_HK_BOOTCOUNT_INVALID,
    .cntWdtI2c = GS_A3200_SDK_EPS_HK_CNTWDTI2C_INVALID,
    .cntWdtGnd = GS_A3200_SDK_EPS_HK_CNTWDTGND_INVALID,
    .cntWdtCsp = {GS_A3200_SDK_EPS_HK_CNTWDTCSP_INVALID, GS_A3200_SDK_EPS_HK_CNTWDTCSP_INVALID},
    .wdtCspC = {GS_A3200_SDK_EPS_HK_WDTCSPC_INVALID, GS_A3200_SDK_EPS_HK_WDTCSPC_INVALID},
    .latchups = {GS_A3200_SDK_EPS_HK_LATCHUPS_INVALID, GS_A3200_SDK_EPS_HK_LATCHUPS_INVALID, GS_A3200_SDK_EPS_HK_LATCHUPS_INVALID, GS_A3200_SDK_EPS_HK_LATCHUPS_INVALID, GS_A3200_SDK_EPS_HK_LATCHUPS_INVALID, GS_A3200_SDK_EPS_HK_LATCHUPS_INVALID},
    .bootcause = GS_A3200_SDK_EPS_HK_BOOTCAUSE_INVALID,
};

