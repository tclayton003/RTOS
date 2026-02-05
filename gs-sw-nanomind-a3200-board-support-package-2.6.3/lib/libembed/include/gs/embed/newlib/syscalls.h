#ifndef GS_EMBED_NEWLIB_SYSCALLS_H
#define GS_EMBED_NEWLIB_SYSCALLS_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Newlib syscall stubs.

   This header file should be included by libraries implementing a file-system, e.g. libstorage (file-system for A3200).

   The syscall stubs provided by libembed can be divided into:
   - basic, all syscalls not related to a file-system, e.g. malloc lock, sbrk.
   - file-system, in libembed only read/write to stdin, stdout and stderr are handled.

   The current newlib stubs are implemented/tested on newlib version 1.16.0 (avr32) and 2.2.0 (samc, stm32).
   All implementation is done in the re-entrant functions: "_<syscall>_r()", all "<syscall>()" and "_<syscall>()" are merely
   wrappers for the re-entrant function.

   The stubs sets \a errno (using the newlib macro, "#include <errno.h>"), i.e. causing the \a errno to be set in the re-entrant
   structure (impure) - either task/thread local (if FreeRTOS compiled with \a configUSE_NEWLIB_REENTRANT) or global. The user should
   only use the standard \a errno reference.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/features.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <newlib.h>
#include <reent.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Prototype for re-entant \a rename syscall.
   rename() (newlib: 1.16.0) and _rename() (newlib: 2.2.0) are implemented in libembed by calling _rename_r().
   @param[in] reent re-entrant data (task/thread local).
   @param[in] oldpath old name of file.
   @param[in] newpath new name of file.
   @return result
*/
int _rename_r(struct _reent *reent, const char *oldpath, const char *newpath);

/**
   Read syscall handling stdin and general error.
*/
_ssize_t gs_embed_syscall_read_r(struct _reent *reent, int fd, void *buf, size_t count);

/**
   Write syscall handling stdout/stderr and general error.
*/
_ssize_t gs_embed_syscall_write_r(struct _reent *reent, int fd, const void *buf, size_t count);

/**
   fstat syscall handling stdin, stdout and stderr and general error.
*/
int gs_embed_syscall_fstat_r(struct _reent *reent, int fd, struct stat *statbuf);

/**
   fstat syscall handling stdin/stdout/stderr and general error.
*/
int gs_embed_syscall_close_r(struct _reent *reent, int fd);

#ifdef __cplusplus
}
#endif
#endif
