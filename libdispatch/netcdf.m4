/*
 Copyright 2010 University Corporation for Atmospheric
 Research/Unidata. See COPYRIGHT file for more info.

 This file defines most of the netcdf API in terms of the dispatch
 functions along with a few functions that are overlays over the
 dispatch functions.

 "$Id: netcdf.m4,v 1.16 2010/06/01 15:34:50 ed Exp $" 
*/

#include "dispatch.h"

#ifdef USE_DAP
#include "oc.h"
#endif

#ifdef ENABLE_RC
#include "ncrc.h"

#define NETCDFRC ".netcdfrc"

ncrcnode* ncrcroot = NULL;
ncrcnode* netcdfroot = NULL;
ncrcnode* netcdf3root = NULL;
ncrcnode* netcdf4root = NULL;

static int loadrcfile(void);

#endif


#define INITCOORD1 if(coord_one[0] != 1) {int i; for(i=0;i<NC_MAX_VAR_DIMS;i++) coord_one[i] = 1;}

#if defined(__cplusplus)
/* C++ consts default to internal linkage and must be initialized */
const size_t coord_zero[NC_MAX_VAR_DIMS] = {0};
const size_t coord_one[NC_MAX_VAR_DIMS] = {1};
#else
static const size_t coord_zero[NC_MAX_VAR_DIMS];
/* initialized int put/get_var1 below */
static size_t coord_one[NC_MAX_VAR_DIMS];
#endif

static nc_type longtype = (sizeof(long) == sizeof(int)?NC_INT:NC_INT64);
static nc_type ulongtype = (sizeof(unsigned long) == sizeof(unsigned int)?NC_UINT:NC_UINT64);

NC_Dispatch* NC3_dispatch_table = NULL;

#ifdef USE_NETCDF4
NC_Dispatch* NC4_dispatch_table = NULL;
#endif

#ifdef USE_DAP
NC_Dispatch* NCD3_dispatch_table = NULL;
#endif

#if defined(USE_DAP) && defined(USE_NETCDF4)
NC_Dispatch* NCD4_dispatch_table = NULL;
#endif

static int NC_get_var(int ncid, int varid, void *value, nc_type memtype);
static int NC_put_var(int ncid, int varid, const void *value, nc_type memtype);
static int NC_get_var1(int ncid, int varid, const size_t *coord, void* value, nc_type memtype);
static int NC_put_var1(int ncid, int varid, const size_t *coord, const void* value, nc_type memtype);

static int NC_get_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            void *value, nc_type memtype);
static int NC_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            const void *value, nc_type memtype);

static int NC_get_varm(int ncid, int varid,
			const size_t * start, const size_t * edges,
			const ptrdiff_t * stride, const ptrdiff_t * imapp,
			void *value, nc_type memtype);
static int NC_put_varm(int ncid, int varid,
			const size_t * start, const size_t * edges,
			const ptrdiff_t * stride, const ptrdiff_t * imapp,
			const void *value, nc_type memtype);
static int NC_get_vars(int ncid, int varid,
			const size_t * start, const size_t * edges,
			const ptrdiff_t * stride,
			void *value, nc_type memtype);
static int NC_put_vars(int ncid, int varid,
			const size_t * start, const size_t * edges,
			const ptrdiff_t * stride,
			const void *value, nc_type memtype);

static int
NC_check_file_type(const char *path,
		 int use_parallel, void* mpi_info,
		 int *cdf, int* hdf);

static int getshape(int ncid, int varid, int ndims, size_t* shape);

#ifndef NC_ENOTNC4
#define NC_ENOTNC4 (-111)
#endif

#ifndef X_INT_MAX
#define  X_INT_MAX 2147483647
#endif

#ifdef USE_NETCDF4
#ifndef LOGGING
static void nc_log(int severity, const char *fmt, ...) {return;}
static void nc_log_hdf5(void) {return;}
#ifndef LOG
# define LOG(e) nc_log e
#endif
#else /*!LOGGING*/
extern void nc_log(int severity, const char *fmt, ...);
extern void nc_log_hdf5(void);
#ifndef LOG
# define LOG(e)
#endif
#endif /*LOGGING*/
#endif /*USE_NETCDF4*/

#ifndef BAIL
/* To log an error message, set retval, and jump to exit. */
#ifdef USE_NETCDF4
#define BAIL(e) do { \
retval = e; \
LOG((0, "file %s, line %d.\n%s", __FILE__, __LINE__, nc_strerror(e))); \
goto exit; \
} while (0) 
#else
#define BAIL(e) do {retval = e; goto exit; } while (0)
#endif
#endif

/**************************************************/

int
NC_testurl(const char* path)
{
#ifdef USE_DAP
    void* tmpurl = NULL;
    if(NCDAP_urlparse(path,&tmpurl) == NC_NOERR) {
	NCDAP_urlfree(tmpurl);
	return 1;
    }
#endif
    return 0;
}

int
NC_urlmodel(const char* path)
{
    int model = 0;
#ifdef USE_DAP
    void* tmpurl = NULL;
    if(NCDAP_urlparse(path,&tmpurl) == NC_NOERR) {
	if(NCDAP_urllookup(tmpurl,"netcdf4")
	   || NCDAP_urllookup(tmpurl,"netcdf-4")) {
	    model = 4;
	} else if(NCDAP_urllookup(tmpurl,"netcdf3")
	   || NCDAP_urllookup(tmpurl,"netcdf-3")) {
	    model = 3;
	} else {
	    model = 0;
	}
	NCDAP_urlfree(tmpurl);
    }
#endif
    return model;
}

/* Override dispatch table management */
static NC_Dispatch* NC_dispatch_override = NULL;

/* Override dispatch table management */
NC_Dispatch*
NC_get_dispatch_override(void) {
    return NC_dispatch_override;
}

void NC_set_dispatch_override(NC_Dispatch* d)
{
    NC_dispatch_override = d;
}

static int initialized = 0;

static int
ncinitialize(void)
{
    int stat = NC_NOERR;
    stat = NC_initialize(); /* in liblib/stub.c */
    if(stat != NC_NOERR) goto done;

#ifdef ENABLE_RC
    stat = loadrcfile();
#endif /*ENABLE_RC*/    

done:
    initialized = 1;
    return stat;
}

#ifdef ENABLE_RC
static char* defaultrc = NULL;

int
nc_set_defaultrc(char* dfalt)
{
    if(dfalt == NULL) return NC_EINVAL;
    if(strlen(dfalt)==0) return NC_EINVAL;
    if(defaultrc != NULL) free(defaultrc);
    defaultrc = strdup(dfalt);
    return NC_NOERR;
}

static int
loadrcfile(void)
{
    char *homepath;
    char *dotpath = NULL;
    char* path = NULL;
    int stat = NC_NOERR;
    FILE* rcfile;
    ncrcerror err;

    /* Ensure defaultrc is defined */
    if(defaultrc == NULL || strlen(defaultrc) == 0)
	nc_set_defaultrc(NETCDFRC);

#ifdef USE_DAP
    /* Make oc use same rc file */
    oc_setrcfile(defaultrc);
#endif

    /* compute the config file path */
    homepath = getenv("HOME");

    if(defaultrc[0] == '/') {
	path = (char*)malloc(strlen(defaultrc)+1);
    } else {
	if (homepath != NULL) {
	    path = (char*)malloc(strlen(homepath)+1+strlen(defaultrc)+1);
	    strcpy(path,homepath);
	    strcat(path,"/");
	    strcat(path,defaultrc);
	}
	dotpath = (char*)malloc(1+1+strlen(defaultrc)+1);
	strcpy(dotpath,"./");
        strcat(dotpath,defaultrc);
    }
    rcfile = NULL;
    /* try ./DEFAULTRC then $HOME/DEFAULTRC */
    if(dotpath != NULL)
        rcfile = fopen(dotpath,"r");
    if(rcfile == NULL && path != NULL) 
        rcfile = fopen(path,"r");
    if(rcfile == NULL)  goto done; /* ignore */
    if(!ncrc(rcfile,&ncrcroot,&err)) {
	fprintf(stderr,"Error parsing %s; %s: lineno=%d charno=%d\n",
			path,err.errmsg,err.lineno,err.charno);
	stat = NC_ERCFILE;
	goto done;
    }
    /* Get the netcdf entry */
    netcdfroot = ncrc_lookup(ncrcroot,"netcdf");
    if(netcdfroot == NULL) {
        fprintf(stderr,"Cannot find \"netcdf\" entry in rc file\n");
	goto done;
    }
    /* Dont' char if missing */
    netcdf3root = ncrc_lookup(netcdfroot,"netcdf3");
    netcdf4root = ncrc_lookup(netcdfroot,"netcdf4");

done:
    if(path != NULL) free(path);
    if(dotpath != NULL) free(dotpath);
    return  stat;
}

#endif /*ENABLE_RC*/
/**************************************************/
int
nc_create(const char * path, int cmode, int *ncid_ptr)
{
	return nc__create_mp(path, cmode, 0, 0, NULL, ncid_ptr);
}

int
nc__create(const char * path, int cmode, size_t initialsz,
	size_t *chunksizehintp, int *ncid_ptr)
{
	return nc__create_mp(path, cmode, initialsz, 0,
		chunksizehintp, ncid_ptr);
}

int
nc__create_mp(const char * path, int cmode, size_t initialsz, int basepe, size_t *chunksizehintp, int *ncidp)
{
    return NC_create(path,cmode,initialsz,basepe,chunksizehintp,0,NULL,ncidp);
}

