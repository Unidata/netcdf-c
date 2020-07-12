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
#include "zjson.h"

#define MAXREAD 8192

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    char text[MAXREAD+1];
    NCjson* json = NULL;
    int i, red;

    /* Read json from stdin */
    for(i=0;;i++) {
	unsigned char c;
	red = fread(&c, 1, 1, stdin);
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
    return 0;
}
