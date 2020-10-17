/*
 * Copyright 2018, University Corporation for Atmospheric Research
 * See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef _WIN32
#include <io.h>
#endif

#include "ncexternl.h"
#include "ncpathmgr.h"

extern char *realpath(const char *path, char *resolved_path);

#undef PATHFORMAT

/*
Code to provide some path conversion code so that
cygwin and (some) mingw paths can be passed to open/fopen
for Windows. Other cases will be added as needed.
Rules:
1. a leading single alpha-character path element (e.g. /D/...)
   will be interpreted as a windows drive letter.
2. a leading '/cygdrive/X' will be converted to
   a drive letter X if X is alpha-char.
3. a leading D:/... is treated as a windows drive letter
4. a relative path will be converted to an absolute path.
5. If any of the above is encountered, then forward slashes
   will be converted to backslashes.
All other cases are passed thru unchanged
*/

/* Define legal windows drive letters */
static const char* windrive = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const size_t cdlen = 10; /* strlen("/cygdrive/") */

static int pathdebug = -1;

static char* makeabsolute(const char* relpath);

EXTERNL
char* /* caller frees */
NCpathcvt(const char* path)
{
    char* outpath = NULL; 
    char* p;
    char* q;
    size_t pathlen;
    int forwardslash;

    if(path == NULL) goto done; /* defensive driving */

#ifdef _WIN32
    forwardslash = 0;
#else
    forwardslash = 1;
#endif

    /* Check for path debug env vars */
    if(pathdebug < 0) {
	const char* s = getenv("NCPATHDEBUG");
        pathdebug = (s == NULL ? 0 : 1);
    }

    pathlen = strlen(path);

    /* 1. look for MSYS path /D/... */
    if(pathlen >= 2
	&& (path[0] == '/' || path[0] == '\\')
	&& strchr(windrive,path[1]) != NULL
	&& (path[2] == '/' || path[2] == '\\' || path[2] == '\0')) {
	/* Assume this is a mingw path */
	outpath = (char*)malloc(pathlen+3); /* conservative */
	if(outpath == NULL) goto done;
	q = outpath;
	*q++ = path[1];
	*q++ = ':';
	strncpy(q,&path[2],pathlen);
	if(strlen(outpath) == 2)
	    strcat(outpath,"/");
	goto slashtrans;
    }

    /* 2. Look for leading /cygdrive/D where D is a single-char drive letter */
    if(pathlen >= (cdlen+1)
	&& memcmp(path,"/cygdrive/",cdlen)==0
	&& strchr(windrive,path[cdlen]) != NULL
	&& (path[cdlen+1] == '/'
	    || path[cdlen+1] == '\\'
	    || path[cdlen+1] == '\0')) {
	/* Assume this is a cygwin path */
	outpath = (char*)malloc(pathlen+1); /* conservative */
	if(outpath == NULL) goto done;
	outpath[0] = path[cdlen]; /* drive letter */
	outpath[1] = ':';
	strcpy(&outpath[2],&path[cdlen+1]);
	if(strlen(outpath) == 2)
	    strcat(outpath,"/");
	goto slashtrans;
    }

    /* 3. Look for leading D: where D is a single-char drive letter */
    /* This could be cygwin or Windows or mingw */
    if(pathlen >= 2
	&& strchr(windrive,path[0]) != NULL
	&& path[1] == ':'
	&& (path[2] == '\0' || path[2] == '/'  || path[2] == '\\')) {
	outpath = strdup(path);
	goto slashtrans;
    }

    /* 4. Look for relative path */
    if(pathlen > 1 && path[0] == '.') {
	outpath = makeabsolute(path);
	goto slashtrans;
    }

    /* Other: just pass thru */
    outpath = strdup(path);
    goto done;

slashtrans:
      /* In order to help debugging, and if not using MSC_VER or MINGW or CYGWIN,
	 convert back slashes to forward, else convert forward to back
      */
    if(!forwardslash) {
        p = outpath;
        /* In all #1 or #2 cases, translate '/' -> '\\' */
        for(;*p;p++) {
	    if(*p == '/') {*p = '\\';}
	}
    }
#ifdef PATHFORMAT
    if(forwardslash) {
	p = outpath;
        /* Convert '\' back to '/' */
        for(;*p;p++) {
            if(*p == '\\') {*p = '/';}
	}
    }
#endif /*PATHFORMAT*/

done:
    if(pathdebug) {
        fprintf(stderr,"XXXX: inpath=|%s| outpath=|%s|\n",
            path?path:"NULL",outpath?outpath:"NULL");
        fflush(stderr);
    }
    return outpath;
}

