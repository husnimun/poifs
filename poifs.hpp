#pragma once

/** Header implementasi fungsi-fungsi fuse */

// pakai versi 26
#define FUSE_USE_VERSION 26

#include <errno.h>
#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "poi.hpp"

int poifs_getattr(const char* path, struct stat* stbuf);

int poifs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

int poifs_mkdir(const char *path, mode_t mode);

int poifs_open(const char* path, struct fuse_file_info* fi);

int poifs_rmdir(const char *path);

int poifs_rename(const char* path, const char* newpath);

int poifs_unlink(const char *path);

int poifs_mknod(const char *path, mode_t mode, dev_t dev);

int poifs_truncate(const char *path, off_t newsize);

int poifs_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi);

int poifs_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);

int poifs_link(const char *path, const char *newpath);

int poifs_chmod(const char *path, unsigned int mode);
