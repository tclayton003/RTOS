#ifndef GS_THIRDPARTY_FRAM_FM33256B_H
#define GS_THIRDPARTY_FRAM_FM33256B_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

/**
    @file

    Driver for Cypress FM33256B 256 KBit (32K x 8).

    This driver provides interfaces compatible with the generic API's for VMEM and RTC.

    Datasheet: https://www.cypress.com/file/126881/download
*/

#include <gs/util/rtc.h>
#include <gs/util/vmem.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   FRAM protect regions.
*/
typedef enum gs_fm33256b_fram_region
{
    GS_FM33256B_PROTECT_NONE = 0,
    GS_FM33256B_PROTECT_UPPER_QUARTER = 1,
    GS_FM33256B_PROTECT_UPPER_HALF = 2,
    GS_FM33256B_PROTECT_ALL = 3
} gs_fm33256b_fram_region_t;

/**
    FRAM driver configuration struct
*/
typedef struct gs_fm33256b_config
{
    /**
       SPI slave handle.
    */
    uint8_t spi_slave;
} gs_fm33256b_config_t;

/**
    Initialize the FRAM device.

    @param[in] device A handle for the FRAM device.
    @param[in] config A configuration struct the FRAM device.
    @return_gs_error_t
*/
gs_error_t gs_fm33256b_init(uint8_t device, const gs_fm33256b_config_t *config);


/**
    Writes to the FRAM device.

    @param[in] device A handle for the FRAM device.
    @param[in] to     Address in FRAM device to write.
    @param[in] from   Pointer to data to write to FRAM.
    @param[in] len    Number of bytes to write.
    @return_gs_error_t
*/
gs_error_t gs_fm33256b_fram_write(uint8_t device, uint32_t to, const void *from, size_t len);

/**
    Reads from the FRAM device.

    @param[in] device A handle for the FRAM device.
    @param[in] to     Address in FRAM device to read.
    @param[in] from   Pointer to data to write from FRAM.
    @param[in] len    Number of bytes to write.
    @return_gs_error_t
*/
gs_error_t gs_fm33256b_fram_read(uint8_t device, uint32_t from, void *to, size_t len);

/**
    Write-protect a region of the FRAM device. Needs reset of the device to clear
    the write protection.

    @param[in] device A handle for the FRAM device.
    @param[in] region The region to protect.
    @return_gs_error_t
*/
gs_error_t gs_fm33256b_fram_protect_region(uint8_t device, gs_fm33256b_fram_region_t region);

/**
   RTC driver interface.

   Call gs_rtc_register(rtc_driver, rtc_data) to register driver. Afterwards gs_rtc_set_time()/gs_rtc_get_time()
   operates on the RTC device in the fm33256b.
*/
extern const gs_rtc_driver_t gs_fm33256b_rtc_driver;

/**
   RTC driver data.
*/
typedef struct {
    /**
       FRAM device, normally 0.
    */
    uint8_t device;
} gs_fm33256b_rtc_driver_data_t;

/**
   VMEM driver interface.
*/
extern const gs_vmem_driver_t gs_fm33256b_vmem_driver;

/**
   VMEM driver data.
*/
typedef struct {
    /**
       FRAM device, normally 0.
    */
    uint8_t device;
    /**
       FRAM Region to protect, see gs_fm33256b_fram_protect_region().
    */
    gs_fm33256b_fram_region_t protect;
} gs_fm33256b_vmem_driver_data_t;

/**
   Register commands.
   @return_gs_error_t
*/
gs_error_t gs_fm33256b_register_commands(void);
    
#ifdef __cplusplus
}
#endif
#endif
