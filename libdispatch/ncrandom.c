/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif


/* Support platform independent generation of 32-bit unsigned int random numbers */

int
main() {
    unsigned int urnd = 0; /* range 0..2147483647 */
#ifdef WIN32
    (void)rand_s(&urnd);
#else
    long rnd;
    rnd = random();
    urnd = (unsigned)(rnd & 0xffffffff);
#endif
    printf("%u",urnd);
    exit(0);    
}
