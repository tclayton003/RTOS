/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/flash/mcu_flash.h>
#include <gs/util/vmem.h>
#include <gs/util/check.h>
#include <gs/util/bytebuffer.h>

/* MCU flash interface is implemented in the MCU specific folders (stm32, asf, ...) */

/* VMEM driver implementation for the MCU Flash */

static gs_error_t mcu_flash_vmem_write(const gs_vmem_t * vmem, void* to, const void * from, size_t size)
{
    return gs_mcu_flash_write_data(to, from, size);
}

static gs_error_t mcu_flash_vmem_read(const gs_vmem_t * vmem, void* to, const void * from, size_t size)
{
    return gs_mcu_flash_read_data(to, from, size);
}

static gs_error_t mcu_flash_vmem_lock(const gs_vmem_t * vmem, bool on)
{
    return gs_mcu_flash_write_protect(vmem->physmem.p, vmem->size, on);
}

// Default it is assumed write protection is not supported - platform must overwrite if it is
__attribute__((weak)) bool gs_mcu_flash_is_write_protect_supported(const void* from, size_t size)
{
    return false;
}

static gs_error_t mcu_flash_vmem_info(const gs_vmem_t * vmem, char * buffer, size_t buffer_size)
{
    gs_bytebuffer_t bb;
    gs_bytebuffer_init(&bb, buffer, buffer_size);
    gs_bytebuffer_printf(&bb, "MCU flash: 0x%08x-0x%08x, lockable: %s",
                         (unsigned int) vmem->physmem.u, (unsigned int)((vmem->size > 0) ? (vmem->physmem.u + vmem->size - 1) : vmem->physmem.u),
                         gs_mcu_flash_is_write_protect_supported(vmem->physmem.p, vmem->size) ? "yes" : "no");
    return GS_OK;
}

const gs_vmem_driver_t gs_mcu_flash_vmem_driver = {
    .write = mcu_flash_vmem_write,
    .read  = mcu_flash_vmem_read,
    .lock  = mcu_flash_vmem_lock,
    .info  = mcu_flash_vmem_info,
};
