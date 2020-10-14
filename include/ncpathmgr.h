/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */
#ifndef _NCWINIO_H_
#define _NCWINIO_H_

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "ncexternl.h"

#ifndef WINPATH
#ifdef _WIN32
#define WINPATH 1
#endif
#ifdef __MINGW32__
#define WINPATH 1
#endif
#endif

/* Define wrapper constants for use with NCaccess */
#ifdef _WIN32
#define ACCESS_MODE_EXISTS 0
#define ACCESS_MODE_R 4
#define ACCESS_MODE_W 2
#define ACCESS_MODE_RW 6
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#define O_RDWR _O_RDWR
#define O_APPEND _O_APPEND
#define O_BINARY _O_BINARY
#define O_CREAT _O_CREAT
#define O_EXCL _O_EXCL
#endif
#else
#define ACCESS_MODE_EXISTS (F_OK)
#define ACCESS_MODE_R (R_OK)
#define ACCESS_MODE_W (W_OK)
#define ACCESS_MODE_RW (R_OK|W_OK)
#endif

/* Path Converter */
EXTERNL char* NCpathcvt(const char* path);

/* path -> URL Path converter */
EXTERNL char* NCurlpath(const char* path);

/* Fix path in case it was escaped by shell */
EXTERNL char* NCdeescape(const char* name);

#ifdef WINPATH
/* path converter wrappers*/
EXTERNL FILE* NCfopen(const char* path, const char* flags);
EXTERNL int NCopen3(const char* path, int flags, int mode);
EXTERNL int NCopen2(const char* path, int flags);
EXTERNL int NCaccess(const char* path, int mode);
EXTERNL int NCremove(const char* path);
EXTERNL int NCmkdir(const char* path, int mode);
EXTERNL char* NCcwd(char* cwdbuf, size_t len);
#ifdef HAV_DIRENT_H
EXTERNL DIR* NCopendir(const char* path);
EXTERNL int NCclosedir(DIR* ent);
#endif
#else /*!WINPATH*/
#define NCfopen(path,flags) fopen((path),(flags))
#define NCopen3(path,flags,mode) open((path),(flags),(mode))
#define NCopen2(path,flags) open((path),(flags))
#define NCremove(path) remove(path)
#ifdef _WIN32
#define NCaccess(path,mode) _access(path,mode)
#else
#define NCaccess(path,mode) access(path,mode)
#endif
#define NCmkdir(path, mode) mkdir(path,mode)
#define NCcwd(buf, len) getcwd(buf,len)
#ifdef HAVE_DIRENT_H
#define NCopendir(path) opendir(path)
#define NCclosedir(ent) closedir(ent)
#endif
#endif /*WINPATH*/

/* Platform independent */
#define NCclose(fd) close(fd)

EXTERNL int NChasdriveletter(const char* path);

#endif /* _NCWINIO_H_ */
