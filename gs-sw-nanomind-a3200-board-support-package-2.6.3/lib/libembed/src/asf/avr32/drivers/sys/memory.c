/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/drivers/sys/memory.h>
#include <malloc.h>
#include <compiler.h>

// linker symbols
extern int __heap_start__;
extern int __heap_end__;

// Allow overwriting heap area, so it can be moved to external memory
// - these should be static/private, but due to compatibility they must be public.
void * heap_start = &__heap_start__;
void * heap_end = &__heap_end__;

void * gs_mem_get_int_ram_size(size_t * size)
{
    *size = AVR32_SRAM_SIZE;
    return GS_TYPES_UINT2PTR(AVR32_SRAM_ADDRESS);
}

gs_error_t gs_mem_heap_set(void * start, size_t size)
{
    heap_start = start;
    heap_end = ((uint8_t*)start) + size - 1;
    return GS_OK;
}

void * gs_mem_heap_get_start(void)
{
    return heap_start;
}

void * gs_mem_heap_get_end(void)
{
    return heap_end;
}
