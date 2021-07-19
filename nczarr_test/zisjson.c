/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

/* Parse input to see if it looks like json.
   Output 1 or 0.
*/

#include "config.h"

#include "stdlib.h"
#include "stdio.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "netcdf.h"
#include "nclist.h"
#include "ncjson.h"

#define MAXREAD 8192

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    char text[MAXREAD+1];
    NCjson* json = NULL;
    int i, red;
    FILE* f = NULL;

    if(argc > 1) {
	/* use argv[1] as input */
	f = fopen(argv[1],"r");
	if(f == NULL) {fprintf(stderr,"No such file: %s\n",argv[1]); exit(1);}
    } else
        f = stdin;

    /* Read json from stdin */
    for(i=0;;i++) {
	unsigned char c;
	red = fread(&c, 1, 1, f);
	if(red != 1) break;
	if(i < MAXREAD) text[i] = (char)c;
    }
    if(i >= MAXREAD) {
	fprintf(stderr,"Input too long\n");	
	exit(1);
    }
    text[i] = '\0';
    if(i == 0) {
	stat = NC_EEMPTY;
    } else {
        stat = NCJparse(text,0,&json);
	NCJreclaim(json);
    }
    printf("%d",(stat==NC_NOERR?1:0)); /* parse success|failure */    
    if(f != stdin) fclose(f);
    return 0;
}
