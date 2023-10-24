/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the netcdf-4 data building process.
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif
#endif

#undef DEBUG

static struct Options {
    char* file;
    char* substratename;
    int debug;
    int checksum;
    int logging;
} options = {NULL,NULL,0,0,0};

static char testname[4096];

/* Forward */
static int test(const char*);

static void
usage(void)
{
    fprintf(stderr,"usage: %s.c [-d] [-v] [-c] [-s <substratename>] <file>\n",testname);
    exit(1);
}

static void
breakpoint(int code, int line)
{
    if(code != NC_NOERR) {
	fprintf(stderr,"***Fail: line: %d err: (%d) %s\n",line,code,nc_strerror(code));
	fflush(stderr);
    }
}
#define CHECK(err) if((ret = (err))) {breakpoint(err,__LINE__); goto done;} else {}

static void
parse(int argc, char** argv)
{
    int c = 0;

    while ((c = getopt(argc, argv, "cds:v")) != EOF) {
	switch(c) {
	case 'c': 
	    options.checksum = 1;	    
	    break;
	case 'd': 
	    options.debug = 1;	    
	    break;
	case 's': 
	    options.substratename = strdup(optarg);
	    break;
	case 'v': 
	    usage();
	    goto done;
	case '?':
	case ':':
	   fprintf(stderr,"unknown option: %d\n",c);
	   usage();
	   goto done;
	}
    }

    /* get url argument */
    argc -= optind;
    argv += optind;

    if (argc > 1) {
	fprintf(stderr, "only one input url argument permitted\n");
	usage();
    }
    if (argc == 0) {
	fprintf(stderr, "no input url specified\n");
	usage();
    }

    options.file = strdup(argv[0]);

    /* defaults */
#ifdef DEBUG
    options.logging = 1;
#endif

done:
    return;
}

static void
gettestname(const char* argv0)
{
    const char* prefix;
    char* suffix;

    if((prefix = strrchr(argv0,'/'))==NULL)
        prefix = strrchr(argv0,'\\');
    if(prefix) prefix++; else prefix = argv0;
    testname[0] = '\0';
    strlcat(testname,prefix,sizeof(testname));    
    suffix = strrchr(testname,'.');
    if(suffix != NULL) *suffix = '\0';
}

int
main(int argc, char** argv)
{
    int ret = NC_NOERR;
    char url[4096];

    gettestname(argv[0]);
    parse(argc,argv);

    /* build the url */
    snprintf(url,sizeof(url),"file://%s?%s#dap4&debug=copy%s%s%s",
	options.file,
	(options.checksum?"dap4.checksum=true":"dap4.checksum=false"),
	(options.substratename?"&substratename=" : ""),
	(options.substratename?options.substratename: ""),
	(options.logging?"&log": ""));

#ifdef DEBUG
    fprintf(stderr,"%s: url=%s\n",testname,url);
#endif
  
    ret = test(url);

#ifdef DEBUG
    fprintf(stderr,"code=%d %s\n",ret,nc_strerror(ret));
#endif
    return (ret ? 1 : 0);
}

static int
test(const char* url)
{
    int ret = NC_NOERR;
    int ncid;

    /* Use the open/close mechanism */
    CHECK(nc_open(url,NC_NETCDF4,&ncid));
    CHECK(nc_close(ncid));

done:
    return ret;
}
