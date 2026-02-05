/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/embed/stm32/stm32l4/error.h>

gs_error_t gs_hal_status_to_error(HAL_StatusTypeDef status)
{
    switch (status) {
        case HAL_OK: //                         =   0,
            return GS_OK;
        case HAL_ERROR: //                      =   1,
            return GS_ERROR_IO;
        case HAL_BUSY: //                       =   2,
            return GS_ERROR_BUSY;
        case HAL_TIMEOUT: //                    =   3,
            return GS_ERROR_TIMEOUT;
    }

    return GS_ERROR_NOT_IMPLEMENTED;
}
