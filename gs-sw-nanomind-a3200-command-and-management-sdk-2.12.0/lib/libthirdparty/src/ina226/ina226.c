/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/ina226/ina226.h>
#include <gs/util/drivers/i2c/master.h>
#include <gs/util/error.h>
#include <gs/util/log.h>
#include <gs/util/byteorder.h>

typedef struct {
    bool     in_use;
    uint8_t  bus;
    uint8_t  addr;
    uint16_t bus_volt;
    int16_t  shunt_volt;
    int16_t  current;
    uint32_t current_lsb;
    int16_t  power;
}gs_ina226_device_handle_t;

typedef enum {
    REG_CFG        = 0,
    REG_SHUNT_VOLT = 1,
    REG_BUS_VOLT   = 2,
    REG_PWR        = 3,
    REG_CURRENT    = 4,
    REG_CALIB      = 5,
    REG_MASK       = 6,
    REG_ALERT      = 7,
    REG_MANUF_ID   = 0xfe,
    REG_DIE_ID     = 0xff
}gs_ina226_regs_t;

#define INA226_MAX_HANDLES 16
static const uint8_t ina226_first_addr = 0x40;
static const uint8_t ina226_last_addr  = 0x4f;

static gs_ina226_device_handle_t g_hdl[INA226_MAX_HANDLES];

gs_error_t gs_ina226_init(const uint8_t bus, const uint8_t addr, uint8_t * const handle)
{
    if ((addr < ina226_first_addr) || addr > ina226_last_addr) {
        return GS_ERROR_ARG;
    }

    int8_t hdl=-1;
    for (uint8_t i=0; i < INA226_MAX_HANDLES; i++) {
        if (!g_hdl[i].in_use) {
            hdl = i;
            break;
        }
    }
    if (hdl < 0) {
        return GS_ERROR_HANDLE;
    };
    g_hdl[hdl].in_use = true;
    g_hdl[hdl].bus    = bus;
    g_hdl[hdl].addr   = addr;
    *handle = hdl;

    uint8_t cmd[1];
    uint8_t reply[2];
    cmd[0] = REG_MANUF_ID;
    if (gs_i2c_master_transaction(g_hdl[hdl].bus, g_hdl[hdl].addr, cmd, sizeof(cmd), reply, 2, 1000) != GS_OK) {
        g_hdl[hdl].in_use = false;
        log_error("INA226 error reading device register %02X %02X %02X", g_hdl[hdl].bus, g_hdl[hdl].addr, cmd[0]);
        return GS_ERROR_IO;
    }

    uint16_t manuf_reg = reply[0] << 8 | reply[1];
    cmd[0] = REG_DIE_ID;
    if (gs_i2c_master_transaction(g_hdl[hdl].bus, g_hdl[hdl].addr, cmd, sizeof(cmd), reply, 2, 1000) != GS_OK) {
        g_hdl[hdl].in_use = false;
        log_error("INA226 error reading device register %02X %02X %02X", g_hdl[hdl].bus, g_hdl[hdl].addr, cmd[0]);
        return GS_ERROR_IO;
    }
    uint16_t die_reg = reply[0] << 8 | reply[1];

    cmd[0] = REG_CFG;
    if (gs_i2c_master_transaction(g_hdl[hdl].bus, g_hdl[hdl].addr, cmd, sizeof(cmd), reply, 2, 1000) != GS_OK) {
        g_hdl[hdl].in_use = false;
        log_error("INA226 error reading device register %02X %02X %02X", g_hdl[hdl].bus, g_hdl[hdl].addr, cmd[0]);
        return GS_ERROR_IO;
    }
    uint16_t cfg_reg = reply[0] << 8 | reply[1];
    log_trace("INA226 identified at %02X (%04X, %04X, %04X)", g_hdl[hdl].addr, manuf_reg, die_reg, cfg_reg);

    return GS_OK;
}

static gs_error_t gs_ina226_read_data_register(const uint8_t handle, gs_ina226_regs_t reg, uint16_t * const value)
{
    uint8_t cmd[1];

    cmd[0] = reg;
    if (gs_i2c_master_transaction(g_hdl[handle].bus, g_hdl[handle].addr, cmd, sizeof(cmd), value, sizeof(*value), 1000) != GS_OK) {
        log_error("INA226 error reading device register %02X %02X %02X", g_hdl[handle].bus, g_hdl[handle].addr, cmd[0]);
        return GS_ERROR_IO;
    }

    *value = util_betoh16(*value);

    return GS_OK;
}

