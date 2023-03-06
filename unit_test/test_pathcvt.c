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

#undef DEBUG

#define NKINDS 4
static const int kinds[NKINDS] = {NCPD_NIX,NCPD_MSYS,NCPD_CYGWIN,NCPD_WIN};

typedef struct Test {
    char* test;
    char* expected[NKINDS];
} Test;

/* Path conversion tests */
static Test PATHTESTS[] = {
{"/xxx/a/b",{
	"/xxx/a/b",		/*NCPD_LINUX*/
	"c:\\xxx\\a\\b",	/*NCPD_MSYS*/
	"/cygdrive/c/xxx/a/b",	/*NCPD_CYGWIN*/
	"c:\\xxx\\a\\b"		/*NCPD_WIN*/
	}},
{"d:/x/y",{
	 "/d/x/y",		/*NCPD_LINUX*/
	"d:\\x\\y",		/*NCPD_MSYS*/
	 "/cygdrive/d/x/y",	/*NCPD_CYGWIN*/
	 "d:\\x\\y"		/*NCPD_WIN*/
	}},
{"d:\\x\\y",{
	 "/d/x/y",		/*NCPD_LINUX*/
	"d:\\x\\y",		/*NCPD_MSYS*/
	 "/cygdrive/d/x/y",	/*NCPD_CYGWIN*/
	 "d:\\x\\y"		/*NCPD_WIN*/
	}},
{"/cygdrive/d/x/y",{
	 "/cygdrive/d/x/y",	/*NCPD_LINUX*/
	"d:\\x\\y",		/*NCPD_MSYS*/
	"/cygdrive/d/x/y",	/*NCPD_CYGWIN*/
	 "d:\\x\\y"		/*NCPD_WIN*/
	}},
{"/d/x/y",{
	 "/d/x/y",		/*NCPD_LINUX*/
	"c:\\d\\x\\y",		/*NCPD_MSYS*/
	 "/cygdrive/c/d/x/y",	/*NCPD_CYGWIN*/
	 "c:\\d\\x\\y"		/*NCPD_WIN*/
	}},
{"/cygdrive/d",{
	 "/cygdrive/d",		/*NCPD_LINUX*/
	"d:",			/*NCPD_MSYS*/
	 "/cygdrive/d",		/*NCPD_CYGWIN*/
	 "d:"			/*NCPD_WIN*/
	}},
{"/d", {
	"/d",			/*NCPD_LINUX*/
	"c:\\d",		/*NCPD_MSYS*/
	 "/cygdrive/c/d",	/*NCPD_CYGWIN*/
	 "c:\\d"		/*NCPD_WIN*/
	}},
{"/cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn",{
    "/cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn",	/*NCPD_LINUX*/
    "d:\\git\\netcdf-c\\dap4_test\\test_anon_dim.2.syn",	/*NCPD_MSYS*/
    "/cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn",	/*NCPD_CYGWIN*/
    "d:\\git\\netcdf-c\\dap4_test\\test_anon_dim.2.syn"		/*NCPD_WIN*/
    }},
/* Test relative path */
{"x/y",{
	 "x/y",	/*NCPD_LINUX*/
	"x\\y",	/*NCPD_MSYS*/
	"x/y",	/*NCPD_CYGWIN*/
	 "x\\y"	/*NCPD_WIN*/
	}},
{"x\\y",{
	 "x/y",/*NCPD_LINUX*/
	"x\\y",	/*NCPD_MSYS*/
	"x/y",	/*NCPD_CYGWIN*/
	 "x\\y"	/*NCPD_WIN*/
	}},
#ifndef _WIN32X
/* Test utf8 path */
{"/海/海",{
	 "/海/海",		/*NCPD_LINUX*/
	"c:\\海\\海",		/*NCPD_MSYS*/
	"/cygdrive/c/海/海",	/*NCPD_CYGWIN*/
	 "c:\\海\\海"		/*NCPD_WIN*/
	}},
/* Test network path */
{"//git/netcdf-c/dap4_test",{
    NULL /*meaningless*/,		/*NCPD_LINUX*/
    "\\\\git\\netcdf-c\\dap4_test",	/*NCPD_MSYS*/
    NULL /*meaningless*/,		/*NCPD_CYGWIN*/
    "\\\\git\\netcdf-c\\dap4_test"	/*NCPD_WIN*/
    }},
#endif
{NULL, {NULL, NULL, NULL, NULL}}
};

