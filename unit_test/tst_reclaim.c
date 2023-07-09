/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the various type walker functions:
nc_reclaim_data => NC_reclaim_data
nc_reclaim_data_all => NC_reclaim_data_all => NC_reclaim_data
nc_copy_data => NC_copy_data
nc_copy_data_all => NC_copy_data_all => NC_copy_data
nc_dump_data => NC_dump_data

See the file "reclaim_tests.cdl" to see the input file semantics.
*/


#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "netcdf.h"
#include "netcdf_aux.h"

#define NCCATCH
#include "nclog.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#define DEBUG

#define FILE "reclaim_tests.nc"

#define MAXOBJECTS 1024

struct Options {
    int debug;
} dumpoptions;

struct Type {
    char name[NC_MAX_NAME];
    int tid;
    size_t size;
};

struct Dim {
    char name[NC_MAX_NAME];
    int did;
    size_t size;
};

struct Var {
    char name[NC_MAX_NAME];
    int vid;
    int tid;
    size_t dimprod;
    void* data;
};

struct Metadata {
    int ncid;
    /* type ids */
    struct Type vint_t;
    struct Type vstr_t;
    struct Type vvint_t;
    struct Type vvstr_t;
    struct Type cmpd_atomic_t;
    struct Type cmpd_str_t;
    struct Type cmpd_vlen_t;
    struct Type cmpd_cmpd_t;
    struct Type vcmpd_atomic_t;
    struct Type vcmpd_str_t;
    struct Type vcmpd_vlen_t;
    struct Type vcmpd_cmpd_t;
    /* dim ids */
    struct Dim d1;
    struct Dim d2;
    struct Dim d4;
    /* var ids */    
    struct Var intvar;
    struct Var strvar;
    struct Var vintvar;
    struct Var vstrvar;
    struct Var vvintvar;
    struct Var vvstrvar;
    struct Var catomvar;
    struct Var cstrvar;
    struct Var cvlenvar;
    struct Var ccmpdvar;
    struct Var vcatomvar;
    struct Var vcstrvar;
    struct Var vcvlenvar;
    struct Var vccmpdvar;
} metadata;

/* atomic Types */
static struct Type atomics[NC_MAX_ATOMIC_TYPE+1];

/* Track the metadata objects */
static struct Type* typemap[MAXOBJECTS];
static struct Dim*  dimmap[MAXOBJECTS];
static struct Var*  varmap[MAXOBJECTS];


#define CHECK(code) do {stat = check(code,__func__,__LINE__); if(stat) {goto done;}} while(0)

static int
check(int code, const char* fcn, int line)
{
    if(code == NC_NOERR) return code;
    fprintf(stderr,"***fail: (%d) %s @ %s:%d\n",code,nc_strerror(code),fcn,line);
#ifdef debug
    abort();
#endif
    exit(1);
}

static int
initialize(void)
{
    int i,stat = NC_NOERR;
    memset(atomics,0,sizeof(atomics));
    memset(typemap,0,sizeof(typemap));
    memset(dimmap,0,sizeof(dimmap));
    memset(varmap,0,sizeof(varmap));
    for(i=1;i<=NC_MAX_ATOMIC_TYPE;i++) {
	struct Type* t = &atomics[i];
	t->tid = i;
        if((stat=ncaux_inq_any_type(0,i,t->name,&t->size,NULL,NULL,NULL))) return NCTHROW(stat);
	typemap[i] = t;
    }
    return NCTHROW(stat);
}

static void
cleanup(void)
{
    int i;
    for(i=0;i<MAXOBJECTS;i++) {
        if(varmap[i] == NULL) break;
	if(varmap[i]->data != NULL) free(varmap[i]->data);
    }
}


static int
setuptype(int ncid, const char* name, struct Type* t)
{
    int stat = NC_NOERR;
    strcpy(t->name,name);
    if((stat=nc_inq_typeid(ncid,name,&t->tid))) return NCTHROW(stat);
    if((stat=ncaux_inq_any_type(ncid,t->tid,NULL,&t->size,NULL,NULL,NULL))) return NCTHROW(stat);
    typemap[t->tid] = t;
    return NCTHROW(stat);
}

