/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   Implementation of re-entrant system calls.
*/

#include <gs/embed/newlib/syscalls.h>
#include <gs/embed/drivers/sys/memory.h>
#include <gs/embed/freertos.h>

// newlib: lock for malloc/free operation.
void __malloc_lock(struct _reent *ptr)
{
    vTaskSuspendAll();
}

// newlib: unlock
void __malloc_unlock(struct _reent *ptr)
{
    xTaskResumeAll();
}

void *_sbrk(ptrdiff_t incr)
{
    return _sbrk_r(_REENT, incr);
}

void *_sbrk_r(struct _reent *reent, ptrdiff_t incr)
{
    static uint8_t * cur_heap_pos = 0;
    if (cur_heap_pos == 0) {
        cur_heap_pos = gs_mem_heap_get_start();
    }

    if (incr == 0) {
        return cur_heap_pos;
    }

    // - 1 to use every single byte of heap
    if ((cur_heap_pos + incr - 1) > ((uint8_t*)gs_mem_heap_get_end())) {
        // Don't print/log here - it may end up causing a deadlock (perhaps you  will be saved by a watchdog)
        reent->_errno = ENOMEM;
        return (void *) -1;
    }

    void * old_heap_pos = cur_heap_pos;
    cur_heap_pos += incr;
    return old_heap_pos;
}
