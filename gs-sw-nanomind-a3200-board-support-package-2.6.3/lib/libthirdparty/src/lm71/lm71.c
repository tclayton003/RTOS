/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/lm71/lm71.h>
#include <gs/embed/asf/drivers/spi/master.h>
#include <math.h>

gs_error_t gs_lm71_read_temp(uint8_t spi_slave, int spi_timeout_ms, int16_t * temperature)
{
    uint8_t tx[] = {0xFF, 0xFF};
    int16_t rx;
    gs_error_t res = gs_spi_master_transaction(spi_slave, tx, &rx, 2, spi_timeout_ms);
    if (res != GS_OK) {
        *temperature = INT16_MIN;
        return res;
    }

    /* Temp in 1/32'th degrees */
    float temp = rx >> 2;

    /* Convert into 1/10'th degrees */
    temp = temp / 3.2;

    *temperature = round(temp);

    return GS_OK;
}
