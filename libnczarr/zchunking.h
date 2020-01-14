/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef NCZSLICEINDICES_H
#define NCZSLICEINDICES_H

typedef struct NCZSlice {
    size_t start;
    size_t stop;
    size_t stride;
} NCZSlice;

typedef struct NCProjection {
    size_t chunkindex; /* chunk index (not length) */
    size_t offset; /* Distance from start of chunk0 to this chunk */
    size_t limit;  /* last accessible position in the chunk */
    size_t last;   /* position of last value extracted */
    size_t len;    /* Not last place touched, but the last place that could be touched */
    size_t outpos;  /* start point in the output to store the extracted data */
    NCZSlice slice; /* slice relative to this chunk */
} NCZProjection;

typedef struct NCZSliceIndex { /* taken from zarr code see SliceDimIndexer */
    size_t chunk0;  /* index of the first chunk touched by the slice */
    size_t nchunks; /* number of chunks touched by this slice index */
    size_t count;   /*total number of output items defined by this slice index */
    NCZProjection* projections; /* Vector of projections
                                derived from the original slice:
                                one for each chunk touched by
                                the original slice;
                                |projections| == nchunks
                             */
} NCZSliceIndex;

typedef struct NCZOdometer {
  size_t R; // rank
  size_t start[NC_MAX_VAR_DIMS];
  size_t stop[NC_MAX_VAR_DIMS];
  size_t stride[NC_MAX_VAR_DIMS];
  size_t index[NC_MAX_VAR_DIMS]; /* current value of the odometer*/
} NCZOdometer;

/* From odom.c */
extern NCZOdometer* nczodom_new(size_t, const size_t*, const size_t*, const size_t*);
extern NCZOdometer* nczodom_fromslices(size_t rank, const NCZSlice* slices);
extern int nczodom_more(NCZOdometer*);
extern int nczodom_next(NCZOdometer*);
extern size_t* nczodom_indices(NCZOdometer*);

/* From sliceindices.c */
extern int ncz_compute_all_slice_projections(size_t R, const NCZSlice*, const size_t*, const size_t*, NCZSliceIndex**);

/* From allchunks.c */
extern int nczhunking_init(void);
extern int ncz_evaluateslices(NC_FILE_INFO_T*, NC_VAR_INFO_T*, NCZSlice*, void*, size_t);

/* From chunkio.c */
extern int ncz_buildchunkkey(int R, size_t* chunkindices, char** keyp);
/* Temporary */
extern int readchunk(NC_FILE_INFO_T*,NC_VAR_INFO_T*,size_t*,void**);

#endif /*NCZSLICEINDICES_H*/
