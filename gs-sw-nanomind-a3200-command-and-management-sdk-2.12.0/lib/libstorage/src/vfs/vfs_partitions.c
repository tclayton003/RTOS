/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "vfs_local.h"
#include "../uffs/drivers/fl512s_local.h"
#include "../uffs/uffs_local.h"
#include <uffs/uffs_public.h>
#include <gs/util/string.h>
#include <gs/util/log.h>

// This structure is not 'const', because 'flash' will have to be re-directed runtime.
static struct vfs_partition vfs_partitions[3];

const struct vfs_partition *vfs_find_partition(const char *name)
{
    for (unsigned int i = 0; i < GS_ARRAY_SIZE(vfs_partitions); i++) {
        if (strcmp(vfs_partitions[i].name, name) == 0) {
            return &vfs_partitions[i];
        }
    }
    return NULL;
}

int vfs_itertate_partitions(vfs_part_cb_t cb, void * ctx)
{
    int count = 0;
    for (unsigned int i = 0; i < GS_ARRAY_SIZE(vfs_partitions); ++i) {
        if (vfs_partitions[i].name) {
            cb(vfs_partitions[i].name, ctx);
            ++count;
        }
    }
    return count;
}

/**
   Here are file-systems are initialized and mounted -> made accessible through VFS.
   This function can be extended to initialize/mount custom file-systems.
*/
gs_error_t vfs_mount(uint8_t default_partition, bool mount_p0, bool mount_p1)
{
    vfs_init();

    memset(vfs_partitions, 0, sizeof(vfs_partitions));
    unsigned int partition = 0;
    unsigned int default_index = -1;
    bool init_uffs = false;

    if (mount_p0) {
        vfs_partitions[partition].name = GS_FL512S_NAME_PARTITION_0;
        vfs_partitions[partition].fops = &vfs_uffs_ops;
        gs_fl512s_uffs_mount(0);
        if (default_partition == 0) {
            default_index = partition;
        }
        init_uffs = true;
        ++partition;
    }

    if (mount_p1) {
        vfs_partitions[partition].name = GS_FL512S_NAME_PARTITION_1;
        vfs_partitions[partition].fops = &vfs_uffs_ops;
        gs_fl512s_uffs_mount(1);
        if (default_partition == 1) {
            default_index = partition;
        }
        init_uffs = true;
        ++partition;
    }

    // Check if we should add the 'alias'
    if (default_index < GS_ARRAY_SIZE(vfs_partitions)) {
        vfs_partitions[partition].name = "flash";
        vfs_partitions[partition].true_name = vfs_partitions[default_index].name;
        vfs_partitions[partition].fops = vfs_partitions[default_index].fops;
        ++partition;
    }

    if (init_uffs) {
        /* Initialize the UFFS system */
        int result_uffs = uffs_InitFileSystemObjects();
        if (result_uffs != U_SUCC) {
            log_error("%s: uffs_InitFileSystemObjects() failed, res: %d", __FUNCTION__, result_uffs);
            return GS_ERROR_UNKNOWN;
        }
    }

    log_debug("%s: mounted %u partitions OK", __FUNCTION__, partition);

    return GS_OK;
}
