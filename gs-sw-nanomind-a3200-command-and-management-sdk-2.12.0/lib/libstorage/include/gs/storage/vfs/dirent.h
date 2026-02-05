#ifndef GS_STORAGE_VFS_DIRENT_H
#define GS_STORAGE_VFS_DIRENT_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#if ( __linux__)
 #include <dirent.h>
#else
 #define GS_STORAGE_VFS 1
 #define _DIRENT_H_  // prevents standard dirent.h from being included
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_FILENAME_LENGTH
#define MAX_FILENAME_LENGTH 128
#endif

#if (GS_STORAGE_VFS)

#define DT_DIR 4

struct dirent {
	int d_ino;				/* inode number (serial number of this object) */
	char d_name[MAX_FILENAME_LENGTH];	/* name of this object */
	int d_off;				/* offset to this dirent */
	unsigned short int d_reclen;		/* length of this uffs_dirent */
	unsigned short int d_namelen;		/* length of this d_name */
	unsigned char d_type;			/* type of this record */
};

typedef struct {
	int fd;
	struct dirent ent;	
	void *real;
} DIR;

DIR *opendir(const char *name);
DIR *fdopendir(int fd);
int closedir(DIR *dirp);
struct dirent *readdir(DIR *dirp);
void seekdir(DIR *dirp, long offset);
void rewinddir(DIR *dirp);
long telldir(DIR *dirp);

#endif
#ifdef __cplusplus
}
#endif
#endif
