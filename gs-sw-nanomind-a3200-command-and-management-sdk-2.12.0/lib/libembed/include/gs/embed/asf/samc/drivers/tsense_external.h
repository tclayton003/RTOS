#ifndef _LIB_LIBSAMC_TSENSE_EXTERNAL_H_
#define _LIB_LIBSAMC_TSENSE_EXTERNAL_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * Interface for accessing external temperature sensor - using the ADC
 */

/* --- includes ----------------------------------------------------------*/

#include <gs/util/error.h>

/* --- definitions -------------------------------------------------------*/

/* Initialize external temperature sensor */
gs_error_t gs_tsense_external_init(void);

/* Read external temperature sensor */
gs_error_t gs_tsense_external_get(int32_t * temperature, uint16_t * adc_value);

#endif
