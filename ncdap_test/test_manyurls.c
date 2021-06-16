#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <string.h>

#include "manyurls.h"

int main()
{
    int i,ncid;
    char** p;

    for(i=1,p=urllist;i<100;p++,i++) {
	    char* tp = *p;	
            int mode = 0;
            int status = -1;
	    printf("Opening: %s\n",tp);
            status = nc_open(tp, mode, &ncid);
	    switch(status) {
	    case NC_NOERR:
		break;
	    case NC_ENOTFOUND:
	        printf("{%d} %s\n",i,tp);
		status = NC_NOERR;
		break;
	    default:
	        fprintf(stderr,"*** %s\n",nc_strerror(status));
		return 1;
            }
            // nc_close(ncid);
    }
    return 0;
}

