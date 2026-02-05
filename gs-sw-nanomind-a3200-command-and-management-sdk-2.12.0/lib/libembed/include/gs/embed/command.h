#ifndef GS_EMBED_COMMAND_H
#define GS_EMBED_COMMAND_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   General commands.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Register general commands (uses FreeRTOS).
   @return_gs_error_t
*/
gs_error_t gs_embed_register_commands(void);

/**
   Register driver commands (show stats).
   @return_gs_error_t
*/
gs_error_t gs_embed_register_driver_commands(void);
    
#ifdef __cplusplus
}
#endif
#endif
