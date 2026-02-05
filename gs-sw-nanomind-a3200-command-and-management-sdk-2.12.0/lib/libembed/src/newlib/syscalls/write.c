/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>
#include <gs/util/stdio.h>

_ssize_t _write(int fd, const void *buf, size_t count)
{
    return _write_r(_REENT, fd, buf, count);
}

#if (GS_EMBED_DISABLE_SYSCALL_FS)
_ssize_t gs_embed_syscall_write_r(struct _reent *reent, int fd, const void *buf, size_t count)
#else
_ssize_t _write_r(struct _reent *reent, int fd, const void *buf, size_t count)
#endif
{
    if (fd == STDOUT_FILENO) {
        if (gs_stdio_put(buf, count, false) == GS_OK) {
            return (int) count;
        }
        // the gs_stdio_put() can't tell how many bytes written
        reent->_errno = EAGAIN;
    } else if (fd == STDERR_FILENO) {
        if (gs_stdio_put_direct(buf, count) == GS_OK) {
            return (int) count;
        }
        // the gs_stdio_put_direct() can't tell how many bytes written
        reent->_errno = EAGAIN;
    } else {
        reent->_errno = ENOSYS;
    }
    return -1;
}

