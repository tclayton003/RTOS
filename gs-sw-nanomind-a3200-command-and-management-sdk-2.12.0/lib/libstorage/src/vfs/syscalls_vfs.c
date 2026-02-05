/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "vfs_local.h"
#include <gs/embed/newlib/syscalls.h>
#include <unistd.h>

int _fstat_r(struct _reent *reent, int fd, struct stat *statbuf)
{
    if (fd >= VFS_FD_OFFSET) {
        return vfs_fstat(fd, statbuf);
    }
    return gs_embed_syscall_fstat_r(reent, fd, statbuf);
}

int _link_r(struct _reent *reent, const char *oldpath, const char *newpath)
{
    return vfs_link(oldpath, newpath);
}

_off_t _lseek_r(struct _reent *reent, int fd, _off_t offset, int whence)
{
    if (fd >= VFS_FD_OFFSET) {
        return vfs_lseek(fd, offset, whence);
    }
    reent->_errno = EBADFD;
    return -1;
}

int _close_r(struct _reent *reent, int fd)
{
    if (fd >= VFS_FD_OFFSET) {
        return vfs_close(fd);
    }
    return gs_embed_syscall_close_r(reent, fd);
}

int _open_r(struct _reent *reent, const char *pathname, int flags, int mode)
{
    return vfs_open(pathname, flags, 0);
}

int _stat_r(struct _reent *reent, const char *pathname, struct stat *statbuf)
{
    return vfs_stat(pathname, statbuf);
}

int _unlink_r(struct _reent *reent, const char *pathname)
{
    return vfs_unlink(pathname);
}

int fsync(int fd)
{
    if (fd >= VFS_FD_OFFSET) {
        return vfs_fsync(fd);
    }
    errno = ENOSYS;
    return -1;
}

/* Newlib may be compiled without HAVE_RENAME, in which case it reverts to a
 * link+unlink combo. For filesystems which doesn't support hard links, such as
 * UFFS, renames wont work. We thus provide this function to override the
 * syscall with a direct call to vfs rename */
int _rename_r(struct _reent *reent, const char *oldpath, const char *newpath)
{
    return vfs_rename(oldpath, newpath);
}

_ssize_t _read_r(struct _reent *reent, int fd, void *buf, size_t count)
{
    if (fd >= VFS_FD_OFFSET) {
        return vfs_read(fd, buf, count);
    }
    return gs_embed_syscall_read_r(reent, fd, buf, count);
}

_ssize_t _write_r(struct _reent *reent, int fd, const void *buf, size_t count)
{
    if (fd >= VFS_FD_OFFSET) {
        return vfs_write(fd, buf, count);
    }
    return gs_embed_syscall_write_r(reent, fd, buf, count);
}
