/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _getpid(void)
{
    return _getpid_r(_REENT);
}

int _getpid_r(struct _reent *reent)
{
    return 1;
}

