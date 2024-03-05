/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"

#include "stdlib.h"
#include "stdio.h"
#include <stddef.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#undef DEBUG

int
main(int argc, char** argv)
{
    unsigned char c;
    FILE* f = NULL;

    if(argc > 1) {
	/* use argv[1] as input */
	f = fopen(argv[1],"r");
	if(f == NULL) {fprintf(stderr,"No such file: %s\n",argv[1]); exit(1);}
    } else
        f = stdin;

    for(;;) {
	size_t ret = fread(&c, 1, 1, f);
	if(ret != 1) break;
        printf("%.2hhx", c);
    }
    if(f != stdin) fclose(f);
    return 0;
}
