/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "zincludes.h"
#include "zplugins.h"
#include "netcdf_filter_build.h"

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
    NC_UNUSED(file);
    NC_UNUSED(fcn);
    NC_UNUSED(line);
    if(err == 0) return err;
    ncbacktrace();
    return zbreakpoint(err);
}

int
zreport(int err, const char* msg, const char* file, const char* fcn, int line)
{
    if(err == 0) return err;
    ZLOG(NCLOGWARN,"!!! zreport: err=%d msg=%s @ %s#%s:%d",err,msg,file,fcn,line);
    ncbacktrace();
    return zbreakpoint(err);
}

#endif /*ZCATCH*/

/**************************************************/
/* Data Structure printers */

static NClist* reclaim = NULL;
static const size_t maxreclaim = 16;

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

    snprintf(value,sizeof(value),"Odometer{rank=%zu ",odom->rank);
    ncbytescat(buf,value);

    ncbytescat(buf," start=");
    txt = nczprint_vector((size_t)odom->rank,odom->start);
    ncbytescat(buf,txt);
    ncbytescat(buf," stop=");
    txt = nczprint_vector((size_t)odom->rank,odom->stop);
    ncbytescat(buf,txt);
    ncbytescat(buf," len=");
    txt = nczprint_vector((size_t)odom->rank,odom->len);
    ncbytescat(buf,txt);
    ncbytescat(buf," stride=");
    txt = nczprint_vector((size_t)odom->rank,odom->stride);
    ncbytescat(buf,txt);
    ncbytescat(buf," index=");
    txt = nczprint_vector((size_t)odom->rank,odom->index);
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
    size_t i;

    snprintf(tmp,sizeof(tmp),"SliceProjection{r=%zu range=%s count=%ld",
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
    for(i=0;i<len;i++) v[i] = (size64_t)ids[i];    
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
    size_t i;
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

char*
nczprint_envlist(const NClist* l)
{
    char* result = NULL;
    size_t i;
    NCbytes* buf = ncbytesnew();

    ncbytescat(buf,"(");
    if(l) {
        for(i=0;i<nclistlength(l);i++) {
	    const char* e = (const char*)nclistget(l,i);
	    if(i > 0) ncbytescat(buf,",");
	    ncbytescat(buf,"'");
	    ncbytescat(buf,e);
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
    size_t r;
    fprintf(stderr,"Common:\n");
#if 0
    fprintf(stderr,"\tfile: %s\n",c->file->controller->path);
    fprintf(stderr,"\tvar: %s\n",c->var->hdr.name);
    fprintf(stderr,"\treading=%d\n",c->reading);
#endif
    fprintf(stderr,"\trank=%zd",c->rank);
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
        fprintf(stderr,"\t\t[%zd] %s\n",r,nczprint_sliceprojectionsx(c->allprojections[r],RAW));
    fflush(stderr);
}

/**************************************************/
/* Filter/Plugin debug */

#if defined(ZDEBUGF) || defined(ZTRACING)
const char*
printfilter(const NCZ_Filter* f)
{
    static char pfbuf[4096];

    if(f == NULL) return "NULL";
    snprintf(pfbuf,sizeof(pfbuf),"{flags=%d hdf5=%s codec=%s plugin=%p}",
		f->flags, printhdf5(&f->hdf5),printcodec(&f->codec),f->plugin);
    return pfbuf;
}

const char*
printplugin(const NCZ_Plugin* plugin)
{
    static char plbuf[4096];
    char plbuf2[2000];
    char plbuf1[2000];

    if(plugin == NULL) return "plugin=NULL";
    plbuf2[0] = '\0'; plbuf1[0] = '\0';
    if(plugin->hdf5.filter)
        snprintf(plbuf1,sizeof(plbuf1),"hdf5={id=%u name=%s}",plugin->hdf5.filter->id,plugin->hdf5.filter->name);
    if(plugin->codec.codec)
        snprintf(plbuf2,sizeof(plbuf2),"codec={codecid=%s hdf5id=%u}",plugin->codec.codec->codecid,plugin->codec.codec->hdf5id);
    snprintf(plbuf,4096,"plugin={%sid=%d %s %s}",plugin->incomplete?"?":"",plugin->hdf5id,plbuf1,plbuf2);
    return plbuf;
}

const char*
printcodec(const NCZ_Codec* c)
{
    static char pcbuf[4096];
    snprintf(pcbuf,sizeof(pcbuf),"{id=%s codec=%s}",c->id,(c->codec==NULL?"null":c->codec));
    return pcbuf;
}

const char*
printhdf5(const NCZ_HDF5* h)
{
    static char phbuf[4096];
    snprintf(phbuf,sizeof(phbuf),"{id=%u visible=%s working=%s}",
    		h->id, printnczparams(&h->visible), printnczparams(&h->working));
    return phbuf;
}

const char*
printhdf5class(const H5Z_class2_t* hdf5)
{
    static char buf[4096];
    snprintf(buf,sizeof(buf),"hdf5_t{ver=%d,id=%d,name=%s}",
    		hdf5->version,hdf5->id,hdf5->name);
    return buf;
}

const char*
printcodecclass(const NCZ_codec_t* codec)
{
    static char buf[4096];
    snprintf(buf,sizeof(buf),"codec_t{ver=%d,sort=%d,codecid=%s,hdf5id=%d}",
    		codec->version,codec->sort,codec->codecid,codec->hdf5id);
    return buf;
}

const char*
printcodecapi(const struct CodecAPI* ca)
{
    static char pcbuf[4096];
    snprintf(pcbuf,sizeof(pcbuf),"CodecAPI{defaulted=%d,ishdf5raw=%d,codec=%s}",ca->defaulted,ca->ishdf5raw,printcodecclass(ca->codec));
    return pcbuf;
}

const char*
printloadedplugins(void)
{
    static char pcbuf[4096*4];
    struct NCglobalstate* gs = NC_getglobalstate();
    NClist* pluginlist = gs->zarr.loaded_plugins;
    size_t i,nplugins;
    const NCZ_Plugin** plugins;
    NCbytes* buf = ncbytesnew();

    ncbytessetcontents(buf,pcbuf,sizeof(pcbuf));
    ncbytessetlength(buf,0);
    nplugins = nclistlength(pluginlist);
    plugins = (const NCZ_Plugin**)nclistcontents(pluginlist);
    for(i=0;i<nplugins;i++) {
	const char* tmp;
	tmp = printplugin(plugins[i]);
        ncbytescat(buf,tmp);
	ncbytescat(buf,"\n");
    }
    (void)ncbytescontents(buf);
    ncbytesfree(buf);
    return pcbuf;
}

const char*
printparams(size_t nparams, const unsigned* params)
{
    static char ppbuf[4096];
    if(nparams == 0)
        snprintf(ppbuf,4096,"{0,%p}",params);
    else 
        snprintf(ppbuf,4096,"{%u %s}",(unsigned)nparams,nczprint_paramvector(nparams,params));
    return ppbuf;
}

const char*
printnczparams(const NCZ_Params* p)
{
    return printparams(p->nparams,p->params);
}

/* Suppress selected unused static functions */
static void
debugf_unused(void)
{
    void* p = NULL;
    (void)p;
    p = debugf_unused;
(void)printfilter;
(void)printplugin;
(void)printcodec;
(void)printhdf5;
(void)printparams;
(void)printnczparams;
(void)printhdf5class;
(void)printcodecclass;
}
#endif /*ZDEBUGF*/
