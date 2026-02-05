/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

// newlib: This one is not guaranteed to be available on all targets.
int _gettimeofday(struct timeval *__tp, void *__tzp)
{
    return _gettimeofday_r(_REENT, __tp, __tzp);
}

// newlib: This one is not guaranteed to be available on all targets.
int _gettimeofday_r(struct _reent *reent, struct timeval *__tp, void *__tzp)
{
    reent->_errno = ENOSYS;
    return -1;
}
