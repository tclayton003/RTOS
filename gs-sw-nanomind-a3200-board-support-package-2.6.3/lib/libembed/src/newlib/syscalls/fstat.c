/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.

   Fstat is called each time a reeent structure is initialized, e.g. as task/thread acceses a file (e.g. stdio) the first time.
   Some implementations from WWW, simply returns 0 or sets the mode (statbuf->st_mode    = S_IFCHR) and returns ok.

   That leaves a number of un-initialized members in the statbuf, and newlib doesn't seem to initialize the strucure before calling
   _fstat_r.
   Therefor we always return error - its seems to work fine on avr32, samc and stm32.
*/

#include <gs/embed/newlib/syscalls.h>

int _fstat(int fd, struct stat *statbuf)
{
    return _fstat_r(_REENT, fd, statbuf);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS)
int gs_embed_syscall_fstat_r(struct _reent *reent, int fd, struct stat *statbuf)
#else
int _fstat_r(struct _reent *reent, int fd, struct stat *statbuf)
#endif
{
    // see note in top.
    reent->_errno = ENOSYS;
    return -1;
}
