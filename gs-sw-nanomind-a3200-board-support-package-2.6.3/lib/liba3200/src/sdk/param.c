/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include "../local.h"
#include <gs/a3200/param/stores.h>
#include <gs/a3200/pwr_switch.h>

const gs_param_store_vmem_map_t gs_a3200_param_store[] = {
    {GS_A3200_BOARD_TABLE_NAME,               0x0000,    0x100},
    {GS_A3200_CONFIGURATION_TABLE_NAME,       0x0100,    0x100},
    {"ap-"GS_A3200_CONFIGURATION_TABLE_NAME,  0x0200,    0x100},
    {"ap-"GS_A3200_TELEMETRY_TABLE_NAME,      0x0300,    0x100},
    {GS_A3200_DOCK_TABLE_NAME,                0x0400,    0x100},
    {.name = NULL}
};

const gs_param_store_vmem_map_t gs_a3200_param_store_protected[] = {
    {GS_A3200_BOARD_TABLE_NAME,               0x0000,    0x100},
    {GS_A3200_CONFIGURATION_TABLE_NAME,       0x0100,    0x100},
    {.name = NULL}
};

const gs_param_store_vmem_map_t gs_a3200_param_store_mcu_flash[] = {
    {GS_A3200_BOARD_TABLE_NAME,               0x0000,    0x200},
    {GS_A3200_CONFIGURATION_TABLE_NAME,       0x0200,    0x200},
    {.name = NULL}
};

static void configuration_callback(uint16_t addr, gs_param_table_instance_t * tinst)
{
    switch(addr) {
        case GS_A3200_CONFIGURATION_GSSB1_PWR_EN:
            gs_a3200_pwr_switch_set(GS_A3200_PWR_GSSB, GS_A3200_CONFIGURATION_GSSB1_PWR_EN_GET_NC());
            break;

        case GS_A3200_CONFIGURATION_GSSB2_PWR_EN:
            gs_a3200_pwr_switch_set(GS_A3200_PWR_GSSB2, GS_A3200_CONFIGURATION_GSSB2_PWR_EN_GET_NC());
            break;

        case GS_A3200_CONFIGURATION_FLASH_PWR_EN:
            gs_a3200_pwr_switch_set(GS_A3200_PWR_SD, GS_A3200_CONFIGURATION_FLASH_PWR_EN_GET_NC());
            break;

        case GS_A3200_CONFIGURATION_PWM_PWR_EN:
            gs_a3200_pwr_switch_set(GS_A3200_PWR_PWM, GS_A3200_CONFIGURATION_PWM_PWR_EN_GET_NC());
            break;
    }
}

// Initialize board table using default values - but overwrite CSP address
static gs_error_t initialize_board_table(gs_param_table_instance_t * tinst)
{
    gs_param_table_memory_initialize_zero(tinst); // clear all memory
    gs_a3200_board_memory_t * board = (gs_a3200_board_memory_t*) gs_param_table_get_memory(tinst, NULL);
    if (board) {
        *board = gs_a3200_board_default_values; // set default values
        board->csp_addr = GS_A3200_NODE;
    }
    return GS_OK;
}

void gs_a3200_sdk_param_init_board(void)
{
    /* Board table (persistent) */
    gs_param_table_instance_t * tinst = NULL;
    const gs_param_table_create_t create = {.name = GS_A3200_BOARD_TABLE_NAME, .id = GS_A3200_BOARD_TABLE_MEM_ID,
                                            .rows = gs_a3200_board_table, .row_count = GS_A3200_BOARD_TABLE_ROWS,
                                            .memory_size = GS_A3200_BOARD_TABLE_SIZE,
                                            .initializer = initialize_board_table,
                                            .stores = GS_A3200_PARAM_STORE","GS_A3200_PARAM_STORE_PROTECTED","GS_A3200_PARAM_STORE_MCU_FLASH,
                                            .flags = GS_PARAM_TABLE_F_ALLOC_MEMORY};
    gs_param_table_create(&create, &tinst);
    gs_param_table_load_fallback(tinst, 0);
}

void gs_a3200_sdk_param_init(void)
{
    /* Configuration table (persisten and auto-persist) */
    {
        gs_param_table_instance_t * tinst = NULL;
        const gs_param_table_create_t create = {.name = GS_A3200_CONFIGURATION_TABLE_NAME, .id = GS_A3200_CONFIGURATION_TABLE_MEM_ID,
                                                .rows = gs_a3200_configuration_table, .row_count = GS_A3200_CONFIGURATION_TABLE_ROWS,
                                                .memory_size = GS_A3200_CONFIGURATION_TABLE_SIZE,
                                                .callback = configuration_callback,
                                                .default_values = &gs_a3200_configuration_default_values,
                                                .stores = GS_A3200_PARAM_STORE","GS_A3200_PARAM_STORE_PROTECTED","GS_A3200_PARAM_STORE_MCU_FLASH,
                                                .auto_persist = GS_A3200_PARAM_STORE,
                                                .flags = GS_PARAM_TABLE_F_ALLOC_MEMORY};
        gs_param_table_create(&create, &tinst);
        gs_param_table_load_fallback(tinst, 0);
    }

    /* Telemetry table (only auto-persist) */
    {
        gs_param_table_instance_t * tinst = NULL;
        const gs_param_table_create_t create = {.name = GS_A3200_TELEMETRY_TABLE_NAME, .id = GS_A3200_TELEMETRY_TABLE_MEM_ID,
                                                .rows = gs_a3200_telemetry_table, .row_count = GS_A3200_TELEMETRY_TABLE_ROWS,
                                                .memory_size = GS_A3200_TELEMETRY_TABLE_SIZE,
                                                .auto_persist = GS_A3200_PARAM_STORE,
                                                .flags = GS_PARAM_TABLE_F_ALLOC_MEMORY};
        gs_param_table_create(&create, &tinst);
        gs_param_table_load_fallback(tinst, 0);
    }
}

gs_error_t gs_a3200_sdk_param_get_dock(char * buf, size_t buf_size)
{
    return GS_A3200_BOARD_DOCK_TYPE_GET(buf, buf_size);
}
