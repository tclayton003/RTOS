#ifndef GS_UTIL_MEMSTREAM_H
#define GS_UTIL_MEMSTREAM_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Memory streaming Interface.

   The memory stream provides a custom stream that streams to a memory buffer
   instead of to e.g. a FILE. When opening a memory stream the normal "FILE"
   handle is returned, which means that all the normal stream operations from
   libc (e.g. fwrite, fread, fprintf, ...) can be used on the stream.
*/

#include <stdio.h>
#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct memstream_handle {
    char*   buf;
    size_t  buf_size;
    bool    allocated;
    bool    buf_allocated;
    off_t   end_pos;
    off_t   cur_pos;
} gs_memstream_handle_t;

/**
   Memory Stream Open.

   Opens a memory (buffer) stream, and return a FILE pointer which can be used
   by the normal libc functions for read/write/seek.
   A FILE (cookie) structure is dynamically created on the Heap. A static version of this
   function is also supported gs_memstream_open_static(), which required the caller to
   have pre-allocated the cookie.

   @param[in] buf pointer to buffer. If NULL the buffer will be allocated by the open function.
   @param[in] size number of bytes.
   @returns FILE* or NULL on error.
*/
FILE* gs_memstream_open(void *buf, size_t size);

/**
   Memory Stream Open.

   Opens a memory (buffer) stream, and return a FILE pointer which can be used
   by the normal libc functions for read/write/seek.
   This function does not allocated the FILE handle on the Heap but uses the
   one provided by the caller. (The handle structure should not be touched by the
   application, as this is internal data for the memstream.

   @param[in] buf pointer to buffer. If NULL the buffer will be allocated by the open function.
   @param[in] size number of bytes.
   @param[in] handle memstream handle provided by the caller. The caller must allocate this (dynamic/static).
   @returns FILE* or NULL on error.
*/
FILE* gs_memstream_open_static(void *buf, size_t size, gs_memstream_handle_t *handle);

#ifdef __cplusplus
}
#endif
#endif
