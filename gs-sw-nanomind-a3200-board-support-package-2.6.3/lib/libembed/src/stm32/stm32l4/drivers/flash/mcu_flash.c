/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/flash/mcu_flash.h>
#include <gs/util/vmem.h>
#include <gs/util/check.h>
#include <gs/util/mutex.h>

#include <stm32l4xx_hal.h>

#define MCU_FLASH_ROW_SIZE          (32*8)
#define MCU_FLASH_ROWS_PER_PAGE     (FLASH_PAGE_SIZE/MCU_FLASH_ROW_SIZE)
#define MCU_FLASH_WORDS_PER_PAGE    (FLASH_PAGE_SIZE/8)

static uint32_t mcu_flash_get_page(uint32_t addr)
{
    return (addr - FLASH_BASE) / FLASH_PAGE_SIZE;
}

static uint32_t mcu_flash_get_page_addr(uint32_t page)
{
    return (page * FLASH_PAGE_SIZE) + FLASH_BASE;
}

static gs_error_t mcu_flash_write_page(uint32_t pg, const void *data)
{
    uint32_t dw_addr = mcu_flash_get_page_addr(pg);
    gs_error_t ret = GS_OK;

    for (uint32_t dw = 0; dw < MCU_FLASH_WORDS_PER_PAGE; dw++)
    {
        uint64_t* dw_value = (((uint64_t*)data) + dw);
        HAL_StatusTypeDef status;
        if ((status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, dw_addr+(dw*8), *dw_value)) != HAL_OK)
        {
            log_error("Failed writing page: <%lu>, offset: <%lu>. Status: <%d>, Error code: <%lu>", pg, dw, status, HAL_FLASH_GetError());
            ret = GS_ERROR_IO;
            break;
        }
    }

    return ret;
    /* Fast programming does not work unless you do MASS Erase */
#if 0
    uint32_t prog_type = FLASH_TYPEPROGRAM_FAST;
    for (uint32_t r=0; r < MCU_FLASH_ROWS_PER_PAGE; r++) {

        if (r == (MCU_FLASH_ROWS_PER_PAGE -1)) {
            prog_type = FLASH_TYPEPROGRAM_FAST_AND_LAST;
        }

        log_debug("Writing page <%lu>. Addr: <%lu>, Data-addr:<%lu>", pg, row_addr, (uint32_t)dat_addr);
        log_debug("Flash regs: SR:<0x%08X> CR: <0x%08X>", (unsigned int)READ_REG(FLASH->SR), (unsigned int)READ_REG(FLASH->CR));
        HAL_StatusTypeDef status;
        if ((status = HAL_FLASH_Program(prog_type, row_addr, dat_addr)) != HAL_OK)
        {
            log_error("Failed writing page: <%lu>, row: <%lu>. Status: <%d>, Error code: <%lu>", pg, r, status, HAL_FLASH_GetError());
            char *tmp = (char*)data;
            log_error("First bytes of data to write: <%02X %02X %02X %02X>", tmp[0], tmp[1], tmp[2], tmp[3]);
            ret = GS_ERROR_IO;
            break;
        }
        row_addr += MCU_FLASH_ROW_SIZE;
        dat_addr += MCU_FLASH_ROW_SIZE;
    }
#endif
}

static gs_error_t mcu_valid_flash_addr(uint32_t addr, uint32_t size)
{
    if (IS_FLASH_MAIN_MEM_ADDRESS(addr) &&
        IS_FLASH_MAIN_MEM_ADDRESS(addr + size -1))
    {
        return GS_OK;
    }

    return GS_ERROR_ARG;
}

static gs_error_t mcu_flash_erase_page(uint32_t pg)
{
    FLASH_EraseInitTypeDef req;
    req.TypeErase = FLASH_TYPEERASE_PAGES;
    req.Banks = FLASH_BANK_1;
    req.NbPages = 1;
    req.Page = pg;
    uint32_t err = 0xFFFFFFFF;
    gs_error_t ret = GS_OK;
    HAL_StatusTypeDef status;

    //log_debug("Flash regs: SR:<0x%08X> CR: <0x%08X>", (unsigned int)READ_REG(FLASH->SR), (unsigned int)READ_REG(FLASH->CR));

    if (((status = HAL_FLASHEx_Erase(&req, &err)) != HAL_OK) || (err != 0xFFFFFFFF)) {
        log_error("Failed erasing page <%lu>. Status: <%d>, Error code: <%lu>", pg, status, HAL_FLASH_GetError());
        ret = GS_ERROR_IO;
    }

    return ret;
}

