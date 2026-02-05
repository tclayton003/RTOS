#ifndef GS_A3200_PWR_SWITCH_H
#define GS_A3200_PWR_SWITCH_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Power switch driver
*/

#include <gs/util/types.h>
#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Power switch definition.
*/
typedef enum {
    /**
       Power GSSB 1.
    */
    GS_A3200_PWR_GSSB = 0,
    /**
       Power external Flash (legacy SD card).
    */
    GS_A3200_PWR_SD = 1,
    /**
       Power GSSB 2.
    */
    GS_A3200_PWR_GSSB2 = 2,
    /**
       Power PWM.
       @note Only supported on A3200 board revision 3 and later.
    */
    GS_A3200_PWR_PWM = 3,
} gs_a3200_pwr_switch_t ;

/**
   Initialize power switch interface.
*/
void gs_a3200_pwr_switch_init(void);

/**
   Turn power switch on or off.
   @param[in] sw switch to turn ON.
   @param[in] state set switch on or off.
*/
void gs_a3200_pwr_switch_set(gs_a3200_pwr_switch_t sw, bool state);

/**
   Get power switch state.
   @param[in] sw switch to get state from.
   @return state of switch (true = ON, false = OFF).
*/
bool gs_a3200_pwr_switch_get(gs_a3200_pwr_switch_t sw);

/**
   Turn power switch on.
   @param sw switch to turn ON.
*/
static inline void gs_a3200_pwr_switch_enable(gs_a3200_pwr_switch_t sw)
{
    gs_a3200_pwr_switch_set(sw, true);
}

/**
   Turn power switch off.
   @param sw switch to turn OFF.
*/
static inline void gs_a3200_pwr_switch_disable(gs_a3200_pwr_switch_t sw)
{
    gs_a3200_pwr_switch_set(sw, false);
}

/**
   Register commands.
   @return_gs_error_t
*/
gs_error_t gs_a3200_pwr_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
