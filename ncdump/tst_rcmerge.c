#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "ncrc.h"

int
main(int argc, char** argv)
{
    size_t i,ntriples = 0;
    NCRCglobalstate* ngs = ncrc_getglobalstate();
    NCRCinfo* info = NULL;
    NCTriple* triple = NULL;

    /* Cause the .rc files to be read and merged */
    nc_initialize();

    if((ngs = ncrc_getglobalstate())==NULL) abort();
    info = &ngs->rcinfo;

    if(info->ignore) {
	fprintf(stderr,".rc ignored\n");
	exit(0);
    }

    /* Print out the .rc triples */
    if((ntriples = NC_rcfile_length(info))==0) {
        printf("<empty>\n");
	exit(0);
    }
    for(i=0;i<ntriples;i++) {
	triple = NC_rcfile_ith(info,i);
	if(triple == NULL) abort();
        if(triple->host != NULL)
	    printf("[%s] ",triple->host);
	printf("|%s|->|%s|\n",triple->key,triple->value);
    }
    return 0;
}
