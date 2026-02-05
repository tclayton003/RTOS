/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/flash/mcu_flash.h>
#include <gs/util/vmem.h>
#include <gs/util/check.h>
#include <gs/util/mutex.h>
#include <sam0/drivers/nvm/nvm.h>
#include <gs/embed/asf/samc/convert_asf_status_code.h>

#define MCU_FLASH_RWWEE_BASE_ADDR   (NVMCTRL_RWW_EEPROM_ADDR)
#define MCU_FLASH_RWWEE_PAGE_SIZE   (NVMCTRL_PAGE_SIZE)
#define MCU_FLASH_RWWEE_ROW_SIZE    (NVMCTRL_ROW_PAGES * NVMCTRL_PAGE_SIZE)
#define MCU_FLASH_RWWEE_ROWS        (NVMCTRL_RWWEE_PAGES / NVMCTRL_ROW_PAGES)


typedef struct mcu_flash_dev {
    bool initialized;
    gs_mutex_t lock;
    uint8_t row_buf[MCU_FLASH_RWWEE_ROW_SIZE];
} mcu_flash_dev_t;

static mcu_flash_dev_t g_mcu_flash_dev = {.initialized = false};

static gs_error_t samc_mcu_flash_write_row(uint16_t row, const void *src, size_t nbytes)
{
    //printf("Write  called with row, src, size : <%04X> <%08X> <%04X> \n", row, (uintptr_t)src, nbytes);

    /* Erase the ROW first */
    intptr_t row_addr = (intptr_t)(MCU_FLASH_RWWEE_BASE_ADDR + row*MCU_FLASH_RWWEE_ROW_SIZE);
    enum status_code status;
    do
    {
        status = nvm_erase_row(row_addr);
    } while (status == STATUS_BUSY);

    if (status != STATUS_OK)
    {
        log_error("%s: nvm_erase_row(0x%08X) failed, returned=%d", __FUNCTION__, row_addr, status);
        return gs_convert_asf_status_code(status);
    }

    /* Write the ROW, one page at a time */
    size_t page_offset = 0;
    while ((status == STATUS_OK) && (page_offset < nbytes)) {

        const size_t remain_size = (nbytes - page_offset);
        const size_t write_size = (remain_size > MCU_FLASH_RWWEE_PAGE_SIZE) ? MCU_FLASH_RWWEE_PAGE_SIZE : remain_size;

        do {
            status = nvm_write_buffer(row_addr + page_offset, (void*)((int)src + page_offset), write_size);
        } while (status == STATUS_BUSY);

        if (status == STATUS_OK) {
            // re-adjust pointer/data-size
            page_offset += write_size;
        } else {
            log_error("%s: nvm_write_buffer(%u -> %p offset=%u, size=%u) failed, returned=%d", __FUNCTION__, row_addr, src, page_offset, write_size, status);
            return gs_convert_asf_status_code(status);
        }
    }

    /* SAM C20/C21 Errata section 1.14.1 (DS80000740B):
     * We need to invalidate cache after NVM erase/write to ensure correct read
     */
    do {
        status = nvm_execute_command(NVM_COMMAND_INVALL_CACHE, MCU_FLASH_RWWEE_BASE_ADDR, 0);
    } while (status == STATUS_BUSY);

    if (status != STATUS_OK) {
        log_error("%s: nvm_execute_command() failed, returned=%d", __FUNCTION__, status);
        return gs_convert_asf_status_code(status);
    }

    return GS_OK;
}

static gs_error_t samc_mcu_flash_update_row(uint16_t row, uint16_t offset, const void *src, size_t nbytes)
{
    //printf("Update called with row, offset, src, size : <%04X> <%04X> <%08X> <%04X> \n", row, offset, (uintptr_t)src, nbytes);

    uint32_t row_addr = MCU_FLASH_RWWEE_BASE_ADDR + row*MCU_FLASH_RWWEE_ROW_SIZE;

    /* Read the entire ROW to the buffer :
     * Could be optimized to only read the parts that is not overwritten
     */
    memcpy(g_mcu_flash_dev.row_buf,(void*)row_addr, MCU_FLASH_RWWEE_ROW_SIZE);
    /* Update the part of the buffer requested */
    memcpy(g_mcu_flash_dev.row_buf+offset, src, nbytes);

    return samc_mcu_flash_write_row(row, g_mcu_flash_dev.row_buf, sizeof(g_mcu_flash_dev.row_buf));
}

