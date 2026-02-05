/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Virtual File System dispatcher

   The VFS acts as a wrapper between the Posix API and the various file system APIs.
*/

#include "vfs_local.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <gs/util/mutex.h>
#include <gs/util/string.h>

#define container_of(ptr, type, member) ({				\
            const typeof( ((type *)0)->member ) *__mptr = (ptr);        \
            (type *)( (void *)__mptr - offsetof(type,member) );})

#define vfs_fd_has_fop(_fildes, _fop)					\
    ((_fildes) >= VFS_FD_OFFSET &&					\
     (_fildes) < VFS_FD_OFFSET + VFS_MAX_OPEN_FILES &&                  \
     fd_table[(_fildes) - VFS_FD_OFFSET].state == VFS_FD_USED &&	\
     fd_table[(_fildes) - VFS_FD_OFFSET].partition &&                   \
     fd_table[(_fildes) - VFS_FD_OFFSET].partition->fops &&		\
     fd_table[(_fildes) - VFS_FD_OFFSET].partition->fops->_fop)

#define vfs_part_has_fop(_part, _fop)           \
    (_part && _part->fops && _part->fops->_fop)

#define vfs_part_call(_part, _fop, ...)         \
    _part->fops->_fop(__VA_ARGS__)

#define vfs_fd_call(_fildes, _fop, ...)					\
    fd_table[(_fildes) - VFS_FD_OFFSET].partition->fops->_fop(&fd_table[_fildes - VFS_FD_OFFSET], ## __VA_ARGS__)

#define vfs_dirp_call(_dirp, _fop, ...)					\
    fd_table[vfs_dirp_to_fd(_dirp) - VFS_FD_OFFSET].partition->fops->_fop(&fd_table[vfs_dirp_to_fd(_dirp) - VFS_FD_OFFSET], ## __VA_ARGS__);

#define vfs_dirp_to_fd(_dirp) ({					\
            (((unsigned)container_of(_dirp, struct vfs_fd, dirp) - (unsigned)fd_table) / sizeof(struct vfs_fd)) + VFS_FD_OFFSET;})

static struct vfs_fd fd_table[VFS_MAX_OPEN_FILES];

static gs_mutex_t g_global_lock = NULL;

/**
   Returns length of partition part of string - ex. path '/flash/file' returns length of '/flash' = 6.
 */
static int get_partition_len(const char *path, unsigned int max_len, unsigned int *len)
{
    if (*path++ != '/') {
        return -1;
    }

    unsigned int part_size = 1;
    while (*path && (*path != '/')) {
        part_size++;
        path++;
        if (part_size > max_len) {
            return -1;
        }
    }
    *len = part_size;
    return 0;
}

/**
   Extract partition from path - ex. path '/flash/file' returns 'flash'.
 */
static int vfs_extract_partition(const char *path, char *out, unsigned int outlen)
{
    unsigned int part_len;
    if (get_partition_len(path, outlen, &part_len) != 0) {
        return -1;
    }
    path++; // Leave out the starting '/' 
    memcpy(out, path, part_len - 1);
    out[part_len - 1] = 0; // Terminate string

    return 0;
}

static int subst_partition(const char *path, const char *part, char *out, unsigned int outlen)
{
    unsigned int original_part_len;
    if (get_partition_len(path, strlen(path), &original_part_len) != 0) {
        return -1;
    }
    unsigned int file_len = strlen(path) - original_part_len; // Length of path without partition
    unsigned int new_path_len = file_len + strlen(part) + 2; // File length + new part length + starting '/' + zero termination
    if (new_path_len >= outlen) {
        return -1;
    }

    out[0] = '/'; // Start new partition with '/'
    strcpy(&out[1], part); // Copy new partition
    strcat(out, &path[original_part_len]); // Append file path

    return 0;
}

static unsigned int cmpxchg(unsigned int *var, unsigned int old, unsigned int new)
{
    unsigned int org;

    gs_mutex_lock(g_global_lock);
    org = *var;
    if (org == old) {
        *var = new;
    }
    gs_mutex_unlock(g_global_lock);

    return org;
}

static int vfs_fd_get(void)
{
    int ret = -1;

    for (int i = 0; i < VFS_MAX_OPEN_FILES; i++) {
        if (cmpxchg(&fd_table[i].state, VFS_FD_FREE, VFS_FD_USED) == VFS_FD_FREE) {
            ret = i + VFS_FD_OFFSET;
            break;
        }
    }

    return ret;
}

static int vfs_fd_put(int fd)
{
    return cmpxchg(&fd_table[fd - VFS_FD_OFFSET].state, VFS_FD_USED, VFS_FD_FREE) != VFS_FD_USED;
}

gs_error_t vfs_init(void)
{
    if (g_global_lock == NULL) {
        return gs_mutex_create(&g_global_lock);
    }
    return GS_OK;
}

