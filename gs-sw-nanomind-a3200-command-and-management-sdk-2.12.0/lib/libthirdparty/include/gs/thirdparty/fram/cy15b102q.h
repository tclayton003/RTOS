#ifndef GS_THIRDPARTY_FRAM_CY15B102Q_H_
#define GS_THIRDPARTY_FRAM_CY15B102Q_H_
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

/**
   @file

   Driver for CY15B102Q 2 MBit (256K x 8) device (FRAM).
*/

#include <gs/util/error.h>
#include <gs/util/vmem.h>
#include <gs/util/drivers/gpio/gpio.h>

/**
   Write protect regions.
*/
typedef enum gs_cy15b102q_fram_region
{
    GS_CY15B102Q_PROTECT_NONE = 0,
    GS_CY15B102Q_PROTECT_UPPER_QUARTER = 1,
    GS_CY15B102Q_PROTECT_UPPER_HALF = 2,
    GS_CY15B102Q_PROTECT_ALL = 3
} gs_cy15b102q_fram_region_t;

/**
   Driver configuration structure.
*/
typedef struct gs_cy15b102q_fram_config
{
    /**
       Pre-initialized SPI slave.
    */
    uint8_t spi_slave;
    /**
       Enable write protect pin \a write_protect.
    */
    bool enable_write_protect;
    /**
       Write protect pin.
       \a enable_write_protect must be \a true.
    */
    gs_gpio_t write_protect;
} gs_cy15b102q_fram_config_t;

/**
   Initialize the device.

   @param[in] device A handle for the FRAM device.
   @param[in] config A config structure for FRAM device.
   @return_gs_error_t
*/
gs_error_t gs_cy15b102q_fram_init(uint8_t device, const gs_cy15b102q_fram_config_t *config);

/**
   Writes to the device.

   @param[in] device A handle for the FRAM device.
   @param[in] to     Address in FRAM device to write.
   @param[in] from   Pointer to data to write to FRAM.
   @param[in] len    Number of bytes to write.
   @return_gs_error_t
*/
gs_error_t gs_cy15b102q_fram_write(uint8_t device, uint32_t to, const void *from, size_t len);

/**
   Reads from the device.

   @param[in] device A handle for the FRAM device.
   @param[in] to     Address in FRAM device to read.
   @param[in] from   Pointer to data to write from FRAM.
   @param[in] len    Number of bytes to write.
   @return_gs_error_t
*/
gs_error_t gs_cy15b102q_fram_read(uint8_t device, uint32_t from, void *to, size_t len);

/**
   Write-protect a region of the device.

   Needs reset of the device to clear the write protection.

   @param[in] device A handle for the FRAM device.
   @param[in] region The region to protect.

   @return_gs_error_t
*/
gs_error_t gs_cy15b102q_fram_protect_region(uint8_t device, gs_cy15b102q_fram_region_t region);

/**
   VMEM driver.
   @{
*/
/**
   VMEM driver interface.
*/
extern const gs_vmem_driver_t gs_cy15b102q_vmem_driver;

/**
   VMEM driver data.
*/
typedef struct {
    /**
       Device, normally 0.
    */
    uint8_t device;
    /**
       Region to protect.
    */
    gs_cy15b102q_fram_region_t protect;
} gs_cy15b102q_vmem_driver_data_t;
/** @} */

/**
   Register commands.
   @return_gs_error_t
*/
gs_error_t gs_cy15b102q_register_commands(void);

/**
   Register commands.
   @deprecated use gs_cy15b102q_register_commands()
*/
static inline void gs_cmd_cy15b102q_setup(void)
{
    gs_cy15b102q_register_commands();
}

#endif
