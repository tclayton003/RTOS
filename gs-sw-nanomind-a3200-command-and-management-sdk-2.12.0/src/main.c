/* Copyright (c) 2013-2020 GomSpace A/S. All rights reserved. */

#include "mission_init.h"
#include <gs/a3200/a3200.h>
#include <gs/util/vmem.h>
#include <gs/util/log/log.h>
#include <gs/util/log/appender/console.h>
#include <gs/thirdparty/fram/fm33256b.h>
#include <gs/a3200/param/stores.h>
#include <gs/param/store/file/file.h>
#include <gs/embed/drivers/flash/mcu_flash.h>
#include <gs/embed/drivers/sys/reset.h>
#include "param/a3200_sdk_store.h"
#include <gs/hk/param/stores.h>

// External linker script defined symbols for flash data region
extern const uint32_t _FLASH_DATA_START;
extern const uint32_t _FLASH_DATA_SIZE;

// Setup locking for protected FRAM region
static const gs_fm33256b_vmem_driver_data_t fm33256b_lock = {
    .device = 0,
    .protect = GS_FM33256B_PROTECT_UPPER_QUARTER,
};

// Setup VMEM - primarily mapping out the FRAM
static const gs_vmem_t vmem_map[] = {
    // FRAM layout: fm33256b - 32KB, 0x0000 - 0x7FFF

    // unprotected area - 0x0000 - 0x5FFF
    {.name = "param-"GS_A3200_PARAM_STORE,                 .virtmem.u = 0x10000000, .physmem.u = 0x0000, .size = 0x0500,
     .drv = &gs_fm33256b_vmem_driver},
    {.name = "param-"GS_HK_PARAM_STORE,                    .virtmem.u = 0x10000500, .physmem.u = 0x0500, .size = 0x0400,
     .drv = &gs_fm33256b_vmem_driver},
    {.name = "param-"GS_A3200_SDK_PARAM_STORE,             .virtmem.u = 0x10001000, .physmem.u = 0x1000, .size = 0x0200,
     .drv = &gs_fm33256b_vmem_driver},
    {.name = "log",                                        .virtmem.u = 0x10005000, .physmem.u = 0x5000, .size = 0x1000,
     .drv = &gs_fm33256b_vmem_driver },

    // protected area - 0x6000 - 0x7FFF
    {.name = "param-"GS_A3200_PARAM_STORE_PROTECTED,       .virtmem.u = 0x10006000, .physmem.u = 0x6000, .size = 0x0500,
     .drv = &gs_fm33256b_vmem_driver, .drv_data = &fm33256b_lock},
    {.name = "param-"GS_A3200_SDK_PARAM_STORE_PROTECTED,   .virtmem.u = 0x10006500, .physmem.u = 0x6500, .size = 0x200,
     .drv = &gs_fm33256b_vmem_driver, .drv_data = &fm33256b_lock},

    // MCU FLASH: reserved area for VMEM/Param
    {.name = "param-"GS_A3200_PARAM_STORE_MCU_FLASH,       .virtmem.u = 0x10010000,
     .physmem.u = (uintptr_t)&_FLASH_DATA_START, .size = (uintptr_t)&_FLASH_DATA_SIZE, .drv = &gs_mcu_flash_vmem_driver},

    {.name = NULL}, // end-of-entries, do not remove
};

static const gs_param_store_file_map_t file_store = { .path = "/flash/param"};

static const gs_param_store_t param_store_map[] = {
    {.name = GS_A3200_PARAM_STORE,                  &gs_param_store_vmem_driver_checksum_first, gs_a3200_param_store},
    {.name = GS_A3200_PARAM_STORE_PROTECTED,        &gs_param_store_vmem_driver_checksum_first, gs_a3200_param_store_protected},
    {.name = GS_A3200_PARAM_STORE_MCU_FLASH,        &gs_param_store_vmem_driver_checksum_first, gs_a3200_param_store_mcu_flash},
    {.name = GS_A3200_SDK_PARAM_STORE,              &gs_param_store_vmem_driver_checksum_first, gs_a3200_sdk_param_store},
    {.name = GS_A3200_SDK_PARAM_STORE_PROTECTED,    &gs_param_store_vmem_driver_checksum_first, gs_a3200_sdk_param_store},
    {.name = GS_HK_PARAM_STORE,                     &gs_param_store_vmem_driver_checksum_first, gs_hk_param_store},
    {.name = "a3200-ffs",                           &gs_param_store_file_driver_bin, &file_store},
    {.name = NULL}
};

static void param_init_stores(void)
{
    // Setup VMEM (virtual memory) mapping
    gs_vmem_set_map(vmem_map);

    // Setup parameter stores (-> regions marked with param- in VMEM).
    gs_param_store_register(param_store_map);
}

static const gs_a3200_hooks_t hooks = {
    // hook called when liba3200 want to configure VMEM and the parameter stores (where to load parameter tables from)
    .init_vmem_param = param_init_stores,
    // hook called when the basic boot process is completed, just before a task is spawned to mount the file system
    .init_complete = mission_init,
    // hook called when the file system is mounted and the boot process is completed.
    .fs_mounted = mission_init_fs_mounted,
};

int main()
{
    // Initialize: clock, watchdog, external memory, malloc
    gs_a3200_init();

    // Early log initialize and register of the console log appender (sends log to stdout)
    gs_log_init(true);

    // Spawn a task to complete the initialization and launch application/mission tasks through hooks.
    gs_a3200_run(&hooks);

    // Will never get here, as gs_a3200_run() never returns
    gs_sys_reset(GS_SYS_BOOT_UNKNOWN);
}
