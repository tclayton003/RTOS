/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

#if (GS_EMBED_DISABLE_SYSCALL_FS == 0)
int _rename_r(struct _reent *reent, const char *oldpath, const char *newpath)
{
    reent->_errno = ENOSYS;
    return -1;
}
#endif

int _rename(const char *oldpath, const char *newpath)
{
    return _rename_r(_REENT, oldpath, newpath);
}

#if (__NEWLIB__ < 2)
int rename(const char *oldpath, const char *newpath)
{
    return _rename_r(_REENT, oldpath, newpath);
}
#endif