static gs_error_t samc_mcu_flash_memcpy(volatile void *dst, const void *src, size_t nbytes, bool erase)
{
    uint16_t start_row = ((uintptr_t)dst - MCU_FLASH_RWWEE_BASE_ADDR) / MCU_FLASH_RWWEE_ROW_SIZE;
    uint16_t end_row   = ((uintptr_t)dst + nbytes - MCU_FLASH_RWWEE_BASE_ADDR) / MCU_FLASH_RWWEE_ROW_SIZE;
    uint16_t row_cnt   = 1 + (end_row - start_row);

    gs_mutex_lock(g_mcu_flash_dev.lock);
    for (int r=0; r < row_cnt && nbytes > 0; r++)
    {
        size_t offset = (uintptr_t)dst - MCU_FLASH_RWWEE_BASE_ADDR - (r+start_row)*MCU_FLASH_RWWEE_ROW_SIZE;
        size_t wr_size = nbytes > (MCU_FLASH_RWWEE_ROW_SIZE-offset) ? (MCU_FLASH_RWWEE_ROW_SIZE-offset) : nbytes;

        if (wr_size != MCU_FLASH_RWWEE_ROW_SIZE)
            samc_mcu_flash_update_row(start_row+r, offset, src, wr_size);
        else
            samc_mcu_flash_write_row(start_row+r, src, wr_size);

        src = (void*)((int)src + wr_size);
        dst = (void*)((int)dst + wr_size);
        nbytes -= wr_size;
    }
    gs_mutex_unlock(g_mcu_flash_dev.lock);
    return GS_OK;

}

static gs_error_t mcu_flash_initialize()
{
    if (gs_mutex_create(&g_mcu_flash_dev.lock) != GS_OK)
        return GS_ERROR_ALLOC;

    gs_error_t ret = GS_OK;
    struct nvm_config config;
    nvm_get_config_defaults(&config);
    config.manual_page_write = false;

    enum status_code status = nvm_set_config(&config);
    if (status != STATUS_OK) {
        log_error("%s: nvm_set_config() failed, status=%d", __FUNCTION__, status);
        ret = gs_convert_asf_status_code(status);
    }

    g_mcu_flash_dev.initialized = true;
    return ret;
}

gs_error_t gs_mcu_flash_write_data(void* to, const void* from, size_t size)
{
    //printf("MCU Flash write called with dst, src, size : <%08X> <%08X> <%04X> \n", (uintptr_t)to, (uintptr_t)from, size);
    if (!g_mcu_flash_dev.initialized)
        mcu_flash_initialize();

    return samc_mcu_flash_memcpy(to, from, size, true);
}

gs_error_t gs_mcu_flash_read_data(void* to, const void* from, size_t size)
{
    //printf("MCU Flash read called with dst, src, size : <%08X> <%08X> <%04X> \n", (uintptr_t)to, (uintptr_t)from, size);
    memcpy(to, from, size);
    return GS_OK;
}

gs_error_t gs_mcu_flash_write_protect(const void* from, size_t size, bool on)
{
    return GS_ERROR_NOT_SUPPORTED;

    /* This implementation is only valid for the NVM main area. The RWW area
     * is always write-able no matter the region locks.
     */
#if 0
    printf("MCU Flash WP called - On: <%d>, Addr: <0x%08X>\n", on, (uintptr_t)from);
    enum status_code status;

    if (on)
        status = nvm_execute_command(NVM_COMMAND_LOCK_REGION, (uint32_t)from, 0);
    else
        status = nvm_execute_command(NVM_COMMAND_UNLOCK_REGION, (uint32_t)from, 0);

    return gs_convert_asf_status_code(status);
#endif
}
