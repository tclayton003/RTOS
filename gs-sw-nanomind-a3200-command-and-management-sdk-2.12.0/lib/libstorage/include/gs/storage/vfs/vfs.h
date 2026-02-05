#ifndef GS_STORAGE_VFS_VFS_H
#define GS_STORAGE_VFS_VFS_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Virtual File System.

   The VFS acts as a wrapper between the Posix API and the various file system APIs.
*/

#include <gs/storage/vfs/dirent.h>
#include <gs/util/error.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <utime.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   File descriptor (fd) state.
*/
enum vfs_fd_state {
    /**
       FD free.
    */
    VFS_FD_FREE = 0,
    /**
       FD in use.
    */
    VFS_FD_USED = 1,
};

struct vfs_fd;

/**
   File system stats.
*/
struct vfs_statfs {
    /**
       Name of file system.
    */
    const char *name;
    /**
       True name of file system.
    */
    const char *true_name;
    /**
       Flags.
    */
    unsigned int flags;
    /**
       Free bytes.
    */
    unsigned int free;
    /**
       Used bytes.
    */
    unsigned int used;
    /**
       Total bytes.
    */
    unsigned int total;
};

/**
   Interface for a file system.
*/
struct vfs_ops {
    /**
       File system name.
    */
    const char *fs;
    /**
       Open.
    */
    int (*open)(struct vfs_fd *fd, const char *path, int oflag, int mode);
    /**
       Close.
    */
    int (*close)(struct vfs_fd *fd);
    /**
       Read.
    */
    int (*read)(struct vfs_fd *fd, void *buf, int nbyte);
    /**
       Write.
    */
    int (*write)(struct vfs_fd *fd, const void *buf, int nbyte);
    /**
       Seek.
    */
    int (*lseek)(struct vfs_fd *fd, off_t *retoffset, off_t offset, int whence);
    /**
       File stat.
    */
    int (*fstat)(struct vfs_fd *fd, struct stat *buf);
    /**
       Sync file.
    */
    int (*fsync)(struct vfs_fd *fd);
    /**
       Truncate file by FD.
    */
    int (*ftruncate)(struct vfs_fd *fd, off_t length);
    /**
       Change modifiers.
    */
    int (*fchmod)(struct vfs_fd *fd, mode_t mode);
    /**
       Truncate file by path.
    */
    int (*truncate)(const char *path, off_t length);
    /**
       Rename.
    */
    int (*rename)(const char *old, const char *new);
    /**
       Link (rename).
    */
    int (*link)(const char *old, const char *new);
    /**
       Un-link (remove).
    */
    int (*unlink)(const char *pathname);
    /**
       Stat.
    */
    int (*stat)(const char *path, struct stat *buf);
    /**
       Link stat (-> stat).
    */
    int (*lstat)(const char *path, struct stat *buf);
    /**
       Make/create directory.
    */
    int (*mkdir)(const char *path, mode_t mode);
    /**
       Remove/delete directory.
    */
    int (*rmdir)(const char *path);
    /**
       Change file mod ebits.
    */
    int (*chmod)(const char *path, mode_t mode);
    //int (*utime)(const char *path, const struct utimbuf *times);
    /**
       Open directory.
    */
    int (*opendir)(struct vfs_fd *fd, const char *name);
    /**
       Close directory.
    */
    int (*closedir)(struct vfs_fd *fd);
    /**
       Read directory.
    */
    int (*readdir)(struct vfs_fd *fd, struct dirent *dirent);
    /**
       Set next position in the directory entry.
    */
    int (*seekdir)(struct vfs_fd *fd, long offset);
    /**
       Get current position in the directory stream.
    */
    int (*telldir)(struct vfs_fd *fd, long *location);
    /**
       Reset directory stream.
    */
    int (*rewinddir)(struct vfs_fd *fd);
    /**
       Create file system.
    */
    int (*mkfs)(const char *path, unsigned int flags);
    /**
       Stat file system.
    */
    int (*statfs)(const char *path, struct vfs_statfs *statfs);
};

/**
   VFS partition.
*/
struct vfs_partition {
    /**
       Name of partition.
    */
    const char *name;
    /**
       True partition name.
    */
    const char *true_name;
    /**
       File interface.
    */
    const struct vfs_ops *fops;
};

/**
   VFS file-descriptor.
*/
struct vfs_fd {
    /**
       State, see #vfs_fd_state.
    */
    unsigned int state;
    /**
       Partition.
    */
    const struct vfs_partition *partition;
    /**
       File-descriptor.
    */
    int fd;
#if (GS_STORAGE_VFS)
    /**
       State information for directory operations.
    */
    DIR dirp;
#endif
    /**
       Path.
    */
    char *path;
};

/**
   Initialize and mount file-systems.

   This function can be extended to initialize/mount custom file-systems.

   Default mounted file-systems:
   - /flash0: partition/die 0 on SPN FL512S.
   - /flash1: partition/die 1 on SPN FL512S.
   - /flash:  alias for either /flash0 or /flash1.

   @param[in] default_partition default partition (0 = '/flash -> /flash0', 1 = '/flash -> /flash1')
   @param[in] mount_p0 flag to mount /flash0
   @param[in] mount_p1 flag to mount /flash1
   @return_gs_error_t
*/
gs_error_t vfs_mount(uint8_t default_partition, bool mount_p0, bool mount_p1);

/**
   mkfs flag.
   Forces reset of the system, after filesystem has been created.
*/
#define VFS_MKFS_FORCE		(1U << 16)

/**
   Make/create file system.
   @param path path or name of file system.
   @param flags control flags, see #VFS_MKFS_FORCE.
   @return 0 on success.
*/
int vfs_mkfs(const char *path, unsigned int flags);

/**
   Remove directory.

   @param[in] path Directory path.
   @param[in] recursive Optional recursive removal of directory (true = recursive removal).
   @return 0 on success and -1 on error
*/
int vfs_rmdir_recursive(const char *path, bool recursive);

/**
   Register commands for VFS or standard filesystem commands.

   @return_gs_error_t
*/
gs_error_t vfs_register_commands(void);

/**
   Register commands for VFS or standard filesystem commands.

   @return_gs_error_t
*/
static inline gs_error_t gs_vfs_register_commands(void)
{
    return vfs_register_commands();
}

#ifdef __cplusplus
}
#endif
#endif
