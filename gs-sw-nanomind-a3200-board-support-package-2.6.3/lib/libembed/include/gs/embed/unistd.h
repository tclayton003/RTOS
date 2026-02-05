#ifndef GS_EMBED_UNISTD_H
#define GS_EMBED_UNISTD_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Extensions to standard \a unistd.h.
*/

#include <gs/util/unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Set current working directory.

   This API can be used on embeeded platforms with a limited file-system, where standard \a getcwd() isn't supported.
   The \a cwd will be stored in a blobal variable and returned by gs_getcwd().

   @note Not thread-safe.

   @param[in] cwd current working directory.
   @return #GS_ERROR_NOT_FOUND if no current directory is set.
   @return_gs_error_t
*/
gs_error_t gs_setcwd(const char * cwd);

#ifdef __cplusplus
}
#endif
#endif