gs_error_t gs_mcu_flash_write_data(void* to, const void* from, size_t size)
{
    if (mcu_valid_flash_addr((uint32_t)to, size) != GS_OK) {
        return GS_ERROR_ARG;
    }

    uint32_t page_start = mcu_flash_get_page((uint32_t)to);
    uint32_t page_end = mcu_flash_get_page((uint32_t)to + size -1);

    uint8_t  page_buf[FLASH_PAGE_SIZE] __aligned(4);

    uint32_t page_align_offset = (uint32_t)to - mcu_flash_get_page_addr(page_start);


    /* Run through all affected pages, and do a read, modify, erase, write cycle
       for each of the affected pages */
    gs_error_t ret;
    uint32_t bytes_copied = 0;

    for (uint32_t pg = page_start; pg <= page_end; pg++)
    {
        /* 1:
         * Read the entire page content of the affected page to the temp
         * page buffer in RAM.
         * TBD: We could perhaps optimize this read to only copy the part of the page
         * that is not to be updated. This will give up to 2 smaller reads instead of one
         * large.
         */
        gs_mcu_flash_read_data(page_buf, (void*)mcu_flash_get_page_addr(pg), FLASH_PAGE_SIZE);

        /* 2:
         * Modify the content of the RAM page buffer with the content of the from buffer that
         * affects this particular page.
         */
        uint32_t copy_size = FLASH_PAGE_SIZE - page_align_offset;
        if (copy_size > (size - bytes_copied))
            copy_size = (size - bytes_copied);

        memcpy(page_buf + page_align_offset,
               (void*)((uint32_t)from + bytes_copied),
               copy_size);

        page_align_offset = 0;
        bytes_copied += copy_size;

        HAL_FLASH_Unlock();
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
        /* 3:
         * Erase the entire Flash page.
         */
        ret = mcu_flash_erase_page(pg);

        /* 4:
         * Write the entire Flash page, from the local page buffer.
         */
        if (ret == GS_OK) {
            ret = mcu_flash_write_page(pg, page_buf);
        }
        HAL_FLASH_Lock();

        if (ret != GS_OK) break;
    }

    return ret;
}

gs_error_t gs_mcu_flash_read_data(void* to, const void* from, size_t size)
{
    memcpy(to, from, size);
    return GS_OK;
}

gs_error_t gs_mcu_flash_write_protect(const void* from, size_t size, bool on)
{
    if (mcu_valid_flash_addr((uint32_t)from, size) != GS_OK) {
        return GS_ERROR_ARG;
    }

    uint32_t wrp_area;
    if (mcu_flash_get_page((uint32_t)from) == 0) {
        wrp_area = OB_WRPAREA_BANK1_AREAA;
    }
    else {
        wrp_area = OB_WRPAREA_BANK1_AREAB;
    }

    gs_error_t ret = GS_OK;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    HAL_StatusTypeDef status = HAL_FLASH_OB_Unlock();

    if (status == HAL_OK)
    {
        FLASH_OBProgramInitTypeDef req;
        req.OptionType = OPTIONBYTE_WRP;
        req.WRPArea = wrp_area;
        if (on) {
            req.WRPStartOffset = mcu_flash_get_page((uint32_t)from);
            req.WRPEndOffset = mcu_flash_get_page((uint32_t)from + size -1);
        }
        else {
            req.WRPStartOffset = 0xFF;
            req.WRPEndOffset = 0x00;
        }

        FLASH_OBProgramInitTypeDef status_req;
        status_req.OptionType = req.OptionType;
        status_req.WRPArea = req.WRPArea;
        HAL_FLASHEx_OBGetConfig(&status_req);

        /* If the write Protection is different from current protection - Then program it */
        if (status_req.WRPStartOffset != req.WRPStartOffset ||
            status_req.WRPEndOffset != req.WRPEndOffset)
        {
            status = HAL_FLASHEx_OBProgram(&req);
            printf("NOTE: MCU Flash Write protection requires a Power-On-Reset to take effect.\r\n");
        }

#if 0
        if (status == HAL_OK) {
            HAL_FLASH_OB_Launch();
        }
#endif
    }

    HAL_FLASH_Lock();
    HAL_FLASH_OB_Lock();
    if (status != HAL_OK) {
        ret = GS_ERROR_IO;
    }


    return ret;
}
