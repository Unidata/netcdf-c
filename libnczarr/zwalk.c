/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "zincludes.h"

#undef WDEBUG

static int initialized = 0;

/* Forward */
static int NCZ_walk(NCZProjection** projv, NCZOdometer* chunkodom, NCZOdometer* slpodom, NCZOdometer* memodom, const struct Common* common, void* chunkdata);
static int rangecount(NCZChunkRange range);
static int readfromcache(void* source, size64_t* chunkindices, void** chunkdata);
static int NCZ_fillchunk(void* chunkdata, struct Common* common);
static int transfern(NCZOdometer* slpodom, NCZOdometer* memodom, const struct Common* common, unsigned char* slpptr0, unsigned char* memptr0);    


const char*
astype(int typesize, void* ptr)
{
    switch(typesize) {
    case 4: {
	static char is[8];
	snprintf(is,sizeof(is),"%u",*((unsigned int*)ptr));
	return is;
        } break;
    default: break;
    }
    return "?";
}

/**************************************************/
int
ncz_chunking_init(void)
{
    initialized = 1;
    return NC_NOERR;
}

/**************************************************/

/**
Goal: Given the slices being applied to the variable, create
and walk all possible combinations of projection vectors that
can be evaluated to provide the output data.
Note that we do not actually pass NCZSlice but rather
(start,count,stride) vectors.

@param var Controlling variable
@param usreading reading vs writing
@param start start vector
@param stop stop vector
@param stride stride vector
@param memory target or source of data
@param typecode nc_type of type being written
@param walkfcn fcn parameter to actually transfer data
*/

int
NCZ_transferslice(NC_VAR_INFO_T* var, int reading,
		  size64_t* start, size64_t* count, size64_t* stride,
		  void* memory, nc_type typecode)
{
    int r,stat = NC_NOERR;
    size64_t dimlens[NC_MAX_VAR_DIMS];
    size64_t chunklens[NC_MAX_VAR_DIMS];
    NCZSlice slices[NC_MAX_VAR_DIMS];
    struct Common common;
    NCZ_FILE_INFO_T* zfile = NULL;
    NCZ_VAR_INFO_T* zvar = NULL;
    size_t typesize;

    if((stat = NC4_inq_atomic_type(typecode, NULL, &typesize))) goto done;

    for(r=0;r<var->ndims;r++) {
	dimlens[r] = var->dim[r]->len;
	chunklens[r] = var->chunksizes[r];
	slices[r].start = start[r];
	slices[r].stride = stride[r];
	slices[r].stop = start[r]+(count[r]*stride[r]);
	slices[r].len = dimlens[r];
    }


    /* Fill in common */
    memset(&common,0,sizeof(common));
    common.var = var;
    common.file = (var->container)->nc4_info;
    zfile = common.file->format_file_info;
    zvar = common.var->format_var_info;

    common.reading = reading;
    common.memory = memory;
    common.typesize = typesize;
    common.cache = zvar->cache;

    if((stat = ncz_get_fill_value(common.file, common.var, &common.fillvalue))) goto done;

    common.rank = var->ndims;
    common.swap = (zfile->native_endianness == var->endianness ? 0 : 1);
    common.dimlens = dimlens;
    common.chunklens = chunklens;
    common.reader.source = ((NCZ_VAR_INFO_T*)(var->format_var_info))->cache;
    common.reader.read = readfromcache;

    if((stat = NCZ_transfer(&common, slices))) goto done;

done:
    NCZ_clearcommon(&common);
    return stat;
}

