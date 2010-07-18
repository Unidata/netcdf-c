/*
 Copyright 2010 University Corporation for Atmospheric
 Research/Unidata. See COPYRIGHT file for more info.

 This file defines the file create and open functions.

 "$Id: nc4.c,v 1.1 2010/06/01 15:46:50 ed Exp $" 
*/

#include "dispatch.h"

static int nc_initialized = 0;

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

    if(!nc_initialized)
	{stat = NC_initialize(); if(stat) return stat; nc_initialized = 1;}

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

    if(!nc_initialized)
	{stat = NC_initialize(); if(stat) return stat; nc_initialized = 1;}

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


