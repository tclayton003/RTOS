/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

_off_t _lseek(int fd, _off_t offset, int whence)
{
    return _lseek_r(_REENT, fd, offset, whence);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS == 0)
_off_t _lseek_r(struct _reent *reent, int fd, _off_t offset, int whence)
{
    reent->_errno = ENOSYS;
    return -1;
}
#endif