static int
setupdim(int ncid, const char* name, struct Dim* dim)
{
    int stat = NC_NOERR;
    strcpy(dim->name,name);
    if((stat = nc_inq_dimid(ncid,name,&dim->did))) return NCTHROW(stat);
    if((nc_inq_dimlen(ncid,dim->did,&dim->size))) return NCTHROW(stat);
    dimmap[dim->did] = dim;
    return NCTHROW(stat);
}

static int
setupvar(int ncid, const char* name, struct Var* var)
{
    int stat = NC_NOERR;
    int i,ndims;
    int dimids[NC_MAX_VAR_DIMS];
    size_t product,dimsizes[NC_MAX_VAR_DIMS];

    strcpy(var->name,name);
    if((stat=nc_inq_varid(ncid,name,&var->vid))) return NCTHROW(stat);
    if((stat=nc_inq_vartype(ncid,var->vid,&var->tid))) return NCTHROW(stat);
    if((stat=nc_inq_varndims(ncid,var->vid,&ndims))) return NCTHROW(stat);
    if((stat=nc_inq_vardimid(ncid,var->vid,dimids))) return NCTHROW(stat);
    for(product=1,i=0;i<ndims;i++) {
        if((stat=nc_inq_dimlen(ncid,dimids[i],&dimsizes[i]))) return NCTHROW(stat);
	product *= dimsizes[i];
    }
    var->dimprod = product;
    varmap[var->vid] = var;
    return NCTHROW(stat);
}

static void
setup(struct Metadata* md)
{
    int stat = NC_NOERR;

    CHECK(nc_open(FILE,NC_NETCDF4,&md->ncid));

    CHECK(setupdim(md->ncid,"d1",&md->d1));
    CHECK(setupdim(md->ncid,"d2",&md->d2));
    CHECK(setupdim(md->ncid,"d4",&md->d4));

    CHECK(setuptype(md->ncid,"vint_t",&md->vint_t));
    CHECK(setuptype(md->ncid,"vstr_t",&md->vstr_t));
    CHECK(setuptype(md->ncid,"vvint_t",&md->vvint_t));
    CHECK(setuptype(md->ncid,"vvstr_t",&md->vvstr_t));
    CHECK(setuptype(md->ncid,"cmpd_atomic_t",&md->cmpd_atomic_t));
    CHECK(setuptype(md->ncid,"cmpd_str_t",&md->cmpd_str_t));
    CHECK(setuptype(md->ncid,"cmpd_vlen_t",&md->cmpd_vlen_t));
    CHECK(setuptype(md->ncid,"cmpd_cmpd_t",&md->cmpd_cmpd_t));
    CHECK(setuptype(md->ncid,"vcmpd_atomic_t",&md->vcmpd_atomic_t));
    CHECK(setuptype(md->ncid,"vcmpd_str_t",&md->vcmpd_str_t));
    CHECK(setuptype(md->ncid,"vcmpd_vlen_t",&md->vcmpd_vlen_t));
    CHECK(setuptype(md->ncid,"vcmpd_cmpd_t",&md->vcmpd_cmpd_t));

    CHECK(setupvar(md->ncid,"intvar",&md->intvar));
    CHECK(setupvar(md->ncid,"strvar",&md->strvar));
    CHECK(setupvar(md->ncid,"vintvar",&md->vintvar));
    CHECK(setupvar(md->ncid,"vstrvar",&md->vstrvar));
    CHECK(setupvar(md->ncid,"vvintvar",&md->vvintvar));
    CHECK(setupvar(md->ncid,"vvstrvar",&md->vvstrvar));
    CHECK(setupvar(md->ncid,"catomvar",&md->catomvar));
    CHECK(setupvar(md->ncid,"cstrvar",&md->cstrvar));
    CHECK(setupvar(md->ncid,"cvlenvar",&md->cvlenvar));
    CHECK(setupvar(md->ncid,"ccmpdvar",&md->ccmpdvar));
    CHECK(setupvar(md->ncid,"vcatomvar",&md->vcatomvar));
    CHECK(setupvar(md->ncid,"vcstrvar",&md->vcstrvar));
    CHECK(setupvar(md->ncid,"vcvlenvar",&md->vcvlenvar));
    CHECK(setupvar(md->ncid,"vccmpdvar",&md->vccmpdvar));

done:
    assert(stat == NC_NOERR);
}

