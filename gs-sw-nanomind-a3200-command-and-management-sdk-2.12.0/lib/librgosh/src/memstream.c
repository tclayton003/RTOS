/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memstream.h"

//#define _MEMSTREAM_DBG_ 1

typedef gs_memstream_handle_t memstream_cookie_t;

static ssize_t memstream_read(void *c, char *buf, size_t size)
{
    gs_memstream_handle_t *cookie = c;

#if (MEMSTREAM_DBG_)
    printf("->memstream_read(): sz:<%zu>, cur:<%zu>, end:<%zu>\n",
           cookie->buf_size, cookie->cur_pos, cookie->end_pos);
#endif

    ssize_t bytes = size;
    if (cookie->cur_pos + size > (size_t)cookie->end_pos) {
        bytes = cookie->end_pos - cookie->cur_pos;
    }

    if (bytes < 0) {
        bytes = 0;
    }

    memcpy(buf, cookie->buf + cookie->cur_pos, bytes);

    cookie->cur_pos += bytes;

#if (_MEMSTREAM_DBG_)
    printf("<-memstream_read(): sz:<%zu>, cur:<%zu>, end:<%zu>, read:<%zd>\n",
            cookie->buf_size, cookie->cur_pos, cookie->end_pos, bytes);
#endif

    return bytes;
}

static ssize_t memstream_write(void *c, const char *buf, size_t size)
{
    gs_memstream_handle_t *cookie = c;

#if (_MEMSTREAM_DBG_)
    printf("->memstream_write(): sz:<%zu>, cur:<%zu>, end:<%zu>\n",
           cookie->buf_size, cookie->cur_pos, cookie->end_pos);
#endif

    if (cookie->cur_pos + size > cookie->buf_size) {
        size = cookie->buf_size - cookie->cur_pos;
    }

    memcpy(cookie->buf + cookie->cur_pos, buf, size);

    cookie->cur_pos += size;
    if (cookie->cur_pos > cookie->end_pos) {
       cookie->end_pos = cookie->cur_pos;
    }

#if (_MEMSTREAM_DBG_)
    printf("<-memstream_write(): sz:<%zu>, cur:<%zu>, end:<%zu>, wrote:<%zu>\n",
           cookie->buf_size, cookie->cur_pos, cookie->end_pos, size);
#endif

    return size;
}

// Modern gcc compilers may always use 64bit files,
#if (__GNUC__ >= 5)
static int memstream_seek(void *c, off64_t *offset, int whence)
#else
static int memstream_seek(void *c, off_t *offset, int whence)
#endif
{
    gs_memstream_handle_t *cookie = c;
    off_t new_pos;

#if (_MEMSTREAM_DBG_)
    printf("->memstream_seek(): sz:<%zu>, cur:<%zu>, end:<%zu>\n",
           cookie->buf_size, cookie->cur_pos, cookie->end_pos);
#endif

    if (whence == SEEK_SET) {
       new_pos = *offset;
    } else if (whence == SEEK_END) {
       new_pos = cookie->end_pos + *offset;
    } else if (whence == SEEK_CUR) {
       new_pos = cookie->cur_pos + *offset;
    } else {
       return -1;
    }

    if (new_pos < 0 || (size_t)new_pos > cookie->buf_size) {
       return -1;
    }

    cookie->cur_pos = new_pos;
    *offset = new_pos;

#if (_MEMSTREAM_DBG_)
    printf("<-memstream_seek(): sz:<%zu>, cur:<%zu>, end:<%zu>\n",
           cookie->buf_size, cookie->cur_pos, cookie->end_pos);
#endif

    return 0;
}

static int memstream_close(void *c)
{
    gs_memstream_handle_t *cookie = c;

    if (cookie->buf_allocated) {
        free(cookie->buf);
    }

    cookie->buf_allocated = false;
    cookie->buf = NULL;
    cookie->buf_size = 0;
    if (cookie->allocated) {
        free(cookie);
    }
    return 0;
}


static const cookie_io_functions_t memstream_fops = {
    .read  = memstream_read,
    .write = memstream_write,
    .seek  = memstream_seek,
    .close = memstream_close,
};

FILE* gs_memstream_open(void *buf, size_t size)
{
    gs_memstream_handle_t * cookie = malloc(sizeof(*cookie));
    if (cookie == NULL) {
        return NULL;
    }

    FILE* fp = gs_memstream_open_static(buf, size, cookie);
    if (fp == NULL) {
        free(cookie);
    } else {
        cookie->allocated = true;
    }
    // cppcheck-suppress memleak - cookie freed, when closing stream
    return fp;
}

FILE* gs_memstream_open_static(void *buf, size_t size, gs_memstream_handle_t *cookie)
{
    if (cookie == NULL) {
        return NULL;
    }
    memset(cookie, 0, sizeof(*cookie));

    if (buf == NULL) {
        buf = calloc(1, size);
        if (buf == NULL) {
            return NULL;
        }
        cookie->buf_allocated = true;
    }

    cookie->buf = buf;
    cookie->buf_size = size;

    return fopencookie(cookie, "w+", memstream_fops);
}
