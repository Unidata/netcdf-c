/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "zincludes.h"

/* Mnemonic */
#define RAW 1

/**************************************************/
/* Data Structure  */

struct ZUTEST* zutest = NULL;

/**************************************************/

#ifdef ZCATCH
/* Place breakpoint here to catch errors close to where they occur*/
int
zbreakpoint(int err)
{
    return ncbreakpoint(err);
}

int
zthrow(int err, const char* file, const char* fcn, int line)
{
    if(err == 0) return err;
    ncbacktrace();
    return zbreakpoint(err);
}

int
zreport(int err, const char* msg, const char* file, const char* fcn, int line)
{
    if(err == 0) return err;
    ZLOG(NCLOGWARN,"!!! zreport: err=%d msg=%s",err,msg);
    ncbacktrace();
    return zbreakpoint(err);
}

#endif /*ZCATCH*/

/**************************************************/
/* Data Structure printers */

static NClist* reclaim = NULL;
static const int maxreclaim = 16;

static char*
capture(char* s)
{
    if(s != NULL) {
	while(nclistlength(reclaim) >= maxreclaim)
	    free(nclistremove(reclaim,0));
        if(reclaim == NULL) reclaim = nclistnew();
        nclistpush(reclaim,s);
    }
    return s;
}

void
nczprint_reclaim(void)
{
    if(reclaim != NULL) {
        nclistfreeall(reclaim);
	reclaim = NULL;
    }
}

char*
nczprint_slice(const NCZSlice slice)
{
    return nczprint_slicex(slice,!RAW);
}

char*
nczprint_slicex(const NCZSlice slice, int raw)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char value[64];

    if(raw)
        ncbytescat(buf,"[");
    else
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
    ncbytescat(buf,"|");
    snprintf(value,sizeof(value),"%lu",(unsigned long)slice.len);
    ncbytescat(buf,value);
    if(raw)
        ncbytescat(buf,"]");
    else
        ncbytescat(buf,"}");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(result);
}

char*
nczprint_slices(int rank, const NCZSlice* slices)
{
    return nczprint_slicesx(rank, slices, !RAW);
}

char*
nczprint_slicesx(int rank, const NCZSlice* slices, int raw)
{
    int i;
    char* result = NULL;
    NCbytes* buf = ncbytesnew();

    for(i=0;i<rank;i++) {
	char* ssl;
	if(!raw)
            ncbytescat(buf,"[");
	ssl = nczprint_slicex(slices[i],raw);
	ncbytescat(buf,ssl);
	if(!raw)
	    ncbytescat(buf,"]");
    }
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(result);
}

char*
nczprint_slab(int rank, const NCZSlice* slices)
{
    return nczprint_slicesx(rank,slices,RAW);
}

char*
nczprint_odom(const NCZOdometer* odom)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char value[128];
    char* txt = NULL;

    snprintf(value,sizeof(value),"Odometer{rank=%d ",odom->rank);
    ncbytescat(buf,value);

    ncbytescat(buf," start=");
    txt = nczprint_vector(odom->rank,odom->start);
    ncbytescat(buf,txt);
    ncbytescat(buf," stop=");
    txt = nczprint_vector(odom->rank,odom->stop);
    ncbytescat(buf,txt);
    ncbytescat(buf," len=");
    txt = nczprint_vector(odom->rank,odom->len);
    ncbytescat(buf,txt);
    ncbytescat(buf," stride=");
    txt = nczprint_vector(odom->rank,odom->stride);
    ncbytescat(buf,txt);
    ncbytescat(buf," index=");
    txt = nczprint_vector(odom->rank,odom->index);
    ncbytescat(buf,txt);
    ncbytescat(buf," offset=");
    snprintf(value,sizeof(value),"%llu",nczodom_offset(odom));
    ncbytescat(buf,value);
    ncbytescat(buf," avail=");
    snprintf(value,sizeof(value),"%llu",nczodom_avail(odom));
    ncbytescat(buf,value);
    ncbytescat(buf," more=");
    snprintf(value,sizeof(value),"%d",nczodom_more(odom));
    ncbytescat(buf,value);
    
    ncbytescat(buf,"}");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(result);
}

char*
nczprint_projection(const NCZProjection proj)
{
   return nczprint_projectionx(proj,!RAW);
}

char*
nczprint_projectionx(const NCZProjection proj, int raw)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char value[128];

    ncbytescat(buf,"Projection{");
    snprintf(value,sizeof(value),"id=%d,",proj.id);
    ncbytescat(buf,value);
    if(proj.skip) ncbytescat(buf,"*");
    snprintf(value,sizeof(value),"chunkindex=%lu",(unsigned long)proj.chunkindex);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",first=%lu",(unsigned long)proj.first);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",last=%lu",(unsigned long)proj.last);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",limit=%lu",(unsigned long)proj.limit);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",iopos=%lu",(unsigned long)proj.iopos);
    ncbytescat(buf,value);
    snprintf(value,sizeof(value),",iocount=%lu",(unsigned long)proj.iocount);
    ncbytescat(buf,value);
    ncbytescat(buf,",chunkslice=");
    result = nczprint_slicex(proj.chunkslice,raw);
    ncbytescat(buf,result);
    ncbytescat(buf,",memslice=");
    result = nczprint_slicex(proj.memslice,raw);
    ncbytescat(buf,result);
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(result);
}

