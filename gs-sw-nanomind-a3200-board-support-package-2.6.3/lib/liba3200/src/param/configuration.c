/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */

#include <gs/a3200/param/host/configuration.h>
#include <gs/util/pgm.h>

const gs_param_table_row_t gs_a3200_configuration_table[] = {
    {.name = "gssb1_pwr_en",  .addr = GS_A3200_CONFIGURATION_GSSB1_PWR_EN,               .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "gssb2_pwr_en",  .addr = GS_A3200_CONFIGURATION_GSSB2_PWR_EN,               .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "flash_pwr_en",  .addr = GS_A3200_CONFIGURATION_FLASH_PWR_EN,               .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "pwm_pwr_en",    .addr = GS_A3200_CONFIGURATION_PWM_PWR_EN,                 .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "swload_img1",   .addr = GS_A3200_CONFIGURATION_SWLOAD_IMG1,                .type = GS_PARAM_STRING,  .size =  32, .array_size =  1, .flags = GS_PARAM_F_AUTO_PERSIST},
    {.name = "swload_cnt1",   .addr = GS_A3200_CONFIGURATION_SWLOAD_CNT1,                .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = GS_PARAM_F_AUTO_PERSIST},
    {.name = "swload_img2",   .addr = GS_A3200_CONFIGURATION_SWLOAD_IMG2,                .type = GS_PARAM_STRING,  .size =  32, .array_size =  1, .flags = GS_PARAM_F_AUTO_PERSIST},
    {.name = "swload_cnt2",   .addr = GS_A3200_CONFIGURATION_SWLOAD_CNT2,                .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = GS_PARAM_F_AUTO_PERSIST},
    {.name = "fs_default",    .addr = GS_A3200_CONFIGURATION_FS_DEFAULT,                 .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "fs_en_p0",      .addr = GS_A3200_CONFIGURATION_FS_EN_P0,                   .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "fs_en_p1",      .addr = GS_A3200_CONFIGURATION_FS_EN_P1,                   .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "log_fram_en",   .addr = GS_A3200_CONFIGURATION_LOG_FRAM_EN,                .type = GS_PARAM_BOOL,    .size =   1, .array_size =  1, .flags = 0},
    {.name = "log_mask",      .addr = GS_A3200_CONFIGURATION_LOG_MASK,                   .type = GS_PARAM_STRING,  .size =  64, .array_size =  1, .flags = 0},
    {.name = "tsync_node",    .addr = GS_A3200_CONFIGURATION_TSYNC_NODE,                 .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
    {.name = "tsync_intv",    .addr = GS_A3200_CONFIGURATION_TSYNC_INTV,                 .type = GS_PARAM_UINT16,  .size =   2, .array_size =  1, .flags = 0},
    {.name = "spi0_cs_mode",  .addr = GS_A3200_CONFIGURATION_SPI0_CS_MODE,               .type = GS_PARAM_UINT8,   .size =   1, .array_size =  1, .flags = 0},
};

const gs_a3200_configuration_memory_t gs_a3200_configuration_default_values GS_PGM_OBJECT = {
    .gssb1_pwr_en = false,
    .gssb2_pwr_en = false,
    .flash_pwr_en = false,
    .pwm_pwr_en = false,
    .fs_default = 0,
    .fs_en_p0 = true,
    .fs_en_p1 = true,
    .log_fram_en = true,
    .log_mask = "",
    .tsync_intv = 60,
    .spi0_cs_mode = 0,
};

