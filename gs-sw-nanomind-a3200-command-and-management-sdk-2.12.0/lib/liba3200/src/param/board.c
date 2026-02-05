/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/a3200/param/host/board.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_a3200_board_table[] = {
    {.name = "uid",           .addr = GS_A3200_BOARD_UID,                                .type = GS_PARAM_STRING,  .size =  16, .array_size =  1, .flags = 0},
    {.name = "type",          .addr = GS_A3200_BOARD_TYPE,                               .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "rev",           .addr = GS_A3200_BOARD_REV,                                .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "dock_type",     .addr = GS_A3200_BOARD_DOCK_TYPE,                          .type = GS_PARAM_STRING,  .size =  16, .array_size =  1, .flags = 0},
    {.name = "csp_addr",      .addr = GS_A3200_BOARD_CSP_ADDR,                           .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "csp_rtable",    .addr = GS_A3200_BOARD_CSP_RTABLE,                         .type = GS_PARAM_STRING,  .size =  96, .array_size =  1, .flags = 0},
    {.name = "gosh_en",       .addr = GS_A3200_BOARD_GOSH_EN,                            .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "gosh_uart",     .addr = GS_A3200_BOARD_GOSH_UART,                          .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "can_en",        .addr = GS_A3200_BOARD_CAN_EN,                             .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "can_brate",     .addr = GS_A3200_BOARD_CAN_BRATE,                          .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "i2c_en",        .addr = GS_A3200_BOARD_I2C_EN,                             .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "i2c_brate",     .addr = GS_A3200_BOARD_I2C_BRATE,                          .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "kiss_en",       .addr = GS_A3200_BOARD_KISS_EN,                            .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "kiss_brate",    .addr = GS_A3200_BOARD_KISS_BRATE,                         .type = GS_PARAM_UINT32,  .size =   4, .array_size =  1, .flags = 0},
    {.name = "kiss_uart",     .addr = GS_A3200_BOARD_KISS_UART,                          .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
};

const gs_a3200_board_memory_t gs_a3200_board_default_values GS_PGM_OBJECT = {
    .type = 0,
    .rev = 0,
    .csp_addr = 1,
    .csp_rtable = "",
    .gosh_en = true,
    .gosh_uart = 2,
    .can_en = true,
    .can_brate = 1000000,
    .i2c_en = true,
    .i2c_brate = 400000,
    .kiss_en = false,
    .kiss_brate = 500000,
    .kiss_uart = 4,
};

