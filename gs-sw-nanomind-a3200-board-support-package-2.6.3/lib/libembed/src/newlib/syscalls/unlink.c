/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _unlink(const char *pathname)
{
    return _unlink_r(_REENT, pathname);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS == 0)
int _unlink_r(struct _reent *reent, const char *pathname)
{
    reent->_errno = ENOSYS;
    return -1;
}
#endif
