#ifndef GS_A3200DOCK_DOCK_H
#define GS_A3200DOCK_DOCK_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Generic interface for all A3200 docks.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   A3200 dock types.
*/
typedef enum {
    /**
       A3200 not placed on a dock
    */
    GS_A3200DOCK_NONE = 0,
    /**
       DMC3
    */
    GS_A3200DOCK_DMC3 = 10,
    /**
       ADCS3
    */
    GS_A3200DOCK_ADCS3 = 20,
    /**
       ADCS6
    */
    GS_A3200DOCK_ADCS6 = 21,
    /**
       ADCS8
    */
    GS_A3200DOCK_ADCS8 = 22,
} gs_a3200dock_type_t;

/**
   Get the initialized dock type.

   @return Initialized dock type or #GS_A3200DOCK_NONE, if none is configured.
*/
gs_a3200dock_type_t gs_a3200dock_get_dock_type(void);

/**
   Initialize dock.

   @param[in] dock dock type.
   @return_gs_error_t
*/
gs_error_t gs_a3200dock_init(const char * dock);

/**
   Updated relevant table parameters, based on ADC readings.

   @param[in] adc array of ADC readings.
   @param[in] adc_count number of elements in \a adc - must be 12!
   @return_gs_error_t
*/
gs_error_t gs_a3200dock_update_adc(int16_t adc[], uint8_t adc_count);

#ifdef __cplusplus
}
#endif
#endif
