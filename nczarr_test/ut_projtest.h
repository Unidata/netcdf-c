/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef ZPROJTEST_H
#define ZPROJTEST_H

#include "ut_includes.h"

typedef struct ProjTest {
    struct Test test; /* Superclass */
    int rank;
    size64_t dimlen[NC_MAX_VAR_DIMS];
    size64_t chunklen[NC_MAX_VAR_DIMS];
    NCZSlice slices[NC_MAX_VAR_DIMS];
    NCZChunkRange range[NC_MAX_VAR_DIMS];
    unsigned int typesize;
}  ProjTest;


#endif /*ZPROJTEST_H*/
