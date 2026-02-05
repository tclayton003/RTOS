/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/isis_ants/isis_ants_driver.h>
#include <gs/util/log.h>
#include <gs/util/error.h>
#include <gs/util/drivers/i2c/master.h>

#include "local.h"

#define I2C_DEVICE 2

gs_error_t gs_isis_ants_deploy_status(uint8_t i2c_addr, gs_isis_ants_status_t * status)
{
    uint8_t tx[1], rx[2], tx_len, rx_len;

    tx[0] = ISIS_ANTS_CMD_STATUS_DEPLOY;
    tx_len = 1;
    rx_len = 2;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    status->armed = rx[0] & 0x01;
    status->switch_ignore = rx[1] & 0x01;
    status->independent_burn = (rx[0] & 0x10) >> 4;

    status->ant[0].not_deployed = (rx[1] & 0x80) >> 7;
    status->ant[0].time_limit_reached = (rx[1] & 0x40) >> 6;
    status->ant[0].deployment_active = (rx[1] & 0x20) >> 5;

    status->ant[1].not_deployed = (rx[1] & 0x08) >> 3;
    status->ant[1].time_limit_reached = (rx[1] & 0x04) >> 2;
    status->ant[1].deployment_active = (rx[1] & 0x02) >> 1;

    status->ant[2].not_deployed = (rx[0] & 0x80) >> 7;
    status->ant[2].time_limit_reached = (rx[0] & 0x40) >> 6;
    status->ant[2].deployment_active = (rx[0] & 0x20) >> 5;

    status->ant[3].not_deployed = (rx[0] & 0x08) >> 3;
    status->ant[3].time_limit_reached = (rx[0] & 0x04) >> 2;
    status->ant[3].deployment_active = (rx[0] & 0x02) >> 1;

    return GS_OK;
}

gs_error_t gs_isis_ants_status(uint8_t i2c_addr, gs_isis_ants_status_t * status)
{
    uint8_t tx[1], rx[2], tx_len, rx_len;

    tx[0] = ISIS_ANTS_CMD_STATUS_DEPLOY;
    tx_len = 1;
    rx_len = 2;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    status->armed = rx[0] & 0x01;
    status->switch_ignore = rx[1] & 0x01;

    status->ant[0].not_deployed = (rx[1] & 0x80) >> 7;
    status->ant[0].time_limit_reached = (rx[1] & 0x40) >> 6;
    status->ant[0].deployment_active = (rx[1] & 0x20) >> 5;

    status->ant[1].not_deployed = (rx[1] & 0x08) >> 3;
    status->ant[1].time_limit_reached = (rx[1] & 0x04) >> 2;
    status->ant[1].deployment_active = (rx[1] & 0x02) >> 1;

    status->ant[2].not_deployed = (rx[0] & 0x80) >> 7;
    status->ant[2].time_limit_reached = (rx[0] & 0x40) >> 6;
    status->ant[2].deployment_active = (rx[0] & 0x20) >> 5;

    status->ant[3].not_deployed = (rx[0] & 0x08) >> 3;
    status->ant[3].time_limit_reached = (rx[0] & 0x04) >> 2;
    status->ant[3].deployment_active = (rx[0] & 0x02) >> 1;

    tx[0] = ISIS_ANTS_CMD_COUNT_1;
    tx_len = 1;
    rx_len = 1;

    i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }
    status->ant[0].activation_count = rx[0];

    tx[0] = ISIS_ANTS_CMD_COUNT_2;
    tx_len = 1;
    rx_len = 1;

    i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }
    status->ant[1].activation_count = rx[0];

    tx[0] = ISIS_ANTS_CMD_COUNT_3;
    tx_len = 1;
    rx_len = 1;

    i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }
    status->ant[2].activation_count = rx[0];

    tx[0] = ISIS_ANTS_CMD_COUNT_4;
    tx_len = 1;
    rx_len = 1;

    i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }
    status->ant[3].activation_count = rx[0];

    tx[0] = ISIS_ANTS_CMD_TIME_1;
    tx_len = 1;
    rx_len = 2;

    i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }
    status->ant[0].activation_time = (rx[1] << 8) | rx[0];

    tx[0] = ISIS_ANTS_CMD_TIME_2;
    tx_len = 1;
    rx_len = 2;

    i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }
    status->ant[1].activation_time = (rx[1] << 8) | rx[0];

    tx[0] = ISIS_ANTS_CMD_TIME_3;
    tx_len = 1;
    rx_len = 2;

    i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }
    status->ant[2].activation_time = (rx[1] << 8) | rx[0];

    tx[0] = ISIS_ANTS_CMD_TIME_4;
    tx_len = 1;
    rx_len = 2;

    i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }
    status->ant[3].activation_time = (rx[1] << 8) | rx[0];

    return GS_OK;
}

