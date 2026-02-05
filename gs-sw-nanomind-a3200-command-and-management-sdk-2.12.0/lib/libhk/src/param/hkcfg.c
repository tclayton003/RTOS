/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/hk/param/host/hkcfg.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_hk_hkcfg_table[] = {
    {.name = "bcn_atx_tpds",  .addr = GS_HK_HKCFG_BCN_ATX_TPDS,                          .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "bcn_atx_da",    .addr = GS_HK_HKCFG_BCN_ATX_DA,                            .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "bcn_atx_dp",    .addr = GS_HK_HKCFG_BCN_ATX_DP,                            .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "bcn_satid",     .addr = GS_HK_HKCFG_BCN_SATID,                             .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "persist_intv",  .addr = GS_HK_HKCFG_PERSIST_INTV,                          .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "persist_en",    .addr = GS_HK_HKCFG_PERSIST_EN,                            .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "data_path",     .addr = GS_HK_HKCFG_DATA_PATH,                             .type = GS_PARAM_STRING,  .size =  50, .array_size =  1, .flags = 0},
    {.name = "hk_en",         .addr = GS_HK_HKCFG_HK_EN,                                 .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
};

const gs_hk_hkcfg_memory_t gs_hk_hkcfg_default_values GS_PGM_OBJECT = {
    .bcn_atx_tpds = 0,
    .bcn_atx_da = 10,
    .bcn_atx_dp = 30,
    .bcn_satid = 1,
    .persist_intv = 900,
    .persist_en = true,
    .data_path = "/flash/hk",
    .hk_en = true,
};