int vfs_open(const char *path, int oflag, int mode)
{
    char part[MAX_FILENAME_LENGTH];

    if (vfs_extract_partition(path, part, MAX_FILENAME_LENGTH) < 0)
        return -1;

    const struct vfs_partition *p = vfs_find_partition(part);
    if (!vfs_part_has_fop(p, open))
        return -1;

    if (p->true_name) {
        subst_partition(path, p->true_name, part, MAX_FILENAME_LENGTH);
        path = part;
    }

    int fd_num = vfs_fd_get();
    if (fd_num < 0)
        return -1;

    fd_table[fd_num - VFS_FD_OFFSET].partition = p;
    fd_table[fd_num - VFS_FD_OFFSET].path = strndup(path, MAX_FILENAME_LENGTH);

    int ret = vfs_part_call(p, open, &fd_table[fd_num - VFS_FD_OFFSET], path, oflag, mode);

    if (ret < 0) {
        if (fd_table[fd_num - VFS_FD_OFFSET].path)
            free(fd_table[fd_num - VFS_FD_OFFSET].path);
        vfs_fd_put(fd_num);
        return -1;
    }

    return fd_num;
}

int vfs_close(int fildes)
{
    if (!vfs_fd_has_fop(fildes, close))
        return -1;

    int ret = vfs_fd_call(fildes, close);
    if (fd_table[fildes - VFS_FD_OFFSET].path)
        free(fd_table[fildes - VFS_FD_OFFSET].path);

    vfs_fd_put(fildes);

    return ret;
}

int vfs_read(int fildes, void *buf, int nbyte)
{
    if (!vfs_fd_has_fop(fildes, read))
        return -1;

    return vfs_fd_call(fildes, read, buf, nbyte);
}

int vfs_write(int fildes, const void *buf, int nbyte)
{
    if (!vfs_fd_has_fop(fildes, write))
        return -1;

    return vfs_fd_call(fildes, write, buf, nbyte);
}

off_t vfs_lseek(int fildes, off_t offset, int whence)
{
    off_t retoffset = -1;

    if (!vfs_fd_has_fop(fildes, lseek))
        return -1;

    if (vfs_fd_call(fildes, lseek, &retoffset, offset, whence) < 0)
        return -1;

    return retoffset;
}

int vfs_fsync(int fildes)
{
    if (!vfs_fd_has_fop(fildes, fsync))
        return -1;

    return vfs_fd_call(fildes, fsync);
}

int vfs_rename(const char *old, const char *new)
{
    char partold[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(old, partold, MAX_FILENAME_LENGTH) < 0)
        return -1;

    char partnew[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(new, partnew, MAX_FILENAME_LENGTH) < 0)
        return -1;

    const struct vfs_partition *pold = vfs_find_partition(partold);
    const struct vfs_partition *pnew = vfs_find_partition(partnew);

    /* We don't support moving between filesystems yet */
    if (pold == pnew) {
        if (!vfs_part_has_fop(pold, rename))
            return -1;

        if (pold->true_name) {
            subst_partition(old, pold->true_name, partold, MAX_FILENAME_LENGTH);
            subst_partition(new, pnew->true_name, partnew, MAX_FILENAME_LENGTH);
            old = partold;
            new = partnew;
        }

        return vfs_part_call(pold, rename, old, new);
    } else {
        return -1;
    }
}

int vfs_link(const char *oldpath, const char *newpath)
{
    char partold[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(oldpath, partold, MAX_FILENAME_LENGTH) < 0)
        return -1;

    char partnew[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(newpath, partnew, MAX_FILENAME_LENGTH) < 0)
        return -1;

    /* link(2) cannot span file systems */
    if (strncmp(partold, partnew, MAX_FILENAME_LENGTH) != 0)
        return -1;

    const struct vfs_partition *p = vfs_find_partition(partold);
    if (!vfs_part_has_fop(p, link))
        return -1;

    if (p->true_name) {
        subst_partition(oldpath, p->true_name, partold, MAX_FILENAME_LENGTH);
        subst_partition(newpath, p->true_name, partnew, MAX_FILENAME_LENGTH);
        oldpath = partold;
        newpath = partnew;
    }

    return vfs_part_call(p, link, oldpath, newpath);
}

int vfs_unlink(const char *path)
{
    char part[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(path, part, MAX_FILENAME_LENGTH) < 0)
        return -1;

    const struct vfs_partition *p = vfs_find_partition(part);
    if (!vfs_part_has_fop(p, unlink))
        return -1;

    if (p->true_name) {
        subst_partition(path, p->true_name, part, MAX_FILENAME_LENGTH);
        path = part;
    }

    return vfs_part_call(p, unlink, path);
}

int vfs_stat(const char *path, struct stat *buf)
{
    char part[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(path, part, MAX_FILENAME_LENGTH) < 0)
        return -1;

    const struct vfs_partition *p = vfs_find_partition(part);
    if (!vfs_part_has_fop(p, stat))
        return -1;

    if (p->true_name) {
        subst_partition(path, p->true_name, part, MAX_FILENAME_LENGTH);
        path = part;
    }

    return vfs_part_call(p, stat, path, buf);
}

