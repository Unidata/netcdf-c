/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"

void ut_chunk_print(int sort, ...);

/**
Test computation of applying a slice to a sequence of chunks
*/

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    NCZSliceProjections slpv[NC_MAX_VAR_DIMS];
    NCZOdometer* odom = NULL;
    Vardef* var = NULL;
    struct Common common;
    
    /* Initialize */
    memset(&slpv,0,sizeof(slpv));
    memset(&common,0,sizeof(common));

    if((stat = ut_init(argc, argv, &utoptions))) goto done;

    /* Set the printer */
    zutester.tests = UTEST_RANGE;
    zutester.print = ut_chunk_print;
    zutest = &zutester;

    var = nclistget(utoptions.vardefs,0);

    /* Fill in parts of common */
    common.rank = var->rank;

    if((stat = NCZ_projectslices(var->dimsizes, var->chunksizes, utoptions.slices, &common, &odom)))
	goto done;

#if 0
    {
	int i;
	size64_t* mem = (size64_t*)printer.output;
	for(i=0;i<printer.used;i++) {
	    printf("[%d] %llu\n",i,mem[i]);
	}
    }
#endif

done:
    if(stat)
	nc_strerror(stat);
    return (stat ? 1 : 0);    
}

void
ut_chunk_print(int sort, ...)
{
    int i;
    va_list ap;    
#if 0
    struct Common* common = NULL;    
#endif
    size64_t rank; /* variable rank */
    NCZSlice* slices = NULL; /* the complete set of slices |slices| == R*/
    size64_t* chunksizes = NULL; /* the chunk length corresponding to the dimensions */
    NCZChunkRange* ranges = NULL; /* computed chunk ranges */

    va_start(ap,sort);
    
    switch (sort) {
    default: break; /* ignore */
    case UTEST_RANGE: /* () */
	rank = va_arg(ap,size64_t);
        slices = va_arg(ap,NCZSlice*);
	chunksizes = va_arg(ap,size64_t*);
        ranges = va_arg(ap,NCZChunkRange*);
        printf("Chunksizes: %s\n",nczprint_vector(rank,chunksizes));
        printf("Slices: ");
	for(i=0;i<rank;i++)
	    printf(" %s",nczprint_slicesx(rank,slices,1));
	printf("\n");
        printf("Ranges: ");
	for(i=0;i<rank;i++)
	    printf(" %s",nczprint_chunkrange(ranges[i]));
	printf("\n");
	break;
    }
    va_end(ap);
}

