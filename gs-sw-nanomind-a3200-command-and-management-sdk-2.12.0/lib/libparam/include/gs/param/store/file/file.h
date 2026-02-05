#ifndef GS_PARAM_STORE_FILE_FILE_H
#define GS_PARAM_STORE_FILE_FILE_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   File based stores.

   A \a file store uses standard file(s) for storing parameters. The store configration contains a \a path (directory), where each file
   is \a slot (= table).
*/

#include <gs/param/store/store.h>
#include <gs/util/string.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
   Store File map.
*/
typedef struct gs_param_store_file_map {
    /**
       Path (directory) where the store saves its slots (tables).
    */
    const char *path;
} gs_param_store_file_map_t;

/**
   Binary file store (.bin).

   Parameters are saved in binary form (big-endian), a long with checksum of table definition and data.

   File extension is: bin
*/
extern const gs_param_store_driver_t gs_param_store_file_driver_bin;

/**
   Human readable file store (.cfg).

   Parameters are saved in textual form, without any form of checksum/validation.

   File extension is: cfg

   Example:
   @include file_store/board.cfg

*/
extern const gs_param_store_driver_t gs_param_store_file_driver_cfg;

#ifdef __cplusplus
}
#endif
#endif
