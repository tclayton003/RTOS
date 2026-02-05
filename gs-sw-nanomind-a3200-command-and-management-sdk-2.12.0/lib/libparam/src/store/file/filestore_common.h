/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include "../store_local.h"
#include <gs/param/store/file/file.h>

/**
   The maximum param file path len.
*/
#define MAX_FILE_PATH_LEN   256

void gs_param_file_store_make_filename(const char * root, const char * slot, const char * ext, char * filename_buf, size_t buf_size);

gs_error_t gs_param_file_store_clear(const gs_param_store_t * store, const char * slot, const char * ext, uint32_t flags);

gs_error_t gs_param_file_store_info(const gs_param_store_t * store, const char * ext, uint8_t detail_level, FILE * out);
