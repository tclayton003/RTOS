/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#define GS_PARAM_INTERNAL_USE 1

#include "../../host_log.h"
#include <gs/param/internal/pp/spi/slave_dispatch.h>
#include <gs/util/check.h>
#include "local.h"

/**
   Dispatch handler state.
*/
typedef enum {
    GS_SPI_DISPATCH_STATE_HEADER = 0,
    GS_SPI_DISPATCH_STATE_DATA,
    GS_SPI_DISPATCH_STATE_COMPLETE,
} gs_spi_dispatch_state_t;

typedef struct {
    //bool in_use;
    //uint8_t device;
    //gs_spi_slave_dispatch_handler_t handlers[MAX_SPI_DISPATCH_HANDLERS_PER_DOMAIN];

    // state of currect request
    gs_spi_dispatch_state_t state;
    uint8_t domain;
    uint8_t command;
    //gs_spi_slave_dispatch_handler_t handler;
} gs_spi_dispatch_t;

// Only support dispatch for one SPI device - if more needed, add init() function to allocate more
static gs_spi_dispatch_t gs_spi_dispatch;

uint8_t gs_pp_spi_slave_rx(uint8_t device, const uint8_t * rx_buffer, size_t rx, bool new_request, gs_context_switch_t * cswitch)
{
    if (new_request) {
        gs_spi_dispatch.state = GS_SPI_DISPATCH_STATE_HEADER;
    }

    if (gs_spi_dispatch.state == GS_SPI_DISPATCH_STATE_HEADER) {
        gs_spi_dispatch.domain = GS_SPI_SLAVE_DISPATCH_HEADER_TO_DOMAIN(rx_buffer[0]);
        gs_spi_dispatch.command = GS_SPI_SLAVE_DISPATCH_HEADER_TO_COMMAND(rx_buffer[0]);
        if (gs_spi_dispatch.domain == GS_SPI_SLAVE_DISPATCH_DOMAIN_PARAM) {
            gs_spi_dispatch.state = GS_SPI_DISPATCH_STATE_DATA;
        } else {
            gs_spi_dispatch.state = GS_SPI_DISPATCH_STATE_COMPLETE;
        }
    }

    if (gs_spi_dispatch.state == GS_SPI_DISPATCH_STATE_DATA) {
        const uint8_t wanted_bytes = gs_param_spi_slave_dispatch_handler(device, gs_spi_dispatch.command, rx_buffer, rx, cswitch);
        if (wanted_bytes == 0) {
            gs_spi_dispatch.state = GS_SPI_DISPATCH_STATE_COMPLETE;
        }
        return wanted_bytes;
    }

    return 0;
}
