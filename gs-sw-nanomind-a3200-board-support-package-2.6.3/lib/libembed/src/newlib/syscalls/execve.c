/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

int _execve(char *filename, char **argv, char **envp)
{
    return _execve_r(_REENT, filename, argv, envp);
}

#if (__NEWLIB__ >= 2)
int _execve_r(struct _reent *reent, const char *filename, char * const * argv, char * const * envp)
#else
int _execve_r(struct _reent *reent, char *filename, char ** argv, char ** envp)
#endif
{
    reent->_errno = ENOSYS;
    return -1;
}
