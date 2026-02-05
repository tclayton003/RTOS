/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "../../host_table_local.h"
#include "../../host_log.h"
#include <gs/param/internal/pp/i2c/i2c.h>
#include <gs/param/pp/pp.h>
#include <gs/util/string.h>

typedef struct {
    uint8_t total;
    uint8_t data_length;
    uint8_t table_id;
    gs_param_type_t param_type;
    uint16_t table_addr;
    const gs_param_table_instance_t * table;
    const gs_param_table_row_t * param;
    void * data;
    union {
        // cppcheck-suppress unusedStructMember
        uint64_t align_me;
        uint8_t  uint8[100];
    } buffer;
} param_dispatch_context_t;

static param_dispatch_context_t pctx;

// fast lookup - assuming row/addr is assending
static inline void _find_addr(void)
{
    // get table
    pctx.table = gs_param_table_raw(pctx.table_id); // minimum checks
    if (pctx.table &&
        pctx.table->rows &&
        pctx.table->memory &&
        (pctx.table->id == pctx.table_id)) {

        if ((pctx.table_addr + pctx.data_length) > pctx.table->memory_size) {
            // overrun
        } else {
            // find table element
            const gs_param_table_row_t * prev = pctx.param = pctx.table->rows;
            for (unsigned int i = 0; (i < pctx.table->row_count); ++i, ++pctx.param) {
                if ((pctx.param->addr < pctx.table_addr)) {
                    // nope
                } else if ((pctx.param->addr == pctx.table_addr)) {
                    // perfect match
                    prev = pctx.param;
                    break;
                } else {
                    // too far - previous element matched
                    break;
                }
                prev = pctx.param;
            }
            pctx.data = (pctx.table->memory + pctx.table_addr);
            pctx.param = prev;
            pctx.param_type = pctx.param->type;
            return;
        }
    }

    // failed
    pctx.table = NULL;
    pctx.param = NULL;
    pctx.data = NULL;
}

static inline bool _copy_data(const void * from_in, void * to_in)
{
    switch (pctx.param_type) {
        case PARAM_X8:
        case PARAM_INT8:
        case PARAM_UINT8:
        case PARAM_BOOL:
        case PARAM_STRING:
        case PARAM_DATA:
            memcpy(to_in, from_in, pctx.data_length);
            break;
        case PARAM_X16:
        case PARAM_INT16:
        case PARAM_UINT16:
            {
                const uint16_t * from = from_in;
                uint16_t * to = to_in;
                if (((intptr_t)from & 1) || ((intptr_t)to & 1)) {
                    // un-aligned
                    return false;
                }
                for (unsigned int i = 0; i < (pctx.data_length / 2); ++i, ++to, ++from) {
                    *to = *from;
                }
            }
            break;
        case PARAM_X32:
        case PARAM_INT32:
        case PARAM_UINT32:
        case PARAM_FLOAT:
            {
                const uint32_t * from = from_in;
                uint32_t * to = to_in;
                if (((intptr_t)from & 3) || ((intptr_t)to & 3)) {
                    // un-aligned
                    return false;
                }
                for (unsigned int i = 0; i < (pctx.data_length / 4); ++i, ++to, ++from) {
                    *to = *from;
                }
            }
            break;
        case PARAM_X64:
        case PARAM_INT64:
        case PARAM_UINT64:
        case PARAM_DOUBLE:
            return false; // can't do this safely without lock
    }

    return true;
}

