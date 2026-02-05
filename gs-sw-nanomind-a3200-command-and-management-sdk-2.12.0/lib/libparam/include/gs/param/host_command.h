#ifndef GS_PARAM_HOST_COMMAND_H
#define GS_PARAM_HOST_COMMAND_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Host command.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Register host commands.
   @return gs_error_t
*/
gs_error_t gs_param_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
