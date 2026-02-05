/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _open(const char *pathname, int flags, int mode)
{
    return _open_r(_REENT, pathname, flags, mode);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS == 0)
int _open_r(struct _reent *reent, const char *pathname, int flags, int mode)
{
    reent->_errno = ENOSYS;
    return -1;
}
#endif