/*
For create, we have the following pieces of information
to use to determine the dispatch table.
1. table specified by override
2. path
3. cmode
*/
int
NC_create(const char *path, int cmode,
	  size_t initialsz, int basepe, size_t *chunksizehintp, 
	  int useparallel, void* mpi_info,
          int *ncidp)
{
    int stat = NC_NOERR;
    NC* ncp = NULL;
    NC_Dispatch* dispatcher = NULL;
    /* Need three pieces of information for now */
    int model = 0; /* 3 vs 4 dispatch table */
    int dap = 0;   /* dap vs !dap */

    if(!initialized)
	{stat = ncinitialize(); if(stat) return stat;}

    if((dap = NC_testurl(path))) model = NC_urlmodel(path);

    /* Look to the incoming cmode for hints */
    if(model == 0) {
	if(cmode & NC_NETCDF4 || cmode & NC_CLASSIC_MODEL) model = 4;
    }

    if(model == 0) model = 3; /* final default */

    /* Force flag consistentcy */
    if(model == 4)
	cmode |= NC_NETCDF4;
    else if(model == 3) {
	cmode &= ~(NC_NETCDF4 | NC_CLASSIC_MODEL); /* must be netcdf-3 */
    }

#ifdef USE_NETCDF4
    if((cmode & NC_MPIIO && cmode & NC_MPIPOSIX))
	return  NC_EINVAL;
#endif

    dispatcher = NC_get_dispatch_override();
    if(dispatcher != NULL) goto havetable;

    /* Figure out what dispatcher to use */
    dispatcher = NC3_dispatch_table; /* default */
#ifdef USE_DAP
    if(dap) dispatcher = NCD3_dispatch_table; /* default */
#endif

#ifdef USE_NETCDF4
    if(model == 4) {
        dispatcher = NC4_dispatch_table;
#ifdef USE_DAP
	if(dap) dispatcher = NCD4_dispatch_table;
#endif
    }
#endif

havetable:
    stat = dispatcher->create(path,cmode,initialsz,basepe,chunksizehintp,
				useparallel,mpi_info,dispatcher,&ncp);
    if(stat == NC_NOERR) {
	ncp->dispatch = dispatcher;
	if(ncidp) *ncidp = ncp->ext_ncid;
    }
    return stat;
}

int
nc_open(const char *path, int mode, int *ncidp)
{
    return nc__open_mp(path,mode,0,NULL,ncidp);
}

int
nc__open(const char * path, int cmode,
	size_t *chunksizehintp, int *ncid_ptr)
{
	return nc__open_mp(path, cmode, 0, chunksizehintp, ncid_ptr);
}

int
nc__open_mp(const char * path, int cmode, int basepe, size_t *chunksizehintp, int *ncidp)

{
    return NC_open(path,cmode,basepe,chunksizehintp,0,NULL,ncidp);
}


/*
For create, we have the following pieces of information
to use to determine the dispatch table.
1. table specified by override
2. path
3. cmode
4. the contents of the file (if it exists);
   basically checking its magic number
*/

int
NC_open(const char *path, int cmode,
	int basepe, size_t *chunksizehintp,
        int useparallel, void* mpi_info,
        int *ncidp)
{
    int stat = NC_NOERR;
    NC* ncp = NULL;
    NC_Dispatch* dispatcher = NULL;
    /* Need two pieces of information for now */
    int model = 0; /* 3 vs 4 dispatch table */
    int dap = 0;   /* dap vs !dap */
    int cdfversion = 0;
    int hdfversion = 0;

    if(!initialized)
	{stat = ncinitialize(); if(stat) return stat;}

    if((dap = NC_testurl(path)))
	model = NC_urlmodel(path);

    if(dap == 0) {
        /* Look at the file if it exists */
        stat = NC_check_file_type(path,useparallel,mpi_info,&cdfversion,&hdfversion);
	if(stat == NC_NOERR) {
	    if(hdfversion != 0) {
		model = 4;
	    } else if(cdfversion != 0) {
		model = 3;
	    }
	} 
        /* else ignore the file */
    }

    /* Look to the incoming cmode for hints */
    if(model == 0) {
	if(cmode & NC_NETCDF4 || cmode & NC_CLASSIC_MODEL) model = 4;
    }

    if(model == 0) model = 3; /* final default */

    /* Force flag consistentcy */
    if(model == 4)
	cmode |= NC_NETCDF4;
    else if(model == 3) {
	cmode &= ~(NC_NETCDF4 | NC_CLASSIC_MODEL); /* must be netcdf-3 */
	if(cdfversion == 2) cmode |= NC_64BIT_OFFSET;
    }

    if((cmode & NC_MPIIO && cmode & NC_MPIPOSIX))
	return  NC_EINVAL;

    /* override overrides any other table choice */
    dispatcher = NC_get_dispatch_override();
    if(dispatcher != NULL) goto havetable;

    /* Figure out what dispatcher to use */
    dispatcher = NC3_dispatch_table; /* default */
#ifdef USE_DAP
    if(dap) dispatcher = NCD3_dispatch_table; /* default */
#endif

#ifdef USE_NETCDF4
    if(model == 4) {
        dispatcher = NC4_dispatch_table;
#ifdef USE_DAP
	if(dap) dispatcher = NCD4_dispatch_table;
#endif
    }
#endif

havetable:
    stat = dispatcher->open(path,cmode,basepe,chunksizehintp,useparallel,mpi_info,dispatcher,&ncp);
    if(stat == NC_NOERR) {
	ncp->dispatch = dispatcher;
	if(ncidp) *ncidp = ncp->ext_ncid;
    }
    return stat;
}

int
nc_redef(int ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->redef(ncid);
}

int
nc__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->_enddef(ncid,h_minfree,v_align,v_minfree,r_align);
}

int
nc_enddef(int ncid)
{
    int status;
    NC *ncp;
    status = NC_check_id(ncid, &ncp); 
    if(status != NC_NOERR) return status;
    return ncp->dispatch->_enddef(ncid,0,1,0,1);
}

int
nc_sync(int ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->sync(ncid);
}

int
nc_abort(int ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->abort(ncid);
}

int
nc_close(int ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->close(ncid);
}

int
nc_set_fill(int ncid, int fillmode, int *old_modep)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->set_fill(ncid,fillmode,old_modep);
}

int
nc_inq_base_pe(int ncid, int *pe)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_base_pe(ncid,pe);
}

int
nc_set_base_pe(int ncid, int pe)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->set_base_pe(ncid,pe);
}

int
nc_inq_format(int ncid, int *formatp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_format(ncid,formatp);
}

int
nc_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq(ncid,ndimsp,nvarsp,nattsp,unlimdimidp);
}

int
nc_inq_type(int ncid, nc_type xtype, char *name, size_t *size)
{
    NC* ncp;
    /* For compatibility, we need to allow inq about
       atomic types, even if ncid is ill-defined */
    if(xtype <= ATOMICTYPEMAX) {
	if(xtype <= NC_NAT) return NC_EBADTYPE;
	if(name) strncpy(name,NC_atomictypename(xtype),NC_MAX_NAME);
	if(size) *size = NC_atomictypelen(xtype);
	return NC_NOERR;
    } else {
        int stat = NC_check_id(ncid, &ncp);
        if(stat != NC_NOERR) return NC_EBADTYPE; /* compatibility */
        return ncp->dispatch->inq_type(ncid,xtype,name,size);
    }
}

int
nc_def_dim(int ncid, const char *name, size_t len, int *idp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_dim(ncid,name,len,idp);
}

int
nc_inq_dimid(int ncid, const char *name, int *idp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_dimid(ncid,name,idp);
}

int
nc_inq_dim(int ncid, int dimid, char *name, size_t *lenp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_dim(ncid,dimid,name,lenp);
}

int
nc_rename_dim(int ncid, int dimid, const char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->rename_dim(ncid,dimid,name);
}

int
nc_inq_att(int ncid, int varid, const char *name, nc_type *xtypep, size_t *lenp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_att(ncid,varid,name,xtypep,lenp);
}

int
nc_inq_attid(int ncid, int varid, const char *name, int *idp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_attid(ncid,varid,name,idp);
}

int
nc_inq_attname(int ncid, int varid, int attnum, char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_attname(ncid,varid,attnum,name);
}

int
nc_rename_att(int ncid, int varid, const char *name, const char *newname)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->rename_att(ncid,varid,name,newname);
}

int
nc_del_att(int ncid, int varid, const char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->del_att(ncid,varid,name);
}

int
nc_def_var(int ncid, const char *name, nc_type xtype, int ndims,  const int *dimidsp, int *varidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var(ncid,name,xtype,ndims,dimidsp,varidp);
}

int
nc_inq_varid(int ncid, const char *name, int *varidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_varid(ncid,name,varidp);
}

int
nc_rename_var(int ncid, int varid, const char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->rename_var(ncid,varid,name);
}

int 
nc_inq_varname(int ncid, int varid, char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(name == NULL) return NC_NOERR;
    return nc_inq_var(ncid,varid,name,NULL,NULL,NULL,NULL);
}

int 
nc_inq_vartype(int ncid, int varid, nc_type *typep)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(typep == NULL) return NC_NOERR;
    return nc_inq_var(ncid,varid,NULL,typep,NULL,NULL,NULL);
}

int 
nc_inq_varndims(int ncid, int varid, int *ndimsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(ndimsp == NULL) return NC_NOERR;
    return nc_inq_var(ncid,varid,NULL,NULL,ndimsp,NULL,NULL);
}

int 
nc_inq_vardimid(int ncid, int varid, int *dimids)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(dimids == NULL) return NC_NOERR;
    return nc_inq_var(ncid,varid,NULL,NULL,NULL,dimids,NULL);
}

