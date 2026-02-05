/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Fake heap/memory implementation for tests only.
*/

#include <gs/util/drivers/sys/memory.h>

__attribute__ ((__weak__)) void * gs_mem_get_int_ram_size(size_t * size)
{
    *size = 0;
    return NULL;
}

__attribute__ ((__weak__)) void * gs_mem_get_ext_ram_size(size_t * size)
{
    *size = 0;
    return NULL;
}

gs_error_t gs_mem_heap_set(void * start, size_t size)
{
    return GS_ERROR_NOT_SUPPORTED;
}

void * gs_mem_heap_get_start(void)
{
    return NULL;
}

void * gs_mem_heap_get_end(void)
{
    return NULL;
}
