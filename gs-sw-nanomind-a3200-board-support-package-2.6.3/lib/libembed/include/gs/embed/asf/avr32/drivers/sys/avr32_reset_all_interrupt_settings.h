/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_ASF_AVR32_RESET_ALL_INTERRUPT_SETTINGS_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_ASF_AVR32_RESET_ALL_INTERRUPT_SETTINGS_H_
/**
   @file

   AVR32: Reset all interrupt settings.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
   Reset and disable all interrupts to avoid problems when booting RAM image
*/
void gs_sys_avr32_reset_all_interrupt_settings(void);

#ifdef __cplusplus
}
#endif
#endif
