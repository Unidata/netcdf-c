/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "zincludes.h"

static int pcounter = 0;

/* Forward */
static int compute_intersection(const NCZSlice* slice, const size64_t chunklen, NCZChunkRange* range);

/**************************************************/
/* Goal:create a vector of chunk ranges: one for each slice in
   the top-level input. For each slice, compute the index (not
   absolute position) of the first chunk that intersects the slice
   and the index of the last chunk that intersects the slice.
   In practice, the count = last - first + 1 is stored instead of the last index.
*/
int
NCZ_compute_chunk_ranges(
	int rank, /* variable rank */
        const NCZSlice* slices, /* the complete set of slices |slices| == R*/
	const size64_t* chunklen, /* the chunk length corresponding to the dimensions */
        NCZChunkRange* ncr)
{
    int stat = NC_NOERR;
    int i;

    for(i=0;i<rank;i++) {
	if((stat = compute_intersection(&slices[i],chunklen[i],&ncr[i])))
	    goto done;
    }

done:
    return stat;
}

static int
compute_intersection(
        const NCZSlice* slice,
	const size64_t chunklen,
        NCZChunkRange* range)
{
    range->start = floordiv(slice->start, chunklen);
    range->stop = ceildiv(slice->stop, chunklen);
    return NC_NOERR;
}

/**
Compute the projection of a slice as applied to n'th chunk.
This is somewhat complex because for the first projection, the
start is the slice start, but after that, we have to take into
account that for a non-one stride, the start point in a
projection may be offset by some value in the range of
0..(slice.stride-1).
*/
int
NCZ_compute_projections(size64_t dimlen, size64_t chunklen, size64_t chunkindex, const NCZSlice* slice, size_t n, NCZProjection* projections)
{
    int stat = NC_NOERR;
    size64_t offset,count,avail;
    NCZProjection* projection;
    
    projection = &projections[n];

    projection->id = ++pcounter;
    projection->chunkindex = chunkindex;

    offset = chunklen * chunkindex; /* with respect to dimension (WRD) */

    /* Actual limit of the n'th touched chunk, taking
       dimlen and stride->stop into account. */
    projection->limit = (chunkindex + 1) * chunklen; /* WRD */
    if(projection->limit > dimlen) projection->limit = dimlen;
    if(projection->limit > slice->stop) projection->limit = slice->stop;

    /* Len is no. of touched indices along this dimension */
    projection->len = projection->limit - offset;

    if(n == 0) {
	/*initial case: original slice start is in 1st projection */
	projection->first = slice->start; /* WRD */
    } else { /* n > 0 */
	NCZProjection* prev = &projections[n-1];
	/* prevunused is the amount unused at end of the previous chunk.
	   => we need to skip (slice->stride-prevunused) in this chunk */
        /* Compute limit of previous chunk */
	size64_t prevunused = prev->limit - prev->last;
	projection->first = offset + (slice->stride - prevunused); /* WRD */
    }
    /* Compute number of places touched in this chunk */
    avail = projection->limit - projection->first; /*WRD*/
    count  = ceildiv(avail, slice->stride);

    /* Last place to be touched */
    projection->last = projection->first + ((slice->stride * count) - 1); /*WRD*/

    /* Compute the slice relative to this chunk.
       Recall the possibility that start+stride >= projection->limit */
    projection->chunkslice.start = (projection->first - offset);
    projection->chunkslice.stop = projection->chunkslice.start + (slice->stride * count);
//+1
    if(slice->stop > projection->limit) {
        projection->chunkslice.stop = projection->len;
    }
    projection->chunkslice.stride = slice->stride;
    projection->chunkslice.len = chunklen;

    /* compute the I/O position: the "location" in the memory
       array to read/write items */
    if(n == 0)
        projection->iopos = 0;
    else 
        projection->iopos = ceildiv(offset - slice->start, slice->stride);
    /* And number of I/O items */
    projection->iocount = count;

    projection->memslice.start = projection->iopos;
    projection->memslice.stop = projection->memslice.start + projection->iocount;
    projection->memslice.stride = 1;
#if 0
    projection->memslice.len = projection->memslice.stop;
#else
    projection->memslice.len = dimlen;
#endif
    return stat;
}

/* Goal:
Create a vector of projections wrt a slice and a sequence of chunks.
*/

int
NCZ_compute_per_slice_projections(
	int r, /* which dimension are we projecting? */
        const NCZSlice* slice, /* the slice for which projections are computed */
	const NCZChunkRange* range, /* range */
	size64_t dimlen, /* the dimension length for r'th dimension */
	size64_t chunklen, /* the chunk length corresponding to the dimension */
	NCZSliceProjections* slp)
{
    int stat = NC_NOERR;
    size64_t index,slicecount;
    size_t n;

    /* Part fill the Slice Projections */
    slp->r = r;
    slp->range = *range;
    slp->count = range->stop - range->start;
    if((slp->projections = calloc(slp->count,sizeof(NCZProjection))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* Compute the total number of output items defined by this slice
           (equivalent to count as used by nc_get_vars) */
    slicecount = ceildiv((slice->stop - slice->start), slice->stride);
    if(slicecount < 0) slicecount = 0;

    /* Iterate over each chunk that intersects slice to produce projection */
    for(n=0,index=range->start;index<range->stop;index++,n++) {
	if((stat = NCZ_compute_projections(dimlen, chunklen, index, slice, n, slp->projections)))
	    goto done;
    }

done:
    return stat;
}

/* Goal:create a vector of SliceProjection instances: one for each
    slice in the top-level input. For each slice, compute a set
    of projections from it wrt a dimension and a chunk size
    associated with that dimension.
*/
int
NCZ_compute_all_slice_projections(
	int rank, /* variable rank */
        const NCZSlice* slices, /* the complete set of slices |slices| == R*/
	const size64_t* dimlen, /* the dimension lengths associated with a variable */
	const size64_t* chunklen, /* the chunk length corresponding to the dimensions */
        const NCZChunkRange* ranges,
        NCZSliceProjections* results)
{
    int stat = NC_NOERR;
    size64_t r; 

    for(r=0;r<rank;r++) {
	/* Compute each of the rank SliceProjections instances */
	NCZSliceProjections* slp = &results[r];
        if((stat=NCZ_compute_per_slice_projections(
					r,
					&slices[r],
					&ranges[r],
					dimlen[r],
					chunklen[r],
                                        slp))) goto done;
    }

done:
    return stat;
}

/**************************************************/
/* Utilities */
    
void
NCZ_clearsliceprojections(int count, NCZSliceProjections* slpv)
{
    if(slpv != NULL) {
	int i;
        for(i=0;i<count;i++) {
	    NCZSliceProjections* slp = &slpv[i];
	    nullfree(slp->projections);	
	}
    }
}

#if 0
static void
clearallprojections(NCZAllProjections* nap)
{
    if(nap != NULL) {
	int i;
	for(i=0;i<nap->rank;i++) 
	    nclistfreeall(&nap->allprojections[i].projections);
    }
}
#endif

