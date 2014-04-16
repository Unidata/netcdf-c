#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"

/* Embedded user:pwd */
static char* URL1 = 
"http://tiggeUser:tigge@remotetest.unidata.ucar.edu/thredds/dodsC/restrict/testData.nc";
/* user:pwd from .dodsrc*/
static char* URL2 = 
"http://remotetest.unidata.ucar.edu/thredds/dodsC/restrict/testData.nc";

/* .dodsrc file */
static char* DODSRC = "HTTP.CREDENTIALS.USER=tiggeUser\nHTTP.CREDENTIALS.PASSWORD=tigge\n";

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
    FILE* dodsrc;

    printf("Testing: Http Basic Authorization\n\n");
    printf("Embedded user:pwd: %s\n",URL1);
    pass = 1; /* assume success */
    retval = nc_open(URL1, 0, &ncid);
    if(retval != NC_NOERR) {
        pass = 0;
        printf("*** FAIL: Embedded user:pwd %s\n",URL1);
    } else
	retval = nc_close(ncid);

    printf(".dodsrc user:pwd: %s\n",URL1);
    dodsrc = fopen(".dodsrc","w");
    if(dodsrc == NULL) {
        fprintf(stderr,"Cannot create .dodsrc\n");
	exit(1);
    }    
    fprintf(dodsrc,DODSRC);
    fclose(dodsrc);
    retval = nc_open(URL1, 0, &ncid);
    if(retval != NC_NOERR) {
        pass = 0;
        printf("*** FAIL: .dodsrc user:pwd %s\n",URL1);
    } else
	retval = nc_close(ncid);
//    unlink(".dodsrc"); /* delete the file */

    if(!pass)
	return 1;
    printf("*** PASS: Http Basic Authorization\n");
    return 0;
}
