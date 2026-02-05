/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/flash/mcu_flash.h>
#include <gs/util/vmem.h>
#include <gs/util/check.h>
#include <flashc.h>

// linker symbols
extern int _start;

static bool within_flash_range(const void * address, size_t length)
{
    uintptr_t start = (uintptr_t) address;
    return ((start >= AVR32_FLASH_ADDRESS) && (start < (AVR32_FLASH_ADDRESS + AVR32_FLASH_SIZE)));
}

static bool running_from_flash(void)
{
    return within_flash_range(&_start, 0);
}

gs_error_t gs_mcu_flash_write_data(void* to, const void* from, size_t size)
{
    if (running_from_flash() == false) {
        return GS_ERROR_PERM;
    }
    
    flashc_memcpy(to, from, size, true);
    if (flashc_is_programming_error())
        return GS_ERROR_IO;

    return GS_OK;
}

gs_error_t gs_mcu_flash_read_data(void* to, const void* from, size_t size)
{
    memcpy(to, from, size);
    return GS_OK;
}

gs_error_t gs_mcu_flash_write_protect(const void* from, size_t size, bool on)
{
    if (gs_mcu_flash_is_write_protect_supported(from, size) == false) {
        return GS_ERROR_PERM;
    }

    const int page_sz = flashc_get_flash_size() / flashc_get_page_count();
    const uint32_t page_no = ((uint32_t)from - (uint32_t)AVR32_FLASH) / page_sz;
    const int page_count = size / page_sz;

    int current_region = -1;
    for (int i=0; i<page_count; i++) {
        if (current_region != (int)flashc_get_page_region(page_no+i)) {
            current_region = flashc_get_page_region(page_no+i);
            if (flashc_is_region_locked(current_region) != on) {
                flashc_lock_page_region(page_no+i, on);
            }
        }
    }

    if (flashc_is_lock_error()) {
        return GS_ERROR_PERM;
    }

    return GS_OK;
}

bool gs_mcu_flash_is_write_protect_supported(const void* from, size_t size)
{
    if (running_from_flash() == false) {
        return false;
    }

    return within_flash_range(from, size); // should check specific flash data range
}
