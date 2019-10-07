/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the ncuri parsing
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "ncuri.h"

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
/* Test embedded user+pwd */
{"http://tiggeUser:tigge@localhost:8081/thredds/dodsC/restrict/testData.nc",
 "http://tiggeUser:tigge@localhost:8081/thredds/dodsC/restrict/testData.nc"},
/* Misc. */
{"http://localhost","http://localhost/"},
{"http:///x","http:///x"},
{"file:///home/osboxes/git/dap4/dap4_test/daptestfiles/test_anon_dim.2.syn#dap4&debug=copy&substratename=./results/test_anon_dim.2.syn.nc","file:///home/osboxes/git/dap4/dap4_test/daptestfiles/test_anon_dim.2.syn#dap4&debug=copy&substratename=./results/test_anon_dim.2.syn.nc"},
{"file://x/y","file://x/y"},
{NULL,NULL}
};

/* Tests that should fail */
static char* XTESTS[] = {
"[dap4http://localhost:8081/x",
NULL
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
	if(ret != NC_NOERR) {
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
	NCURI* uri = NULL;
	if(!ncuriparse(*xtest,&uri)) {
	    fprintf(stderr,"XTEST succeeded: %s\n",*xtest);
	    failcount++;
	}
    }

    fprintf(stderr,"%s test_ncuri\n",failcount > 0 ? "***FAIL":"***PASS");
    return (failcount > 0 ? 1 : 0);
}
