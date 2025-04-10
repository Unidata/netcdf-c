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
#ifdef NETCDF_ENABLE_S3
#include "ncpathmgr.h" // to initialize global state needed by NC_s3urlrebuild
#endif

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

#if defined(NETCDF_ENABLE_S3)
static Test S3TESTS[] = {
//Virtual
	{"https://<bucket-name>.s3.<region>.amazonaws.com/<path>","https://s3.<region>.amazonaws.com/<bucket-name>/<path>#mode=s3"},
	{"https://<bucket-name>.s3.amazonaws.com/<path>","https://s3.us-east-1.amazonaws.com/<bucket-name>/<path>#mode=s3"},
//Path
	{"https://s3.<region>.amazonaws.com/<bucket-name>/<path>","https://s3.<region>.amazonaws.com/<bucket-name>/<path>#mode=s3"},
	{"https://s3.amazonaws.com/<bucket-name>/<path>","https://s3.us-east-1.amazonaws.com/<bucket-name>/<path>#mode=s3"},
//s3
	{"s3://<bucket-name>/<path>","https://s3.us-east-1.amazonaws.com/<bucket-name>/<path>#mode=s3"},
//Google
	{"https://storage.googleapis.com/<bucket-name>/<path>","https://storage.googleapis.com/<bucket-name>/<path>#mode=s3"},
	{"gs3://<bucket-name>/<path>","https://storage.googleapis.com/<bucket-name>/<path>#mode=s3"},
//Other
// (8) https://<host>/<bucket-name>/<path>
	{"https://<host>/<bucket>/path/2/resource/#mode=s3,zarr", "https://<host>/<bucket>/path/2/resource#mode=s3,zarr"},
// (9) https://<bucket-name>.s3.<region>.domain.example.com/<path>
	{"https://<bucket>.s3.<region>.<domain>/path/2/resource/#mode=s3,zarr", "https://s3.<region>.<domain>/<bucket>/path/2/resource#mode=s3,zarr"},
// (10)https://s3.<region>.example.com/<bucket>/<path>
	{"https://s3.<region>.example.com/bucket/path/2/resource/#mode=s3,zarr", "https://s3.<region>.example.com/bucket/path/2/resource#mode=s3,zarr"},
	{"https://s3.example.com/bucket/path/2/resource/#mode=s3,zarr", "https://s3.example.com/bucket/path/2/resource#mode=s3,zarr"},
	{"https://server.example.com/bucket/path/2/resource#mode=s3", "https://server.example.com/bucket/path/2/resource#mode=s3"},
	{"https://prefix.<bucket>.s3.localhost.example.cloud/path/2/resource#mode=s3", "https://prefix.<bucket>.s3.localhost.example.cloud/path/2/resource#mode=s3"},
	{"https://<bucket>.s3.<region>.localhost/path/2/resource#mode=s3", "https://s3.<region>.localhost/<bucket>/path/2/resource#mode=s3"},
	{NULL,NULL}
};
#endif

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
    int index;

    for(index=0,test=TESTS;test->url;test++,index++) {
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
	            fprintf(stderr,"Mismatch: [%d] expected=|%s| actual=|%s|\n",index,test->expected,built);
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

#ifdef NETCDF_ENABLE_S3
	nc_initialize();
	for(index=0, test=S3TESTS;test->url;test++,index++) {
		int ret = 0;
		NCURI* uri = NULL;
		ret = ncuriparse(test->url,&uri);
		if(ret != NC_NOERR) {
			fprintf(stderr,"Parse fail: %s\n",test->url);
			failcount++;
		} else {
			int iss3 = NC_iss3(uri,NULL);
			if(iss3 != 0){
				NCURI * newuri = NULL;
				if(NC_s3urlrebuild(uri,NULL,&newuri)){
					fprintf(stderr, "Could not reinterpret url [%d] with s3urlrebuild: %s\n",index,test->url);
					fprintf(stderr,"Mismatch: [%d] expected=|%s| actual=|%s|\n",index,test->expected,"NULL");
					failcount ++;
				}else{
					char* built = ncuribuild(newuri,NULL,NULL,NCURIALL);
					if(built == NULL) {
						fprintf(stderr,"Build fail: %s\n",test->url);
						failcount++;
					} else {
						if(strcmp(test->expected,built) != 0) {
							fprintf(stderr,"Mismatch: [%d] expected=|%s| actual=|%s|\n",index,test->expected,built);
							failcount++;
						}
						free(built);
					}
				}
				ncurifree(newuri);
			}
		}
		ncurifree(uri);
	}
	nc_finalize();
#endif
    fprintf(stderr,"%s test_ncuri\n",failcount > 0 ? "***FAIL":"***PASS");
    return (failcount > 0 ? 1 : 0);
}
