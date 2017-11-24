/*
 * Copyright 1996, University Corporation for Atmospheric Research
 * See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef _MSC_VER
#include <io.h>
#endif

#include "ncexternl.h"
#include "ncwinpath.h"

/* Debugging aid */
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
4. If #1, #2, or #3 is encounterd, then forward slashes
   will be converted to backslashes.
5. All other cases are passed thru unchanged
*/

/* Define legal windows drive letters */
static char* windrive = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define CDPATH "/cygdrive/"

static const size_t cdlen = 10; /*strlen(CDPATH)*/

static int pathdebug = -1;

EXTERNL
char* /* caller frees */
NCpathcvt(const char* path)
{
    char* outpath = NULL; 
    size_t pathlen;
    char* p;
    size_t outlen = 0; /*|outpath|*/

    if(path == NULL) goto done; /* defensive driving */

    /* Check for path debug env vars */
    if(pathdebug < 0) {
	const char* s = getenv("NCPATHDEBUG");
        pathdebug = (s == NULL ? 0 : 1);
    }

    pathlen = strlen(path);
    if(pathlen == 0) goto done;

    /* Allocate outpath of maximal size needed */
    outlen = pathlen+3+1; /* conservative */
    outlen++; /*strlcat*/
    outpath = (char*)malloc(outlen+1);
    if(outpath == NULL) goto done;
    outpath[0] = '\0'; /* so we can use strlcat */

    /* 1. look for MSYS path /D[/|0]... => only can occur if |path| >= 2;
	  Also assumes that path has trailing nul not counted in pathlen.
     */
    if(pathlen >= 2
	&& (path[0] == '/' || path[0] == '\\')
	&& strchr(windrive,path[1]) != NULL
	&& (path[2] == '/' || path[2] == '\\' || path[2] == '\0')) {
	/* Assume this is a mingw path */
	outpath[0] = path[1];
	outpath[1] = ':';
	outpath[2] = '\0';
	if(path[2] != '\0') {
	    strlcat(outpath,&path[2],outlen);
	} else
	    strlcat(outpath,"/",outlen);
	goto slashtrans;
    }

    /* 2. Look for leading /cygdrive/D where D is a single-char drive letter */
    if(pathlen >= (cdlen+1)) {
	int drive = path[cdlen];
	int next = path[cdlen+1];
	if(memcmp(path,CDPATH,cdlen)==0
	   && strchr(windrive,drive) != NULL
	   && (next == '/' || next == '\\' || next == '\0')) {
	    /* Assume this is a cygwin path */
	    outpath[0] = drive;
	    outpath[1] = ':';
	    outpath[2] = '\0';
	    if(next != '\0')
		strlcat(outpath,&path[cdlen+1],outlen);
	    else
	        strlcat(outpath,"/",outlen);
	    goto slashtrans;
	}
    } /* else fall thru */

    /* 3. Look for leading D: where D is a single-char drive letter
          => standard windows path
     */
    if(pathlen >= 2
	&& strchr(windrive,path[0]) != NULL
	&& path[1] == ':'
	&& (path[2] == '\0' || path[2] == '/' || path[2] == '\\')) {
	strlcat(outpath,path,outlen);
	goto slashtrans;
    }

    /* 4. Other: just pass thru */
    strlcat(outpath,path,outlen);
    goto done;

slashtrans:
      /* In order to help debugging, and if not using MSC_VER or MINGW,
	 convert back slashes to forward, else convert forward to back
      */
    p = outpath;
    /* In all #1 or #2 cases, translate '/' -> '\\' */
    for(;*p;p++) {
	if(*p == '/') {*p = '\\';}
    }
#ifdef PATHFORMAT
#ifndef _MSC_VER
	p = outpath;
        /* Convert '\' back to '/' */
        for(;*p;p++) {
            if(*p == '\\') {*p = '/';}
	}
    }
#endif /*!_MSC_VER*/
#endif /*PATHFORMAT*/

done:
    if(pathdebug) {
        fprintf(stderr,"XXXX: inpath=|%s| outpath=|%s|\n",
            path?path:"NULL",outpath?outpath:"NULL");
        fflush(stderr);
    }
    return outpath;
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

#endif /*WINPATH*/
