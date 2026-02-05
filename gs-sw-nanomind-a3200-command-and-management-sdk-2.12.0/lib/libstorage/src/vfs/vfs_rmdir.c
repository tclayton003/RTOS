/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/storage/vfs/vfs.h>
#include <gs/util/string.h>
#include <stdio.h>
#include <unistd.h>

static int rmdir_recursive(const char *path)
{
    int ret = 0;
    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        return -1;
    }
    /* Loop through directories */
    struct dirent *ent;
    // cppcheck-suppress readdirCalled - readdir_r not supported
    while (((ent = readdir(dirp)) && (ret == 0))) {
        if (ent->d_name[0] == '.') {
            continue;
        }
        char buf[MAX_FILENAME_LENGTH + 2]; // add space for /
        GS_STRNCPY(buf, path);
        char *sub = buf;
        if (path[strlen(path) - 1] != '/') {
            sub = strcat(buf, "/");
        }
        sub = strcat(sub, ent->d_name);

        struct stat st;
        ret = stat(sub, &st);
        if (ret == 0) {
            if (st.st_mode & S_IFDIR) {
                ret = rmdir_recursive(sub);
            } else {
                ret = remove(sub);
            }
        }
    }
    closedir(dirp);
    if (ret != 0) {
        return ret;
    }
    return rmdir(path);

}

int vfs_rmdir_recursive(const char *path, bool recursive)
{
    if (recursive) {
        return rmdir_recursive(path);
    }
    return rmdir(path);
}
