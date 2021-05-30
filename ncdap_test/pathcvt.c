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

pathcvt [-u|-w|-m|-c] PATH

Options

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
    int debug;
} cvtoptions;

static void
usage(const char* msg)
{
    if(msg != NULL) fprintf(stderr,"%s\n",msg);
    fprintf(stderr,"pathcvt [-u|-w|-m|-c] PATH\n");
    if(msg == NULL) exit(0); else exit(1);
}

int
main(int argc, char** argv)
{
    int c;
    char* cvtpath = NULL;
    char* inpath;

    memset((void*)&cvtoptions,0,sizeof(cvtoptions));

    while ((c = getopt(argc, argv, "cD:hmuw")) != EOF) {
	switch(c) {
	case 'c': cvtoptions.target = NCPD_CYGWIN; break;
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
        cvtpath = NCpathcvt_test(inpath,cvtoptions.target,'c');
    printf("%s",cvtpath);
    if(cvtpath) free(cvtpath);
    return 0;
}
