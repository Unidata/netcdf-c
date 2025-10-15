#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <string.h>

#include "manyurls.h"

/* Turn on to verify that all the URLS in manyurls.h are valid */
#undef VERIFY

int main()
{
    int i,ncid;
    char** p;

    for(i=1,p=urllist;i<100;p++,i++) {
	    char* tp = *p;	
            int mode = 0;
            int status = -1;
	    printf("Testing: %s\n",tp);
            status = nc_open(tp, mode, &ncid);
	    switch(status) {
	    case NC_NOERR:
		break;
	    case NC_ENOTFOUND:
	        printf("Warning: notfound: {%d} %s\n",i,tp);
		status = NC_NOERR;
		break;
	    default:
	        printf("Failure: %s\n",nc_strerror(status));
		break;
            }
#ifdef VERIFY
            nc_close(ncid); /* Do not close to see how many urls we can keep open */
#endif
    }
    return 0;
}

