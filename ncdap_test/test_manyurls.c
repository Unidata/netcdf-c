#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <string.h>

#include "manyurls.h"

#define VERBOSE

int main()
{
    int i,ncid;
    char** p;
    int nerrors = 0;
    int nnotfound = 0;

    for(i=1,p=urllist;i<100;p++,i++) {
	    char* tp = *p;	
            int mode = 0;
            int status = -1;
#ifdef VERBOSE
	    printf("Opening: %s\n",tp);
#endif
            status = nc_open(tp, mode, &ncid);
	    switch(status) {
	    case NC_NOERR:
		break;
	    case NC_ENOTFOUND:
#ifdef VERBOSE
	        printf("{%d} %s\n",i,tp);
#endif
		nnotfound++;
		break;
	    default:
	        fprintf(stderr,"*** %s\n",nc_strerror(status));
		nerrors++;
		break;
            }
            if(status) nc_close(ncid);
	    status = NC_NOERR;
    }
    fprintf(stderr,">>> not-found=%d errors=%d\n",nnotfound,nerrors);
    fflush(stderr);
    return (nnotfound+nerrors > 0 ? 1: 0);
}

