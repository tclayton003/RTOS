/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   #file 

   UFFS driver for NanoMind's flash storage S70FL01S.
*/

#include "fl512s_local.h"
#include <uffs/uffs_device.h>
#include <uffs/uffs_flash.h>
#include <uffs/uffs_mtb.h>
#include <uffs/uffs_fs.h>
#include <uffs/uffs_fd.h>
#include <uffs/uffs_os.h>
#include <uffs/uffs_utils.h>
#include <gs/util/string.h>
#include <gs/util/log.h>
#include <stdlib.h>

/**
   Total blocks/sectors.
*/
#define FL512S_TOTAL_BLOCKS     (SPN_FL512S_SIZE / SPN_FL512S_SECTOR_SIZE)

/**
   Pages per block/sector.
*/
#define FL512S_PAGES_PER_BLOCK  (SPN_FL512S_SECTOR_SIZE / SPN_FL512S_PAGE_SIZE)

/**
   UFFS partion names - must have / before/after.
*/
#define UFFS_NAME_PARTITION_0  "/" GS_FL512S_NAME_PARTITION_0 "/"
#define UFFS_NAME_PARTITION_1  "/" GS_FL512S_NAME_PARTITION_1 "/"

/**
   Spare size per page (bytes)
   There is no dedicated spare area in each page, so we reserve the last 16
   bytes of each page for spare data (Tag, ECC). This leaves 496 bytes of data
   per page.
*/
#define FL512S_PAGE_SPARE_SIZE  16

/**
   Page data size (bytes).
*/
#define FL512S_PAGE_DATA_SIZE   (SPN_FL512S_PAGE_SIZE - FL512S_PAGE_SPARE_SIZE)

/**
   NOR flash does not have a status byte, so we just use the fifth byte
   in the spare area (offset 4) which should always be 0xFF.
*/
#define FL512S_BLOCK_ST_OFFSET  4

#if defined(GS_LOG_CAT_DRIVER) && defined(GS_LOG_CAT_FILE_SYSTEM)
#define LOG_CATEGORY (GS_LOG_CAT_DRIVER | GS_LOG_CAT_FILE_SYSTEM)
#else
#define LOG_CATEGORY GS_LOG_CAT_DEFAULT
#endif

static GS_LOG_GROUP(gs_fl512s_uffs_log, "fl512s-uffs", LOG_CATEGORY, LOG_DEFAULT_MASK);
#define LOG_DEFAULT gs_fl512s_uffs_log


/**
 * Generic helper function for reading flash pages.
 * @note This function should only be called by UFFS and not from user space
 * @param dev UFFS device
 * @param block Block number
 * @param page Page number
 * @param data Pointer to data buffer
 * @param data_len
 * @param ecc ECC of data
 * @param space Pointer to spare buffer
 * @param spare_len Length of spare to read
 */
static int fl512s_read_page(uffs_Device *dev, u32 block, u32 page, u8 *data, int data_len, u8 *ecc, u8 *spare, int spare_len)
{
    uint32_t addr;
    uint8_t bad_block;
    addr = block * SPN_FL512S_SECTOR_SIZE + page * SPN_FL512S_PAGE_SIZE;
    log_trace("fl512s[%d] read d %p(%d) s %p(%d) - block %3u page %3u addr %08x",
              dev->dev_num, data, data_len, spare, spare_len, block, page, (unsigned)addr);

    if (data_len > FL512S_PAGE_DATA_SIZE) {
        log_warning("fl512s[%d] trying to read %u data bytes, max %u", dev->dev_num, data_len, FL512S_PAGE_DATA_SIZE);
        return UFFS_FLASH_IO_ERR;
    }

    if (spare_len > FL512S_PAGE_SPARE_SIZE) {
        log_warning("fl512s[%d] trying to read %u spare bytes. max %u", dev->dev_num, spare_len, FL512S_PAGE_SPARE_SIZE);
        return UFFS_FLASH_IO_ERR;
    }

    /* If no data and no spare, read bad block mark */
    if (!data && !spare) {
        addr += FL512S_PAGE_DATA_SIZE + FL512S_BLOCK_ST_OFFSET;
        if(spn_fl512s_read_data((dev->dev_num - 1), addr, &bad_block, sizeof(bad_block)) != GS_OK) {
            return UFFS_FLASH_IO_ERR;
        }

        if (bad_block != 0xff) {
            log_warning("fl512s[%d] block %u page %u is marked bad!", dev->dev_num, block, page);
            return UFFS_FLASH_BAD_BLK;
        }

        return UFFS_FLASH_NO_ERR;
    }

    /* Read data */
    if (data && data_len > 0) {
        if(spn_fl512s_read_data((dev->dev_num - 1), addr, (uint8_t *)data, data_len) != GS_OK) {
            return UFFS_FLASH_IO_ERR;
        }
        dev->st.page_read_count++;
    }

    /* Read spare */
    if (spare && spare_len > 0) {
        if(spn_fl512s_read_data((dev->dev_num - 1), addr + FL512S_PAGE_DATA_SIZE, (uint8_t *)spare, spare_len) != GS_OK) {
            return UFFS_FLASH_IO_ERR;
        }
        dev->st.spare_read_count++;
    }
    return UFFS_FLASH_NO_ERR;
}

