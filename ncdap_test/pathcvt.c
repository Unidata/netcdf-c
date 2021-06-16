/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#include "ncpathmgr.h"

/*
Synopsis

pathcvt [-u|-w|-m|-c] [-e] [-d <driveletter>] PATH

Options
  -e add backslash escapes to '\' and ' '
  -d <driveletter> use driveletter when needed; defaults to 'c'
Output type options:
  -u convert to Unix form of path
  -w convert to Windows form of path
  -m convert to MSYS form of path
  -c convert to Cygwin form of path
  
Default is to convert to the format used by the platform.

*/

#define DEBUG

struct Options {
    int target;
    int escapes;
    int drive;
    int debug;
} cvtoptions;

static char* escape(const char* path);
static void usage(const char* msg);

int
main(int argc, char** argv)
{
    int c;
    char* cvtpath = NULL;
    char* inpath;

    memset((void*)&cvtoptions,0,sizeof(cvtoptions));
    cvtoptions.drive = 'c';

    while ((c = getopt(argc, argv, "cD:d:ehmuw")) != EOF) {
	switch(c) {
	case 'c': cvtoptions.target = NCPD_CYGWIN; break;
	case 'd': cvtoptions.drive = optarg[0]; break;
	case 'e': cvtoptions.escapes = 1; break;
	case 'h': usage(NULL); break;
	case 'm': cvtoptions.target = NCPD_MSYS; break;
	case 'u': cvtoptions.target = NCPD_NIX; break;
	case 'w': cvtoptions.target = NCPD_WIN; break;
	case 'D':
	    sscanf(optarg,"%d",&cvtoptions.debug);
	    break;
	case '?':
	   usage("unknown option");
	   break;
	}
    }

    argc -= optind;
    argv += optind;

    /* If no file arguments left or more than one, print usage message. */
    if (argc == 0)
       usage("no path specified");
    if (argc > 1)
       usage("more than one path specified");
    inpath = argv[0];
    if(cvtoptions.target == NCPD_UNKNOWN)
        cvtpath = NCpathcvt(inpath);
    else
        cvtpath = NCpathcvt_test(inpath,cvtoptions.target,(char)cvtoptions.drive);
    if(cvtpath && cvtoptions.escapes) {
	char* path = cvtpath; cvtpath = NULL;
        cvtpath = escape(path);
	free(path);
    }
    printf("%s",cvtpath);
    if(cvtpath) free(cvtpath);
    return 0;
}

static void
usage(const char* msg)
{
    if(msg != NULL) fprintf(stderr,"%s\n",msg);
    fprintf(stderr,"pathcvt [-u|-w|-m|-c] PATH\n");
    if(msg == NULL) exit(0); else exit(1);
}

static char*
escape(const char* path)
{
    size_t slen = strlen(path);
    const char* p;
    char* q;
    char* epath = NULL;
    const char* escapes = " \\";

    epath = (char*)malloc((2*slen) + 1);
    if(epath == NULL) usage("out of memtory");
    p = path;
    q = epath;
    for(;*p;p++) {
	if(strchr(escapes,*p) != NULL)
	    *q++ = '\\';
        *q++ = *p;
    }
    *q = '\0';
    return epath;
}

