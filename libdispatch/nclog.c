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
#include <stdarg.h>
#include <string.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "nclog.h"
#include "ncglobal.h"

#define PREFIXLEN 8
#define MAXTAGS 256
#define NCTAGDFALT "Log";

static char* nctagsetdfalt[] = {"Warning","Error","Note","Debug"};

static const char* nctagname(int tag);

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
    nc_global->logstate->tagdfalt = NCTAGDFALT;
    nc_global->logstate->tagset = nctagsetdfalt;
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
    NCLOCK();    
    was = nc_global->logstate->logging;
    nc_global->logstate->logging = tf;
    NCUNLOCK();
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
    NCLOCK();
    if(!nc_global->logstate) ncloginit();
    NCUNLOCK();
    nclogclose();
    if(file == NULL || strlen(file) == 0) {
	/* use stderr*/
        NCLOCK();
	nc_global->logstate->logstream = stderr;
	nc_global->logstate->logfile = NULL;
	nc_global->logstate->systemfile = 1;
	NCUNLOCK();
    } else if(strcmp(file,"stdout") == 0) {
	/* use stdout*/
	NCLOCK();
	nc_global->logstate->logstream = stdout;
	nc_global->logstate->logfile = NULL;
	nc_global->logstate->systemfile = 1;
	NCUNLOCK();
    } else if(strcmp(file,"stderr") == 0) {
	/* use stderr*/
	NCLOCK();
	nc_global->logstate->logstream = stderr;
	nc_global->logstate->logfile = NULL;
	nc_global->logstate->systemfile = 1;
	NCUNLOCK();
    } else {
	int fd;
	NCLOCK();
	nc_global->logstate->logfile = strdup(file);
	nc_global->logstate->logstream = NULL;
	NCUNLOCK();
	/* We need to deal with this file carefully
	   to avoid unauthorized access*/
	fd = open(file,O_WRONLY|O_APPEND|O_CREAT,0600);
	if(fd >= 0) {
	    FILE* stream = fdopen(fd,"a");
	    NCLOCK();
	    nc_global->logstate->logstream = stream;
	    NCUNLOCK();
	} else {
	    NCLOCK();
	    free(nc_global->logstate->logfile);
	    nc_global->logstate->logfile = NULL;
	    nc_global->logstate->logstream = NULL;
	    NCUNLOCK();
	    ncsetlogging(0);
	    return 0;
	}
	NCLOCK();
	nc_global->logstate->systemfile = 0;
	NCUNLOCK();
    }
    return 1;
}

void
nclogclose(void)
{
    NCLOCK();
    if(!nc_global->logstate) ncloginit();
    if(nc_global->logstate->logstream != NULL
       && !nc_global->logstate->systemfile) {
	fclose(nc_global->logstate->logstream);
    }
    if(nc_global->logstate->logfile != NULL) {
	free(nc_global->logstate->logfile);
    }
    nc_global->logstate->logstream = NULL;
    nc_global->logstate->logfile = NULL;
    nc_global->logstate->systemfile = 0;
    NCUNLOCK();
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
    const char* prefix;

    NCLOCK();
    
    if(!nc_global->logstate->logging
       || nc_global->logstate->logstream == NULL) goto done;

    prefix = nctagname(tag);
    fprintf(nc_global->logstate->logstream,"%s:",prefix);

    if(fmt != NULL) {
      va_start(args, fmt);
      vfprintf(nc_global->logstate->logstream, fmt, args);
      va_end( args );
    }
    fprintf(nc_global->logstate->logstream, "\n" );
    fflush(nc_global->logstate->logstream);
done:
    NCUNLOCK();
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
    NCLOCK();
    if(!nc_global->logstate->logging
       || nc_global->logstate->logstream == NULL) goto done;
    fwrite(text,1,count,nc_global->logstate->logstream);
    fflush(nc_global->logstate->logstream);
done:
    NCUNLOCK();
}

/* The tagset is null terminated */
void
nclogsettags(char** tagset, char* dfalt)
{
    NCLOCK();
    nc_global->logstate->tagdfalt = dfalt;
    if(tagset == NULL) {
	nc_global->logstate->tagsize = 0;
    } else {
        int i;
	/* Find end of the tagset */
	for(i=0;i<MAXTAGS;i++) {if(tagset[i]==NULL) break;}
	nc_global->logstate->tagsize = i;
    }
    nc_global->logstate->tagset = tagset;
    NCUNLOCK();
}

static const char*
nctagname(int tag)
{
    const char* result;
    NCLOCK();
    if(tag < 0 || tag >= nc_global->logstate->tagsize) {
	result = nc_global->logstate->tagdfalt;
    } else {
	result = nc_global->logstate->tagset[tag];
    }
    NCUNLOCK();
    return result;
}

/**@}*/