int vfs_lstat(const char *path, struct stat *buf)
{
    char part[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(path, part, MAX_FILENAME_LENGTH) < 0)
        return -1;

    const struct vfs_partition *p = vfs_find_partition(part);
    if (!vfs_part_has_fop(p, lstat))
        return -1;

    if (p->true_name) {
        subst_partition(path, p->true_name, part, MAX_FILENAME_LENGTH);
        path = part;
    }

    return vfs_part_call(p, lstat, path, buf);
}

int vfs_fstat(int fildes, struct stat *buf)
{
    if (!vfs_fd_has_fop(fildes, fstat))
        return -1;

    return vfs_fd_call(fildes, fstat, buf);
}

DIR *vfs_opendir(const char *path)
{
    char part[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(path, part, MAX_FILENAME_LENGTH) < 0)
        return NULL;

    const struct vfs_partition *p = vfs_find_partition(part);
    if (!vfs_part_has_fop(p, opendir))
        return NULL;

    if (p->true_name) {
        subst_partition(path, p->true_name, part, MAX_FILENAME_LENGTH);
        path = part;
    }

    int fd_num = vfs_fd_get();
    if (fd_num < 0)
        return NULL;

    fd_table[fd_num - VFS_FD_OFFSET].partition = p;
    fd_table[fd_num - VFS_FD_OFFSET].path = strndup(path, MAX_FILENAME_LENGTH);

    if (vfs_part_call(p, opendir, &fd_table[fd_num - VFS_FD_OFFSET], path) < 0) {
        if (fd_table[fd_num - VFS_FD_OFFSET].path)
            free(fd_table[fd_num - VFS_FD_OFFSET].path);
        vfs_fd_put(fd_num);
        return NULL;
    }

    return &fd_table[fd_num - VFS_FD_OFFSET].dirp;
}

int vfs_closedir(DIR *dirp)
{
    int fd = vfs_dirp_to_fd(dirp);
    if (!vfs_fd_has_fop(fd, closedir))
        return -1;

    int ret = vfs_fd_call(fd, closedir);
    if (fd_table[fd - VFS_FD_OFFSET].path)
        free(fd_table[fd - VFS_FD_OFFSET].path);

    vfs_fd_put(vfs_dirp_to_fd(dirp));

    return ret;
}

struct dirent *vfs_readdir(DIR *dirp)
{
    int ret;

    if (!vfs_fd_has_fop(vfs_dirp_to_fd(dirp), readdir))
        return NULL;

    ret = vfs_dirp_call(dirp, readdir, &dirp->ent);
    if (ret < 0)
        return NULL;

    return &dirp->ent;
}

void vfs_rewinddir(DIR *dirp)
{
    if (!vfs_fd_has_fop(vfs_dirp_to_fd(dirp), rewinddir))
        return;

    vfs_dirp_call(dirp, rewinddir);
}

int vfs_mkdir(const char *path, mode_t mode)
{
    char part[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(path, part, MAX_FILENAME_LENGTH) < 0)
        return -1;

    const struct vfs_partition *p = vfs_find_partition(part);
    if (!p || !p->fops || !p->fops->mkdir)
        return -1;

    if (p->true_name) {
        subst_partition(path, p->true_name, part, MAX_FILENAME_LENGTH);
        path = part;
    }

    return p->fops->mkdir(path, mode);
}

int vfs_rmdir(const char *path)
{
    char part[MAX_FILENAME_LENGTH];
    if (vfs_extract_partition(path, part, MAX_FILENAME_LENGTH) < 0)
        return -1;

    const struct vfs_partition *p = vfs_find_partition(part);
    if (!p || !p->fops || !p->fops->rmdir)
        return -1;

    if (p->true_name) {
        subst_partition(path, p->true_name, part, MAX_FILENAME_LENGTH);
        path = part;
    }

    return p->fops->rmdir(path);
}

void vfs_seekdir(DIR *dirp, long offset)
{
    if (!vfs_fd_has_fop(vfs_dirp_to_fd(dirp), seekdir))
        return;

    vfs_dirp_call(dirp, seekdir, offset);
}

long vfs_telldir(DIR *dirp)
{
    long location;

    if (!vfs_fd_has_fop(vfs_dirp_to_fd(dirp), telldir))
        return -1;

    vfs_dirp_call(dirp, telldir, &location);

    return location;
}

int vfs_mkfs(const char *path, unsigned int flags)
{
    const struct vfs_partition *p = vfs_find_partition(path);
    if (!vfs_part_has_fop(p, mkfs)) {
        return -1;
    }

    if (p->true_name) {
        path = p->true_name;
    }

    return vfs_part_call(p, mkfs, path, flags);
}

int vfs_statfs(const char *path, struct vfs_statfs *statfs)
{
    const struct vfs_partition *p = vfs_find_partition(path);
    if (!vfs_part_has_fop(p, statfs)) {
        return -1;
    }

    if (p->true_name) {
        path = p->true_name;
        statfs->true_name = p->true_name;
    }

    statfs->name = p->fops->fs;

    return vfs_part_call(p, statfs, path, statfs);
}