/**
 * Write page to flash page (data and spare area).
 * @note This function should only be called by UFFS and not from user space
 * @param dev UFFS device
 * @param block Block number
 * @param page Page number
 * @param data Pointer to data buffer
 * @param data_len Length of data in data buffer
 * @param spare Pointer to spare buffer
 * @param spare_len Length of spare buffer
 */
static int fl512s_write_page(uffs_Device *dev, u32 block, u32 page, const u8 *data, int data_len, const u8 *spare, int spare_len)
{
    uint32_t addr;
    uint8_t bad_block = 0x00;

    addr = block * SPN_FL512S_SECTOR_SIZE + page * SPN_FL512S_PAGE_SIZE;
    log_trace("fl512s[%d] write d %p(%d) s %p(%d) - block %3u page %3u addr %08x",
              dev->dev_num, data, data_len, spare, spare_len, block, page, (unsigned)addr);

    if (data_len > FL512S_PAGE_DATA_SIZE) {
        log_warning("fl512s[%d] trying to write %u data bytes, max %u", dev->dev_num, data_len, FL512S_PAGE_DATA_SIZE);
        return UFFS_FLASH_IO_ERR;
    }

    if (spare_len > FL512S_PAGE_SPARE_SIZE) {
        log_warning("fl512s[%d] trying to write %u spare bytes, max %u", dev->dev_num, spare_len, FL512S_PAGE_SPARE_SIZE);
        return UFFS_FLASH_IO_ERR;
    }

    /* If no data and no spare, we should mark the page as bad (see uffs_flash.h) */
    if (!data && !spare) {
        addr += FL512S_PAGE_DATA_SIZE + FL512S_BLOCK_ST_OFFSET;
        if(spn_fl512s_write_data((dev->dev_num - 1), addr, &bad_block, sizeof(bad_block)) != GS_OK) {
            return UFFS_FLASH_IO_ERR;
        }

        log_warning("fl512s[%d] marking block %u page %u bad!", dev->dev_num, block, page);

        return UFFS_FLASH_NO_ERR;
    }

    /* Write data */
    if (data && data_len > 0) {
        if(spn_fl512s_write_data((dev->dev_num - 1), addr, (uint8_t *)data, data_len) != GS_OK) {
            return UFFS_FLASH_IO_ERR;
        }
        dev->st.page_write_count++;
    }

    /* Write spare */
    if (spare && spare_len > 0) {
        if(spn_fl512s_write_data((dev->dev_num - 1), addr + FL512S_PAGE_DATA_SIZE, (uint8_t *)spare, spare_len) != GS_OK) {
            return UFFS_FLASH_IO_ERR;
        }
        dev->st.spare_write_count++;
    }
    return UFFS_FLASH_NO_ERR;
}


/**
 * Erase flash block.
 * @note This function should only be called by UFFS and not from user space.
 * @param dev UFFS device
 * @param blockNumber Block number
 */
static int fl512s_erase_block(uffs_Device *dev, u32 blockNumber)
{
    uint32_t addr = blockNumber * SPN_FL512S_SECTOR_SIZE;
    log_trace("fl512s[%d] erase block %3u addr %08x", dev->dev_num, blockNumber, (unsigned)addr);

    gs_error_t error = spn_fl512s_erase_block((dev->dev_num - 1), addr);
    if (error)
        return UFFS_FLASH_IO_ERR;

    dev->st.block_erase_count++;

    return UFFS_FLASH_NO_ERR;
}

