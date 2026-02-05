/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "../../host_table_local.h"
#include "../../host_log.h"
#include <gs/param/internal/pp/spi/spi.h>
#include <gs/param/table.h>
#include <gs/util/string.h>
#include "local.h"

typedef struct {
    uint8_t total;
    uint8_t data_length;
    uint8_t table_id;
    gs_param_type_t param_type;
    uint16_t table_addr;
    const gs_param_table_instance_t * tinst;
    const gs_param_table_row_t * rows;
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
    pctx.tinst = gs_param_table_raw(pctx.table_id); // minimum checks
    if (pctx.tinst &&
        pctx.tinst->rows &&
        pctx.tinst->memory &&
        (pctx.tinst->id == pctx.table_id)) {

        if ((pctx.table_addr + pctx.data_length) > pctx.tinst->memory_size) {
            // overrun
        } else {
            // find table element
            const gs_param_table_row_t * prev = pctx.rows = pctx.tinst->rows;
            for (unsigned int i = 0; (i < pctx.tinst->row_count); ++i, ++pctx.rows) {
                if ((pctx.rows->addr < pctx.table_addr)) {
                    // nope
                } else if ((pctx.rows->addr == pctx.table_addr)) {
                    // perfect match
                    prev = pctx.rows;
                    break;
                } else {
                    // too far - previous element matched
                    break;
                }
                prev = pctx.rows;
            }
            pctx.data = (pctx.tinst->memory + pctx.table_addr);
            pctx.rows = prev;
            pctx.param_type = pctx.rows->type;
            return;
        }
    }

    // failed
    pctx.tinst = NULL;
    pctx.rows = NULL;
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

uint8_t gs_param_spi_slave_dispatch_handler(uint8_t device, uint8_t cmd, const uint8_t * rx_buffer, size_t rx, gs_context_switch_t * cswitch)
{
    switch (cmd) {
        case GS_PARAM_SPI_COMMAND_SET:
            {
                gs_param_spi_set_t * req = (void *) rx_buffer;
                if (rx >= sizeof(*req)) {
                    pctx.data_length = GS_PARAM_SPI_LENGTH_TABLE_TO_LENGTH(req->length_table);
                    pctx.total = (sizeof(*req) + pctx.data_length);
                    if (rx >= pctx.total) {
                        pctx.table_id = GS_PARAM_SPI_LENGTH_TABLE_TO_TABLE(req->length_table);
                        pctx.table_addr = req->addr;
                        _find_addr();
                        if (pctx.data) {
                            memcpy(pctx.buffer.uint8, req->data, pctx.data_length);
                            _copy_data(pctx.buffer.uint8, pctx.data);
                        }
                        return 0;
                    }
                    return pctx.total;
                }
                return sizeof(*req);
            }
            break;
        case GS_PARAM_SPI_COMMAND_GET:
            {
                gs_param_spi_get_t * req = (void *) rx_buffer;
                pctx.total = ((sizeof*req) - sizeof(req->filler));
                if (rx >= pctx.total) {
                    pctx.data_length = GS_PARAM_SPI_LENGTH_TABLE_TO_LENGTH(req->length_table);
                    pctx.table_id = GS_PARAM_SPI_LENGTH_TABLE_TO_TABLE(req->length_table);
                    pctx.table_addr = req->addr;
                    _find_addr();
                    if (pctx.rows) {
                        if (_copy_data(pctx.data, pctx.buffer.uint8)) {
                            gs_spi_slave_set_response(device, offsetof(gs_param_spi_get_t, data), pctx.buffer.uint8, pctx.data_length);
                        }
                    }
                    return 0;
                }
                return pctx.total;
            }
            break;
        case GS_PARAM_SPI_COMMAND_SET_WITH_CHECKSUM:
            {
                gs_param_spi_set_t * req = (void *) rx_buffer;
                if (rx >= sizeof(*req)) {
                    pctx.data_length = GS_PARAM_SPI_LENGTH_TABLE_TO_LENGTH(req->length_table);
                    pctx.total = (sizeof(*req) + pctx.data_length + 1); // add 1 for CHECKSUM
                    if (rx >= pctx.total) {
                        pctx.table_id = GS_PARAM_SPI_LENGTH_TABLE_TO_TABLE(req->length_table);
                        pctx.table_addr = req->addr;
                        _find_addr();
                        if (pctx.data) {
                            if (gs_pp_checksum8(rx_buffer, pctx.total - 1) == rx_buffer[pctx.total - 1]) {
                                memcpy(pctx.buffer.uint8, req->data, pctx.data_length);
                                _copy_data(pctx.buffer.uint8, pctx.data);
                            }
                        }
                        return 0;
                    }
                    return pctx.total;
                }
                return sizeof(*req);
            }
            break;
        case GS_PARAM_SPI_COMMAND_GET_WITH_CHECKSUM:
            {
                gs_param_spi_get_with_checksum_t * req = (void *) rx_buffer;
                pctx.total = ((sizeof*req) - sizeof(req->filler));
                if (rx >= pctx.total) {
                    pctx.data_length = GS_PARAM_SPI_LENGTH_TABLE_TO_LENGTH(req->length_table);
                    pctx.table_id = GS_PARAM_SPI_LENGTH_TABLE_TO_TABLE(req->length_table);
                    pctx.table_addr = req->addr;
                    _find_addr();
                    if (pctx.rows) {
                        if (gs_pp_checksum8(rx_buffer, pctx.total - 1) == rx_buffer[pctx.total - 1]) {
                            if (_copy_data(pctx.data, pctx.buffer.uint8)) {
                                pctx.buffer.uint8[pctx.data_length] = gs_pp_checksum8(pctx.buffer.uint8, pctx.data_length);
                                gs_spi_slave_set_response(device, offsetof(gs_param_spi_get_with_checksum_t, data), pctx.buffer.uint8, pctx.data_length + 1);
                            }
                        }
                    }
                    return 0;
                }
                return pctx.total;
            }
            break;
    }
    return 0;
}
