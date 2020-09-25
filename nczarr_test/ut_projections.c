/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"

void ut_chunk_test(int sort, ...);

/**
Test computation of applying a slice to a sequence of chunks
*/

int
main(int argc, char** argv)
{
    int i,r,stat = NC_NOERR;
    Vardef* var = NULL;
    struct Common common;
    NCZSliceProjections slpv[NC_MAX_VAR_DIMS];
    NCZChunkRange ncrv[NC_MAX_VAR_DIMS];
    
    /* Initialize */
    memset(&slpv,0,sizeof(slpv));
    memset(&common,0,sizeof(common));

    if((stat = ut_init(argc, argv, &utoptions))) goto done;

    /* printer off for these tests */
    zutester.tests = 0;
    zutester.print = NULL;
    zutest = &zutester;

    var = nclistget(utoptions.vardefs,0);

    printoptions(&utoptions);

    /* Compute chunk ranges */
    if((stat = NCZ_compute_chunk_ranges(var->rank,utoptions.slices,var->chunksizes,ncrv)))
	goto done;

    if((stat=NCZ_compute_all_slice_projections(
	var->rank,
        utoptions.slices,
        var->dimsizes,
        var->chunksizes,
        ncrv,
	slpv))) goto done;

    /* Dump Results */
    for(r=0;r<var->rank;r++) {
	NCZSliceProjections* slp = &slpv[r];
        if(r != slp->r) usage(NC_EINTERNAL);
        printf("[r=%d] %s %s\n",r,nczprint_chunkrange(slp->range),nczprint_slice(utoptions.slices[r]));
        for(i=0;i<slp->count;i++) {
            NCZProjection* proj = &slp->projections[i];
            printf("[%d] %s\n",i,nczprint_projection(*proj));
	}
    }

    /* Cleanup */
    NCZ_clearsliceprojections(var->rank,slpv);

#if 0
    /* Compute corresponding slice projections  */
    for(r=0;r<var->rank;r++) {
        if((stat = NCZ_compute_per_slice_projections(
			r,
			&utoptions.slices[r],
			&ncrv[r],
			var->dimsizes[r],
			var->chunksizes[r],
			&slpv[r]))) goto done;
    }

    /* Dump Results */
    for(r=0;r<var->rank;r++) {
	NCZSliceProjections* slp = &slpv[r];
	char *sr, *sl;
        if(r != slp->r) usage(NC_EINTERNAL);
	sr = nczprint_chunkrange(slp->range);
	sl = nczprint_slice(utoptions.slices[r]);
        printf("[r=%d] %s %s\n",r,sr,sl);
	nullfree(sr); nullfree(sl);
        for(i=0;i<slp->count;i++) {
            NCZProjection* proj = &slp->projections[i];
            printf("[%d] %s\n",i,nczprint_projection(*proj));
	}
    }
    /* Cleanup */
    NCZ_clearsliceprojections(var->rank,slpv);
#endif

done:
    fflush(stdout);
    nczprint_reclaim();
    if(stat) usage(stat);
    return  0;
}

void
ut_chunk_test(int sort,...)
{
    int i;
    va_list ap;    
#if 0
    struct Common* common = NULL;    
#endif
    int rank; /* variable rank */
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
        printf("[r=%d] Chunksizes: %s\n",rank,nczprint_vector(rank,chunksizes));
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

