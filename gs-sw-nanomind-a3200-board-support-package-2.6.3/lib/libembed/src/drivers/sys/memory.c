/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/embed/drivers/sys/memory.h>
#include <malloc.h>

// By default embedded platform do not have external memory - any platforms (i.e. A3200) must implement this function.
__attribute__ ((__weak__)) void * gs_mem_get_ext_ram_size(size_t * size)
{
    *size = 0;
    return NULL;
}

// The default memory is where the heap is placed.
gs_mem_ram_type_t gs_mem_get_ram_default()
{
    size_t mem_size;
    uint8_t * mem_start = gs_mem_get_ext_ram_size(&mem_size);
    if (mem_size) {
        const uint8_t * heap_start = gs_mem_heap_get_start();
        if ((heap_start >= mem_start) && (heap_start < (mem_start + mem_size))) {
            return GS_MEM_RAM_TYPE_EXTERNAL;
        }
    }
    return GS_MEM_RAM_TYPE_INTERNAL;
}

static gs_error_t get_mem_stat(gs_mem_ram_stat_t * ram_stat, uint8_t * mem_start, size_t mem_size)
{
    if (mem_size) {
        ram_stat->total = mem_size;

        // Check if heap is located here
        const uint8_t * heap_start = gs_mem_heap_get_start();
        if ((heap_start >= mem_start) && (heap_start < (mem_start + mem_size))) {
            const uint8_t * heap_end = gs_mem_heap_get_end();

            ram_stat->max_available = (heap_end - heap_start + 1);

            extern int __malloc_max_sbrked_mem; // from newlib
            ram_stat->min_available = (ram_stat->max_available - __malloc_max_sbrked_mem);

            struct mallinfo m_info = mallinfo(); // from newlib
            ram_stat->available = (ram_stat->max_available - m_info.uordblks);

        } else {
            // heap not placed here.
            ram_stat->max_available = ram_stat->available = ram_stat->min_available = 0;
        }
        return GS_OK;
    }
    return GS_ERROR_NOT_SUPPORTED;
}

gs_error_t gs_mem_get_ext_ram_stat(gs_mem_ram_stat_t * ram_stat)
{
    size_t size;
    void * start = gs_mem_get_ext_ram_size(&size);
    return get_mem_stat(ram_stat, start, size);
}

gs_error_t gs_mem_get_int_ram_stat(gs_mem_ram_stat_t * ram_stat)
{
    size_t size;
    void * start = gs_mem_get_int_ram_size(&size);
    return get_mem_stat(ram_stat, start, size);
}
