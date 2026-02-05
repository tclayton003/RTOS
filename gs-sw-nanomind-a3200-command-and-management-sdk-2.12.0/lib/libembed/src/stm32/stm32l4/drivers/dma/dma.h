/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
#ifndef LIB_LIBEMBED_SRC_STM32_STM32L4_DRIVERS_DMA_DMA_H_
#define LIB_LIBEMBED_SRC_STM32_STM32L4_DRIVERS_DMA_DMA_H_

#include <stm32l4xx_hal.h>

typedef struct gs_dma_channel {
    //! DMA controller (can be 1 or 2, referring to DMA1 and DMA2)
    uint8_t dma;
    //! DMA channel on specific DMA controller (can be in range 1 to 7)
    uint8_t channel;
} gs_dma_channel_t;

/**
   Assign specific peripheral DMA handle to DMA ISR
   @param[in] channel Specific DMA channel on one of the two DMA controllers
   @param[in] handle_in Peripheral handle (could be a DMA handle related to SPI).
   @return_gs_error_t
*/
gs_error_t gs_dma_assign_handle_to_channel(gs_dma_channel_t channel, DMA_HandleTypeDef * handle_in);

#endif /* LIB_LIBEMBED_SRC_STM32_STM32L4_DRIVERS_DMA_DMA_H_ */
