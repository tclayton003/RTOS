#ifndef _LIB_LIBSAMC_ADC_H_
#define _LIB_LIBSAMC_ADC_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * Interface for initializing and read from ADC.
 * Currently locked to ADC0.
 */

/* --- includes ----------------------------------------------------------*/

#include <adc.h> // libasf
#include <gs/util/error.h>

typedef enum {
  GS_ADC0 = 0,
  GS_ADC1,
} gs_adc_t;

/* --- definitions -------------------------------------------------------*/

/**
 * Initialize instance of ADC.
 */
gs_error_t gs_adc_init(void);

/**
 * Read value from ADC.
 */
gs_error_t gs_adc_read(gs_adc_t adc, struct adc_config * config, uint16_t * value);

#endif
