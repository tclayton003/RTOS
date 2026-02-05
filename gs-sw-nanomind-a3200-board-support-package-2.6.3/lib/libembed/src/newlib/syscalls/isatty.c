/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _isatty_r(struct _reent *reent, int fd)
{
    if ((fd >= STDIN_FILENO) && (fd <= STDERR_FILENO)) {
        return 1;
    }
    reent->_errno = ENOSYS;
    return 0;
}

int _isatty(int fd)
{
    return _isatty_r(_REENT, fd);
}