int 
nc_inq_varnatts(int ncid, int varid, int *nattsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(nattsp == NULL) return NC_NOERR;
    if(varid == NC_GLOBAL)
	return nc_inq_natts(ncid,nattsp);
    /*else*/
    return nc_inq_var(ncid,varid,NULL,NULL,NULL,NULL,nattsp);
}


/* get and put are defined in putget.m4 */

int
nc_inq_ndims(int ncid, int *ndimsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(ndimsp == NULL) return NC_NOERR;
    return ncp->dispatch->inq(ncid,ndimsp,NULL,NULL,NULL);
}

int
nc_inq_nvars(int ncid, int *nvarsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(nvarsp == NULL) return NC_NOERR;
    return ncp->dispatch->inq(ncid,NULL,nvarsp,NULL,NULL);
}

int
nc_inq_natts(int ncid, int *nattsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(nattsp == NULL) return NC_NOERR;
    return ncp->dispatch->inq(ncid,NULL,NULL,nattsp,NULL);
}

int
nc_inq_unlimdim(int ncid, int *unlimdimidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_unlimdim(ncid,unlimdimidp);
}

int
nc_inq_dimname(int ncid, int dimid, char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(name == NULL) return NC_NOERR;
    return ncp->dispatch->inq_dim(ncid,dimid,name,NULL);
}

int
nc_inq_dimlen(int ncid, int dimid, size_t *lenp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(lenp == NULL) return NC_NOERR;
    return ncp->dispatch->inq_dim(ncid,dimid,NULL,lenp);
}

int
nc_inq_atttype(int ncid, int varid, const char *name, nc_type *xtypep)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(xtypep == NULL) return NC_NOERR;
    return ncp->dispatch->inq_att(ncid,varid,name,xtypep,NULL);
}

int
nc_inq_attlen(int ncid, int varid, const char *name, size_t *lenp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(lenp == NULL) return NC_NOERR;
    return ncp->dispatch->inq_att(ncid,varid,name,NULL,lenp);
}

/**************************************************/
/* Wrapper */

static int
NC_get_var(int ncid, int varid, void *value, nc_type memtype)
{
    int ndims;
    size_t shape[NC_MAX_VAR_DIMS];
    int stat = nc_inq_varndims(ncid,varid, &ndims);
    if(stat) return stat;
    stat = getshape(ncid,varid, ndims, shape);
    if(stat) return stat;
    return NC_get_vara(ncid, varid, coord_zero, shape, value, memtype);
}

static int
NC_put_var(int ncid, int varid, const void *value, nc_type memtype)
{
    int ndims;
    size_t shape[NC_MAX_VAR_DIMS];
    int stat = nc_inq_varndims(ncid,varid, &ndims);
    if(stat) return stat;
    stat = getshape(ncid,varid, ndims, shape);
    if(stat) return stat;
    return NC_put_vara(ncid, varid, coord_zero, shape, value, memtype);
}

static int
NC_get_var1(int ncid, int varid, const size_t *coord, void* value, nc_type memtype)
{
    INITCOORD1;
    return NC_get_vara(ncid, varid, coord, coord_one, value, memtype);
}

static int
NC_put_var1(int ncid, int varid, const size_t *coord, const void* value, nc_type memtype)
{
    INITCOORD1;
    return NC_put_vara(ncid, varid, coord, coord_one, value, memtype);
}

static int
NC_get_vars(
	int ncid,
	int varid,
	const size_t * start,
	const size_t * edges,
	const ptrdiff_t * stride,
	void *value,
	nc_type memtype)
{
	return NC_get_varm (ncid, varid, start, edges,
			 stride, 0, value, memtype);
}

static int
NC_put_vars(
	int ncid,
	int varid,
	const size_t * start,
	const size_t * edges,
	const ptrdiff_t * stride,
	const void *value,
	nc_type memtype)
{
	return NC_put_varm (ncid, varid, start, edges,
			 stride, 0, value, memtype);
}

static int
NC_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            const void *value, nc_type memtype)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(edges == NULL) {
	size_t shape[NC_MAX_VAR_DIMS];
	int ndims;
        stat = nc_inq_varndims(ncid, varid, &ndims); 
        if(stat != NC_NOERR) return stat;
	stat = getshape(ncid,varid,ndims,shape);
        if(stat != NC_NOERR) return stat;
        return ncp->dispatch->put_vara(ncid,varid,start,shape,value,memtype);
    } else
        return ncp->dispatch->put_vara(ncid,varid,start,edges,value,memtype);
}

static int
NC_get_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            void *value, nc_type memtype)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
    if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
    if(edges == NULL) {
	size_t shape[NC_MAX_VAR_DIMS];
	int ndims;
        stat = nc_inq_varndims(ncid, varid, &ndims); 
        if(stat != NC_NOERR) return stat;
	stat = getshape(ncid,varid,ndims,shape);
        if(stat != NC_NOERR) return stat;
        return ncp->dispatch->get_vara(ncid,varid,start,shape,value,memtype);
    } else
        return ncp->dispatch->get_vara(ncid,varid,start,edges,value,memtype);
}

static int
is_recvar(int ncid, int varid, size_t* nrecs)
{
    int status;
    int unlimid;
    int ndims;
    int dimset[NC_MAX_VAR_DIMS];
    
    status = nc_inq_unlimdim(ncid,&unlimid);
    if(status != NC_NOERR) return 0; /* no unlimited defined */
    status = nc_inq_varndims(ncid,varid,&ndims);
    if(status != NC_NOERR) return 0; /* no unlimited defined */
    if(ndims == 0) return 0; /* scalar */
    status = nc_inq_vardimid(ncid,varid,dimset);
    if(status != NC_NOERR) return 0; /* no unlimited defined */
    status = nc_inq_dim(ncid,dimset[0],NULL,nrecs);
    if(status != NC_NOERR) return 0;
    return (dimset[0] == unlimid ? 1: 0);
}

static int
getshape(int ncid, int varid, int ndims, size_t* shape)
{
    int status = NC_NOERR;
    int dimids[NC_MAX_VAR_DIMS];
    int i;
    status = nc_inq_vardimid(ncid,varid,dimids);
    if(status != NC_NOERR) return status;
    for(i=0;i<ndims;i++) {
	status = nc_inq_dimlen(ncid,dimids[i],&shape[i]);
	if(status != NC_NOERR) break;
    } 
    return status;
}

/* Ok to use NC pointers because
   all IOSP's will use that structure,
   but not ok to use e.g. NC_Var pointers
   because they may be different structure
   entirely.
*/