char* macros[128];

/*Forward */
static char* expand(const char* s);
static void setmacros(void);
static void reclaimmacros(void);
#ifdef DEBUG
static const char* kind2string(int kind);
#endif

int
main(int argc, char** argv)
{
    Test* test;
    int failcount = 0;
    char* cvt = NULL;
    char* unescaped = NULL;
    char* expanded = NULL;
    int k;
    int drive = 'c';

    nc_initialize();

    setmacros();

    /* Test localkind X path-kind */
    for(test=PATHTESTS;test->test;test++) {
#ifdef DEBUG
	int inputkind = NCgetinputpathkind(test->test);
#endif
        /* Iterate over the test paths */
        for(k=0;k<NKINDS;k++) {
	    int kind = kinds[k];
	    /* Compare output for the localkind */
            if(test->expected[k] == NULL) {
#ifdef DEBUG
	        fprintf(stderr,"TEST input=%s target=%s: %s ignored\n",kind2string(inputkind),kind2string(kind),test->test);
#endif
	        continue;
	    }
	    /* ensure that NC_shellUnescape does not affect result */
	    unescaped = NC_shellUnescape(test->test);	
	    expanded = expand(test->expected[k]);
#ifdef DEBUG
	    fprintf(stderr,">>> unescaped=|%s| expanded=|%s|\n",unescaped,expanded);
#endif
#ifdef DEBUG
	    fprintf(stderr,"TEST input=%s target=%s: input: |%s|\n",
			kind2string(inputkind),kind2string(kind),test->test);
#endif
   	    cvt = NCpathcvt_test(unescaped,kind,drive);
#ifdef DEBUG
	    fprintf(stderr,"\texpected=|%s| actual=|%s|: ",expanded,cvt);
#endif
	    fflush(stderr); fflush(stdout);
	    if(cvt == NULL) {
#ifdef DEBUG
		fprintf(stderr," ILLEGAL");
#endif
		failcount++;
	    } else if(strcmp(cvt,expanded) != 0) {
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
	    nullfree(expanded); expanded = NULL;
	    nullfree(cvt); cvt = NULL;
	}
    }
    nullfree(cvt); nullfree(unescaped);
    fprintf(stderr,"%s test_pathcvt\n",failcount > 0 ? "***FAIL":"***PASS");

    reclaimmacros();

    nc_finalize();
    return (failcount > 0 ? 1 : 0);
}

#ifdef DEBUG
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
    case NCPD_REL:
	return "Relative";
    default: break;
    }
    return "Unknown";
}
#endif

static char*
expand(const char* s)
{
    const char *p;
    char expanded[8192];
    char q[2];

    q[1] = '\0';
    expanded[0] = '\0';
    for(p=s;*p;p++) {
	char c = *p;
	if(c == '%') {
	    p++;
	    c = *p;
	    if(macros[(int)c] != NULL)
	        strlcat(expanded,macros[(int)c],sizeof(expanded));
	} else {
	    q[0] = c;
	    strlcat(expanded,q,sizeof(expanded));
	}
    }
    return strdup(expanded);
}

static void
setmacros(void)
{
    int i;
    const char* m;
    for(i=0;i<128;i++) macros[i] = NULL;
    if((m=getenv("MSYS2_PREFIX"))) {
	macros['m'] = strdup(m);    
    }
}

static void
reclaimmacros(void)
{
    int i;
    for(i=0;i<128;i++) {
	if(macros[i]) free(macros[i]);
	macros[i] = NULL;
    }
}
