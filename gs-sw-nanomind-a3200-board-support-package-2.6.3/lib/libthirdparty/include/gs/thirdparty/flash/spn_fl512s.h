#ifndef SPN_FL512S_H_
#define SPN_FL512S_H_
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file 

   Interface for S70FL01GSAGMFI011 (1 Gbit (128 MB) 3.0V SPI Flash).

   The S70FL01S consists of 2 FL512S dies with individual chip selects.
   Each die is 512 Mbit (64 MB) divided into 256 sectors (or blocks) of 256 KB. A sector is the smallest erase unit.
   Each sector is split into 512 pages of 512 bytes.
*/

#include <gs/util/error.h>

/**
   Size of a single FLS512SSize die (bytes).
*/
#define SPN_FL512S_SIZE         (64 * 1024 * 1024)

/**
   Sector (block) size (bytes).
   Minimum erase size.
*/
#define SPN_FL512S_SECTOR_SIZE  (256 * 1024)

/**
   Page size (bytes).
*/
#define SPN_FL512S_PAGE_SIZE    512

/**
   Number of dies/partitions.
*/
#define SPN_FL512S_DIES         2

/**
   Configuration for FL512S
*/
typedef struct spn_fl512s_config {
    /**
       Chip select for partition 0 (die 0).
    */
    uint8_t cs_part_0;
    /**
       Chip select for partition 1 (die 1).
    */
    uint8_t cs_part_1;
    /**
       Slave number for partition 0 (-> gs_spi_master_transactions()).
    */
    uint8_t spi_slave_part_0;
    /**
       Slave number for partition 1 (-> gs_spi_master_transactions()).
    */
    uint8_t spi_slave_part_1;
    /**
       SPI device/handle.
    */
    uint8_t spi_handle;
    /**
       Transfer rate, bits per second.
    */
    uint32_t bps;
} spn_fl512s_config_t;

/**
   Initialize SPI interface.
   @param[in] config configuration for FL512S
   @return_gs_error_t
*/
gs_error_t spn_fl512s_init(const spn_fl512s_config_t * config);

/**
   Read status register.
   @param[in] partition partition (0 | 1)
   @param[out] status status value
   @return_gs_error_t
*/
gs_error_t spn_fl512s_read_status_register(uint8_t partition, uint8_t * status);

/**
   Read device id.
   @param[in] partition partition (0 | 1)
   @param[out] data user supplied buffer for returning device id.
   @return_gs_error_t
*/
gs_error_t spn_fl512s_read_device_id(uint8_t partition, char data[64]);

/**
   Read data.
   @param[in] partition partition (0 | 1)
   @param[in] addr address to read from
   @param[out] data user supplied buffer for returning data.
   @param[in] len length of data to read (bytes).
   @return_gs_error_t
*/
gs_error_t spn_fl512s_read_data(uint8_t partition, uint32_t addr, uint8_t *data, uint16_t len);

/**
   Write data.
   @param[in] partition partition (0 | 1)
   @param[in] addr address to write to
   @param[in] data data to write
   @param[in] len length of data to write (bytes).
   @return_gs_error_t
*/
gs_error_t spn_fl512s_write_data(uint8_t partition, uint32_t addr, const uint8_t *data, uint16_t len);

/**
   Erase block.
   @param[in] partition partition (0 | 1)
   @param[in] addr the block address to erase (if within a block the entire block is erased).
   @return_gs_error_t
*/
gs_error_t spn_fl512s_erase_block(uint8_t partition, uint32_t addr);

/**
   Erase entire partition/die.
   Takes up to 2 minutes.
   @param[in] partition partition (0 | 1)
   @return_gs_error_t
*/
gs_error_t spn_fl512s_erase_chip(uint8_t partition);

/**
   Register commands.
*/
gs_error_t gs_spn_fl512s_register_commands(void);

#endif
