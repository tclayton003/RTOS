#ifndef GS_A3200_ADC_REFS_H
#define GS_A3200_ADC_REFS_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   ADC constants.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
   ADC reference voltage in milli volt (mV).
*/
#define ADC_REF         2500

/**
   ADC max value.
*/
#define ADC_MAX_VAL     (2048 - 1)

/**
   ADC mV per bit.
*/
#define ADC_TO_MV       ((float) ADC_REF / (float) ADC_MAX_VAL)

#ifdef __cplusplus
}
#endif
#endif