static int
readvar(int ncid, struct Var* v)
{
    int stat = NC_NOERR;
    size_t size;
    struct Type* basetype = typemap[v->tid];
    size = (basetype->size * v->dimprod);
    v->data = calloc(1,size);
    if((stat=nc_get_var(ncid,v->vid,v->data)))
        return NCTHROW(stat);
    return NCTHROW(stat);
}

static int
dumpvar(int ncid, struct Var* v, void* data, char** bufp)
{
    int stat = NC_NOERR;
    if((stat=ncaux_dump_data(ncid,v->tid,data,v->dimprod,bufp))) return NCTHROW(stat);
    return NCTHROW(stat);
}

static int
testvar(int ncid, struct Var* v)
{
    int stat = NC_NOERR;
    char* buforig = NULL;
    char* bufcopy = NULL;
    void* copy = NULL;

    
    if((stat=readvar(ncid,v))) return NCTHROW(stat);
    if((stat=dumpvar(ncid,v,v->data,&buforig))) return NCTHROW(stat);    
    printf("(o) %s: %s\n",v->name,buforig);
    // Test copying
    if((stat=nc_copy_data_all(ncid,v->tid,v->data,v->dimprod,&copy))) return NCTHROW(stat);
    // Print copy
    if((stat=dumpvar(ncid,v,copy,&bufcopy))) return NCTHROW(stat);    
    printf("(c) %s: %s\n",v->name,bufcopy);
    /* Compare */
    if(strcmp(buforig,bufcopy) != 0)
        fprintf(stderr,"*** orig != copy\n");
    if(buforig) free(buforig);
    if(bufcopy) free(bufcopy);

    // reclaim original    
    if((stat=nc_reclaim_data_all(ncid,v->tid,v->data,v->dimprod))) return NCTHROW(stat);
    // reclaim copy
    if((stat=nc_reclaim_data_all(ncid,v->tid,copy,v->dimprod))) return NCTHROW(stat);
    v->data = NULL;
    return NCTHROW(stat);     
}

static void
test(struct Metadata* md)
{
    int stat = NC_NOERR;
    CHECK(testvar(md->ncid,&md->intvar));
    CHECK(testvar(md->ncid,&md->strvar));
    CHECK(testvar(md->ncid,&md->vintvar));
    CHECK(testvar(md->ncid,&md->vstrvar));
    CHECK(testvar(md->ncid,&md->vvintvar));
    CHECK(testvar(md->ncid,&md->vvstrvar));
    CHECK(testvar(md->ncid,&md->catomvar));
    CHECK(testvar(md->ncid,&md->cstrvar));
    CHECK(testvar(md->ncid,&md->cvlenvar));
    CHECK(testvar(md->ncid,&md->ccmpdvar));
    CHECK(testvar(md->ncid,&md->vcatomvar));
    CHECK(testvar(md->ncid,&md->vcstrvar));
    CHECK(testvar(md->ncid,&md->vcvlenvar));
    CHECK(testvar(md->ncid,&md->vccmpdvar));    
done:
    return;
}

int
main(int argc, char** argv)
{
    int c,stat = NC_NOERR;

    CHECK(nc_initialize());
    CHECK(initialize());

    /* Init options */
    memset((void*)&dumpoptions,0,sizeof(dumpoptions));

    while ((c = getopt(argc, argv, "dhk:tu:")) != EOF) {
        switch(c) {
        case 'd':
            dumpoptions.debug = 1;
            break;
        case '?':
           fprintf(stderr,"unknown option\n");
           stat = NC_EINVAL;
           goto done;
        }
    }
 
    setup(&metadata);
    test(&metadata);

done:
    cleanup();
    if(metadata.ncid) nc_close(metadata.ncid);
    (void)nc_finalize();
    exit(stat?1:0);
}