static int
NC_get_varm(
	int ncid,
	int varid,
	const size_t * start,
	const size_t * edges,
	const ptrdiff_t * stride,
	const ptrdiff_t * imapp,
	void *value0,
	nc_type memtype)
{
    int status;
    nc_type vartype;
    int varndims,maxidim;
    NC* ncp;
    size_t memtypelen;
    ptrdiff_t cvtmap[NC_MAX_VAR_DIMS];
    char* value = (char*)value0;

    status = NC_check_id (ncid, &ncp);
    if(status != NC_NOERR) return status;

/*
    if(NC_indef(ncp)) return NC_EINDEFINE;
*/

    status = nc_inq_vartype(ncid, varid, &vartype); 
    if(status != NC_NOERR) return status;

    status = nc_inq_varndims(ncid, varid, &varndims); 
    if(status != NC_NOERR) return status;

    if(memtype == NC_NAT) {
	if(imapp != NULL && varndims != 0) {
            /*
             * convert map units from bytes to units of sizeof(type)
            */
            size_t ii;
            const ptrdiff_t szof = (ptrdiff_t) nctypelen(vartype);
            for(ii = 0; ii < varndims; ii++) {
                if(imapp[ii] % szof != 0) {
                    /*free(cvtmap);*/
                    return NC_EINVAL;
                }
                cvtmap[ii] = imapp[ii] / szof;
            }
            imapp = cvtmap;
        }
	memtype = vartype;
    }

    if(memtype == NC_CHAR && vartype != NC_CHAR)
      return NC_ECHAR;
    else if(memtype != NC_CHAR && vartype == NC_CHAR)  
      return NC_ECHAR;

    memtypelen = nctypelen(memtype);

    maxidim = (int) varndims - 1;

    if (maxidim < 0)
    {
        /*
         * The variable is a scalar; consequently,
         * there s only one thing to get and only one place to put it.
         * (Why was I called?)
         */
	size_t edge1[1] = {1};
        return NC_get_vara(ncid, varid, start, edge1, value, memtype);
    }

    /*
     * else
     * The variable is an array.
     */
    {
        int idim;
        size_t *mystart = NULL;
        size_t *myedges;
        size_t *iocount;    /* count vector */
        size_t *stop;   /* stop indexes */
        size_t *length; /* edge lengths in bytes */
        ptrdiff_t *mystride;
        ptrdiff_t *mymap;
	size_t varshape[NC_MAX_VAR_DIMS];
        int isrecvar;
        size_t numrecs;

	/* Compute some dimension related values */
        isrecvar = is_recvar(ncid,varid,&numrecs);
	getshape(ncid,varid,varndims,varshape);	

        /*
         * Verify stride argument.
         */
	if(stride != NULL)
        for (idim = 0; idim <= maxidim; ++idim)
        {
            if (stride[idim] == 0
        /* cast needed for braindead systems with signed size_t */
                || ((unsigned long) stride[idim] >= X_INT_MAX))
            {
                return NC_ESTRIDE;
            }
        }

        /* assert(sizeof(ptrdiff_t) >= sizeof(size_t)); */
	/* Allocate space for mystart,mystride,mymap etc.all at once */
        mystart = (size_t *)calloc(varndims * 7, sizeof(ptrdiff_t));
        if(mystart == NULL) return NC_ENOMEM;
        myedges = mystart + varndims;
        iocount = myedges + varndims;
        stop = iocount + varndims;
        length = stop + varndims;
        mystride = (ptrdiff_t *)(length + varndims);
        mymap = mystride + varndims;

        /*
         * Initialize I/O parameters.
         */
        for (idim = maxidim; idim >= 0; --idim)
        {
            mystart[idim] = start != NULL
                ? start[idim]
                : 0;

            if (edges != NULL && edges[idim] == 0)
            {
                status = NC_NOERR;    /* read/write no data */
                goto done;
            }

#ifdef COMPLEX
            myedges[idim] = edges != NULL
                ? edges[idim]
                : idim == 0 && isrecvar
                ? numrecs - mystart[idim]
                : varshape[idim] - mystart[idim];
#else
            if(edges != NULL)
                myedges[idim] = edges[idim];
	    else if (idim == 0 && isrecvar)
                myedges[idim] = numrecs - mystart[idim];
	    else
                myedges[idim] = varshape[idim] - mystart[idim];
#endif

            mystride[idim] = stride != NULL
                ? stride[idim]
                : 1;

	    /* Remember: imapp is byte oriented, not index oriented */
#ifdef COMPLEX
            mymap[idim] = (imapp != NULL
		? imapp[idim]
	        : (idim == maxidim ? 1
                                   : mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1]));
#else
            if(imapp != NULL)
                mymap[idim] = imapp[idim];
	    else if (idim == maxidim)
                mymap[idim] = 1;
	    else
                mymap[idim] = 
                    mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1];
#endif
            iocount[idim] = 1;
            length[idim] = mymap[idim] * myedges[idim];
            stop[idim] = mystart[idim] + myedges[idim] * mystride[idim];
        }

        /*
         * Check start, edges
         */
        for (idim = maxidim; idim >= 0; --idim)
        {
            size_t dimlen = 
                idim == 0 && isrecvar
                    ? numrecs
                      : varshape[idim];
            if (mystart[idim] >= dimlen)
            {
                status = NC_EINVALCOORDS;
                goto done;
            }

            if (mystart[idim] + myedges[idim] > dimlen)
            {
                status = NC_EEDGE;
                goto done;
            }

        }


        /* Lower body */
        /*
         * As an optimization, adjust I/O parameters when the fastest 
         * dimension has unity stride both externally and internally.
         * In this case, the user could have called a simpler routine
         * (i.e. ncvar$1()
         */
        if (mystride[maxidim] == 1
            && mymap[maxidim] == 1)
        {
            iocount[maxidim] = myedges[maxidim];
            mystride[maxidim] = (ptrdiff_t) myedges[maxidim];
            mymap[maxidim] = (ptrdiff_t) length[maxidim];
        }

        /* 
        * Perform I/O.  Exit when done.
         */
        for (;;)
        {
            /* TODO: */
            int lstatus = NC_get_vara(ncid, varid, mystart, iocount,
                        		value, memtype);
            if (lstatus != NC_NOERR) {
                if(status == NC_NOERR || lstatus != NC_ERANGE)
                    status = lstatus;
	    }
            /*
             * The following code permutes through the variable s
             * external start-index space and it s internal address
             * space.  At the UPC, this algorithm is commonly
             * called "odometer code".
             */
            idim = maxidim;
        carry:
            value += (mymap[idim] * memtypelen);
            mystart[idim] += mystride[idim];
            if (mystart[idim] == stop[idim])
            {
                mystart[idim] = start[idim];
                value -= (length[idim] * memtypelen);
                if (--idim < 0)
                    break; /* normal return */
                goto carry;
            }
        } /* I/O loop */
    done:
        free(mystart);
    } /* variable is array */
    return status;
}


static int
NC_put_varm(
	int ncid,
	int varid,
	const size_t * start,
	const size_t * edges,
	const ptrdiff_t * stride,
	const ptrdiff_t * imapp,
	const void *value0,
	nc_type memtype)
{
    int status;
    nc_type vartype;
    int varndims,maxidim;
    NC* ncp;
    size_t memtypelen;
    ptrdiff_t cvtmap[NC_MAX_VAR_DIMS];
    const char* value = (char*)value0;

    status = NC_check_id (ncid, &ncp);
    if(status != NC_NOERR) return status;

/*
    if(NC_indef(ncp)) return NC_EINDEFINE;
    if(NC_readonly (ncp)) return NC_EPERM;
*/

    /* mid body */
    status = nc_inq_vartype(ncid, varid, &vartype); 
    if(status != NC_NOERR) return status;

    status = nc_inq_varndims(ncid, varid, &varndims); 
    if(status != NC_NOERR) return status;

    if(memtype == NC_NAT) {
	if(imapp != NULL && varndims != 0) {
            /*
             * convert map units from bytes to units of sizeof(type)
            */
            size_t ii;
            const ptrdiff_t szof = (ptrdiff_t) nctypelen(vartype);
            for(ii = 0; ii < varndims; ii++) {
                if(imapp[ii] % szof != 0) {
                    /*free(cvtmap);*/
                    return NC_EINVAL;
                }
                cvtmap[ii] = imapp[ii] / szof;
            }
            imapp = cvtmap;
        }
	memtype = vartype;
    }

    if(memtype == NC_CHAR && vartype != NC_CHAR)
      return NC_ECHAR;
    else if(memtype != NC_CHAR && vartype == NC_CHAR)  
      return NC_ECHAR;

    memtypelen = nctypelen(memtype);

    maxidim = (int) varndims - 1;

    if (maxidim < 0)
    {
        /*
         * The variable is a scalar; consequently,
         * there s only one thing to get and only one place to put it.
         * (Why was I called?)
         */
	size_t edge1[1] = {1};
        return NC_put_vara(ncid, varid, start, edge1, value, memtype);
    }

    /*
     * else
     * The variable is an array.
     */
    {
        int idim;
        size_t *mystart = NULL;
        size_t *myedges;
        size_t *iocount;    /* count vector */
        size_t *stop;   /* stop indexes */
        size_t *length; /* edge lengths in bytes */
        ptrdiff_t *mystride;
        ptrdiff_t *mymap;
	size_t varshape[NC_MAX_VAR_DIMS];
        int isrecvar;
        size_t numrecs;

	/* Compute some dimension related values */
        isrecvar = is_recvar(ncid,varid,&numrecs);
	getshape(ncid,varid,varndims,varshape);	

        /*
         * Verify stride argument.
         */
	if(stride != NULL)
        for (idim = 0; idim <= maxidim; ++idim)
        {
            if ((stride[idim] == 0)
        /* cast needed for braindead systems with signed size_t */
                || ((unsigned long) stride[idim] >= X_INT_MAX))
            {
                return NC_ESTRIDE;
            }
        }

        /* assert(sizeof(ptrdiff_t) >= sizeof(size_t)); */
        mystart = (size_t *)calloc(varndims * 7, sizeof(ptrdiff_t));
        if(mystart == NULL) return NC_ENOMEM;
        myedges = mystart + varndims;
        iocount = myedges + varndims;
        stop = iocount + varndims;
        length = stop + varndims;
        mystride = (ptrdiff_t *)(length + varndims);
        mymap = mystride + varndims;

        /*
         * Initialize I/O parameters.
         */
        for (idim = maxidim; idim >= 0; --idim)
        {
            mystart[idim] = start != NULL
                ? start[idim]
                : 0;

            if (edges != NULL && edges[idim] == 0)
            {
                status = NC_NOERR;    /* read/write no data */
                goto done;
            }

            myedges[idim] = edges != NULL
                ? edges[idim]
                : idim == 0 && isrecvar
                ? numrecs - mystart[idim]
                : varshape[idim] - mystart[idim];
            mystride[idim] = stride != NULL
                ? stride[idim]
                : 1;
            mymap[idim] = imapp != NULL
                ? imapp[idim]
                : idim == maxidim
                ? 1
                : mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1];

            iocount[idim] = 1;
            length[idim] = mymap[idim] * myedges[idim];
            stop[idim] = mystart[idim] + myedges[idim] * mystride[idim];
        }

        /*
         * Check start, edges
         */
        for (idim = isrecvar; idim < maxidim; ++idim)
        {
            if (mystart[idim] > varshape[idim])
            {
                status = NC_EINVALCOORDS;
                goto done;
            }
            if (mystart[idim] + myedges[idim] > varshape[idim])
            {
                status = NC_EEDGE;
                goto done;
            }
        }

        /* Lower body */
        /*
         * As an optimization, adjust I/O parameters when the fastest 
         * dimension has unity stride both externally and internally.
         * In this case, the user could have called a simpler routine
         * (i.e. ncvar$1()
         */
        if (mystride[maxidim] == 1
            && mymap[maxidim] == 1)
        {
            iocount[maxidim] = myedges[maxidim];
            mystride[maxidim] = (ptrdiff_t) myedges[maxidim];
            mymap[maxidim] = (ptrdiff_t) length[maxidim];
        }

        /*
         * Perform I/O.  Exit when done.
         */
        for (;;)
        {
            /* TODO: */
            int lstatus = NC_put_vara(ncid, varid, mystart, iocount,
                        		value, memtype);
            if (lstatus != NC_NOERR) {
                if(status == NC_NOERR || lstatus != NC_ERANGE)
                    status = lstatus;
	    }	    

            /*
             * The following code permutes through the variable s
             * external start-index space and it s internal address
             * space.  At the UPC, this algorithm is commonly
             * called "odometer code".
             */
            idim = maxidim;
        carry:
            value += (mymap[idim] * memtypelen);
            mystart[idim] += mystride[idim];
            if (mystart[idim] == stop[idim])
            {
                mystart[idim] = start[idim];
                value -= (length[idim] * memtypelen);
                if (--idim < 0)
                    break; /* normal return */
                goto carry;
            }
        } /* I/O loop */
    done:
        free(mystart);
    } /* variable is array */
    return status;
}