/*
Walk the possible projections.
Broken out so we can use it for unit testing
@param reader to get data
@param common, common parameters
@param slices
@param walkfcn to do transfer
*/
int
NCZ_transfer(struct Common* common, NCZSlice* slices)
{
    int stat = NC_NOERR;
    NCZOdometer* chunkodom =  NULL;
    NCZOdometer* slpodom = NULL;
    NCZOdometer* memodom = NULL;
    void* chunkdata = NULL;

    /*
     We will need three sets of odometers.
     1. Chunk odometer to walk the chunk ranges to get all possible
        combinations of chunkranges over all dimensions.
     2. For each chunk odometer set of indices, we need a projection
        odometer that walks the set of projection slices for a given
        set of chunk ranges over all dimensions.
     3. A memory odometer that walks the memory data to specify
        the locations in memory for read/write
    */     

    if((stat = NCZ_projectslices(common->dimlens, common->chunklens, slices,
		  common, &chunkodom)))
	goto done;
#if 0
fprintf(stderr,"allprojections:\n%s",nczprint_allsliceprojections(common->rank,common->allprojections)); fflush(stderr);
#endif

    /* iterate over the odometer: all combination of chunk
       indices in the projections */
    for(;nczodom_more(chunkodom);) {
	int r;
	size64_t* chunkindices = NULL;
        NCZSlice slpslices[NC_MAX_VAR_DIMS];
        NCZSlice memslices[NC_MAX_VAR_DIMS];
        NCZProjection* proj[NC_MAX_VAR_DIMS];

	chunkindices = nczodom_indices(chunkodom);
	for(r=0;r<common->rank;r++) {
	    NCZSliceProjections* slp = &common->allprojections[r];
	    NCZProjection* projlist = slp->projections;
	    size64_t indexr = chunkindices[r];
  	    /* use chunkindices[r] to find the corresponding projection slice */
	    /* We must take into account that the chunkindex of projlist[r]
               may be greater than zero */
	    /* note the 2 level indexing */
	    indexr -= slp->range.start;
	    NCZProjection* pr = &projlist[indexr];
	    proj[r] = pr;
	}
	for(r=0;r<common->rank;r++) {
	    slpslices[r] = proj[r]->chunkslice;
	    memslices[r] = proj[r]->memslice;
	}
	if(zutest && zutest->tests & UTEST_TRANSFER)
	    zutest->print(UTEST_TRANSFER, common, chunkodom, slpslices, memslices);

	slpodom = nczodom_fromslices(common->rank,slpslices);
	memodom = nczodom_fromslices(common->rank,memslices);
        /* Read from cache */
        switch ((stat = common->reader.read(common->reader.source, chunkindices, &chunkdata))) {
        case NC_EEMPTY: /* cache created the chunk */
	    if((stat = NCZ_fillchunk(chunkdata,common))) goto done;
	    break;
        case NC_NOERR: break;
        default: goto done;
        }

	/* This is the key action: walk this set of slices and transfer data */
	if((stat = NCZ_walk(proj,chunkodom,slpodom,memodom,common,chunkdata))) goto done;

        nczodom_free(slpodom); slpodom = NULL;
        nczodom_free(memodom); memodom = NULL;

        nczodom_next(chunkodom);
    }

done:
    nczodom_free(slpodom);
    nczodom_free(memodom);
    nczodom_free(chunkodom);
    return stat;
}


/*
@param projv
@param chunkodom
@param slpodom
@param memodom
@param common
@param chunkdata
@return NC_NOERR
*/
static int
NCZ_walk(NCZProjection** projv, NCZOdometer* chunkodom, NCZOdometer* slpodom, NCZOdometer* memodom, const struct Common* common, void* chunkdata)
{
    int stat = NC_NOERR;

    for(;;) {
        if(nczodom_more(slpodom)) {
            size64_t slpoffset = 0;
            size64_t memoffset = 0;
            unsigned char* memptr0 = NULL;
            unsigned char* slpptr0 = NULL;

            /* Convert the indices to a linear offset WRT to chunk */
            slpoffset = nczodom_offset(slpodom);
            memoffset = nczodom_offset(memodom);

            /* transfer data */
            memptr0 = ((unsigned char*)common->memory)+(memoffset * common->typesize);
            slpptr0 = ((unsigned char*)chunkdata)+(slpoffset * common->typesize);
#ifdef WDEBUG
fprintf(stderr,"xx.slp: odom: %s ptr=%d\n",
nczprint_odom(slpodom),(int)(slpptr0-(unsigned char*)chunkdata));
fflush(stderr);
fprintf(stderr,"xx.mem: odom: %s ptr=%d\n",
nczprint_odom(memodom),(int)(memptr0-(unsigned char*)common->memory));
fflush(stderr);
if(common->reading) {
fprintf(stderr,"\t%d->",*((int*)slpptr0));
fprintf(stderr,"%d\n",*((int*)memptr0));
} else {/* writing */
fprintf(stderr,"\t%d->",*((int*)memptr0));
fprintf(stderr,"%d\n",*((int*)slpptr0));
}
fflush(stderr);
#endif

	    LOG((1,"%s: slpptr0=%p memptr0=%p slpoffset=%llu memoffset=%lld",__func__,slpptr0,memptr0,slpoffset,memoffset));
	    if(zutest && zutest->tests & UTEST_WALK)
		zutest->print(UTEST_WALK, common, chunkodom, slpodom, memodom);
	    if((stat = transfern(slpodom,memodom,common,slpptr0,memptr0))) goto done;
            nczodom_next(memodom);
        } else break; /* slpodom exhausted */
        nczodom_next(slpodom);
    }
done:
    return stat;    
}

static int
transfern(NCZOdometer* slpodom, NCZOdometer* memodom, const struct Common* common, unsigned char* slpptr0, unsigned char* memptr0)
{
    int stat = NC_NOERR;
    if(common->reading) {
        memcpy(memptr0,slpptr0,common->typesize);
        if(common->swap)
            NCZ_swapatomicdata(common->typesize,memptr0,common->typesize);
    } else { /*writing*/
        memcpy(slpptr0,memptr0,common->typesize);
        if(common->swap)
            NCZ_swapatomicdata(common->typesize,slpptr0,common->typesize);
    }
    return THROW(stat);
}

/* This function may not be necessary if code in zvar does it instead */
static int
NCZ_fillchunk(void* chunkdata, struct Common* common)
{
    int stat = NC_NOERR;    

    if(common->fillvalue == NULL) {
        memset(chunkdata,0,common->chunksize*common->typesize);
	goto done;
    }	

    if(common->cache->fillchunk == NULL) {
        /* Get fill chunk*/
        if((stat = NCZ_create_fill_chunk(common->cache->chunksize, common->typesize, common->fillvalue, &common->cache->fillchunk)))
	    goto done;
    }
    memcpy(chunkdata,common->cache->fillchunk,common->cache->chunksize);
done:
    return stat;
}

