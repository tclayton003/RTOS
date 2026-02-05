/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/flash/spn_fl512s.h>
#include <gs/embed/asf/drivers/spi/master.h>
#include <gs/util/time.h>
#include <gs/util/string.h>
#include <gs/util/mutex.h>

/* FL512S op-codes */
#define SPN_FL512S_CMD_WREN           0x06  // Write enable
#define SPN_FL512S_CMD_WRDI           0x04  // Write disable
#define SPN_FL512S_CMD_WRSR           0x01  // Write register
#define SPN_FL512S_CMD_RDSR           0x05  // Read status register 1
#define SPN_FL512S_CMD_READ           0x13  // Read w/ 4 byte address
#define SPN_FL512S_CMD_FAST_READ      0x0B  // Fast read 
#define SPN_FL512S_CMD_RDID           0x9F  // Read ID
#define SPN_FL512S_CMD_SE             0xDC  // Sector erase w/ 4 byte addr (256kb)
#define SPN_FL512S_CMD_BE             0xC7  // Bulk erase
#define SPN_FL512S_CMD_PP             0x12  // Page program w/ 4 byte addr
#define SPN_FL512S_CMD_DP             0xB9  // Bank register access
#define SPN_FL512S_CMD_RES            0xAB  // Read electronic signature

#define DEFAULT_TIMEOUT_MS            1000  // default timeout for operations

static gs_mutex_t spn_fl512s_lock;
static uint8_t spn_partition_to_spi_slave[SPN_FL512S_DIES] = {UINT8_MAX, UINT8_MAX};

__attribute__((always_inline)) static inline uint8_t find_slave(uint8_t partition)
{
    if (partition < SPN_FL512S_DIES) {
        return spn_partition_to_spi_slave[partition];
    }
    return UINT8_MAX;
}

gs_error_t spn_fl512s_init(const spn_fl512s_config_t * config)
{
    gs_spi_chip_t spi_chip = {
        .handle = config->spi_handle,
        .baudrate = config->bps,
        .bits = 8,
        .spck_delay = 0,
        .spi_mode = GS_SPI_MODE_CPOL0_CPHA0,
        .stay_act = 1,
        .trans_delay = 0,
    };

    for (uint8_t partition = 0; partition < SPN_FL512S_DIES; ++partition) {
        spn_partition_to_spi_slave[partition] = (partition == 0) ? config->spi_slave_part_0 : config->spi_slave_part_1;
        spi_chip.chipselect = (partition == 0) ? config->cs_part_0 : config->cs_part_1;

        const uint8_t slave = find_slave(partition);
        if (slave < UINT8_MAX) {
            gs_error_t error = gs_spi_asf_master_configure_slave(slave, &spi_chip);
            if (error) {
                log_error("%s: failed to configure SPI for partition %u, spi slave: %u, chip-select: %u, error: %s",
                          __FUNCTION__, partition, slave, spi_chip.chipselect, gs_error_string(error));
                return error;
            }
        }
    }

    return gs_mutex_create(&spn_fl512s_lock);
}

static gs_error_t spn_fl512s_wait_for_status_ok(uint8_t partition, uint32_t timeout_ms)
{
    const uint8_t slave = find_slave(partition);

    uint8_t cmd[1] = {SPN_FL512S_CMD_RDSR};
    uint8_t status;
    gs_spi_master_trans_t trans[2] = {{
            .tx = cmd,
            .rx = NULL,
            .size = sizeof(cmd),
        } , {
            .tx = NULL,
            .rx = &status,
            .size = sizeof(status),
        }};

    gs_error_t res;

    uint32_t start = gs_time_rel_ms();
    uint32_t now = start;
    while(gs_time_diff_ms(start, now) < timeout_ms) {
        res = gs_spi_master_transactions(slave, trans, 2, -1);
        now = gs_time_rel_ms();
        if (res != GS_OK) {
            return res;
        }
        if (status & 0x60) {
            return GS_ERROR_IO;
        }
        if (status & 0x01) {
            continue;
        }
        return GS_OK;
    }
    return GS_ERROR_TIMEOUT;
}

gs_error_t spn_fl512s_read_status_register(uint8_t partition, uint8_t * pstatus)
{
    const uint8_t slave = find_slave(partition);

    uint8_t cmd[1] = {SPN_FL512S_CMD_FAST_READ};
    uint8_t status;
    gs_spi_master_trans_t trans[2] = {{
            .tx = cmd,
            .rx = NULL,
            .size = sizeof(cmd),
        } , {
            .tx = NULL,
            .rx = &status,
            .size = sizeof(status),
        }};

    gs_error_t error = gs_mutex_lock(spn_fl512s_lock);
    if (error == GS_OK) {
        error = gs_spi_master_transactions(slave, trans, 2, -1);
        gs_mutex_unlock(spn_fl512s_lock);

        *pstatus = status;
    }

    return error;
}

