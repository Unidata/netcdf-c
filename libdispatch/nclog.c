/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>

#include "nclog.h"

#define PREFIXLEN 8

#define MAXTAGS 256

#define NCTAGDFALT "Log";

static int ncinit = 0;
static int nclogging = 0;
static char* nclogfile = NULL;
static FILE* nclogstream = NULL;

static int nctagsize = 0;
static char** nctagset = NULL;
static char* nctagdfalt = NULL;
static char* nctagsetdfalt[] = {"Warning","Error","Note","Debug"};

static char* nctagname(int tag);

void
ncloginit(void)
{
    ncinit = 1;
    ncsetlogging(0);
    nclogfile = NULL;
    nclogstream = NULL;
    /* Use environment variables to preset nclogging state*/
    /* I hope this is portable*/
    if(getenv(ENVFLAG) != NULL) {
	const char* file = getenv(ENVFLAG);
	ncsetlogging(1);
	nclogopen(file);
    }
    nctagdfalt = NCTAGDFALT;
    nctagset = nctagsetdfalt;
}

void
ncsetlogging(int tf)
{
    if(!ncinit) ncloginit();
    nclogging = tf;
}

void
nclogopen(const char* file)
{
    if(!ncinit) ncloginit();
    if(nclogfile != NULL) {
	fclose(nclogstream);
	free(nclogfile);
	nclogfile = NULL;
    }
    if(file == NULL || strlen(file) == 0) {
	/* use stderr*/
	nclogstream = stderr;
	nclogfile = NULL;
    } else if(strcmp(file,"stdout") == 0) {
	/* use stdout*/
	nclogstream = stdout;
	nclogfile = NULL;
    } else if(strcmp(file,"stderr") == 0) {
	/* use stderr*/
	nclogstream = stderr;
	nclogfile = NULL;
    } else {
	int fd;
	nclogfile = strdup(file);
	nclogstream = NULL;
	/* We need to deal with this file carefully
	   to avoid unauthorized access*/
	fd = open(nclogfile,O_WRONLY|O_APPEND|O_CREAT,0600);
	if(fd >= 0) {
	    nclogstream = fdopen(fd,"a");
	} else {
	    free(nclogfile);
	    nclogfile = NULL;
	    ncsetlogging(0);
	}
    }
}

void
nclogclose(void)
{
    if(nclogfile != NULL && nclogstream != NULL) {
	fclose(nclogstream);
	nclogstream = NULL;
	if(nclogfile != NULL) free(nclogfile);
	nclogfile = NULL;
    }
}

void
nclog(int tag, const char* fmt, ...)
{
    va_list args;
    char* prefix;
    if(!nclogging || nclogstream == NULL) return;

    prefix = nctagname(tag);
    fprintf(nclogstream,"%s:",prefix);

    if(fmt != NULL) {
      va_start(args, fmt);
      vfprintf(nclogstream, fmt, args);
      va_end( args );
    }
    fprintf(nclogstream, "\n" );
    fflush(nclogstream);
}

void
nclogtext(int tag, const char* text, size_t count)
{
    char line[1024];
    size_t delta = 0;
    const char* eol = text;
    size_t i,pos;

    if(!nclogging || nclogstream == NULL) return;

#ifdef IGNORE
    for(pos=0,i=0;i<count;i++) {
	if(text[i] == '\n') {	
	    fprintf(nclogstream,"\t");
	    fwrite(text+pos,1,(i-pos)+1,nclogstream);
	    fprintf(nclogstream,"\n");
	    pos = i+1;
	}
    }
#else
    fwrite(text,1,count,nclogstream);
#endif
    fflush(nclogstream);
}

/* The tagset is null terminated */
void
nclogsettags(char** tagset, char* dfalt)
{
    nctagdfalt = dfalt;
    if(tagset == NULL) {
	nctagsize = 0;
    } else {
        int i;
	char** p = tagset;
	while(*p && i < MAXTAGS) {i++;}
	nctagsize = i;
    }
    nctagset = tagset;
}

static char*
nctagname(int tag)
{
    if(tag < 0 || tag >= nctagsize) {
	return nctagdfalt;
    } else {
	return nctagset[tag];
    }
}
