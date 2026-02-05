/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/sys/memory.h>
#include <compiler.h>

// linker symbols
extern int _end;
extern int _heap_end;

void * gs_mem_get_int_ram_size(size_t * size)
{
    *size = HSRAM_SIZE;
    return GS_TYPES_UINT2PTR(HSRAM_ADDR);
}

gs_error_t gs_mem_heap_set(void * start, size_t size)
{
    return GS_ERROR_NOT_SUPPORTED;
}

void * gs_mem_heap_get_start(void)
{
    return &_end;
}

void * gs_mem_heap_get_end(void)
{
    return &_heap_end;
}
