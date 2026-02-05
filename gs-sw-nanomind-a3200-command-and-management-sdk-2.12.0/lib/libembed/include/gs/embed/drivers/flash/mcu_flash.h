#ifndef GS_EMBED_DRIVERS_FLASH_MCU_FLASH_H
#define GS_EMBED_DRIVERS_FLASH_MCU_FLASH_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   MCU Flash API - read/write on-board flash.

   @note AVR32: On-board FLASH is read-only and protect status cannot be modified, when running from a RAM image.
*/

#include <gs/util/error.h>
#include <gs/util/vmem.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Write data to internal flash.

   @note AVR32: On-board FLASH is read-only, when running from a RAM image.
   @param[in] to target address
   @param[in] from source address
   @param[in] size number of bytes to write.
   @return_gs_error_t
*/
gs_error_t gs_mcu_flash_write_data(void* to, const void* from, size_t size);

/**
   Read data from internal flash.

   @param[in] to target address
   @param[in] from source address
   @param[in] size number of bytes to read.
   @return_gs_error_t
*/
gs_error_t gs_mcu_flash_read_data(void* to, const void* from, size_t size);

/**
   Turn protection on/off on segment.

   @param[in] from from address.
   @param[in] size number of bytes.
   @param[in] on \a true to turn protection on, \a false to turn off.
   @return_gs_error_t
*/
gs_error_t gs_mcu_flash_write_protect(const void* from, size_t size, bool on);

/**
   Check state of protrection.

   @param[in] from from address.
   @param[in] size number of bytes.
   @return \a true if protected, \a false if not.
*/
bool gs_mcu_flash_is_write_protect_supported(const void* from, size_t size);

/**
   VMEM driver for MCU flash area.
*/
extern const gs_vmem_driver_t gs_mcu_flash_vmem_driver;

#ifdef __cplusplus
}
#endif
#endif