/* Make path suitable for inclusion in url */
EXTERNL
char* /* caller frees */
NCurlpath(const char* path)
{
    char* upath = NCpathcvt(path);
    char* p = upath;
    for(;*p;p++) {
	if(*p == '\\') {*p = '/';}
    }
    return upath;    
}

static char*
makeabsolute(const char* relpath)
{
    char* path = NULL;
#ifdef _WIN32
    path = _fullpath(NULL,relpath,8192);
#else
    path = realpath(relpath, NULL);
#endif
    if(path == NULL)
	path = strdup(relpath);
    return path;    
}

/* Fix up a path in case extra escapes were added by shell */
EXTERNL
char*
NCdeescape(const char* name)
{
    char* ename = NULL;
    const char* p;
    char* q;

    if(name == NULL) return NULL;
    ename = strdup(name);
    if(ename == NULL) return NULL;
    for(p=name,q=ename;*p;) {
	switch (*p) {
	case '\0': break;
	case '\\':
	    if(p[1] == '#') {
	        p++;
		break;
	    }
	    /* fall thru */
        default: *q++ = *p++; break;
	}
    }
    *q++ = '\0';
    return ename;
}


int
NChasdriveletter(const char* path)
{
    /* Check for windows drive letter */
    if(path == NULL || strlen(path) < 2)
        return 0;
    if(strchr(windrive,path[0]) != NULL && path[1] == ':')
        return 1; /* windows path with drive letter */
    return 0;
}

#ifdef WINPATH

/*
Provide wrappers for open and fopen.
*/

EXTERNL
FILE*
NCfopen(const char* path, const char* flags)
{
    FILE* f = NULL;
    char* cvtname = NCpathcvt(path);
    if(cvtname == NULL) return NULL;
    f = fopen(cvtname,flags);
    free(cvtname);    
    return f;
}

EXTERNL
int
NCopen3(const char* path, int flags, int mode)
{
    int fd = -1;
    char* cvtname = NCpathcvt(path);
    if(cvtname == NULL) return -1;
    fd = open(cvtname,flags,mode);
    free(cvtname);    
    return fd;
}

EXTERNL
int
NCopen2(const char *path, int flags)
{
    return NCopen3(path,flags,0);
}

#ifdef HAVE_DIRENT_H
EXTERNL
DIR*
NCopendir(const char* path)
{
    DIR* ent = NULL;
    char* cvtname = NCpathcvt(path);
    if(cvtname == NULL) return -1;
    ent = opendir(cvtname);
    free(cvtname);    
    return ent;
}

EXTERNL
int
NCclosedir(DIR* ent)
{
    int stat = 0;
    char* cvtname = NCpathcvt(path);
    if(cvtname == NULL) return -1;
    stat = closedir(cvtname);
    free(cvtname);    
    return stat;
}
#endif

/*
Provide wrappers for other file system functions
*/

/* Return access applied to path+mode */
EXTERNL
int
NCaccess(const char* path, int mode)
{
    int status = 0;
    char* cvtname = NCpathcvt(path);
    if(cvtname == NULL) return -1;
#ifdef _WIN32
    status = _access(cvtname,mode);
#else
    status = access(cvtname,mode);
#endif
    free(cvtname);    
    return status;
}

EXTERNL
int
NCremove(const char* path)
{
    int status = 0;
    char* cvtname = NCpathcvt(path);
    if(cvtname == NULL) return ENOENT;
    status = remove(cvtname);
    free(cvtname);    
    return status;
}

EXTERNL
int
NCmkdir(const char* path, int mode)
{
    int status = 0;
    char* cvtname = NCpathcvt(path);
    if(cvtname == NULL) return -1;
    status = mkdir(cvtname,mode);
    free(cvtname);    
    return status;
}

EXTERNL
char*
NCcwd(char* cwdbuf, size_t len)
{
    int ret = 0;
    char* cvtname = NULL;

    errno = 0;
    if(cwdbuf == NULL || len == 0) {errno = ENAMETOOLONG; goto done;}
    if(getcwd(cwdbuf,len) == NULL) {goto done;}
    cvtname = NCpathcvt(cwdbuf);
    if(cvtname == NULL || strlen(cvtname)+1 > len)
	{errno = ENAMETOOLONG; goto done;}
    cwdbuf[0] = '\0';
    strlcat(cwdbuf,cvtname,len);
done:
    nullfree(cvtname);
    if(errno) return NULL;
    return cwdbuf;
}
#endif /*WINPATH*/
