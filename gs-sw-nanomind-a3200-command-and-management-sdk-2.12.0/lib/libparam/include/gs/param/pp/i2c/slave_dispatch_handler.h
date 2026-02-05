#ifndef GS_PARAM_PP_I2C_SLAVE_DISPATCH_HANDLER_H
#define GS_PARAM_PP_I2C_SLAVE_DISPATCH_HANDLER_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   I2C dispatch handler.
*/

#include <gs/util/drivers/i2c/slave.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Dispatch handler - processes a request.

   Register using gs_i2c_slave_set_rx().

   @param[in] device I2C bus (handle).
   @param[in] rx receive buffer.
   @param[in] rx_length number of bytes received.
   @param_cswitch
*/
void gs_pp_i2c_slave_rx(uint8_t device, const uint8_t * rx, size_t rx_length, gs_context_switch_t * cswitch);

#ifdef __cplusplus
}
#endif
#endif
