/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _fcntl(int fd, int cmd, int arg)
{
    return _fcntl_r(_REENT, fd, cmd, arg);
}

int _fcntl_r(struct _reent *reent, int fd, int cmd, int arg)
{
    reent->_errno = ENOSYS;
    return -1;
}

