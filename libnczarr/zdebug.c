/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "zincludes.h"

#ifdef ZCATCH
/* Place breakpoint here to catch errors close to where they occur*/
int
zbreakpoint(int err)
{
    return err;
}

int
zthrow(int err, const char* file, int line)
{
    if(err == 0) return err;
#ifdef ZDEBUG
    fprintf(stderr,"THROW: %s/%d: (%d) %s\n",file,line,err,nc_strerror(err));
    fflush(stderr);
#endif
    return zbreakpoint(err);
}
#endif

/**************************************************/
/* Data Structure printers */

char*
nczprint_slice(NCZSlice slice)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char value[64];

    ncbytescat(buf,"Slice{");
    snprintf(value,sizeof(value),"%lu",(unsigned long)slice.start);
    ncbytescat(buf,value);
    ncbytescat(buf,":");
    snprintf(value,sizeof(value),"%lu",(unsigned long)slice.stop);
    ncbytescat(buf,value);
    if(slice.stride != 1) {
        ncbytescat(buf,":");
        snprintf(value,sizeof(value),"%lu",(unsigned long)slice.stride);
        ncbytescat(buf,value);
    }
    ncbytescat(buf,"}");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return result;
}

char*
nczprint_odom(NCZOdometer odom)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char value[128];
    int r;

    snprintf(value,sizeof(value),"Odometer{R=%lu,",(unsigned long)odom.R);
    ncbytescat(buf,value);

    ncbytescat(buf,"start=[");
    for(r=0;r<odom.R;r++) {
	if(r > 0) ncbytescat(buf,",");
        snprintf(value,sizeof(value),"%lu",(unsigned long)odom.start[r]);
        ncbytescat(buf,value);
    }
    ncbytescat(buf,"]");
    ncbytescat(buf," stop=[");
    for(r=0;r<odom.R;r++) {
	if(r > 0) ncbytescat(buf,",");
        snprintf(value,sizeof(value),"%lu",(unsigned long)odom.stop[r]);
        ncbytescat(buf,value);
    }
    ncbytescat(buf,"]");
    ncbytescat(buf," stride=[");
    for(r=0;r<odom.R;r++) {
	if(r > 0) ncbytescat(buf,",");
        snprintf(value,sizeof(value),"%lu",(unsigned long)odom.stride[r]);
        ncbytescat(buf,value);
    }
    ncbytescat(buf,"]");

    ncbytescat(buf," index=[");
    for(r=0;r<odom.R;r++) {
	if(r > 0) ncbytescat(buf,",");
        snprintf(value,sizeof(value),"%lu",(unsigned long)odom.index[r]);
        ncbytescat(buf,value);
    }
    ncbytescat(buf,"]");

    ncbytescat(buf,"}");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return result;
}

char*
nczprint_projection(NCZProjection proj)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char value[128];

    ncbytescat(buf,"Projection{");
    snprintf(value,sizeof(value),"chunkindex=%lu",(unsigned long)proj.chunkindex);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",offset=%lu",(unsigned long)proj.offset);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",limit=%lu",(unsigned long)proj.limit);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",last=%lu",(unsigned long)proj.last);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",len=%lu",(unsigned long)proj.len);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",outpos=%lu",(unsigned long)proj.outpos);
    ncbytescat(buf,value);
    ncbytescat(buf,",slices={");
    result = nczprint_slice(proj.slice);
    ncbytescat(buf,result);
    result = NULL;
    ncbytescat(buf,"}");
    ncbytescat(buf,"}");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return result;
}

char*
nczprint_sliceindex(NCZSliceIndex si)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char value[128];
    int i;

    ncbytescat(buf,"SliceIndex{");
    snprintf(value,sizeof(value),"chunk0=%lu",(unsigned long)si.chunk0);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",nchunks=%lu",(unsigned long)si.nchunks);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",count=%lu",(unsigned long)si.count);
    ncbytescat(buf,value);
    ncbytescat(buf,",projections=[");
    for(i=0;i<si.nchunks;i++) {
        result = nczprint_projection(si.projections[i]);
        ncbytescat(buf,result);
    }
    result = NULL;
    ncbytescat(buf,"]");
    ncbytescat(buf,"}");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return result;
}

