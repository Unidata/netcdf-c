#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "ncrc.h"
#include "nc4internal.h"

int
main(int argc, char** argv)
{
    size_t i,nentries = 0;
    NCglobalstate* ngs = NC_getglobalstate();
    NCRCinfo* info = NULL;
    NCRCentry* entry = NULL;

    /* Cause the .rc files to be read and merged */
    nc_initialize();

    if((ngs = NC_getglobalstate())==NULL) abort();
    info = ngs->rcinfo;

    if(info->ignore) {
	fprintf(stderr,".rc ignored\n");
	exit(0);
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
            if(entry->path != NULL)
	        printf("/%s] ",entry->path);
	    printf("]");					
        }
	printf("|%s|->|%s|\n",entry->key,entry->value);
    }
    return 0;
}
