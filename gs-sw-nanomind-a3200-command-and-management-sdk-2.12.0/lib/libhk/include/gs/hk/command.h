#ifndef GS_HK_COMMAND_H
#define GS_HK_COMMAND_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Commands.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Register host commands.
   @return_gs_error_t
*/
gs_error_t gs_hk_srv_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
