/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the ncuri parsing
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ncuri.h"

#define WINDEBUG 1

typedef struct Test {
    char* url;
    char* expected;
} Test;

static Test TESTS[] = {
/* file: tests */
{"file:d:/x/y","file://d:/x/y"},
{"file://d:/x/y","file://d:/x/y"},
{"file:/x/y","file:///x/y"},
{"file:///x/y","file:///x/y"},
/* prefix param tests */
{"[dap4]http://localhost:8081/x","http://localhost:8081/x#dap4"},
{"[show=fetch]http://localhost:8081/x","http://localhost:8081/x#show=fetch"},
{"[dap4][show=fetch][log]http://localhost:8081/x","http://localhost:8081/x#dap4&show=fetch&log"},
/* suffix param tests */
{"http://localhost:8081/x#dap4","http://localhost:8081/x#dap4"},
{"http://localhost:8081/x#show=fetch","http://localhost:8081/x#show=fetch"},
{"http://localhost:8081/x#dap4&show=fetch&log","http://localhost:8081/x#dap4&show=fetch&log"},
/* prefix+suffix param tests */
{"[dap4]http://localhost:8081/x#show=fetch&log","http://localhost:8081/x#dap4&show=fetch&log"},
/* suffix param tests with constraint*/
{"http://localhost:8081/x?dap4.ce=x#dap4&show=fetch&log","http://localhost:8081/x?dap4.ce=x#dap4&show=fetch&log"},
/* Misc. */
{"http://localhost","http://localhost/"},
{"http:///x","http:///x"},
{"file:///home/osboxes/git/dap4/dap4_test/daptestfiles/test_anon_dim.2.syn#dap4&debug=copy&substratename=./results/test_anon_dim.2.syn.nc","file:///home/osboxes/git/dap4/dap4_test/daptestfiles/test_anon_dim.2.syn#dap4&debug=copy&substratename=./results/test_anon_dim.2.syn.nc"},

{NULL,NULL}
};

/* Tests that should fail */
static char* XTESTS[] = {
"file://x/y",
"[dap4http://localhost:8081/x",
NULL
};

/* Path conversion tests */
static Test PATHTESTS[] = {
/*
{"/xxx/a/b","\\xxx\\a\\b"},
{"d:/x/y","d:\\x\\y"},
{"/cygdrive/d/x/y","d:\\x\\y"},
{"/d/x/y","d:\\x\\y"},
*/
{"/cygdrive/d","d:\\"},
{"/d","d:\\"},
{NULL,NULL}
};

int
main(int argc, char** argv)
{
    Test* test;
    char** xtest;
    int failcount = 0;

    for(test=TESTS;test->url;test++) {
	int ret = 0;
	NCURI* uri = NULL;
	ret = ncuriparse(test->url,&uri);
	if(ret != NCU_OK) {
	    fprintf(stderr,"Parse fail: %s\n",test->url);
	    failcount++;
	} else {
	    char* built = ncuribuild(uri,NULL,NULL,NCURIALL);
	    if(built == NULL) {
	        fprintf(stderr,"Build fail: %s\n",test->url);
		failcount++;
	    } else {
		if(strcmp(test->expected,built) != 0) {
	            fprintf(stderr,"Mismatch: expected=|%s| actual=|%s|\n",test->expected,built);
		    failcount++;
		}
		free(built);
	    }
	    ncurifree(uri);
	}
    }

    for(xtest=XTESTS;*xtest;xtest++) {
	int ret = 0;
	NCURI* uri = NULL;
	ret = ncuriparse(*xtest,&uri);
	if(ret == NCU_OK) {
	    fprintf(stderr,"XTEST succeeded: %s\n",*xtest);
	    failcount++;
	}
    }

#if defined(_WIN32) || defined(WINDEBUG)
    for(test=PATHTESTS;test->url;test++) {
	int ret = 0;
	char* cvt = NCpathcvt(test->url);
	if(cvt == NULL) {
	    fprintf(stderr,"TEST returned NULL: %s\n",test->url);
	    exit(1);
	}
	if(strcmp(cvt,test->expected) != 0) {
	    fprintf(stderr,"NCpathcvt failed:: input: |%s| expected=|%s| actual=|%s|\n",test->url,test->expected,cvt);
	    failcount++;
	}
	free(cvt);
    }
#endif /* _WIN32 */

    fprintf(stderr,"%s test_ncuri\n",failcount > 0 ? "***FAIL":"***PASS");
    return (failcount > 0 ? 1 : 0);
}
