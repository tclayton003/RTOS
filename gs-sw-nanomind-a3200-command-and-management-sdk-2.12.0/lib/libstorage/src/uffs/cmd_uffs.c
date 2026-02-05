/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <gs/storage/uffs/command.h>
#include <gs/embed/freertos.h>
#include <gs/util/gosh/command.h>
#include <gs/util/string.h>
#include <gs/util/crc32.h>

#include <uffs/uffs_utils.h>
#include <uffs/uffs_mtb.h>
#include <uffs/uffs_fd.h>
#include <uffs/uffs_config.h>
#include <uffs/uffs_pool.h>
#include <uffs/uffs_version.h>

static int cmd_uffs_stat(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];

    uffs_Device * dev = uffs_GetDeviceFromMountPoint(path);
    if (dev == NULL) {
        printf("Can't get device from mount point %s\r\n", path);
        return GS_ERROR_IO;
    }

    uffs_FlashStat * s = &(dev->st);

    printf("UFFS version:            %d.%d.%d\r\n", uffs_GetMainVersion(uffs_GetVersion()), uffs_GetMinorVersion(uffs_GetVersion()), uffs_GetTrivialVersion(uffs_GetVersion()));
    printf("Basic info\r\n");
    printf("  TreeNode size:         %u\r\n", (unsigned)sizeof(TreeNode));
    printf("  TagStore size:         %u\r\n", (unsigned)sizeof(struct uffs_TagStoreSt));
    printf("  MaxCachedBlockInfo:    %d\r\n", MAX_CACHED_BLOCK_INFO);
    printf("  MaxPageBuffers:        %d\r\n", MAX_PAGE_BUFFERS);
    printf("  MaxDirtyPagesPerBlock: %d\r\n", MAX_DIRTY_PAGES_IN_A_BLOCK);
    printf("  MaxPathLength:         %d\r\n", MAX_FILENAME_LENGTH);
    printf("  MaxObjectHandles:      %d\r\n", MAX_OBJECT_HANDLE);
    printf("  FreeObjectHandles:     %d\r\n", uffs_PoolGetFreeCount(uffs_GetObjectPool()));
    printf("  MaxDirHandles:         %d\r\n", MAX_DIR_HANDLE);
    printf("  FreeDirHandles:        %d\r\n", uffs_PoolGetFreeCount(uffs_DirEntryBufGetPool()));

    printf("Statistics for '%s'\r\n", path);
    printf("  Block Erased:          %d\r\n", s->block_erase_count);
    printf("  Write Page:            %d\r\n", s->page_write_count);
    printf("  Write Spare:           %d\r\n", s->spare_write_count);
    printf("  Read Page:             %d\r\n", s->page_read_count - s->page_header_read_count);
    printf("  Read Header:           %d\r\n", s->page_header_read_count);
    printf("  Read Spare:            %d\r\n", s->spare_read_count);

    printf("Partition info for '%s'\r\n", path);
    printf("  Space total:           %d\r\n", uffs_GetDeviceTotal(dev));
    printf("  Space used:            %d\r\n", uffs_GetDeviceUsed(dev));
    printf("  Space free:            %d\r\n", uffs_GetDeviceFree(dev));
    printf("  Page Size:             %d\r\n", dev->attr->page_data_size);
    printf("  Spare Size:            %d\r\n", dev->attr->spare_size);
    printf("  Pages Per Block:       %d\r\n", dev->attr->pages_per_block);
    printf("  Block size:            %d\r\n", dev->attr->page_data_size * dev->attr->pages_per_block);
    printf("  Total blocks:          %d of %u\r\n", (dev->par.end - dev->par.start + 1), dev->attr->total_blocks);
    if (dev->tree.bad) {
        printf("Bad blocks: ");
        TreeNode * node = dev->tree.bad;
        while(node) {
            printf("%d, ", node->u.list.block);
            node = node->u.list.next;
        }
        printf("\r\n");
    }

    uffs_BufInspect(dev);

    uffs_PutDevice(dev);

    return GS_OK;
}

static const gs_command_t uffs_subcommands[] = {
    {
        .name = "stat",
        .help = "Show file system statistics",
        .handler = cmd_uffs_stat,
        .usage = "<partition>",
        .mandatory_args = 1,
    },
};

static const gs_command_t GS_COMMAND_ROOT uffs_commands[] = {
    {
        .name = "uffs",
        .help = "fs: UFFS",
        .chain = GS_COMMAND_INIT_CHAIN(uffs_subcommands),
    },
};

gs_error_t gs_storage_register_uffs_commands(void)
{
    return GS_COMMAND_REGISTER(uffs_commands);
}
