#ifndef GS_A3200_FILE_SYSTEM_H
#define GS_A3200_FILE_SYSTEM_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   Mount the UFFS file system.
*/

#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Callback with the result of mounting the file system.

   @param[in] result result of the mount, #GS_OK on success.
*/
typedef void (*gs_a3200_mount_fs_completed_t)(gs_error_t result);

/**
   Start of task for mounting the file system.

   Because the process is so slow, the actual mount is done in a separate task.

   @param[in] cb callback with the result of the mount.
   @return_gs_error_t
*/
gs_error_t gs_a3200_fs_mount(gs_a3200_mount_fs_completed_t cb);

#ifdef __cplusplus
}
#endif
#endif
