#ifndef SRC_STORE_STORE_LOCAL_H
#define SRC_STORE_STORE_LOCAL_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/param/store/store.h>

#define GS_STORE_MAX_STORE_NAME 25
#define GS_STORE_PRINTF_NAME    "%-25.25s "

#define GS_STORE_MAX_SLOT_NAME  25

gs_error_t gs_param_store_iterate(const char * name_pattern, void * ctx, bool(*iter)(void *ctx, const gs_param_store_t * store));

#endif