gs_error_t gs_ina226_sample(const uint8_t handle)
{
    uint16_t bus_volt;
    int32_t shunt_volt;

    return gs_ina226_read_parameters(handle, &bus_volt, &shunt_volt, NULL, NULL, false);
}

gs_error_t gs_ina226_read(const uint8_t handle, uint16_t * const bus_volt, int32_t * const shunt_volt)
{
    return gs_ina226_read_parameters(handle, bus_volt, shunt_volt, NULL, NULL, true);
}

gs_error_t gs_ina226_read_voltage(const uint8_t handle, uint16_t * const bus_volt, int32_t * const shunt_volt, bool cached)
{
    return gs_ina226_read_parameters(handle, bus_volt, shunt_volt, NULL, NULL, cached);
}

gs_error_t gs_ina226_read_current(const uint8_t handle, int32_t * const current, bool cached)
{
    return gs_ina226_read_parameters(handle, NULL, NULL, current, NULL, cached);
}

gs_error_t gs_ina226_read_power(const uint8_t handle, int32_t * const power, bool cached)
{
    return gs_ina226_read_parameters(handle, NULL, NULL, NULL, power, cached);
}

gs_error_t gs_ina226_read_parameters(const uint8_t handle, uint16_t * const bus_volt, int32_t * const shunt_volt,
                                     int32_t * const current, int32_t * const power, bool cached)
{
    gs_error_t ret;

    if (handle >= INA226_MAX_HANDLES || !g_hdl[handle].in_use) {
        return GS_ERROR_HANDLE;
    }

    if (bus_volt) {
        if (!cached) {
            ret = gs_ina226_read_data_register(handle, REG_BUS_VOLT, &g_hdl[handle].bus_volt);
            if (ret != GS_OK) {
                return ret;
            }
        }

        *bus_volt = (uint16_t)(((float)g_hdl[handle].bus_volt)*1.25);
    }
    if (shunt_volt) {
        if (!cached) {
            ret = gs_ina226_read_data_register(handle, REG_SHUNT_VOLT, (uint16_t *)&g_hdl[handle].shunt_volt);
            if (ret != GS_OK) {
                return ret;
            }
        }

        *shunt_volt = (int32_t)(((float)g_hdl[handle].shunt_volt)*2.5);
    }
    if (current) {
        if (!cached) {
            ret = gs_ina226_read_data_register(handle, REG_CURRENT, (uint16_t *)&g_hdl[handle].current);
            if (ret != GS_OK) {
                return ret;
            }
        }

        *current = g_hdl[handle].current * g_hdl[handle].current_lsb;
    }
    if (power) {
        if (!cached) {
            ret = gs_ina226_read_data_register(handle, REG_PWR, (uint16_t *)&g_hdl[handle].power);
            if (ret != GS_OK) {
                return ret;
            }
        }

        /* Power LSB is a fixed ratio of 25 to the current LSB */
        *power = g_hdl[handle].power * g_hdl[handle].current_lsb * 25;
    }

    return GS_OK;
}

gs_error_t gs_ina226_write_calibration(const uint8_t handle, uint16_t calibration, uint16_t current_lsb)
{
    if (handle >= INA226_MAX_HANDLES || !g_hdl[handle].in_use) {
        return GS_ERROR_HANDLE;
    }
    uint8_t cmd[3];

    cmd[0] = REG_CALIB;
    cmd[1] = (calibration >> 8) & 0xFF;
    cmd[2] = calibration & 0xFF;
    if (gs_i2c_master_transaction(g_hdl[handle].bus, g_hdl[handle].addr, cmd, sizeof(cmd), NULL, 0, 1000) != GS_OK) {
        log_error("INA226 error writing device register %02X %02X %02X", g_hdl[handle].bus, g_hdl[handle].addr, cmd[0]);
        return GS_ERROR_IO;
    }

    g_hdl[handle].current_lsb = current_lsb;

    return GS_OK;
}
