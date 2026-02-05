/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>
#include <gs/util/stdio.h>

_ssize_t _read(int fd, void *buf, size_t count)
{
    return _read_r(_REENT, fd, buf, count);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS)
_ssize_t gs_embed_syscall_read_r(struct _reent *reent, int fd, void *buf, size_t count)
#else
_ssize_t _read_r(struct _reent *reent, int fd, void *buf, size_t count)
#endif
{
    if (fd == STDIN_FILENO) {
        if (gs_stdio_get(buf, 1) == GS_OK) {
            return 1;
        }
        reent->_errno = EAGAIN;
    } else {
        reent->_errno = ENOSYS;
    }
    return -1;
}

