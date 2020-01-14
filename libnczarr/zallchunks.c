/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "zincludes.h"

static int initialized = 0;

/* Combine some values to simplify internal argument lists */
struct Common {
    NC_FILE_INFO_T* file;
    NC_VAR_INFO_T* var;
    size_t R;
    size_t* dimlen;
    size_t* chunklen;
    void* output;
    size_t typesize;
    size_t* shape; /* shape of the output hyperslab */
};

/* Forward */
static int applychunkindices(const size_t*, const NCZSliceIndex*, const struct Common);
static int getdata(const NCZSlice*, size_t, void*, const struct Common);
static size_t computelinearoffset(size_t, const size_t*, const size_t*);

/**************************************************/
int
ncchunking_init(void)
{
    initialized = 1;
    return NC_NOERR;
}

/**************************************************/

/**
Goal: Given the slices being applied to the variable, create
and walk all possible combinations of projection vectors that
can be evaluated to provide the output data.

@param file Controlling file
@param var Controlling variable
@param slices Slices being applied to variable
@param output
@param typesize Size of type being written
*/

int
ncz_evaluateslices(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var,
		  NCZSlice* slices, void* output, size_t typesize)
{
    int stat = NC_NOERR;
    int i;
    NCZSliceIndex* allsliceindices = NULL;
    size_t nchunks[NC_MAX_VAR_DIMS];
    NCZOdometer* odom = NULL;
    struct Common common;

    if(!initialized) ncchunking_init();

    /* Package common arguments */
    common.file = file;
    common.var = var;
    common.R = var->ndims;
    if((common.dimlen = calloc(var->ndims,sizeof(size_t)))==NULL)
	{stat = NC_ENOMEM; goto done;}
    if((common.chunklen = calloc(var->ndims,sizeof(size_t)))==NULL)
	{stat = NC_ENOMEM; goto done;}
    for(i=0;i<var->ndims;i++) {
	common.dimlen[i] = var->dim[i]->len;
	common.chunklen[i] = var->chunksizes[i];
    }
    common.output = output;
    common.typesize = typesize;
    /* Fill in shape below */
    if((common.shape = calloc(var->ndims,sizeof(size_t))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* Compute the slice index vector */
    if((ncz_compute_all_slice_projections(common.R,slices,common.dimlen,common.chunklen,&allsliceindices)))
	goto done;

    /* Extract the nchunks and shape vectors */
    for(i=0;i<common.R;i++) {
	nchunks[i] = allsliceindices[i].nchunks;
	common.shape[i] = allsliceindices[i].count;
    }

    /* Create an odometer to walk nchunk combinations */
    /* iterate each "wheel[i]" over 0..nchunk[i] with R wheels */
    if((odom = nczodom_new(common.R,NC_coord_zero,nchunks,NC_coord_one)) == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* iterate over the odometer: all combination of chunk
       indices in the projections */
    for(;nczodom_more(odom);nczodom_next(odom)) {
	size_t* projindices = nczodom_indices(odom);
	if((applychunkindices(projindices,allsliceindices,common)))
	    goto done;
    }

done:
    return stat;
}

/* Goal: given a vector of chunk indices from projections,
         extract the corresponding data and store it into the
         output target
*/
static int
applychunkindices(const size_t* projindices, const NCZSliceIndex* allsliceindices, const struct Common common)
{
    int stat = NC_NOERR;
    int i;

    NCZProjection projections[NC_MAX_VAR_DIMS];
    NCZSlice slices[NC_MAX_VAR_DIMS];
    size_t outpos[NC_MAX_VAR_DIMS];
    size_t chunkindices[NC_MAX_VAR_DIMS]; /* global chunk indices */
    void* chunkdata;
    size_t outputstart;

    /* This is complicated. We need to construct a vector (of size R)
       of slices where the ith slice is determined from a projection
       for the ith chunk index of chunkindices. We then iterate over
       that odometer to extract values and store them in the output.
    */
    for(i=0;i<common.R;i++) {
      NCZSliceIndex sliceindex = allsliceindices[i];
      size_t projchunk = projindices[i]; /* projection index */
      projections[i] = sliceindex.projections[projchunk]; /* corresponding projection */
      slices[i] = projections[i].slice;   
      outpos[i] = projections[i].outpos;
      chunkindices[i] = sliceindex.chunk0 + projchunk; /* global chunk index */
    }
    /* Compute where the extracted data will go in the output vector */
    outputstart = computelinearoffset(common.R,outpos,common.shape);  
    /* read the chunk */
    if((stat=readchunk(common.file,common.var,chunkindices,&chunkdata)))
	goto done;
    if((stat=getdata(slices,outputstart,chunkdata,common))) goto done;
done:
    return stat;
}
  
/* Goal: given a set of indices pointing to projections,
         extract the corresponding data and store it into the
         output target.
*/
static int
getdata(const NCZSlice* slices, size_t outputstart, void* chunkdata, const struct Common common)
{
    int stat = NC_NOERR;
    NCZOdometer* sliceodom = NULL;
    char* target = chunkdata;
  
    if((sliceodom = nczodom_fromslices(common.R,slices)) == NULL)
	{stat = NC_ENOMEM; goto done;}
  
    /* iterate over the odometer to get a point in the chunk space */
    for(;nczodom_more(sliceodom);nczodom_next(sliceodom)) {
        size_t* indices = nczodom_indices(sliceodom);
        size_t offset = computelinearoffset(common.R,indices,common.dimlen);
        size_t pos = offset * common.typesize;
        memcpy(target,chunkdata+pos,common.typesize);
	target += common.typesize;
    }
done:
    return stat;
}

/***************************************************/
/* Utilities */

/* Goal: Given a set of per-dimension indices,
     compute the corresponding linear position.
*/
static size_t
computelinearoffset(size_t R, const size_t* indices, const size_t* dimlen)
{
      size_t offset;
      int i;

      offset = 0;
      for(i=0;i<R;i++) {
          offset *= dimlen[i];
          offset += indices[i];
      } 
      return offset;
}

