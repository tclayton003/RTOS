/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _mkdir_r(struct _reent *reent, const char *pathname, int mode)
{
    reent->_errno = ENOSYS;
    return -1;
}

int _mkdir(const char *pathname, int mode)
{
    return _mkdir_r(_REENT, pathname, mode);
}

