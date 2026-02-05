#ifndef GS_EMBED_DRIVERS_CAN_CAN_H
#define GS_EMBED_DRIVERS_CAN_CAN_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Common CAN driver API.
*/

#include <gs/util/drivers/can/can.h>
#include <gs/util/stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Print CAN driver statistics.
   @param[in] out output stream
   @param[in] device CAN device
   @return_gs_error_t
*/
gs_error_t gs_can_driver_print_stats(FILE * out, uint8_t device);

/**
   Print CAN driver statistics.
   @param[in] device CAN device
   @return_gs_error_t
*/
gs_error_t gs_can_print_driver_stats(uint8_t device);

#ifdef __cplusplus
}
#endif
#endif