/* Break out this piece so we can use it for unit testing */
int
NCZ_projectslices(size64_t* dimlens,
                  size64_t* chunklens,
                  NCZSlice* slices,
                  struct Common* common, 
                  NCZOdometer** odomp)
{
    int stat = NC_NOERR;
    int r;
    NCZOdometer* odom = NULL;
    NCZSliceProjections* allprojections = NULL;
    NCZChunkRange ranges[NC_MAX_VAR_DIMS];
    size64_t start[NC_MAX_VAR_DIMS];
    size64_t stop[NC_MAX_VAR_DIMS];
    size64_t stride[NC_MAX_VAR_DIMS];
    size64_t len[NC_MAX_VAR_DIMS];

    if(!initialized) ncz_chunking_init();

    if((allprojections = calloc(common->rank,sizeof(NCZSliceProjections))) == NULL)
        {stat = NC_ENOMEM; goto done;}
    memset(ranges,0,sizeof(ranges));

    /* Package common arguments */
    common->dimlens = dimlens;
    common->chunklens = chunklens;
    /* Compute the chunk ranges for each chunk in a given dim */
    if((stat = NCZ_compute_chunk_ranges(common->rank,slices,common->chunklens,ranges)))
        goto done;

    /* Compute the slice index vector */
    if((stat=NCZ_compute_all_slice_projections(common->rank,slices,common->dimlens,common->chunklens,ranges,allprojections)))
        goto done;

    /* Verify */
    for(r=0;r<common->rank;r++) {
        assert(rangecount(ranges[r]) == allprojections[r].count);
    }

    /* Compute the shape vector */
    for(r=0;r<common->rank;r++) {
        int j;
        size64_t iocount = 0;
        NCZProjection* projections = allprojections[r].projections;
        for(j=0;j<allprojections[r].count;j++) {
            NCZProjection* proj = &projections[j];
            iocount += proj->iocount;
        }
        common->shape[r] = iocount;
    }
    common->allprojections = allprojections;
    allprojections = NULL;

    /* Create an odometer to walk all the range combinations */
    for(r=0;r<common->rank;r++) {
        start[r] = ranges[r].start; 
        stop[r] = ranges[r].stop;
        stride[r] = 1;
        len[r] = ceildiv(common->dimlens[r],common->chunklens[r]);
    }   

    if((odom = nczodom_new(common->rank,start,stop,stride,len)) == NULL)
        {stat = NC_ENOMEM; goto done;}
    if(odomp) *odomp = odom;

done:
    return stat;
}

/***************************************************/
/* Utilities */

static int
rangecount(NCZChunkRange range)
{
    return (range.stop - range.start);
}

/* Goal: Given a set of per-dimension indices,
     compute the corresponding linear position.
*/
size64_t
NCZ_computelinearoffset(size_t R, const size64_t* indices, const size64_t* dimlens)
{
      size64_t offset;
      int i;

      offset = 0;
      for(i=0;i<R;i++) {
          offset *= dimlens[i];
          offset += indices[i];
      } 
      return offset;
}

#if 0
/* Goal: Given a linear position
     compute the corresponding set of R indices
*/
void
NCZ_offset2indices(size_t R, size64_t offset, const size64_t* dimlens, size64_t* indices)
{
      int i;

      for(i=0;i<R;i++) {
          indices[i] = offset % dimlens[i];
          offset = offset / dimlens[i];
      } 
}
#endif

/**************************************************/
/* Unit test entry points */

int
NCZ_chunkindexodom(int rank, const NCZChunkRange* ranges, size64_t* chunkcounts, NCZOdometer** odomp)
{
    int stat = NC_NOERR;
    int r;
    NCZOdometer* odom = NULL;
    size64_t start[NC_MAX_VAR_DIMS];
    size64_t stop[NC_MAX_VAR_DIMS];
    size64_t stride[NC_MAX_VAR_DIMS];
    size64_t len[NC_MAX_VAR_DIMS];

    for(r=0;r<rank;r++) {
        start[r] = ranges[r].start; 
        stop[r] = ranges[r].stop;
        stride[r] = 1;
        len[r] = chunkcounts[r];
    }   

    if((odom = nczodom_new(rank, start, stop, stride, len))==NULL)
        {stat = NC_ENOMEM; goto done;}

    if(odomp) {*odomp = odom; odom = NULL;}

done:
    nczodom_free(odom);
    return stat;
}

static int
readfromcache(void* source, size64_t* chunkindices, void** chunkdatap)
{
    return NCZ_read_cache_chunk((struct NCZChunkCache*)source, chunkindices, chunkdatap);
}

void
NCZ_clearcommon(struct Common* common)
{
    NCZ_clearsliceprojections(common->rank,common->allprojections);
    nullfree(common->allprojections);
    nullfree(common->fillvalue);
}