/**
   Flash driver function table
*/
static struct uffs_FlashOpsSt fl512s_driver_ops = {
    .ReadPage       = fl512s_read_page,
    .WritePage      = fl512s_write_page,
    .EraseBlock     = fl512s_erase_block,
};

/**
   Flash chip configuration
*/
static struct uffs_StorageAttrSt fl512s_storage = {
    .total_blocks       = FL512S_TOTAL_BLOCKS,
    .page_data_size     = FL512S_PAGE_DATA_SIZE,
    .pages_per_block    = FL512S_PAGES_PER_BLOCK,
    .spare_size         = FL512S_PAGE_SPARE_SIZE,
    .block_status_offs  = FL512S_BLOCK_ST_OFFSET,
    .ecc_opt            = UFFS_ECC_SOFT,
    .layout_opt         = UFFS_LAYOUT_UFFS,
};

#if (CONFIG_ENABLE_UFFS_DEBUG_MSG)
static void uffs_output(const char *msg)
{
    printf("%s", msg);
}
static void uffs_vprintf(const char *fmt, va_list args)
{
    vprintf(fmt, args);
}

static struct uffs_DebugMsgOutputSt debug = {
    .output = &uffs_output,
    .vprintf = &uffs_vprintf,
};
#endif

/**
   Initialise DataFlash device
   @note This function should only be called by UFFS and not from user space.
   @param dev UFFS device
   @return U_SUCC on success
*/
static URET fl512s_init_device(uffs_Device * dev)
{
    log_info("fl512s[%d] initialize device", dev->dev_num);

    /* Set driver device functions */
    dev->ops = &fl512s_driver_ops;

#if (CONFIG_ENABLE_UFFS_DEBUG_MSG)
    return uffs_InitDebugMessageOutput(&debug, 0);
#else
    return U_SUCC;
#endif
}

/**
   Release DataFlash device
   @note This function should only be called by UFFS and not from user space.
   @param dev UFFS device
   @return U_SUCC on success
*/
static URET fl512s_release_device(uffs_Device * dev)
{
    log_info("fl512s[%d] release device", dev->dev_num);
    return U_SUCC;
}

static uffs_Device fl512s_devices[] = {
    {
        .Init    = fl512s_init_device,
        .Release = fl512s_release_device,
        .attr    = &fl512s_storage,
    },{
        .Init = fl512s_init_device,
        .Release = fl512s_release_device,
        .attr = &fl512s_storage,
    }
};

/* Define two partitions - use entire die for each partition */
static uffs_MountTable fl512s_tables[] = {
    {
        .dev         = &fl512s_devices[0],
        .start_block = 0,
        .end_block   = FL512S_TOTAL_BLOCKS - 1,
        .mount       = UFFS_NAME_PARTITION_0
    },{
        .dev         = &fl512s_devices[1],
        .start_block = 0,
        .end_block   = FL512S_TOTAL_BLOCKS - 1,
        .mount       = UFFS_NAME_PARTITION_1
    }
};

gs_error_t gs_fl512s_uffs_mount(unsigned int partition)
{
    gs_log_group_register(LOG_DEFAULT);

    if (partition > GS_ARRAY_SIZE(fl512s_tables)) {
        return GS_ERROR_ARG;
    }
    
    /* Allocate memory for file system buffers, state and cache. */
    const unsigned int flash_static_buffer_size = UFFS_STATIC_BUFF_SIZE(FL512S_PAGES_PER_BLOCK, SPN_FL512S_PAGE_SIZE, FL512S_TOTAL_BLOCKS);

    void * flash_static_buffer = malloc(flash_static_buffer_size);
    if (flash_static_buffer == NULL) {
        log_error("%s: partition: %u, failed to allocate UFFS buffers of %u bytes", __FUNCTION__, partition, flash_static_buffer_size);
        return GS_ERROR_ALLOC;
    }

    uffs_MemSetupStaticAllocator(&fl512s_devices[partition].mem, flash_static_buffer, flash_static_buffer_size);
    uffs_RegisterMountTable(&fl512s_tables[partition]);
    uffs_Mount(fl512s_tables[partition].mount);

    return GS_OK;
}
