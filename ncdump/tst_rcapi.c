/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test RC interface
   Dennis Heimbigner
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "netcdf.h"

#undef DEBUG

#ifndef nullfree
#define nullfree(x) {if((x)!=NULL) free(x);}
#endif

#if 0
static void
printrc(NCglobalstate* ngs)
{
    size_t i,nentries = 0;
    NCRCinfo* info = NULL;
    NCRCentry* entry = NULL;

    info = ngs->rcinfo;
    if(info->ignore) {
	fprintf(stderr,".rc ignored\n");
	return;
    }

    /* Print out the .rc entries */
    if((nentries = NC_rcfile_length(info))==0) {
        printf("<empty>\n");
	exit(0);
    }
    for(i=0;i<nentries;i++) {
	entry = NC_rcfile_ith(info,i);
	if(entry == NULL) abort();
        if(entry->host != NULL) {
	    printf("[%s ",entry->host);
            if(entry->urlpath != NULL)
	        printf("/%s] ",entry->urlpath);
	    printf("]");					
        }
	printf("|%s|->|%s|\n",entry->key,entry->value);
    }
}
#endif

int
main(int argc, char **argv)
{
    int stat = NC_NOERR;
    const char* key;
    const char* value;
    char* newvalue;

    printf("load:\n");

    key = "key1";
    value = "value1";
    printf("insert: key=%s value=%s\n",key, value);
    newvalue = nc_rc_get(key);
    printf("insert: before: %s = %s\n",key,newvalue);
    nullfree(newvalue);
    stat = nc_rc_set(key, value);
    if(stat) {fprintf(stderr,"***Fail: nc_rc_set: %s\n",nc_strerror(stat)); goto done;}
    newvalue = nc_rc_get(key);
    printf("insert: after: %s = %s\n",key,newvalue);
    nullfree(newvalue);

    key = "key1";
    value = "value2";
    printf("replace: key=%s value=%s\n",key, value);
    newvalue = nc_rc_get(key);
    printf("replace: before: %s = %s\n",key,newvalue);
    nullfree(newvalue);
    stat = nc_rc_set(key, value);
    if(stat) {fprintf(stderr,"***Fail: nc_rc_set: %s\n",nc_strerror(stat)); goto done;}
    newvalue = nc_rc_get(key);
    printf("replace: after: %s = %s\n",key,newvalue);
    nullfree(newvalue);

done:
    nc_finalize();
    return 0;
}
