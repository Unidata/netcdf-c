/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "zincludes.h"

NCZOdometer*
nczodom_new(size_t R, const size_t* start, const size_t* stop, const size_t* stride)
{
    NCZOdometer* odom = NULL;
    if((odom = calloc(1,sizeof(NCZOdometer))) == NULL)
	goto done;   
    odom->R = R;
    memcpy(odom->start,start,sizeof(size_t)*R);
    memcpy(odom->stop,stop,sizeof(size_t)*R);
    memcpy(odom->stride,stride,sizeof(size_t)*R);
    memset(odom->index,0,sizeof(size_t)*R);
done:
    return odom;
}

NCZOdometer*
nczodom_fromslices(size_t rank, const NCZSlice* slices)
{
    size_t i;
    NCZOdometer* odom = NULL;

    if((odom = calloc(1,sizeof(NCZOdometer))) == NULL)
	goto done;   
    odom->R = rank;
    for(i=0;i<rank;i++) {    
	odom->start[i] = slices[i].start;
	odom->stop[i] = slices[i].stop;
	odom->stride[i] = slices[i].stride;
    }
    memset(odom->index,0,sizeof(size_t)*rank);
done:
    return odom;
}
  
int
nczodom_more(NCZOdometer* odom)
{
    return (odom->index[0] < odom->stop[0]);
}
  
int
nczodom_next(NCZOdometer* odom)
{
    size_t i;
    for(i=odom->R-1;i>=0;i--) {
	odom->index[i] += odom->stride[i];
        if(odom->index[i] < odom->stop[i]) break;
        if(i == 0) return 0; /* leave the 0th entry if it overflows */
        odom->index[i] = odom->start[i]; /* reset this position */
    }
    return 1;
}
  
/* Get the value of the odometer */
size_t*
nczodom_indices(NCZOdometer* odom)
{
    return odom->index;
}
