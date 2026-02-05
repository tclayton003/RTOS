/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>

// A pointer to a list of environment variables and their values. For a minimal environment, this empty list is adequate:
char *__env[1] = { 0 };
char **environ = __env;
