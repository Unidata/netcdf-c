/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef ZODOM_H
#define ZODOM_H

struct NCZSlice;

typedef struct NCZOdometer {
    int rank; /*rank */
    size64_t* start;
    size64_t* stop; /* start + (count*stride) */
    size64_t* stride;
    size64_t* max; /* full dimension length */
    size64_t* index; /* current value of the odometer*/
} NCZOdometer;

/**************************************************/
/* From zodom.c */
extern NCZOdometer* nczodom_new(int rank, const size64_t*, const size64_t*, const size64_t*, const size64_t*);
extern NCZOdometer* nczodom_fromslices(int rank, const struct NCZSlice* slices);
extern int nczodom_more(NCZOdometer*);
extern void nczodom_next(NCZOdometer*);
extern size64_t* nczodom_indices(NCZOdometer*);
extern size64_t nczodom_offset(NCZOdometer*);
extern void nczodom_reset(NCZOdometer* odom);
extern void nczodom_free(NCZOdometer*);
extern size64_t nczodom_avail(NCZOdometer*);
extern void nczodom_incr(NCZOdometer*,size64_t);

#endif /*ZODOM_H*/
