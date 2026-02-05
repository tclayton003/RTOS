#ifndef GS_EMBED_DRIVERS_SYS_MEMORY_H
#define GS_EMBED_DRIVERS_SYS_MEMORY_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   MCU memory API.
*/

#include <gs/util/drivers/sys/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Get intern RAM start and size.
   @param[out] size size of memory. 0 if no memory present.
   @return Start of memory.
*/
void * gs_mem_get_int_ram_size(size_t * size);

/**
   Get extern RAM start and size.
   This function is weak in libembed and returns NULL/0 (no external memory).
   @param[out] size size of memory. 0 if no memory present.
   @return Start of memory.
*/
void * gs_mem_get_ext_ram_size(size_t * size);

/**
   Set heap area.
   Only supported on AVR32.
   @param[in] start start of heap.
   @param[in] size size of heap.
   @return_gs_error_t
*/
gs_error_t gs_mem_heap_set(void * start, size_t size);

/**
   Get start of heap.
   @return Start of heap (first byte). NULL if no heap present.
*/
void * gs_mem_heap_get_start(void);

/**
   Get end of heap.
   @return End of heap (last byte). NULL if no heap present.
*/
void * gs_mem_heap_get_end(void);

#ifdef __cplusplus
}
#endif
#endif
