/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _link(const char *oldpath, const char *newpath)
{
    return _link_r(_REENT, oldpath, newpath);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS == 0)
int _link_r(struct _reent *reent, const char *oldpath, const char *newpath)
{
    reent->_errno = ENOSYS;
    return -1;
}
#endif
