/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test miscellaneous netcdf_aux functions.
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "netcdf.h"
#include "netcdf_aux.h"

#define NCCATCH
#include "nclog.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#define DEBUG

typedef enum CMD {cmd_none=0, cmd_prov=1} CMD;

struct Options {
    int debug;
    CMD cmd;
    int argc;
    char** argv;
} options;

#define CHECK(code) do {stat = check(code,__func__,__LINE__); if(stat) {goto done;}} while(0)

static int
check(int code, const char* fcn, int line)
{
    if(code == NC_NOERR) return code;
    fprintf(stderr,"***fail: (%d) %s @ %s:%d\n",code,nc_strerror(code),fcn,line);
#ifdef debug
    abort();
#endif
    exit(1);
}

static void
testprov(void)
{
    int stat = NC_NOERR;
    int i;
    char** list = NULL;
    assert(options.argc > 0);
    for(i=0;i<options.argc;i++) {
        char** p;
	CHECK(ncaux_parse_provenance(options.argv[i],&list));
	/* Print and reclaim */
	printf("%s -> ",options.argv[i]);
	for(p=list;*p;p+=2) {
	    printf(" (/%s/,/%s/)",p[0],p[1]);
	    free(p[0]);
	    if(p[1]) free(p[1]);
	}
	printf("\n");
	free(list); list = NULL;
    }
done:
    return;
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;
    /* Init options */
    memset((void*)&options,0,sizeof(options));

    while ((c = getopt(argc, argv, "dP")) != EOF) {
        switch(c) {
        case 'd':
            options.debug = 1;
            break;
	case 'P':
	    options.cmd = cmd_prov;
	    break;
        case '?':
           fprintf(stderr,"unknown option\n");
           stat = NC_EINVAL;
           goto done;
        }
    }

    /* Setup args */
    argc -= optind;
    argv += optind;
    options.argc = argc;
    options.argv = argv;
    switch (options.cmd) {
    case cmd_prov: testprov(); break;
    default: fprintf(stderr,"Unknown cmd\n"); abort(); break;
    }
done:
    return (stat?1:0);
}