#ifdef USE_NETCDF4

int
nc_inq_ncid(int ncid, const char *name, int *grp_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_ncid(ncid,name,grp_ncid);
}

int
nc_inq_grps(int ncid, int *numgrps, int *ncids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grps(ncid,numgrps,ncids);
}

int
nc_inq_grpname(int ncid, char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grpname(ncid,name);
}

int
nc_inq_grpname_full(int ncid, size_t *lenp, char *full_name)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grpname_full(ncid,lenp,full_name);
}

int
nc_inq_grpname_len(int ncid, size_t *lenp)
{
    int stat = nc_inq_grpname_full(ncid,lenp,NULL);    
    return stat;
}

int
nc_inq_grp_parent(int ncid, int *parent_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grp_parent(ncid,parent_ncid);
}

/* This has same semantics as nc_inq_ncid */ 
int
nc_inq_grp_ncid(int ncid, const char *grp_name, int *grp_ncid)
{
    return nc_inq_ncid(ncid,grp_name,grp_ncid);    
}

int
nc_inq_grp_full_ncid(int ncid, const char *full_name, int *grp_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grp_full_ncid(ncid,full_name,grp_ncid);
}

int 
nc_inq_varids(int ncid, int *nvars, int *varids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_varids(ncid,nvars,varids);
}

int 
nc_inq_dimids(int ncid, int *ndims, int *dimids, int include_parents)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_dimids(ncid,ndims,dimids,include_parents);
}

int 
nc_inq_typeids(int ncid, int *ntypes, int *typeids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_typeids(ncid,ntypes,typeids);
}

int
nc_inq_type_equal(int ncid1, nc_type typeid1, int ncid2, 
		  nc_type typeid2, int *equal)
{
    NC* ncp1;
    int stat = NC_check_id(ncid1,&ncp1);
    if(stat != NC_NOERR) return stat;
    return ncp1->dispatch->inq_type_equal(ncid1,typeid1,ncid2,typeid2,equal);
}

int
nc_def_grp(int parent_ncid, const char *name, int *new_ncid)
{
    NC* ncp;
    int stat = NC_check_id(parent_ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_grp(parent_ncid,name,new_ncid);
}

int
nc_def_compound(int ncid, size_t size, const char *name, nc_type *typeidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_compound(ncid,size,name,typeidp);
}

int
nc_insert_compound(int ncid, nc_type xtype, const char *name, size_t offset, nc_type field_typeid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->insert_compound(ncid,xtype,name,offset,field_typeid);
}

int
nc_insert_array_compound(int ncid, nc_type xtype, const char *name, size_t offset, nc_type field_typeid, int ndims, const int *dim_sizes)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->insert_array_compound(ncid,xtype,name,offset,field_typeid,ndims,dim_sizes);
}

int
nc_inq_typeid(int ncid, const char *name, nc_type *typeidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_typeid(ncid,name,typeidp);
}

int
nc_inq_compound(int ncid, nc_type xtype, char *name, size_t *sizep, size_t *nfieldsp)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,sizep,NULL,nfieldsp,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_COMPOUND) stat = NC_EBADTYPE;
    return stat;
}

int
nc_inq_compound_name(int ncid, nc_type xtype, char *name)
{
    return nc_inq_compound(ncid,xtype,name,NULL,NULL);
}

int
nc_inq_compound_size(int ncid, nc_type xtype, size_t *sizep)
{
    return nc_inq_compound(ncid,xtype,NULL,sizep,NULL);
}

int
nc_inq_compound_nfields(int ncid, nc_type xtype, size_t *nfieldsp)
{
    return nc_inq_compound(ncid,xtype,NULL,NULL,nfieldsp);
}

int
nc_inq_compound_field(int ncid, nc_type xtype, int fieldid, char *name, size_t *offsetp, nc_type *field_typeidp, int *ndimsp, int *dim_sizesp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_compound_field(ncid,xtype,fieldid,name,offsetp,field_typeidp,ndimsp,dim_sizesp);
}

int
nc_inq_compound_fieldname(int ncid, nc_type xtype, int fieldid, 
			  char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_compound_field(ncid,xtype,fieldid,name,NULL,NULL,NULL,NULL);
}

int
nc_inq_compound_fieldoffset(int ncid, nc_type xtype, int fieldid, 
			    size_t *offsetp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_compound_field(ncid,xtype,fieldid,NULL,offsetp,NULL,NULL,NULL);
}

int
nc_inq_compound_fieldtype(int ncid, nc_type xtype, int fieldid, 
			  nc_type *field_typeidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_compound_field(ncid,xtype,fieldid,NULL,NULL,field_typeidp,NULL,NULL);
}

int
nc_inq_compound_fieldndims(int ncid, nc_type xtype, int fieldid, 
			   int *ndimsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_compound_field(ncid,xtype,fieldid,NULL,NULL,NULL,ndimsp,NULL);
}

int
nc_inq_compound_fielddim_sizes(int ncid, nc_type xtype, int fieldid, 
			       int *dim_sizes)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_compound_field(ncid,xtype,fieldid,NULL,NULL,NULL,NULL,dim_sizes);
}

int
nc_inq_compound_fieldindex(int ncid, nc_type xtype, const char *name, 
			   int *fieldidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_compound_fieldindex(ncid,xtype,name,fieldidp);
}

int
nc_def_vlen(int ncid, const char *name, nc_type base_typeid, nc_type *xtypep)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_vlen(ncid,name,base_typeid,xtypep);
}

int
nc_inq_vlen(int ncid, nc_type xtype, char *name, size_t *datum_sizep, nc_type *base_nc_typep)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,datum_sizep,base_nc_typep,NULL,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_VLEN) stat = NC_EBADTYPE;
    return stat;
}

int
nc_put_vlen_element(int ncid, int typeid1, void *vlen_element, size_t len, const void *data)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->put_vlen_element(ncid,typeid1,vlen_element,len,data);
}

int
nc_get_vlen_element(int ncid, int typeid1, const void *vlen_element, size_t *len, void *data)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_vlen_element(ncid,typeid1,vlen_element,len,data);
}

int
nc_inq_user_type(int ncid, nc_type xtype, char *name, size_t *size, nc_type *base_nc_typep, size_t *nfieldsp, int *classp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_user_type(ncid,xtype,name,size,base_nc_typep,nfieldsp,classp);
}

int
nc_def_enum(int ncid, nc_type base_typeid, const char *name, nc_type *typeidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_enum(ncid,base_typeid,name,typeidp);
}

int
nc_insert_enum(int ncid, nc_type xtype, const char *name, const void *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->insert_enum(ncid,xtype,name,value);
}

int
nc_inq_enum(int ncid, nc_type xtype, char *name, nc_type *base_nc_typep, size_t *base_sizep, size_t *num_membersp)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,base_sizep,base_nc_typep,num_membersp,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_ENUM) stat = NC_EBADTYPE;
    return stat;
}

int
nc_inq_enum_member(int ncid, nc_type xtype, int idx, char *name, void *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_enum_member(ncid,xtype,idx,name,value);
}

int
nc_inq_enum_ident(int ncid, nc_type xtype, long long value, char *identifier)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_enum_ident(ncid,xtype,value,identifier);
}

int
nc_def_opaque(int ncid, size_t size, const char *name, nc_type *xtypep)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_opaque(ncid,size,name,xtypep);
}

int
nc_inq_opaque(int ncid, nc_type xtype, char *name, size_t *sizep)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,sizep,NULL,NULL,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_OPAQUE) stat = NC_EBADTYPE;
    return stat;
}

int
nc_def_var_deflate(int ncid, int varid, int shuffle, int deflate, int deflate_level)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_deflate(ncid,varid,shuffle,deflate,deflate_level);
}

int
nc_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_fletcher32(ncid,varid,fletcher32);
}

int
nc_def_var_chunking(int ncid, int varid, int storage, const size_t *chunksizesp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_chunking(ncid,varid,storage,chunksizesp);
}

int
nc_def_var_fill(int ncid, int varid, int no_fill, const void *fill_value)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_fill(ncid,varid,no_fill,fill_value);
}

int
nc_def_var_endian(int ncid, int varid, int endian)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_endian(ncid,varid,endian);
}

int
nc_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, float preemption)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->set_var_chunk_cache(ncid,varid,size,nelems,preemption);
}

int
nc_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp, float *preemptionp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_var_chunk_cache(ncid,varid,sizep,nelemsp,preemptionp);
}

int 
nc_inq_unlimdims(int ncid, int *nunlimdimsp, int *unlimdimidsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_unlimdims(ncid,nunlimdimsp,unlimdimidsp);
}

int 
nc_show_metadata(int ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->show_metadata(ncid);
}
#endif /*USE_NETCDF4*/

/*
 *  This is how much space is required by the user, as in
 *
 *   vals = malloc(nel * nctypelen(var.type));
 *   ncvarget(cdfid, varid, cor, edg, vals);
 */
int
nctypelen(nc_type type) 
{
	switch(type){
	case NC_CHAR :
		return((int)sizeof(char));
	case NC_BYTE :
		return((int)sizeof(signed char));
	case NC_SHORT :
		return(int)(sizeof(short));
	case NC_INT :
		return((int)sizeof(int));
	case NC_FLOAT :
		return((int)sizeof(float));
	case NC_DOUBLE : 
		return((int)sizeof(double));

	/* These can occur in netcdf-3 code */ 
	case NC_UBYTE :
		return((int)sizeof(unsigned char));
	case NC_USHORT :
		return((int)(sizeof(unsigned short)));
	case NC_UINT :
		return((int)sizeof(unsigned int));
	case NC_INT64 :
		return((int)sizeof(signed long long));
	case NC_UINT64 :
		return((int)sizeof(unsigned long long));
#ifdef USE_NETCDF4
	case NC_STRING :
		return((int)sizeof(char*));
#endif /*USE_NETCDF4*/

	default:
	        return -1;
	}
}