char*
nczprint_allsliceprojections(int r, const NCZSliceProjections* slp)
{
    int i;
    char* s;    
    NCbytes* buf = ncbytesnew();
    for(i=0;i<r;i++) {
	s = nczprint_sliceprojections(slp[i]);
	ncbytescat(buf,s);
    } 	
    s = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(s);
}

char*
nczprint_sliceprojections(const NCZSliceProjections slp)
{
    return nczprint_sliceprojectionsx(slp,!RAW);
}

char*
nczprint_sliceprojectionsx(const NCZSliceProjections slp, int raw)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char tmp[4096];
    int i;

    snprintf(tmp,sizeof(tmp),"SliceProjection{r=%d range=%s count=%ld",
    		slp.r,nczprint_chunkrange(slp.range),(long)slp.count);
    ncbytescat(buf,tmp);
    ncbytescat(buf,",projections=[\n");
    for(i=0;i<slp.count;i++) {
	NCZProjection* p = (NCZProjection*)&slp.projections[i];
	ncbytescat(buf,"\t");
        result = nczprint_projectionx(*p,raw);
        ncbytescat(buf,result);
	ncbytescat(buf,"\n");
    }
    result = NULL;
    ncbytescat(buf,"]");
    ncbytescat(buf,"}\n");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(result);
}

char*
nczprint_chunkrange(const NCZChunkRange range)
{
    char* result = NULL;
    NCbytes* buf = ncbytesnew();
    char digits[64];

    ncbytescat(buf,"ChunkRange{start=");
    snprintf(digits,sizeof(digits),"%llu",range.start);
    ncbytescat(buf,digits);
    ncbytescat(buf," stop=");
    snprintf(digits,sizeof(digits),"%llu",range.stop);
    ncbytescat(buf,digits);
    ncbytescat(buf,"}");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(result);
}

char*
nczprint_idvector(size_t len, const int* ids)
{
    size64_t v[4096];
    size_t i;
    for(i=0;i<len;i++) v[i] = ids[i];    
    return nczprint_vector(len,v);
}

char*
nczprint_paramvector(size_t len, const unsigned* params)
{
    size64_t v[4096];
    size_t i;
    for(i=0;i<len;i++) v[i] = params[i];    
    return nczprint_vector(len,v);
}

char*
nczprint_sizevector(size_t len, const size_t* sizes)
{
    size64_t v[4096];
    size_t i;
    for(i=0;i<len;i++) v[i] = sizes[i];    
    return nczprint_vector(len,v);
}

char*
nczprint_vector(size_t len, const size64_t* vec)
{
    char* result = NULL;
    int i;
    char value[128];
    NCbytes* buf = ncbytesnew();

    ncbytescat(buf,"(");
    for(i=0;i<len;i++) {
        if(i > 0) ncbytescat(buf,",");
        snprintf(value,sizeof(value),"%lu",(unsigned long)vec[i]);	
	ncbytescat(buf,value);
    }
    ncbytescat(buf,")");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(result);
}

char*
nczprint_envv(const char** envv)
{
    char* result = NULL;
    int i;
    NCbytes* buf = ncbytesnew();
    const char** p;

    ncbytescat(buf,"(");
    if(envv) {
        for(i=0,p=envv;*p;p++,i++) {
        if(i > 0) ncbytescat(buf,",");
	    ncbytescat(buf,"'");
	    ncbytescat(buf,*p);
	    ncbytescat(buf,"'");
	}
    }
    ncbytescat(buf,")");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return capture(result);
}

void
zdumpcommon(const struct Common* c)
{
    int r;
    fprintf(stderr,"Common:\n");
#if 0
    fprintf(stderr,"\tfile: %s\n",c->file->controller->path);
    fprintf(stderr,"\tvar: %s\n",c->var->hdr.name);
    fprintf(stderr,"\treading=%d\n",c->reading);
#endif
    fprintf(stderr,"\trank=%d",c->rank);
    fprintf(stderr," dimlens=%s",nczprint_vector(c->rank,c->dimlens));
    fprintf(stderr," chunklens=%s",nczprint_vector(c->rank,c->chunklens));
#if 0
    fprintf(stderr,"\tmemory=%p\n",c->memory);
    fprintf(stderr,"\ttypesize=%d\n",c->typesize);
    fprintf(stderr,"\tswap=%d\n",c->swap);
#endif
    fprintf(stderr," shape=%s\n",nczprint_vector(c->rank,c->shape));
    fprintf(stderr,"\tallprojections:\n");
    for(r=0;r<c->rank;r++)
        fprintf(stderr,"\t\t[%d] %s\n",r,nczprint_sliceprojectionsx(c->allprojections[r],RAW));
    fflush(stderr);
}
