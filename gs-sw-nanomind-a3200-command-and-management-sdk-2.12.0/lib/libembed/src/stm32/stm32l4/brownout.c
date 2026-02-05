/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/stm32/stm32l4/brownout.h>
#include <gs/embed/stm32/stm32l4/error.h>

gs_error_t gs_stm32l4_brownout_set_level(gs_stm32l4_brownout_level_t brownout_level)
{
    gs_stm32l4_brownout_level_t running_brownout_level;
    gs_error_t res = gs_stm32l4_brownout_get_level(&running_brownout_level);
    if (res != GS_OK) {
        return res;
    }

    if (running_brownout_level == brownout_level) {
        return GS_OK;
    }

    HAL_StatusTypeDef HAL_res = HAL_FLASH_Unlock();
    if (HAL_res == HAL_OK) {
        HAL_res = HAL_FLASH_OB_Unlock();
        if (HAL_res == HAL_OK) {
            FLASH_OBProgramInitTypeDef option_bytes;
            option_bytes.OptionType = OPTIONBYTE_USER;
            option_bytes.USERType = OB_USER_BOR_LEV;
            option_bytes.USERConfig = brownout_level;
            HAL_res = HAL_FLASHEx_OBProgram(&option_bytes);
            HAL_FLASH_OB_Lock();
            if (HAL_res == HAL_OK) {
                HAL_res = HAL_FLASH_OB_Launch();
            }
        }
        HAL_FLASH_Lock();
    }
    return gs_hal_status_to_error(HAL_res);
}

gs_error_t gs_stm32l4_brownout_get_level(gs_stm32l4_brownout_level_t * brownout_level)
{
    FLASH_OBProgramInitTypeDef option_bytes;
    HAL_FLASHEx_OBGetConfig(&option_bytes);
    *brownout_level = option_bytes.USERConfig & FLASH_OPTR_BOR_LEV;
    return GS_OK;
}


const char * gs_stm32l4_brownout_level_to_string(gs_stm32l4_brownout_level_t brownout_level)
{
    switch (brownout_level) {
        case GS_STM32L4_BROWNOUT_LEVEL_1_7_V: return "1.7 V";
        case GS_STM32L4_BROWNOUT_LEVEL_2_0_V: return "2.0 V";
        case GS_STM32L4_BROWNOUT_LEVEL_2_2_V: return "2.2 V";
        case GS_STM32L4_BROWNOUT_LEVEL_2_5_V: return "2.5 V";
        case GS_STM32L4_BROWNOUT_LEVEL_2_8_V: return "2.8 V";
    }
    return "INVALID LEVEL";
}
