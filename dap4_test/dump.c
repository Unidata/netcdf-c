/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen3/main.c,v 1.20 2010/03/31 18:18:40 dmh Exp $
 *********************************************************************/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef _MSC_VER
#include "XGetopt.h"
#define snprintf _snprintf
#endif
    
#include "netcdf.h"
#include "ncbytes.h"
#include "ncpathmgr.h"

extern void NCD4_dumpbytes(size_t size, const void* data0, int swap);
extern void NCD4_tagdump(size_t size, const void* data0, int swap, const char* tag);
extern int NC_readfile(const char* filename, NCbytes* content);

static char* progname = NULL;

static void
usage(void)
{
    fprintf(stderr,"Usage: %s [ -O ][ -f infile | file ]",progname);
    exit(1);
}

int
main(int argc, char *argv[])
{
    int c;
    char* fname = NULL;
    NCbytes* contents = ncbytesnew();
    char* tag = NULL;
    size_t offset = 0;
    size_t len = 0;
    char* data = NULL;
    int swap = 0;

    progname = strdup(argv[0]);

    while ((c = getopt(argc, argv, "SO:f:t:")) != EOF) {
      switch(c) {
	case 'f':
	    fname = strdup(optarg);
	    break;
	case 't':
	    tag = strdup(optarg);
	    break;
	case 'S':
	    swap = 1;
	    break;
	case 'O':
	    offset = atol(optarg);
	    break;
	case '?':
	  usage();
      }
    }
    argc -= optind;
    argv += optind;

    if(fname == NULL) {
        if (argc > 1) {
	    fprintf(stderr,"%s: only one input file argument permitted",progname);
	    return(1);
        }
	fname = strdup(argv[0]);
    }

    if(tag == NULL) tag = strdup(progname);    
    if(NC_readfile(fname,contents)) usage();
    len = ncbyteslength(contents) - offset;
    data = (char*)ncbytescontents(contents);
    data += offset;
    NCD4_tagdump(len,data,swap,tag);

    return 0;
}


