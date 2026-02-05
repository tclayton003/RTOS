/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/a3200_sdk/param/host/scheduler.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_a3200_sdk_scheduler_table[] = {
    {.name = "enabled",       .addr = GS_A3200_SDK_SCHEDULER_ENABLED,                    .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "delayed_start", .addr = GS_A3200_SDK_SCHEDULER_DELAYED_START,              .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "bpx_col_intv",  .addr = GS_A3200_SDK_SCHEDULER_BPX_COL_INTV,               .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "bpx_csp_addr",  .addr = GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR,               .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "eps_col_intv",  .addr = GS_A3200_SDK_SCHEDULER_EPS_COL_INTV,               .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "eps_csp_addr",  .addr = GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR,               .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "adsb_col_intv", .addr = GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV,              .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "adsb_csp_addr", .addr = GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR,              .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
};

const gs_a3200_sdk_scheduler_memory_t gs_a3200_sdk_scheduler_default_values GS_PGM_OBJECT = {
    .enabled = false,
    .delayed_start = 30,
    .bpx_col_intv = 0,
    .bpx_csp_addr = 7,
    .eps_col_intv = 0,
    .eps_csp_addr = 2,
    .adsb_col_intv = 0,
    .adsb_csp_addr = 4,
};

