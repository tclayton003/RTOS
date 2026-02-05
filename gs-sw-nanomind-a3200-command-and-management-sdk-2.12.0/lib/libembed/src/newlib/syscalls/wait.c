/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _wait(int *wstatus)
{
    return _wait_r(_REENT, wstatus);
}

int _wait_r(struct _reent *reent, int *wstatus)
{
    reent->_errno = ENOSYS;
    return -1;
}

