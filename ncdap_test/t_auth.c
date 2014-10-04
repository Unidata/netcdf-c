#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "netcdf.h"

#define USERPWD "tiggeUser:tigge"

#define URL1 "https://tiggeUser:tigge@%s/thredds/dodsC/restrict/testData.nc"
#define URL2 "https://%s/thredds/dodsC/restrict/testData.nc"

/* Embedded user:pwd */
static char url1[1024];

/* user:pwd from .dodsrc*/
static char url2[1024];

/* Test redirect */
static char* url3 = 
"http://tiggeUser:tigge@thredds-test.ucar.edu/thredds/dodsC/restrict/testData.nc";

/* .dodsrc file */
static char* CONTENT = "HTTP.CREDENTIALS.USER=tiggeUser\nHTTP.CREDENTIALS.PASSWORD=tigge\n";

#ifdef DEBUG
static void
CHECK(int e, const char* msg)
{
    if(e == NC_NOERR) return;
    if(msg == NULL) msg = "Error";
    printf("%s: %s\n", msg, nc_strerror(e));
    exit(1);
}
#endif

int
main(int argc, char** argv)
{
    int ncid,retval,pass;
    FILE* dodsrc;
    char* envv;

    envv = getenv("THREDDSTESTSERVER");
    if(envv == NULL) {
	envv = "remotetest.unidata.ucar.edu";
    } else { /* walk past the schema and // */
        char* p = strchr(envv,':');
        if(p == NULL) {
	    fprintf(stderr,"URL has no schema: %s\n",url1);
	    exit(1);    
        }
	envv = p+3;
    }
    snprintf(url1,sizeof(url1),URL1,envv);
    snprintf(url2,sizeof(url2),URL2,envv);

printf("url1: %s\n",url1);
printf("url2: %s\n",url2);
fflush(stdout);

    pass = 1; /* assume success */

    dodsrc = fopen(".dodsrc","w");
    if(dodsrc == NULL) {
        fprintf(stderr,"Cannot create .dodsrc\n");
        exit(1);
    }    
    fprintf(dodsrc,CONTENT);
    fclose(dodsrc);

    printf("Testing: Http Basic Authorization\n\n");
    if(1) {
        printf("Embedded user:pwd: %s\n",url1);
        retval = nc_open(url1, 0, &ncid);
        if(retval != NC_NOERR) {
            pass = 0;
            printf("*** FAIL: Embedded user:pwd\n");
        } else {
            printf("*** PASS: Embedded user:pwd\n");
	    retval = nc_close(ncid);
	}
        fflush(stdout);
    }

    if(1) {
        printf(".dodsrc user:pwd: %s\n",url2);

        retval = nc_open(url2, 0, &ncid);
        if(retval != NC_NOERR) {
            pass = 0;
            printf("*** FAIL: .dodsrc user:pwd\n");
        } else {
	    retval = nc_close(ncid);
            printf("*** PASS: .dodsrc user:pwd\n");
        }
        fflush(stdout);
    }
    unlink(".dodsrc"); /* delete the file */

    printf("Testing: Http Basic Redirect\n\n");
    if(1) {
        printf("Basic redirect: %s\n",url3);
        retval = nc_open(url3, 0, &ncid);
        if(retval != NC_NOERR) {
            printf("*** XFAIL: Basic redirect\n");
        } else {
            printf("*** PASS: Basic redirect\n");
	    retval = nc_close(ncid);
	}
        fflush(stdout);
    }

    return !pass;
}
