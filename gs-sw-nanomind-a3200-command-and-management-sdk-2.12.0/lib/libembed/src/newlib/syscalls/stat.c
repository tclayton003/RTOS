/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _stat(const char *pathname, struct stat *statbuf)
{
    return _stat_r(_REENT, pathname, statbuf);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS == 0)
int _stat_r(struct _reent *reent, const char *pathname, struct stat *statbuf)
{
    // see note in fstat.c
    reent->_errno = ENOSYS;
    return -1;
}
#endif

