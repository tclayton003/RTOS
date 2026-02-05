/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "vfs_local.h"

int closedir(DIR *drp)
{
    return vfs_closedir(drp);
}

DIR *opendir(const char *name)
{
    return vfs_opendir(name);
}

struct dirent *readdir(DIR *drp)
{
    return vfs_readdir(drp);
}

void rewinddir(DIR *drp)
{
    vfs_rewinddir(drp);
}

int mkdir(const char *path, mode_t mode)
{
    return vfs_mkdir(path, mode);
}

int rmdir(const char *path)
{
    return vfs_rmdir(path);
}

void seekdir(DIR *drp, long offset)
{
    vfs_seekdir(drp, offset);
}

long telldir(DIR *drp)
{
    return vfs_telldir(drp);
}
