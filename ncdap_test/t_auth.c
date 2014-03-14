#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"

static char* URLS[] = {
"http://tiggeUser:tigge@remotetest.unidata.ucar.edu/thredds/dodsC/restrict/testData.nc",
NULL
};

static void
CHECK(int e, const char* msg)
{
    if(e == NC_NOERR) return;
    if(msg == NULL) msg = "Error";
    printf("%s: %s\n", msg, nc_strerror(e));
    exit(1);
}

int
main()
{
    int ncid,retval,pass;
    char** url;

    printf("Testing: Http Basic Authorization\n");
    pass = 0;
    for(url=URLS;*url;url++) {
        retval = nc_open(*url, 0, &ncid);
	if(retval == NC_NOERR) {
	    printf("URL: %s\n",*url);
	    pass = 1;
	    break;
	}
        printf("fail: %s: %s\n", nc_strerror(retval),*url);
    }
    if(pass) {
	retval = nc_close(ncid);
        printf("*** PASS: Http Basic Authorization\n");
        return 0;
    } else {
        CHECK(retval,"*** Fail: Http Basic Authorization");
	return 1;
    }
}