/* utility functions */
/* Redunant over nctypelen above */
int
NC_atomictypelen(nc_type xtype)
{
    int sz = 0;
    switch(xtype) {
    case NC_NAT: sz = 0; break;
    case NC_BYTE: sz = sizeof(signed char); break;
    case NC_CHAR: sz = sizeof(char); break;
    case NC_SHORT: sz = sizeof(short); break;
    case NC_INT: sz = sizeof(int); break;
    case NC_FLOAT: sz = sizeof(float); break;
    case NC_DOUBLE: sz = sizeof(double); break;
    case NC_INT64: sz = sizeof(signed long long); break;
    case NC_UBYTE: sz = sizeof(unsigned char); break;
    case NC_USHORT: sz = sizeof(unsigned short); break;
    case NC_UINT: sz = sizeof(unsigned int); break;
    case NC_UINT64: sz = sizeof(unsigned long long); break;
#ifdef USE_NETCDF4
    case NC_STRING: sz = sizeof(char*); break;
#endif
    default: break;
    }	
    return sz;
}

char*
NC_atomictypename(nc_type xtype)
{
    char* nm = NULL;
    switch(xtype) {
    case NC_NAT: nm = "undefined"; break;
    case NC_BYTE: nm = "byte"; break;
    case NC_CHAR: nm = "char"; break;
    case NC_SHORT: nm = "short"; break;
    case NC_INT: nm = "int"; break;
    case NC_FLOAT: nm = "float"; break;
    case NC_DOUBLE: nm = "double"; break;
    case NC_INT64: nm = "int64"; break;
    case NC_UBYTE: nm = "ubyte"; break;
    case NC_USHORT: nm = "ushort"; break;
    case NC_UINT: nm = "uint"; break;
    case NC_UINT64: nm = "uint64"; break;
#ifdef USE_NETCDF4
    case NC_STRING: nm = "string"; break;
#endif
    default: break;
    }	
    return nm;
}

/* Overlay by treating the tables as arrays of void*.
   Overlay rules are:
        overlay    base    merge
        -------    ----    -----
          null     null     null
          null      y        y
           x       null      x
           x        y        x
*/

int
NC_dispatch_overlay(const NC_Dispatch* overlay, const NC_Dispatch* base, NC_Dispatch* merge)
{
    void** voverlay = (void**)overlay;
    void** vmerge;
    int i, count = sizeof(NC_Dispatch) / sizeof(void*);
    /* dispatch table must be exact multiple of sizeof(void*) */
    assert(count * sizeof(void*) == sizeof(NC_Dispatch));
    *merge = *base;
    vmerge = (void**)merge;
    for(i=0;i<count;i++) {
        if(voverlay[i] == NULL) continue;
        vmerge[i] = voverlay[i];
    }
    /* Finally, the merge model should always be the overlay model */
    merge->model = overlay->model;
    return NC_NOERR;
}

/**************************************************/

/* Define a type equality fcn that used recursion rather than
   e.g H5Tequal.
*/
/* Duplicate (sort-of) code from nc4attr.c */
/* Compare two netcdf types for equality. Must have the ncids as well,
   to find nested types. */
#ifdef USE_NETCDF4
static int
NC_compare_nc_types(int ncid1, int typeid1,
                 int ncid2, int typeid2, int *equalp)
{
    int ret = NC_NOERR;
    if(equalp == NULL) return NC_NOERR;

    /* Assume the types are not equal. If we find any inequality, then
       exit with NC_NOERR and we're done. */
    *equalp = 0;

    /* Atomic types are so easy! */
    if (typeid1 <= ATOMICTYPEMAX) {
	if (typeid2 != typeid1) return NC_NOERR;
	*equalp = 1;
    }
    else {
	int i, ret, equal1;
	char name1[NC_MAX_NAME];
	char name2[NC_MAX_NAME];
	size_t size1, size2;
	nc_type base1, base2;
	size_t nelems1, nelems2;
	int class1, class2;
	void* value1 = NULL;
	void* value2 = NULL;
	size_t offset1, offset2;
	nc_type ftype1, ftype2;
	int ndims1, ndims2;
	int dimsizes1[NC_MAX_VAR_DIMS];
	int dimsizes2[NC_MAX_VAR_DIMS];

	ret = nc_inq_user_type(ncid1,typeid1,name1,&size1,&base1,&nelems1,&class1);
	if(ret) return ret;      
	ret = nc_inq_user_type(ncid2,typeid2,name2,&size2,&base2,&nelems2,&class2);
	if(ret) return ret;      

        /* Check the obvious. */
        if(size1 != size2 || class1 != class2 || strcmp(name1,name2))
	    return NC_NOERR;

        /* Check user-defined types in detail. */
        switch(class1) {
	case NC_VLEN:
	    if(base1 <= NC_STRING) {
	        if(base1 != base2) return NC_NOERR;
	    } else {
		/* User defined type in VLEN! */
		if((ret = NC_compare_nc_types(ncid1,base1,ncid2,base1,&equal1)))
		    return ret;
	        if(!equal1) return NC_NOERR;
	    }
	    break;
	case NC_OPAQUE:
	    /* Already checked size above. */
	    break;
	case NC_ENUM:
	    if(base1 != base2 || nelems1 != nelems2) return NC_NOERR;
	    value1 = malloc(size1);
	    value2 = malloc(size2);
	    for(i=0;i<nelems1;i++) {
	        ret = nc_inq_enum_member(ncid1,typeid1,i,name1,value1);
	        if(ret) return ret;
	        ret = nc_inq_enum_member(ncid2,typeid2,i,name2,value2);
	        if(ret) goto enumdone;
		if(strcmp(name1,name2) != 0
		   || memcmp(value1,value2,size1) != 0)
		    return NC_NOERR;
	    }
enumdone:
	    free(value1); free(value2);
	    break;
	case NC_COMPOUND:
	    if(nelems1 != nelems2) return NC_NOERR;
	    /* Compare each field. Each must be equal! */
	    for(i=0;i<nelems1;i++) {
		int j;
	        ret = nc_inq_compound_field(ncid1,typeid1,i,name1,&offset1,&ftype1,&ndims1,dimsizes1);
	        if(ret) return ret;
	        ret = nc_inq_compound_field(ncid2,typeid2,i,name2,&offset2,&ftype2,&ndims2,dimsizes2);
	        if(ret) return ret;
	        if(ndims1 != ndims2) return NC_NOERR;
		for(j=0;j<ndims1;j++) {
		    if(dimsizes1[j] != dimsizes2[j]) return NC_NOERR;
		}		
	        /* Handle atomic types. */
	        if(ftype1 <= NC_STRING) {
		    if(ftype1 != ftype2) return NC_NOERR;
	        } else { /* Dang! *More* user-defined types!
                            Look up the field types in each file. */
	            /* Compare user-defined field types. */
	            if((ret = NC_compare_nc_types(ncid1,ftype1,ncid2,ftype2,&equal1)))
			return ret;
		    if(!equal1) return NC_NOERR;
		}
	    }
	    break;
	default:
	    return NC_EINVAL;
	}
        *equalp = 1;
    }
    return ret;
}
#endif /*USE_NETCDF4*/

#ifdef USE_NETCDF4
/* Recursively hunt for a netCDF type id. (code from nc4internal.c);
   return matching typeid or 0 if not found
*/
static int
NC_rec_find_nc_type(int ncid1, nc_type tid1, int ncid2, nc_type* tid2)
{
    int i,ret = NC_NOERR;
    int nids;
    int* ids = NULL;
    /* Get all types in grp ncid2 */
    if(tid2) *tid2 = 0;
    ret = nc_inq_typeids(ncid2,&nids,NULL);
    if(ret) return ret;
    ids = (int*)malloc(nids*sizeof(int));
    if(ids == NULL) return NC_ENOMEM;
    ret = nc_inq_typeids(ncid2,&nids,ids);
    if(ret) return ret;
    for(i=0;i<nids;i++) {
	int equal = 0;
	ret = NC_compare_nc_types(ncid1,tid1,ncid2,ids[i],&equal);
	if(equal) {if(tid2) *tid2 = ids[i]; return NC_NOERR;}
    }
    free(ids);

    /* recurse */
    ret = nc_inq_grps(ncid1,&nids,NULL);
    if(ret) return ret;
    ids = (int*)malloc(nids*sizeof(int));
    if(ids == NULL) return NC_ENOMEM;
    ret = nc_inq_grps(ncid1,&nids,ids);
    if(ret) return ret;
    for(i=0;i<nids;i++) {
	ret = NC_rec_find_nc_type(ncid1, tid1, ids[i], tid2);
	if(ret && ret != NC_EBADTYPE) break;
	if(tid2 && *tid2 != 0) break; /* found */
    }
    free(ids);
    return NC_EBADTYPE; /* not found */
}
#endif

/* Given a type in one file, find its equal (if any) in another
 * file. It sounds so simple, but it's a real pain! */
