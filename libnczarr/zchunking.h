/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef NCZSLICEINDICES_H
#define NCZSLICEINDICES_H

typedef struct NCZSlice {
    size64_t start;
    size64_t stop;
    size64_t stride;
} NCZSlice;

typedef struct NCProjection {
    size64_t chunkindex; /* chunk index (not length) */
    size64_t offset; /* Distance from start of chunk0 to this chunk */
    size64_t limit;  /* last accessible position in the chunk */
    size64_t last;   /* position of last value touched */
    size64_t len;    /* Not last place touched, but the last place that could be touched */
    size64_t iopos;  /* start point in the data memory to access the data */
    NCZSlice slice; /* slice relative to this chunk */
} NCZProjection;

typedef struct NCZSliceIndex { /* taken from zarr code see SliceDimIndexer */
    size64_t chunk0;  /* index of the first chunk touched by the slice */
    size64_t nchunks; /* number of chunks touched by this slice index */
    size64_t count;   /*total number of io items defined by this slice index */
    NCZProjection* projections; /* Vector of projections
                                derived from the original slice:
                                one for each chunk touched by
                                the original slice;
                                |projections| == nchunks
                             */
} NCZSliceIndex;

typedef struct NCZOdometer {
  size64_t R; // rank
  size64_t start[NC_MAX_VAR_DIMS];
  size64_t stop[NC_MAX_VAR_DIMS];
  size64_t stride[NC_MAX_VAR_DIMS];
  size64_t index[NC_MAX_VAR_DIMS]; /* current value of the odometer*/
} NCZOdometer;

/* From odom.c */
extern NCZOdometer* nczodom_new(size64_t, const size64_t*, const size64_t*, const size64_t*);
extern NCZOdometer* nczodom_fromslices(size64_t rank, const NCZSlice* slices);
extern int nczodom_more(NCZOdometer*);
extern int nczodom_next(NCZOdometer*);
extern size64_t* nczodom_indices(NCZOdometer*);

/* From sliceindices.c */
extern int ncz_compute_all_slice_projections(size64_t R, const NCZSlice*, const size64_t*, const size64_t*, NCZSliceIndex**);

/* From allchunks.c */
extern int nczhunking_init(void);
extern int ncz_evaluateslices(NC_FILE_INFO_T*, NC_VAR_INFO_T*, NCZSlice*, void*, size64_t);

/* From zchunkio.c */
extern int ncz_buildchunkkey(int R, size64_t* chunkindices, char** keyp);

/* From zput.c */
extern int ncz_put_chunk(NC_FILE_INFO_T*,NC_VAR_INFO_T*,?);

#endif /*NCZSLICEINDICES_H*/
