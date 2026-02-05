/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/error.h>
#include <gs/util/check.h>
#include "dma.h"

DMA_HandleTypeDef * gs_dma1_channel_1_handle = NULL;
DMA_HandleTypeDef * gs_dma1_channel_2_handle = NULL;
DMA_HandleTypeDef * gs_dma1_channel_3_handle = NULL;
DMA_HandleTypeDef * gs_dma1_channel_4_handle = NULL;
DMA_HandleTypeDef * gs_dma1_channel_5_handle = NULL;
DMA_HandleTypeDef * gs_dma1_channel_6_handle = NULL;
DMA_HandleTypeDef * gs_dma1_channel_7_handle = NULL;

DMA_HandleTypeDef * gs_dma2_channel_1_handle = NULL;
DMA_HandleTypeDef * gs_dma2_channel_2_handle = NULL;
DMA_HandleTypeDef * gs_dma2_channel_3_handle = NULL;
DMA_HandleTypeDef * gs_dma2_channel_4_handle = NULL;
DMA_HandleTypeDef * gs_dma2_channel_5_handle = NULL;
DMA_HandleTypeDef * gs_dma2_channel_6_handle = NULL;
DMA_HandleTypeDef * gs_dma2_channel_7_handle = NULL;



gs_error_t gs_dma_assign_handle_to_channel(gs_dma_channel_t channel, DMA_HandleTypeDef * handle_in)
{
    GS_CHECK_ARG(channel.channel <= 7);
    GS_CHECK_ARG(channel.channel >= 1);
    GS_CHECK_ARG(channel.dma == 1 || channel.dma == 2);

    DMA_HandleTypeDef ** handle = NULL;
    switch (channel.dma) {
        case 1:
            switch (channel.channel) {
                case 1:
                    handle = &gs_dma1_channel_1_handle;
                    break;
                case 2:
                    handle = &gs_dma1_channel_2_handle;
                    break;
                case 3:
                    handle = &gs_dma1_channel_3_handle;
                    break;
                case 4:
                    handle = &gs_dma1_channel_4_handle;
                    break;
                case 5:
                    handle = &gs_dma1_channel_5_handle;
                    break;
                case 6:
                    handle = &gs_dma1_channel_6_handle;
                    break;
                case 7:
                    handle = &gs_dma1_channel_7_handle;
                    break;
                default:
                    return GS_ERROR_ARG;
            }
            break;
       case 2:
           switch (channel.channel) {
               case 1:
                   handle = &gs_dma2_channel_1_handle;
                   break;
               case 2:
                   handle = &gs_dma2_channel_2_handle;
                   break;
               case 3:
                   handle = &gs_dma2_channel_3_handle;
                   break;
               case 4:
                   handle = &gs_dma2_channel_4_handle;
                   break;
               case 5:
                   handle = &gs_dma2_channel_5_handle;
                   break;
               case 6:
                   handle = &gs_dma2_channel_6_handle;
                   break;
               case 7:
                   handle = &gs_dma2_channel_7_handle;
                   break;
               default:
                   return GS_ERROR_ARG;
           }
           break;
       default:
           return GS_ERROR_ARG;
    }
    if (*handle != NULL) {
        return GS_ERROR_IN_USE;
    } else {
        *handle = handle_in;
        return GS_OK;
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma1_channel_1_handle);
}

void DMA1_Channel2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma1_channel_2_handle);
}

void DMA1_Channel3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma1_channel_3_handle);
}

void DMA1_Channel4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma1_channel_4_handle);
}

void DMA1_Channel5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma1_channel_5_handle);
}

void DMA1_Channel6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma1_channel_6_handle);
}

void DMA1_Channel7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma1_channel_7_handle);
}



void DMA2_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma2_channel_1_handle);
}

void DMA2_Channel2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma2_channel_2_handle);
}

void DMA2_Channel3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma2_channel_3_handle);
}

void DMA2_Channel4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma2_channel_4_handle);
}

void DMA2_Channel5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma2_channel_5_handle);
}

void DMA2_Channel6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma2_channel_6_handle);
}

void DMA2_Channel7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(gs_dma2_channel_7_handle);
}







