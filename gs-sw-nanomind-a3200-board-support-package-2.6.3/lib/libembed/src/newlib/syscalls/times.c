/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <sys/times.h>
#include <gs/embed/newlib/syscalls.h>
#include <gs/util/time.h>

_CLOCK_T_ _times(struct tms *buf)
{
    return _times_r(_REENT, buf);
}

_CLOCK_T_ _times_r(struct _reent *reent, struct tms *buf)
{
    // from avr32 port - not sure it is used anywhere
    buf->tms_utime = 0;
    buf->tms_stime = gs_time_rel_ms();;
    buf->tms_cutime = 0;
    buf->tms_cstime = 0;
    return buf->tms_stime;
}