#ifdef USE_NETCDF4
static int
NC_find_equal_type(int ncid1, nc_type xtype1, int ncid2, nc_type *xtype2)
{
    int ret = NC_NOERR;
    /* Check input */
    if(xtype1 <= NC_NAT) return NC_EINVAL;
    /* Handle atomic types. */
    if (xtype1 <= ATOMICTYPEMAX) {
        if(xtype2) *xtype2 = xtype1;
	return NC_NOERR;
    }

    /* Recursively search group ncid2 and its children
       to find a type that is equal (using compare_type)
       to xtype1. */
    ret = NC_rec_find_nc_type(ncid1, xtype1 , ncid2, xtype2);
    return ret;
}
#endif

static int
NC_check_file_type(const char *path, int use_parallel, void* mpi_info,
		 int *cdf, int* hdf)
{
    char magic[MAGIC_NUMBER_LEN];
    
    *hdf = 0; *cdf = 0;

    /* Get the 4-byte magic from the beginning of the file. Don't use posix
    * for parallel, use the MPI functions instead. */
#ifdef USE_PARALLEL_MPIO
    if (use_parallel) {
        MPI_File fh;
        MPI_Status status;
        int retval;
	MPI_Comm comm = 0;
	MPI_Info info = 0;
	if(mpi_info != NULL) {
	    comm = ((NC_MPI_INFO*)mpi_info)->comm;
	    info = ((NC_MPI_INFO*)mpi_info)->info;
	}
        if((retval = MPI_File_open(comm, (char *)path, MPI_MODE_RDONLY,info, &fh)) != MPI_SUCCESS)
	    return NC_EPARINIT;
        if((retval = MPI_File_read(fh, magic, MAGIC_NUMBER_LEN, MPI_CHAR,&status)) != MPI_SUCCESS)
	    return NC_EPARINIT;
        if((retval = MPI_File_close(&fh)) != MPI_SUCCESS)
	    return NC_EPARINIT;
    } else
#endif /* USE_PARALLEL */
    {
        FILE *fp;
        int i;
        fp = fopen(path, "r");
	if(fp == NULL)
	    return errno;
	i = fread(magic, MAGIC_NUMBER_LEN, 1, fp);
	if(i != 1)
	    return errno;
        fclose(fp);
    }
    
    /* Ignore the first byte for HDF */
    if(magic[1] == 'H' && magic[2] == 'D' && magic[3] == 'F')
	*hdf = 5;
    else if(magic[0] == '\016' && magic[1] == '\003'
       && magic[2] == '\023' && magic[3] == '\001')
	*hdf = 4;
    else if(magic[0] == 'C' && magic[1] == 'D' && magic[2] == 'F') {
	if(magic[3] == '\001') *cdf = 1;
	else if(magic[3] == '\002') *cdf = 2;
    }
    
    return NC_NOERR;
}

int
nc_inq_var(int ncid, int varid, char *name, nc_type *xtypep,  int *ndimsp, int *dimidsp, int *nattsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_var_all(
	ncid, varid,
	name, /*name*/
	xtypep, /*xtypep*/
	ndimsp, /*ndimsp*/
	dimidsp, /*dimidsp*/
	nattsp, /*nattsp*/
	NULL, /*shufflep*/
	NULL, /*deflatep*/
	NULL, /*deflatelevelp*/
	NULL, /*fletcher32p*/
	NULL, /*contiguousp*/
	NULL, /*chunksizep*/
	NULL, /*nofillp*/
	NULL, /*fillvaluep*/
	NULL, /*endianp*/
	NULL, /*optionsmaskp*/
	NULL /*pixelsp*/
	);
}

#ifdef USE_NETCDF4


#endif /*USE_NETCDF4*/

/**************************************************/
/* Output type specific interface */

/* Public */


/**************************************************/

int
nc_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            const void *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    nc_type xtype;
    if(stat != NC_NOERR) return stat;
    stat = nc_inq_vartype(ncid, varid, &xtype);
    if(stat != NC_NOERR) return stat;
    return NC_put_vara(ncid,varid,start,edges,value,xtype);
}

int
nc_get_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            void *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    nc_type xtype;
    if(stat != NC_NOERR) return stat;
    stat = nc_inq_vartype(ncid, varid, &xtype);
    if(stat != NC_NOERR) return stat;
    return NC_get_vara(ncid,varid,start,edges,value,xtype);
}

int
nc_get_var(int ncid, int varid, void *value)
{
    return NC_get_var(ncid,varid,value,NC_NAT);
}

int
nc_put_var(int ncid, int varid, const void *value)
{
    return NC_put_var(ncid,varid,value,NC_NAT);
}

int
nc_get_var1(int ncid, int varid, const size_t *coord, void *value)
{
    return NC_get_var1(ncid,varid,coord,value,NC_NAT);
}

int
nc_put_var1(int ncid, int varid, const size_t *coord, const void *value)
{
    return NC_put_var1(ncid,varid,coord,value,NC_NAT);
}

/* This has a different signature than the others */
int
nc_put_att_text(int ncid, int varid, const char *name,
		size_t len, const char *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->put_att(ncid,varid,name,NC_CHAR,len,(void*)value,T_text);
}

#ifdef USE_NETCDF4
/* This has a different signature than the others */
int
nc_put_att_string(int ncid, int varid, const char *name,
		size_t len, const char** value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->put_att(ncid,varid,name,NC_STRING,len,(void*)value,T_string);
}
#endif

/* no longer deprecated, used to support the 2.x interface  and also the netcdf-4 api. */
int
nc_get_att(int ncid, int varid, const char *name, void *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    nc_type xtype;
    if(stat != NC_NOERR) return stat;
    /* Need to get the type */
    stat = nc_inq_atttype(ncid, varid, name, &xtype);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_att(ncid,varid,name,value,xtype);
}

int
nc_put_att(
	int ncid,
	int varid,
	const char *name,
	nc_type type,
	size_t nelems,
	const void *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->put_att(ncid,varid,name,type,nelems,value,type);
}

int
nc_get_varm(
	int ncid,
	int varid,
	const size_t * start,
	const size_t * edges,
	const ptrdiff_t * stride,
	const ptrdiff_t * imapp,
	void *value)
{
	return NC_get_varm (ncid, varid, start, edges, stride, imapp,
				value, NC_NAT);
}

int
nc_put_varm (
	int ncid,
	int varid,
	const size_t * start,
	const size_t * edges,
	const ptrdiff_t * stride,
	const ptrdiff_t * imapp,
	const void *value)
{
	return NC_put_varm (ncid, varid, start, edges, stride, imapp,
				value, NC_NAT);
}

int
nc_get_vars(
	int ncid,
	int varid,
	const size_t * start,
	const size_t * edges,
	const ptrdiff_t * stride,
	void *value)
{
	return nc_get_varm (ncid, varid, start, edges,
			 stride, NULL, value);
}

int
nc_put_vars (
	int ncid,
	int varid,
	const size_t * start,
	const size_t * edges,
	const ptrdiff_t * stride,
	const void *value)
{
	return nc_put_varm (ncid, varid, start, edges,
			 stride, NULL, value);
}

/**************************************************/

#ifdef USE_DAP

/* allow access dapurlparse and params while minimizing exposing dapurl.h */
int
NCDAP_urlparse(const char* s, void** dapurlp)
{
    DAPURL* dapurl = NULL;
    dapurl = calloc(1,sizeof(DAPURL));
    if(dapurl == 0) return NC_ENOMEM;
    if(!dapurlparse(s,dapurl)) {
	dapurlclear(dapurl);
	free(dapurl);
	return NC_EINVAL;
    }
    if(dapurlp) *dapurlp = dapurl;
    return NC_NOERR;
}

void
NCDAP_urlfree(void* durl)
{
    DAPURL* dapurl = (DAPURL*)durl;
    if(dapurl != NULL) {
	dapurlclear(dapurl);
	free(dapurl);
    }
}

const char*
NCDAP_urllookup(void* durl, const char* param)
{
    DAPURL* dapurl = (DAPURL*)durl;
    if(param == NULL || strlen(param) == 0 || dapurl == NULL) return NULL;
    return dapurllookup(dapurl,param);
}

#else /*!USE_DAP*/
int
NCDAP_urlparse(const char* s, void** dapurlp)
{
    return NC_EINVAL;
}

void
NCDAP_urlfree(void* durl)
{
    return;
}

const char*
NCDAP_urllookup(void* durl, const char* param)
{
    return NULL;
}

#endif /*!USE_DAP*/

