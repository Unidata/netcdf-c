/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"

#include "stdlib.h"
#include "stdio.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#undef DEBUG

static char hex[16] = "0123456789abcdef";

int
main(int argc, char** argv)
{
    unsigned char c;
    unsigned int c0,c1;
    FILE* f = NULL;

    if(argc > 1) {
	/* use argv[1] as input */
	f = fopen(argv[1],"r");
	if(f == NULL) {fprintf(stderr,"No such file: %s\n",argv[1]); exit(1);}
    } else
        f = stdin;

    for(;;) {
	int ret = fread(&c, 1, 1, f);
	if(ret != 1) break;
        c1 = c;
        c0 = c1 & 0xf;
	c1 = (c1 >> 4);
        c0 = hex[c0];
        c1 = hex[c1];
        printf("%c%c",(char)c1,(char)c0);
    }
    if(f != stdin) fclose(f);
    return 0;
}