const uint16_t kVcc = 3300;
gs_error_t gs_isis_ants_temp(uint8_t i2c_addr, uint16_t * v_out)
{

    uint8_t tx[1], rx[2], tx_len, rx_len;

    tx[0] = ISIS_ANTS_CMD_TEMP;
    tx_len = 1;
    rx_len = 2;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, &rx, rx_len, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    float conversion_factor = kVcc/1023.0;
    *v_out = ((rx[1] << 8) | rx[0])*conversion_factor;

    return GS_OK;
}

gs_error_t gs_isis_ants_deploy_single(uint8_t i2c_addr, int isis_ant_nr, uint8_t time_sec, bool override)
{

    uint8_t tx[2], tx_len;

    if (override) {
        switch(isis_ant_nr) {
        case 0: tx[0] = ISIS_ANTS_CMD_O_DEPLOY_1; break;
        case 1: tx[0] = ISIS_ANTS_CMD_O_DEPLOY_2; break;
        case 2: tx[0] = ISIS_ANTS_CMD_O_DEPLOY_3; break;
        case 3: tx[0] = ISIS_ANTS_CMD_O_DEPLOY_4; break;
        default: return GS_ERROR_RANGE;
        }
    } else {
        switch(isis_ant_nr) {
        case 0: tx[0] = ISIS_ANTS_CMD_DEPLOY_1; break;
        case 1: tx[0] = ISIS_ANTS_CMD_DEPLOY_2; break;
        case 2: tx[0] = ISIS_ANTS_CMD_DEPLOY_3; break;
        case 3: tx[0] = ISIS_ANTS_CMD_DEPLOY_4; break;
        default: return GS_ERROR_RANGE;
        }
    }

    tx[1] = time_sec;
    tx_len = 2;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, NULL, 0, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    return GS_OK;
}

gs_error_t gs_isis_ants_deploy_auto(uint8_t i2c_addr, uint8_t time_sec)
{

    uint8_t tx[2], tx_len;

    tx[0] = ISIS_ANTS_CMD_DEPLOY_AUTO;
    tx[1] = time_sec;
    tx_len = 2;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, NULL, 0, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    return GS_OK;
}

gs_error_t gs_isis_ants_deploy_cancel(uint8_t i2c_addr)
{

    uint8_t tx[1], tx_len;

    tx[0] = ISIS_ANTS_CMD_DEPLOY_CANCEL;
    tx_len = 1;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, NULL, 0, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    return GS_OK;
}

gs_error_t gs_isis_ants_disarm(uint8_t i2c_addr)
{

    uint8_t tx[1], tx_len;

    tx[0] = ISIS_ANTS_CMD_DISARM;
    tx_len = 1;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, NULL, 0, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    return GS_OK;
}

gs_error_t gs_isis_ants_arm(uint8_t i2c_addr)
{

    uint8_t tx[1], tx_len;

    tx[0] = ISIS_ANTS_CMD_ARM;
    tx_len = 1;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, NULL, 0, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    return GS_OK;
}

gs_error_t gs_isis_ants_reset(uint8_t i2c_addr)
{

    uint8_t tx[1], tx_len;

    tx[0] = ISIS_ANTS_CMD_RESET;
    tx_len = 1;

    gs_error_t i2c_result = gs_i2c_master_transaction(I2C_DEVICE, i2c_addr, &tx, tx_len, NULL, 0, 50);
    if (i2c_result != GS_OK) {
        log_error("I2C transaction error: [%s]", gs_error_string(i2c_result));
        return i2c_result;
    }

    return GS_OK;
}
