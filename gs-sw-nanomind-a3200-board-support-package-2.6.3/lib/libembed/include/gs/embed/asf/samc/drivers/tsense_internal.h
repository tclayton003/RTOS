#ifndef _LIB_LIBSAMC_TSENSE_INTERNAL_H_
#define _LIB_LIBSAMC_TSENSE_INTERNAL_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * Interface for accessing internal temperature sensor.
 */

/* --- includes ----------------------------------------------------------*/

#include <gs/util/error.h>

/* --- definitions -------------------------------------------------------*/

/**
 * @brief Initialize module.
 * @return 0 on success, negative on failure.
 */
gs_error_t gs_tsense_internal_init();

/**
 * @brief Returns instant reading - no average.
 * @param[out] temperature current temperature.
 * @return 0 on success, negative on failure.
 */
gs_error_t gs_tsense_internal_get(int32_t * temperature);

#endif
