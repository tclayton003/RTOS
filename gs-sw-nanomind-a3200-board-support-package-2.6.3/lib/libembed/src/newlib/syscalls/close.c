/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _close(int fd)
{
    return _close_r(_REENT, fd);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS)
int gs_embed_syscall_close_r(struct _reent *reent, int fd)
#else
int _close_r(struct _reent *reent, int fd)
#endif
{
    // Don't close stdin, stdout, stderr
    reent->_errno = ENOSYS;
    return -1;
}