gs_error_t spn_fl512s_read_device_id(uint8_t partition, char data[64])
{
    const uint8_t slave = find_slave(partition);

    uint8_t cmd[1] = {SPN_FL512S_CMD_RDID};
    gs_spi_master_trans_t trans[2] = {{
            .tx = cmd,
            .rx = NULL,
            .size = sizeof(cmd),
        } , {
            .tx = NULL,
            .rx = data,
            .size = 64,
        }};

    gs_error_t error = gs_mutex_lock(spn_fl512s_lock);
    if (error == GS_OK) {
        error = gs_spi_master_transactions(slave, trans, 2, -1);
        gs_mutex_unlock(spn_fl512s_lock);
    }
    return error;
}

gs_error_t spn_fl512s_read_data(uint8_t partition, uint32_t addr, uint8_t *data, uint16_t len)
{
    const uint8_t slave = find_slave(partition);

    if ((addr + len) > SPN_FL512S_SIZE) {
        return GS_ERROR_RANGE;
    }

    uint8_t cmd[5] = {SPN_FL512S_CMD_READ, (addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    gs_spi_master_trans_t trans[2] = {{
            .tx = cmd,
            .rx = NULL,
            .size = sizeof(cmd),
        } , {
            .tx = NULL,
            .rx = data,
            .size = len,
        }};

    gs_error_t error = gs_mutex_lock(spn_fl512s_lock);
    if (error == GS_OK) {
        error = gs_spi_master_transactions(slave, trans, 2, -1);
        gs_mutex_unlock(spn_fl512s_lock);
    }
    if (error != GS_OK) {
        memset(data, 0, len);
    }

    return error;
}

gs_error_t spn_fl512s_write_data(uint8_t partition, uint32_t addr, const uint8_t *data, uint16_t len)
{
    const uint8_t slave = find_slave(partition);

    if ((addr + len) > SPN_FL512S_SIZE) {
        return GS_ERROR_RANGE;
    }

    uint8_t cmd_0[1] = {SPN_FL512S_CMD_WREN};
    uint8_t cmd_1[5] = {SPN_FL512S_CMD_PP, (addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    gs_spi_master_trans_t trans_0 = {
        .tx = cmd_0,
        .rx = NULL,
        .size = sizeof(cmd_0),
    };
    gs_spi_master_trans_t trans_1[2] = {{
            .tx = cmd_1,
            .rx = NULL,
            .size = sizeof(cmd_1),
        }, {
            .tx = data,
            .rx = NULL,
            .size = len,
        }};

    gs_error_t error = gs_mutex_lock(spn_fl512s_lock);
    if (error == GS_OK) {
        error = gs_spi_master_transactions(slave, &trans_0, 1, -1);
        if (error == GS_OK) {
            error = gs_spi_master_transactions(slave, trans_1, 2, -1);
            if (error == GS_OK) {
                error = spn_fl512s_wait_for_status_ok(partition, DEFAULT_TIMEOUT_MS);
            }
        }
        gs_mutex_unlock(spn_fl512s_lock);
    }

    return error;
}

gs_error_t spn_fl512s_erase_block(uint8_t partition, uint32_t addr)
{
    const uint8_t slave = find_slave(partition);

    uint8_t cmd_0[1] = {SPN_FL512S_CMD_WREN};
    uint8_t cmd_1[5] = {SPN_FL512S_CMD_SE, (addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    gs_spi_master_trans_t trans_0 = {
        .tx = cmd_0,
        .rx = NULL,
        .size = sizeof(cmd_0),
    };
    gs_spi_master_trans_t trans_1 = {
        .tx = cmd_1,
        .rx = NULL,
        .size = sizeof(cmd_1),
    };

    gs_error_t error = gs_mutex_lock(spn_fl512s_lock);
    if (error == GS_OK) {
        error = gs_spi_master_transactions(slave, &trans_0, 1, -1);
        if (error == GS_OK) {
            error = gs_spi_master_transactions(slave, &trans_1, 1, -1);
            if (error == GS_OK) {
                error = spn_fl512s_wait_for_status_ok(partition, DEFAULT_TIMEOUT_MS);
            }
        }
        gs_mutex_unlock(spn_fl512s_lock);
    }

    return error;
}

gs_error_t spn_fl512s_erase_chip(uint8_t partition)
{
    const uint8_t slave = find_slave(partition);

    uint8_t cmd_0[1] = {SPN_FL512S_CMD_WREN};
    uint8_t cmd_1[1] = {SPN_FL512S_CMD_BE};

    gs_spi_master_trans_t trans_0 = {
        .tx = cmd_0,
        .rx = NULL,
        .size = sizeof(cmd_0),
    };
    gs_spi_master_trans_t trans_1 = {
        .tx = cmd_1,
        .rx = NULL,
        .size = sizeof(cmd_1),
    };

    gs_error_t error = gs_mutex_lock(spn_fl512s_lock);
    if (error == GS_OK) {
        error = gs_spi_master_transactions(slave, &trans_0, 1, -1);
        if (error == GS_OK) {
            error = gs_spi_master_transactions(slave, &trans_1, 1, -1);
            if (error == GS_OK) {
                error = spn_fl512s_wait_for_status_ok(partition,  5 * 60 * 1000);
            }
        }
        gs_mutex_unlock(spn_fl512s_lock);
    }

    return error;
}
