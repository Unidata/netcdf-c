/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 alignment
   Dennis Heimbigner
*/

#include <nc_tests.h>
#include "err_macros.h"

#include <hdf5.h>
#include <H5DSpublic.h>

#undef DEBUG

#define THRESHOLD 512
#define ALIGNMENT 4096
#define CHUNKSIZE 513

int
main(int argc, char **argv)
{
    int i,ncid, varid, dimids[1];
    size_t chunks[1];
    unsigned char data[CHUNKSIZE];
    hid_t fileid, grpid, datasetid;
    hid_t dxpl_id = H5P_DEFAULT; /*data transfer property list */
    unsigned int filter_mask = 0;
    hsize_t hoffset[1];
    haddr_t addr;
    hsize_t size ;
    hid_t fspace;

    H5Eset_auto2(H5E_DEFAULT,(H5E_auto2_t)H5Eprint1,stderr);

    printf("\n*** Testing HDF5 alignment.\n");

    printf("chunksize=%d threshold=%d alignment=%d\n",CHUNKSIZE,THRESHOLD,ALIGNMENT);

    if(nc_set_alignment(THRESHOLD,ALIGNMENT)) ERR;
    if (nc_create("tst_alignment.nc", NC_NETCDF4, &ncid)) ERR;
    if (nc_def_dim(ncid, "d0", CHUNKSIZE, &dimids[0])) ERR;
    if (nc_def_var(ncid, "var", NC_UBYTE, 1, dimids, &varid)) ERR;
    chunks[0] = CHUNKSIZE;
    if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunks)) ERR;
    if (nc_enddef(ncid)) ERR;     

    for(i=0;i<CHUNKSIZE;i++) data[i] = (i)%2;
    if(nc_put_var(ncid,varid,data)) ERR;

    if (nc_close(ncid)) ERR;

    /* Use HDF5 API to verify */

    if ((fileid = H5Fopen("tst_alignment.nc", H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
    if ((grpid = H5Gopen1(fileid, "/")) < 0) ERR;
    if ((datasetid = H5Dopen1(grpid, "var")) < 0) ERR;

    /* Test the offset */
    if((fspace = H5Dget_space(datasetid)) < 0) ERR;
    if(H5Dget_chunk_info(datasetid, fspace, 0,	hoffset, &filter_mask, &addr, &size) < 0) ERR;
#ifdef DEBUG
    fprintf(stderr,"H5Dget_chunk_info: offset=%lu addr=%lu size=%lu\n",(unsigned long)hoffset[0], (unsigned long)addr, (unsigned long)size);
    fprintf(stderr,"\t%% alignment: offset=%lu, addr=%lu\n",
        (((unsigned long)hoffset[0]) % ALIGNMENT),
        (((unsigned long)addr) % ALIGNMENT)
	);
#endif

    printf("H5Dget_chunk_info: addr=%lu (addr %% alignment)=%lu\n", (unsigned long)addr, (((unsigned long)hoffset[0]) % ALIGNMENT));
    if((addr % ALIGNMENT) != 0) ERR;

    /* Test chunk content */
    memset(data,0,sizeof(data));
    hoffset[0] = 0;

    if(H5Dread_chunk(datasetid, dxpl_id, hoffset, &filter_mask, data) < 0) ERR;

#ifdef DEBUG
    fprintf(stderr,"H5Dread_chunk: offset=%lu   offset %% alignment=%lu\n",(unsigned long)hoffset[0], (((unsigned long)hoffset) % ALIGNMENT));
#endif
    for(i=0;i<CHUNKSIZE;i++) {
        if(data[i] != (i)%2) {
	    fprintf(stderr,"data[%d] mismatch\n",i);
	    ERR;
	}
    }    

    if (H5Dclose(datasetid) < 0) ERR;
    if (H5Gclose(grpid) < 0) ERR;
    if (H5Fclose(fileid) < 0) ERR;

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
