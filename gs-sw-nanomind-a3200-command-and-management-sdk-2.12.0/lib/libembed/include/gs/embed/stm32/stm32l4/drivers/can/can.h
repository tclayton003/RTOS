#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_CAN_CAN_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_CAN_CAN_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   @brief STM32L4 CAN API

   This API includes initialization routine of CAN device.
   The cross platform CAN API holds functions for send/receive, etc.
*/

#include <gs/util/drivers/can/can.h>

/**
   Init CAN
   Note: it is needed to configure filter and callback afterwards, before CAN will function
         it is done with function 'gs_can_set_standard_filter_mask' or 'gs_can_set_extended_filter_mask'
   @param[in]  device hardware CAN device (currently device must be 1)
   @param[in]  bps CAN baud rate (bits per second), default GS_CAN_DEFAULT_BPS
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_can_init(uint8_t device, size_t bps);

#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_CAN_CAN_H_ */