/**************************************************/
/* M4 generated */
dnl
dnl NCGETVAR1(Abbrev, Type)
dnl
define(`NCGETVAR1',dnl
`dnl
int
nc_get_var1_$1(int ncid, int varid, const size_t *coord, $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    INITCOORD1;
    return NC_get_var1(ncid,varid,coord,(void*)value,T_$1);
}
')dnl
NCGETVAR1(text,char)
NCGETVAR1(schar,signed char)
NCGETVAR1(uchar,unsigned char)
NCGETVAR1(short,short)
NCGETVAR1(int,int)
NCGETVAR1(long,long)
dnl NCGETVAR1(ulong,ulong)
NCGETVAR1(float,float)
NCGETVAR1(double,double)
NCGETVAR1(ubyte,unsigned char)
NCGETVAR1(ushort,unsigned short)
NCGETVAR1(uint,unsigned int)
NCGETVAR1(longlong,long long)
NCGETVAR1(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVAR1(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCPUTVAR1(Abbrev, Type)
dnl
define(`NCPUTVAR1',dnl
`dnl
int
nc_put_var1_$1(int ncid, int varid, const size_t *coord, const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    INITCOORD1;
    return NC_put_var1(ncid,varid,coord,(void*)value,T_$1);
}
')dnl
NCPUTVAR1(text,char)
NCPUTVAR1(schar,signed char)
NCPUTVAR1(uchar,unsigned char)
NCPUTVAR1(short,short)
NCPUTVAR1(int,int)
NCPUTVAR1(long,long)
dnl NCPUTVAR1(ulong,ulong)
NCPUTVAR1(float,float)
NCPUTVAR1(double,double)
NCPUTVAR1(ubyte,unsigned char)
NCPUTVAR1(ushort,unsigned short)
NCPUTVAR1(uint,unsigned int)
NCPUTVAR1(longlong,long long)
NCPUTVAR1(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVAR1(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCGETVAR(Abbrev, Type)
dnl
define(`NCGETVAR',dnl
`dnl
int
nc_get_var_$1(int ncid, int varid, $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_get_var(ncid,varid,(void*)value,T_$1);
}
')dnl
NCGETVAR(text,char)
NCGETVAR(schar,signed char)
NCGETVAR(uchar,unsigned char)
NCGETVAR(short,short)
NCGETVAR(int,int)
NCGETVAR(long,long)
dnl NCGETVAR(ulong,ulong)
NCGETVAR(float,float)
NCGETVAR(double,double)
NCGETVAR(ubyte,unsigned char)
NCGETVAR(ushort,unsigned short)
NCGETVAR(uint,unsigned int)
NCGETVAR(longlong,long long)
NCGETVAR(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVAR(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCPUTVAR(Abbrev, Type)
dnl
define(`NCPUTVAR',dnl
`dnl
int
nc_put_var_$1(int ncid, int varid, const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_put_var(ncid,varid,(void*)value,T_$1);
}
')dnl
NCPUTVAR(text,char)
NCPUTVAR(schar,signed char)
NCPUTVAR(uchar,unsigned char)
NCPUTVAR(short,short)
NCPUTVAR(int,int)
NCPUTVAR(long,long)
dnl NCPUTVAR(ulong,ulong)
NCPUTVAR(float,float)
NCPUTVAR(double,double)
NCPUTVAR(ubyte,unsigned char)
NCPUTVAR(ushort,unsigned short)
NCPUTVAR(uint,unsigned int)
NCPUTVAR(longlong,long long)
NCPUTVAR(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVAR(string,char*)
#endif /*USE_NETCDF4*/


dnl
dnl NCPUTVARA(Abbrv, Type)
dnl
define(`NCPUTVARA',dnl
`dnl
int
nc_put_vara_$1(int ncid, int varid,
	 const size_t *start, const size_t *edges, const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_put_vara(ncid,varid,start,edges,(void*)value,T_$1);
}
')dnl
NCPUTVARA(text,char)
NCPUTVARA(schar,signed char)
NCPUTVARA(uchar,unsigned char)
NCPUTVARA(short,short)
NCPUTVARA(int,int)
NCPUTVARA(long,long)
dnl NCPUTVARA(ulong,ulong)
NCPUTVARA(float,float)
NCPUTVARA(double,double)
NCPUTVARA(ubyte,unsigned char)
NCPUTVARA(ushort,unsigned short)
NCPUTVARA(uint,unsigned int)
NCPUTVARA(longlong,long long)
NCPUTVARA(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVARA(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCGETVARA(Abbrv, Type)
dnl
define(`NCGETVARA',dnl
`dnl
int
nc_get_vara_$1(int ncid, int varid,
	 const size_t *start, const size_t *edges, $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_get_vara(ncid,varid,start,edges,(void*)value,T_$1);
}
')dnl
NCGETVARA(text,char)
NCGETVARA(schar,signed char)
NCGETVARA(uchar,unsigned char)
NCGETVARA(short,short)
NCGETVARA(int,int)
NCGETVARA(long,long)
dnl NCGETVARA(ulong,ulong)
NCGETVARA(float,float)
NCGETVARA(double,double)
NCGETVARA(ubyte,unsigned char)
NCGETVARA(ushort,unsigned short)
NCGETVARA(uint,unsigned int)
NCGETVARA(longlong,long long)
NCGETVARA(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVARA(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCPUTVARM(Abbrv, Type)
dnl
define(`NCPUTVARM',dnl
`dnl
int
nc_put_varm_$1(int ncid, int varid,
	    const size_t *start, const size_t *edges,
	    const ptrdiff_t * stride, const ptrdiff_t * imapp,
            const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_put_varm(ncid,varid,start,edges,stride,imapp,(void*)value,T_$1);
}
')dnl
NCPUTVARM(text,char)
NCPUTVARM(schar,signed char)
NCPUTVARM(uchar,unsigned char)
NCPUTVARM(short,short)
NCPUTVARM(int,int)
NCPUTVARM(long,long)
dnl NCPUTVARM(ulong,ulong)
NCPUTVARM(float,float)
NCPUTVARM(double,double)
NCPUTVARM(ubyte,unsigned char)
NCPUTVARM(ushort,unsigned short)
NCPUTVARM(uint,unsigned int)
NCPUTVARM(longlong,long long)
NCPUTVARM(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVARM(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCGETVARM(Abbrv, Type)
dnl
define(`NCGETVARM',dnl
`dnl
int
nc_get_varm_$1(int ncid, int varid,
	    const size_t *start, const size_t *edges,
	    const ptrdiff_t * stride, const ptrdiff_t * imapp,
            $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_get_varm(ncid,varid,start,edges,stride,imapp,(void*)value,T_$1);
}
')dnl
NCGETVARM(text,char)
NCGETVARM(schar,signed char)
NCGETVARM(uchar,unsigned char)
NCGETVARM(short,short)
NCGETVARM(int,int)
NCGETVARM(long,long)
dnl NCGETVARM(ulong,ulong)
NCGETVARM(float,float)
NCGETVARM(double,double)
NCGETVARM(ubyte,unsigned char)
NCGETVARM(ushort,unsigned short)
NCGETVARM(uint,unsigned int)
NCGETVARM(longlong,long long)
NCGETVARM(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVARM(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCPUTVARS(Abbrv, Type)
dnl
define(`NCPUTVARS',dnl
`dnl
int
nc_put_vars_$1(int ncid, int varid,
	    const size_t *start, const size_t *edges,
	    const ptrdiff_t * stride,
            const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_put_vars(ncid,varid,start,edges,stride,(void*)value,T_$1);
}
')dnl
NCPUTVARS(text,char)
NCPUTVARS(schar,signed char)
NCPUTVARS(uchar,unsigned char)
NCPUTVARS(short,short)
NCPUTVARS(int,int)
NCPUTVARS(long,long)
dnl NCPUTVARS(ulong,ulong)
NCPUTVARS(float,float)
NCPUTVARS(double,double)
NCPUTVARS(ubyte,unsigned char)
NCPUTVARS(ushort,unsigned short)
NCPUTVARS(uint,unsigned int)
NCPUTVARS(longlong,long long)
NCPUTVARS(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVARS(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCGETVARS(Abbrv, Type)
dnl
define(`NCGETVARS',dnl
`dnl
int
nc_get_vars_$1(int ncid, int varid,
	    const size_t *start, const size_t *edges,
	    const ptrdiff_t * stride,
            $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_get_vars(ncid,varid,start,edges,stride,(void*)value,T_$1);
}
')dnl
NCGETVARS(text,char)
NCGETVARS(schar,signed char)
NCGETVARS(uchar,unsigned char)
NCGETVARS(short,short)
NCGETVARS(int,int)
NCGETVARS(long,long)
dnl NCGETVARS(ulong,ulong)
NCGETVARS(float,float)
NCGETVARS(double,double)
NCGETVARS(ubyte,unsigned char)
NCGETVARS(ushort,unsigned short)
NCGETVARS(uint,unsigned int)
NCGETVARS(longlong,long long)
NCGETVARS(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVARS(string,char*)
#endif /*USE_NETCDF4*/


dnl
dnl NC_GET_ATT(Abbrv, Type)
dnl
define(`NC_GET_ATT',dnl
`dnl
int
nc_get_att_$1(int ncid, int varid, const char *name, $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_att(ncid,varid,name,(void*)value,T_$1);
}
')dnl
NC_GET_ATT(text,char)
NC_GET_ATT(schar, signed char)
NC_GET_ATT(uchar, unsigned char)
NC_GET_ATT(short, short)
NC_GET_ATT(int, int)
NC_GET_ATT(long,long)
dnl NC_GET_ATT(ulong,ulong)
NC_GET_ATT(float, float)
NC_GET_ATT(double, double)
NC_GET_ATT(ubyte,unsigned char)
NC_GET_ATT(ushort,unsigned short)
NC_GET_ATT(uint,unsigned int)
NC_GET_ATT(longlong,long long)
NC_GET_ATT(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NC_GET_ATT(string,char*)
#endif /*USE_NETCDF4*/


dnl
dnl NC_PUT_ATT(Abbrv, Type)
dnl
define(`NC_PUT_ATT',dnl
`dnl
int
nc_put_att_$1(int ncid, int varid, const char *name,
	nc_type type, size_t nelems, const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->put_att(ncid,varid,name,type,nelems,(void*)value,T_$1);
}
')dnl
NC_PUT_ATT(schar, signed char)
NC_PUT_ATT(uchar, unsigned char)
NC_PUT_ATT(short, short)
NC_PUT_ATT(int, int)
NC_PUT_ATT(long,long)
dnl NC_PUT_ATT(ulong,ulong)
NC_PUT_ATT(float, float)
NC_PUT_ATT(double, double)
NC_PUT_ATT(ubyte,unsigned char)
NC_PUT_ATT(ushort,unsigned short)
NC_PUT_ATT(uint,unsigned int)
NC_PUT_ATT(longlong,long long)
NC_PUT_ATT(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
/*NC_PUT_ATT(string,char*) defined separately */
#endif /*USE_NETCDF4*/

