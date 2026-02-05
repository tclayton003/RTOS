#ifndef GS_EMBED_STDIO_H
#define GS_EMBED_STDIO_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Extensions to standard \a stdio.h.
*/

#include <gs/util/stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Set uart for stdio.
   Initialized to option --embed-uart-console.
   @param[in] uart less than 0 means not enabled/set, >= 0 is active.
   @return_gs_error_t
*/
gs_error_t gs_stdio_set_uart(int uart);

/**
   Get the current uart for stdio
   @return current uart.
*/
int gs_stdio_get_uart(void);

/**
   Get the uart default uart
   @return default uart (compile time configuration --embed-uart-console).
*/
int gs_stdio_get_default_uart(void);

/**
   Write directly to stdout.
   Circumvents any interrupt based uart implementation.

   @param[in] buf characters to write.
   @param[in] len number of characters to write.
   @return_gs_error_t
*/
gs_error_t gs_stdio_put_direct(const char * buf, size_t len);

#ifdef __cplusplus
}
#endif
#endif
