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

#define DEBUG

static char hex[16] = "0123456789abcdef";

int
main(int argc, char** argv)
{
    unsigned char c;
    unsigned int c0,c1;

    for(;;) {
	int ret = fread(&c, 1, 1, stdin);
	if(ret != 1) break;
        c1 = c;
        c0 = c1 & 0xf;
	c1 = (c1 >> 4);
        c0 = hex[c0];
        c1 = hex[c1];
        printf("%c%c",(char)c1,(char)c0);
    }
    return 0;
}
