/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _fork(void)
{
    return _fork_r(_REENT);
}

int _fork_r(struct _reent *reent)
{
    reent->_errno = ENOSYS;
    return -1;
}

