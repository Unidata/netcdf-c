/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/

#include "config.h"

#ifdef _MSC_VER
#include<io.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>

#include "nclog.h"
#include "ncglobal.h"

#define PREFIXLEN 8
#define MAXTAGS 256
#define NCTAGDFALT "Log";

static char* nctagsetdfalt[] = {"Warning","Error","Note","Debug"};

typedef struct NC_LOGSTATE {
    int nclogging;
    int ncsystemfile; /* 1 => we are logging to file we did not open */
    char* nclogfile;
    FILE* nclogstream;
    int nctagsize;
    char** nctagset;
    char* nctagdfalt;
} NC_LOGSTATE;

static char* nctagname(int tag);

/*!\defgroup NClog NClog Management
@{*/

/*!\internal
*/

void
ncloginit(void)
{
    const char* file;

    if(nc_global->logstate != NULL)
	return;
    nc_global->logstate = (NC_LOGSTATE*)calloc(1,sizeof(NC_LOGSTATE));
    ncsetlogging(0);
    /* Use environment variables to preset nclogging state*/
    /* I hope this is portable*/
    file = getenv(NCENVFLAG);
    if(file != NULL && strlen(file) > 0) {
        if(nclogopen(file)) {
	    ncsetlogging(1);
	}
    }
    nc_global->logstate->nctagdfalt = NCTAGDFALT;
    nc_global->logstate->nctagset = nctagsetdfalt;
}

/*!
Enable/Disable logging.

\param[in] tf If 1, then turn on logging, if 0, then turn off logging.

\return The previous value of the logging flag.
*/

int
ncsetlogging(int tf)
{
    int was;
    LOCK;    
    if(!nc_global->logstate) ncloginit();
    was = nc_global->logstate->nclogging;
    nc_global->logstate->nclogging = tf;
    UNLOCK;
    return was;
}

/*!
Specify a file into which to place logging output.

\param[in] file The name of the file into which to place logging output.
If the file has the value NULL, then send logging output to
stderr.

\return zero if the open failed, one otherwise.
*/

int
nclogopen(const char* file)
{
    LOCK;
    if(!nc_global->logstate) ncloginit();
    UNLOCK;
    nclogclose();
    if(file == NULL || strlen(file) == 0) {
	/* use stderr*/
        LOCK;
	nc_global->logstate->nclogstream = stderr;
	nc_global->logstate->nclogfile = NULL;
	nc_global->logstate->ncsystemfile = 1;
	UNLOCK;
    } else if(strcmp(file,"stdout") == 0) {
	/* use stdout*/
	LOCK;
	nc_global->logstate->nclogstream = stdout;
	nc_global->logstate->nclogfile = NULL;
	nc_global->logstate->ncsystemfile = 1;
	UNLOCK;
    } else if(strcmp(file,"stderr") == 0) {
	/* use stderr*/
	LOCK;
	nc_global->logstate->nclogstream = stderr;
	nc_global->logstate->nclogfile = NULL;
	nc_global->logstate->ncsystemfile = 1;
	UNLOCK;
    } else {
	int fd;
	LOCK;
	nc_global->logstate->nclogfile = strdup(file);
	nc_global->logstate->nclogstream = NULL;
	UNLOCK;
	/* We need to deal with this file carefully
	   to avoid unauthorized access*/
	fd = open(file,O_WRONLY|O_APPEND|O_CREAT,0600);
	if(fd >= 0) {
	    FILE* stream = fdopen(fd,"a");
	    LOCK;
	    nc_global->logstate->nclogstream = stream;
	    UNLOCK
	} else {
	    LOCK;
	    free(nc_global->logstate->nclogfile);
	    nc_global->logstate->nclogfile = NULL;
	    nc_global->logstate->nclogstream = NULL;
	    UNLOCK;
	    ncsetlogging(0);
	    return 0;
	}
	LOCK;
	nc_global->logstate->ncsystemfile = 0;
	UNLOCK;
    }
    return 1;
}

void
nclogclose(void)
{
    LOCK;
    if(!nc_global->logstate) ncloginit();
    if(nc_global->logstate->nclogstream != NULL
       && !nc_global->logstate->ncsystemfile) {
	fclose(nc_global->logstate->nclogstream);
    }
    if(nc_global->logstate->nclogfile != NULL) {
	free(nc_global->logstate->nclogfile);
    }
    nc_global->logstate->nclogstream = NULL;
    nc_global->logstate->nclogfile = NULL;
    nc_global->logstate->ncsystemfile = 0;
    UNLOCK;
}

/*!
Send logging messages. This uses a variable
number of arguments and operates like the stdio
printf function.

\param[in] tag Indicate the kind of this log message.
\param[in] format Format specification as with printf.
*/

void
nclog(int tag, const char* fmt, ...)
{
    va_list args;
    char* prefix;

    LOCK;
    if(!nc_global->logstate->nclogginginitialized) ncloginit();
    
    if(!nc_global->logstate->nclogging
       || nc_global->logstate->nclogstream == NULL) {UNLOCK; return;}

    prefix = nctagname(tag);
    fprintf(nclogstream,"%s:",prefix);

    if(fmt != NULL) {
      va_start(args, fmt);
      vfprintf(nclogstream, fmt, args);
      va_end( args );
    }
    fprintf(nclogstream, "\n" );
    fflush(nclogstream);
    UNLOCK;
}

void
nclogtext(int tag, const char* text)
{
    nclogtextn(tag,text,strlen(text));
}

/*!
Send arbitrarily long text as a logging message.
Each line will be sent using nclog with the specified tag.
\param[in] tag Indicate the kind of this log message.
\param[in] text Arbitrary text to send as a logging message.
*/

void
nclogtextn(int tag, const char* text, size_t count)
{
    LOCK;
    if(!nc_global->logstate->nclogging
       || nc_global->logstate->nclogstream == NULL) {UNLOCK; return;}
    fwrite(text,1,count,nclogstream);
    fflush(nclogstream);
    UNLOCK;
}

/* The tagset is null terminated */
void
nclogsettags(char** tagset, char* dfalt)
{
    LOCK;
    nc_global->logstate->nctagdfalt = dfalt;
    if(tagset == NULL) {
	nc_global->logstate->nctagsize = 0;
    } else {
        int i;
	/* Find end of the tagset */
	for(i=0;i<MAXTAGS;i++) {if(tagset[i]==NULL) break;}
	nc_global->logstate->nctagsize = i;
    }
    nc_global->logstate->nctagset = tagset;
    UNLOCK;
}

static char*
nctagname(int tag)
{
    if(tag < 0 || tag >= nc_global->logstate->nctagsize) {
	return nc_global->logstate->nctagdfalt;
    } else {
	return nc_global->logstate->nctagset[tag];
    }
}

/**@}*/
