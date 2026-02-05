/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/a3200/fs.h>
#include <gs/a3200/spi_slave.h>
#include <gs/a3200/pwr_switch.h>
#include "../local.h"
#include <gs/thirdparty/flash/spn_fl512s.h>
#include <gs/storage/vfs/vfs.h>
#include <gs/util/string.h>
#include <gs/util/time.h>

static gs_error_t fs_mount(void)
{
    gs_error_t error = GS_OK;
#if (GS_STORAGE_VFS)
    uint32_t ms = gs_time_rel_ms();

    if (!GS_A3200_CONFIGURATION_FS_EN_P0_GET_NC() &&
        !GS_A3200_CONFIGURATION_FS_EN_P1_GET_NC()) {
        log_notice("FileSystem: All partitions disabled");
        return GS_ERROR_STATE;
    }

    /* Turn on power */
    gs_a3200_pwr_switch_enable(GS_A3200_PWR_SD);

    /* Initialize spansion chip.
       Requires that the SPI device has been initialized */
    const spn_fl512s_config_t config = {
        .bps = 8000000,
        .cs_part_0 = SPN_FL512S_CS0,
        .cs_part_1 = SPN_FL512S_CS1,
        .spi_slave_part_0 = GS_A3200_SPI_SLAVE_SPN_FL512_0,
        .spi_slave_part_1 = GS_A3200_SPI_SLAVE_SPN_FL512_1,
        .spi_handle = 1,
    };

    error = spn_fl512s_init(&config);
    if (error) {
        log_warning("FileSystem: failed to initialize flash driver, error=%s", gs_error_string(error));
        return error;
    }

    error = vfs_mount(GS_A3200_CONFIGURATION_FS_DEFAULT_GET_NC(),
                      GS_A3200_CONFIGURATION_FS_EN_P0_GET_NC(),
                      GS_A3200_CONFIGURATION_FS_EN_P1_GET_NC());
    ms = gs_time_diff_ms(ms, gs_time_rel_ms());
    if (error == GS_OK) {
        log_notice("FileSystem mounted OK in %" PRIu32 " mS", ms);
    } else {
        log_warning("FileSystem failed to mount after %" PRIu32 " mS, error: %d", ms, error);
    }
#endif

    return error;
}

static void * fs_mount_task(void * param)
{
    gs_a3200_mount_fs_completed_t cb = (gs_a3200_mount_fs_completed_t) param;
    gs_error_t error =  fs_mount();
    if (cb) {
        cb(error);
    }

    gs_thread_exit(NULL);
}

gs_error_t gs_a3200_fs_mount(gs_a3200_mount_fs_completed_t cb)
{
    return gs_thread_create("MNT", fs_mount_task, cb, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_LOW, 0, NULL);
}
