/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _kill(int pid, int sig)
{
    return _kill_r(_REENT, pid, sig);
}

int _kill_r(struct _reent *reent, int pid, int sig)
{
    reent->_errno = ENOSYS;
    return -1;
}

