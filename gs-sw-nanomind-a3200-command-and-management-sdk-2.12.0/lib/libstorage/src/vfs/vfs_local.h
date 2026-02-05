#ifndef LIBSTORAGE_SRC_VFS_LOCAL_VFS_H
#define LIBSTORAGE_SRC_VFS_LOCAL_VFS_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/storage/vfs/vfs.h>

#define VFS_MAX_OPEN_FILES	16
#define VFS_FD_OFFSET		1000

// vfs.c
gs_error_t vfs_init(void);
int vfs_open(const char *path, int oflag, int mode);
int vfs_close(int fildes);
int vfs_read(int fildes, void *buf, int nbyte);
int vfs_write(int fildes, const void *buf, int nbyte);
off_t vfs_lseek(int fildes, off_t offset, int whence);
int vfs_fstat(int fildes, struct stat *buf);
int vfs_fsync(int fildes);
int vfs_rename(const char *old, const char *new);
int vfs_unlink(const char *path);
int vfs_link(const char *oldpath, const char *newpath);
int vfs_stat(const char *path, struct stat *buf);
int vfs_lstat(const char *path, struct stat *buf);
int vfs_mkdir(const char *path, mode_t mode);
int vfs_rmdir(const char *path);
DIR *vfs_opendir(const char *path);
int vfs_closedir(DIR *dirp);
struct dirent *vfs_readdir(DIR *dirp);
void vfs_seekdir(DIR *dirp, long offset);
long vfs_telldir(DIR *dirp);
void vfs_rewinddir(DIR *dirp);
int vfs_statfs(const char *path, struct vfs_statfs *statfs);

// partitions.c
const struct vfs_partition *vfs_find_partition(const char *name);
typedef void (*vfs_part_cb_t)(const char *path, void * ctx);
int vfs_itertate_partitions(vfs_part_cb_t cb, void * ctx);

#endif
