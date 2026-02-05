#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_ERROR_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_ERROR_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
    @file

    @brief STM HAL error API

    HAL (Hardware abstraction layer) consists of drivers provided by ST.
    HAL uses its own error codes/status, which this API helps translating to gs_error_t
*/

#include <stm32l4xx_hal.h>
#include <gs/util/error.h>

/**
   Converts HAL error code to gs_error_t
   @param[in] status HAL
   @return_gs_error_t
 */
gs_error_t gs_hal_status_to_error(HAL_StatusTypeDef status);


#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_ERROR_H_ */
