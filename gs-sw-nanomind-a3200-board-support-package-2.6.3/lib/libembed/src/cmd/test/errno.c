/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <errno.h>

#if defined(_NEWLIB_VERSION)
// Get to the "int" errno, by un-defining the errno macro from newlib
#undef errno
extern int errno;
#endif

void gs_embed_test_get_errno(void ** ptr, int * value)
{
    *value = errno;
    *ptr = &errno;
}
