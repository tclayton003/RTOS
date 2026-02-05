#ifndef GS_EMBED_ASF_AVR32_DRIVERS_CAN_CAN_H
#define GS_EMBED_ASF_AVR32_DRIVERS_CAN_CAN_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   AVR32: CAN interface (initialization).
*/

#include <gs/embed/drivers/can/can.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Initialize CAN device/interface with fallback bit rate.

   @param[in] device device to initialize.
   @param[in] bps bits per second, valid values are 250000, 500000 or 1000000 Mbps.
   @param[in] fallback_bps value used if the \a bps cannot be applied.
   @return_gs_error_t
*/
gs_error_t gs_can_init_w_fallback(uint8_t device, uint32_t bps, uint32_t fallback_bps);

/**
   Initialize CAN device/interface.

   @param[in] device device to initialize.
   @param[in] bps bits per second, valid values are 250000, 500000 or 1000000 Mbps.
   @return_gs_error_t
*/
gs_error_t gs_can_init(uint8_t device, uint32_t bps);

#ifdef __cplusplus
}
#endif
#endif
