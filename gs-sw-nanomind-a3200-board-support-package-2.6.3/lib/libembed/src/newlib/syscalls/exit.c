/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>
#include <gs/embed/drivers/sys/reset.h>
#include <stdio.h>

// if function actually called, samc can't link without this stub (missing _fini)
void exit(int status)
{
    _exit(status);
}

void _exit(int status)
{
    fprintf(stderr, "WARNING - do not call _exit(), status: %d -> reset\r\n", status);
    gs_sys_reset(GS_SYS_RESET_UNKNOWN); // there is no meaningful reason, but we should never get here anyways :)
    for (;;);
}
