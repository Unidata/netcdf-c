/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the NCpathcvt
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "ncpathmgr.h"

#define DEBUG

#define NKINDS 4
static const int kinds[NKINDS] = {NCPD_NIX,NCPD_MSYS,NCPD_CYGWIN,NCPD_WIN};

typedef struct Test {
    char* test;
    char* expected[NKINDS];
} Test;

/* Path conversion tests */
static Test PATHTESTS[] = {
{"/xxx/a/b",{"/xxx/a/b", "/c/xxx/a/b", "/cygdrive/c/xxx/a/b", "c:\\xxx\\a\\b"}},
{"d:/x/y",{ "/d/x/y", "/d/x/y",  "/cygdrive/d/x/y",  "d:\\x\\y"}},
{"d:\\x\\y",{ "/d/x/y", "/d/x/y",  "/cygdrive/d/x/y",  "d:\\x\\y"}},
{"/cygdrive/d/x/y",{ "/d/x/y", "/d/x/y", "/cygdrive/d/x/y",  "d:\\x\\y"}},
{"/d/x/y",{ "/d/x/y", "/d/x/y",  "/cygdrive/d/x/y",  "d:\\x\\y"}},
{"/cygdrive/d",{ "/d", "/d",  "/cygdrive/d",  "d:"}},
{"/d", {"/d", "/d",  "/cygdrive/d",  "d:"}},
{"/cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn",{
    "/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn",
    "/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn",
    "/cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn",
    "d:\\git\\netcdf-c\\dap4_test\\test_anon_dim.2.syn"}},
/* Test relative path */
{"x/y",{ "x/y", "x/y", "x/y",  "x\\y"}},
{"x\\y",{ "x/y", "x/y", "x/y",  "x\\y"}},
#ifndef _WIN32
/* Test utf8 path */
{"/海/海",{ "/海/海", "/c/海/海", "/cygdrive/c/海/海",  "c:\\海\\海"}},
/* Test network path */
{"//git/netcdf-c/dap4_test",{
    "/@/git/netcdf-c/dap4_test",
    "/@/git/netcdf-c/dap4_test",
    "/cygdrive/@/git/netcdf-c/dap4_test",
    "\\\\git\\netcdf-c\\dap4_test"}},
#endif
{NULL, {NULL, NULL, NULL, NULL}}
};

/*Forward */
static const char* kind2string(int kind);

int
main(int argc, char** argv)
{
    Test* test;
    int failcount = 0;
    char* cvt = NULL;
    char* unescaped = NULL;
    int k;
    int drive = 'c';

    nc_initialize();

    /* Test localkind X path-kind */
    for(test=PATHTESTS;test->test;test++) {
        /* Iterate over the test paths */
        for(k=0;k<NKINDS;k++) {
	    int kind = kinds[k];
	    /* Compare output for the localkind */
            if(test->expected[k] == NULL) {
#ifdef DEBUG
	        fprintf(stderr,"TEST local=%s: %s ignored\n",kind2string(kind),test->test);
#endif
	        continue;
	    }
	    /* ensure that NC_shellUnescape does not affect result */
	    unescaped = NC_shellUnescape(test->test);	
   	    cvt = NCpathcvt_test(unescaped,kind,drive);
#ifdef DEBUG
	    fprintf(stderr,"TEST local=%s: input: |%s| expected=|%s| actual=|%s|: ",
			kind2string(kind),test->test,test->expected[k],cvt);
#endif
	    fflush(stderr); fflush(stdout);
	    if(cvt == NULL) {
#ifdef DEBUG
		fprintf(stderr," ILLEGAL");
#endif
		failcount++;
	    } else if(strcmp(cvt,test->expected[k]) != 0) {
#ifdef DEBUG
		fprintf(stderr," FAIL");
#endif
	        failcount++;
	    } else {
#ifdef DEBUG
		fprintf(stderr," PASS");
#endif
	    }
#ifdef DEBUG
	    fprintf(stderr,"\n");
#endif	    
	    nullfree(unescaped); unescaped = NULL;
	    nullfree( cvt); cvt = NULL;
	}
    }
    nullfree(cvt); nullfree(unescaped);
    fprintf(stderr,"%s test_pathcvt\n",failcount > 0 ? "***FAIL":"***PASS");
    nc_finalize();
    return (failcount > 0 ? 1 : 0);
}

static const char*
kind2string(int kind)
{
    switch(kind) {
    case NCPD_NIX:
	return "Linux";
    case NCPD_MSYS:
	return "MSYS";
    case NCPD_CYGWIN:
	return "Cygwin";
    case NCPD_WIN:
	return "Windows";
    default: break;
    }
    return "unknown";
}