static void gs_param_i2c_slave_dispatch_handler(uint8_t bus, uint8_t cmd, const uint8_t * rx, size_t rx_length, gs_context_switch_t * cswitch)
{
    
    switch (cmd) {
        case GS_PARAM_I2C_COMMAND_SET:
            {
                gs_param_i2c_set_request_t * req = (void *) rx;
                if (rx_length >= sizeof(*req)) {
                    pctx.data_length = GS_PARAM_I2C_LENGTH_TABLE_TO_LENGTH(req->length_table);
                    pctx.total = (sizeof(*req) + pctx.data_length);
                    if (rx_length >= pctx.total) {
                        pctx.table_id = GS_PARAM_I2C_LENGTH_TABLE_TO_TABLE(req->length_table);
                        pctx.table_addr = req->addr;
                        _find_addr();
                        if (pctx.data) {
                            memcpy(pctx.buffer.uint8, req->data, pctx.data_length);
                            _copy_data(pctx.buffer.uint8, pctx.data);
                        }
                    }
                }
            }
            break;
        case GS_PARAM_I2C_COMMAND_GET:
            {
                gs_param_i2c_get_request_t * req = (void *) rx;
                pctx.total = (sizeof(*req) - sizeof(req->checksum));
                if (rx_length >= pctx.total) {
                    pctx.data_length = GS_PARAM_I2C_LENGTH_TABLE_TO_LENGTH(req->length_table);
                    pctx.table_id = GS_PARAM_I2C_LENGTH_TABLE_TO_TABLE(req->length_table);
                    pctx.table_addr = req->addr;
                    _find_addr();
                    if (pctx.param) {
                        if (_copy_data(pctx.data, pctx.buffer.uint8)) {
                            gs_i2c_slave_set_response(bus, pctx.buffer.uint8, pctx.data_length);
                        }
                    }
                }
            }
            break;
        case GS_PARAM_I2C_COMMAND_SET_WITH_CHECKSUM:
            {
                gs_param_i2c_set_request_t * req = (void *) rx;
                if (rx_length >= sizeof(*req)) {
                    pctx.data_length = GS_PARAM_I2C_LENGTH_TABLE_TO_LENGTH(req->length_table);
                    pctx.total = (sizeof(*req) + pctx.data_length + 1); // add 1 for CHECKSUM
                    if (rx_length >= pctx.total) {
                        pctx.table_id = GS_PARAM_I2C_LENGTH_TABLE_TO_TABLE(req->length_table);
                        pctx.table_addr = req->addr;
                        _find_addr();
                        if (pctx.data) {
                            if (gs_pp_checksum8(rx, pctx.total - 1) == rx[pctx.total - 1]) {
                                memcpy(pctx.buffer.uint8, req->data, pctx.data_length);
                                _copy_data(pctx.buffer.uint8, pctx.data);
                            }
                        }
                    }
                }
            }
            break;
        case GS_PARAM_I2C_COMMAND_GET_WITH_CHECKSUM:
            {
                gs_param_i2c_get_request_t * req = (void *) rx;
                pctx.total = sizeof(*req);
                if (rx_length >= pctx.total) {
                    pctx.data_length = GS_PARAM_I2C_LENGTH_TABLE_TO_LENGTH(req->length_table);
                    pctx.table_id = GS_PARAM_I2C_LENGTH_TABLE_TO_TABLE(req->length_table);
                    pctx.table_addr = req->addr;
                    _find_addr();
                    if (pctx.param) {
                        if (gs_pp_checksum8(rx, pctx.total - 1) == rx[pctx.total - 1]) {
                            if (_copy_data(pctx.data, pctx.buffer.uint8)) {
                                pctx.buffer.uint8[pctx.data_length] = gs_pp_checksum8(pctx.buffer.uint8, pctx.data_length);
                                gs_i2c_slave_set_response(bus, pctx.buffer.uint8, pctx.data_length + 1);
                            }
                        }
                    }
                }
            }
            break;
    }
}

void gs_pp_i2c_slave_rx(uint8_t bus, const uint8_t * rx_buffer, size_t rx_length, gs_context_switch_t * cswitch)
{
    // hardcoded dispatch handler, to be changed to something like gs_spi_slave_dispatch()
    if ((rx_length > 1) && (GS_I2C_SLAVE_DISPATCH_HEADER_TO_DOMAIN(rx_buffer[0]) == GS_I2C_SLAVE_DISPATCH_DOMAIN_PARAM)) {
        gs_param_i2c_slave_dispatch_handler(bus, GS_I2C_SLAVE_DISPATCH_HEADER_TO_COMMAND(rx_buffer[0]), rx_buffer, rx_length, cswitch);
    }
}
