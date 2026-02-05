#ifndef GS_A3200_ADC_CHANNELS_H
#define GS_A3200_ADC_CHANNELS_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   ADC channel driver
*/

#include <gs/util/error.h>
#include <conf_a3200.h> // -> GS_A3200_BOARD_REVISION

#ifdef __cplusplus
extern "C" {
#endif

/**
   Numbe of ADC channels.
*/
#if (GS_A3200_BOARD_REVISION >= 3)
#define GS_A3200_ADC_NCHANS     12
#else
#define GS_A3200_ADC_NCHANS     3
#endif

/**
   ADC reference voltage.
*/
#define GS_A3200_ADC_REF        2500
/**
   ADC max value.
*/
#define GS_A3200_ADC_MAX_VAL 	(2048 - 1)
/**
   Convert ADC value to milli-volt.
*/
#define GS_A3200_ADC_TO_MV	 	((float) GS_A3200_ADC_REF / (float) GS_A3200_ADC_MAX_VAL)

/** 
    Convert voltage (over resistor) to milli-amps.
 
    0.025 Resistor, 
    100V/V gain in curr sensor,
    2.5V reference,  
    11-bit resolution from 0 to 2.5V
*/
#define GS_A3200_ADC_CONVERT_MA(val)		(val * (2.5/2048) / (0.0025)) 

/**
   Initialise ADC.

   This function must be called before any other function in this file.
   This will call adc_channels_calibrate, so power switches must be OFF.
   Also, the adc_channels_ functions should have exclusive access to the ADC hardware, or it will not work.
*/
void gs_a3200_adc_channels_init(void);

/**
   Get a single sample from each channel.

   This function does a blocking sample on all used adc channels and writes those to the array pointed
   to by adc_values.
   This function must only be called from Task context.
   
   @param[out] adc_values Pointer to array of dimension #GS_A3200_ADC_NCHANS
   @return 0 on success, negative value on failure.
*/
gs_error_t gs_a3200_adc_channels_sample(int16_t *adc_values);

/**
   Register commands.
*/
gs_error_t gs_a3200_adc_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
