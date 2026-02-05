#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_BROWNOUT_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_BROWNOUT_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
    @file

    @brief STM32L4 brownout detection API

   This API includes SW configuration of brownout detection.
   It includes writing to the option bytes in the flash, which also can be programmed directly.
*/
#include <gs/util/error.h>
#include <stm32l4xx_hal.h>

/**
   Brown-out level.
*/
typedef enum {
    //! Reset level threshold is around 1.7 V
    GS_STM32L4_BROWNOUT_LEVEL_1_7_V = OB_BOR_LEVEL_0,
    //! Reset level threshold is around 2.0 V
    GS_STM32L4_BROWNOUT_LEVEL_2_0_V = OB_BOR_LEVEL_1,
    //! Reset level threshold is around 2.2 V
    GS_STM32L4_BROWNOUT_LEVEL_2_2_V = OB_BOR_LEVEL_2,
    //! Reset level threshold is around 2.5 V
    GS_STM32L4_BROWNOUT_LEVEL_2_5_V = OB_BOR_LEVEL_3,
    //! Reset level threshold is around 2.8 V
    GS_STM32L4_BROWNOUT_LEVEL_2_8_V = OB_BOR_LEVEL_4,
} gs_stm32l4_brownout_level_t;


/**
   Set brownout detection level

   @note Must be called as soon as possible in the initializing code as it writes to option bytes in flash

   @param[in] brownout_level Level on which the brownout detection works
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_brownout_set_level(gs_stm32l4_brownout_level_t brownout_level);

/**
   Get brownout detection level

   @param[in] brownout_level Level on which the brownout detection works
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_brownout_get_level(gs_stm32l4_brownout_level_t * brownout_level);

/**
   Convert brownout level to string

   @param[in] brownout_level Brownout level
   @return string containing the brownout level
*/
const char * gs_stm32l4_brownout_level_to_string(gs_stm32l4_brownout_level_t brownout_level);

#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_BROWNOUT_H_ */
