#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netcdf.h>

#define VAR "i32"

#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

#undef DEBUG


/* Figure out topsrcdir; assume we are running in ncdap_test */
static char*
gettopsrcdir(void)
{
    char *p,*q, tmp[4096];
    char* topsrcdir = getenv("TOPSRCDIR");
    if(topsrcdir != NULL) {
	strcpy(tmp,topsrcdir);
    } else {
	fprintf(stderr,"$abs_top_srcdir not defined: using 'getcwd'");
        getcwd(tmp,sizeof(tmp));
    }
    /* Remove trailing filename */
    for(p=tmp,q=NULL;*p;p++) {
	if(*p == '\\') *p  = '/';
	if(*p == '/') q = p;		
    }
    if(q == NULL)
       q = tmp; /* should not ever happen, but oh well*/
    else
       *q = '\0';    
    return strdup(tmp);
}    

int
main()
{
    int ncid, varid;
    int retval;
    int i32[100];
    size_t start[1];
    size_t count[1];
    int ok = 1;    
    char* topsrcdir;
    char url[4096];

    /* Assume that TESTS_ENVIRONMENT was set */
    topsrcdir = gettopsrcdir();
    strcat(url,"");
    strcat(url,"file://");
    strcat(url,topsrcdir);
    strcat(url,"/ncdap_test/testdata3/test.02");
    strcat(url,"#dap2");

    if ((retval = nc_open(url, 0, &ncid)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid, VAR, &varid)))
       ERR(retval);

    start[0] = 0;
    count[0] = 26;
    if ((retval = nc_get_vara_int(ncid, varid, start, count, i32)))
    if(retval != NC_EINVALCOORDS) {
	printf("nc_get_vara_int did not return NC_EINVALCOORDS");
	ok = 0;
    }

    nc_close(ncid);

    printf(ok?"*** PASS\n":"*** FAIL\n");
    return 0;
}
