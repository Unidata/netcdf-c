/* This is part of the netCDF package. Copyright 2018 University
   Corporation for Atmospheric Research/Unidata.  See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a chunkable test file for nccopy to test chunking.
*/

#include "ut_includes.h"
#include "test_nczarr_utils.h"

#define DEBUG

static int ret = NC_NOERR;
#define FILE_NAME "tst_chunks3.nc"

#define VAR_RANK 7
#define IVAR_NAME "ivar"
#define FVAR_NAME "fvar"
#define NVALS 45360		/* 7 * 4 * 2 * 3 * 5 * 6 * 9 */

/* Make trackable ERR macro replacement */
static int lerr(int stat, const char* file, int lineno) {
    fflush(stdout); /* Make sure our stdout is synced with stderr. */
    err++;
    fprintf(stderr, "Sorry! Unexpected result(%d), %s, line: %d\n",ret,file,lineno);
    fflush(stderr);                                             \
    return 2;                                                   \
}
#define LERR lerr(ret,__FILE__,__LINE__)

static const char *dim_names[VAR_RANK] = {"dim0", "dim1", "dim2", "dim3", "dim4", "dim5", "dim6"};
static const size_t dim_lens[VAR_RANK] = {7, 4, 2, 3, 5, 6, 9};

int
main(int argc, char** argv)
{
    /* file metadata */
    int mode = NC_CLOBBER|NC_NETCDF4;
    int ncid;
    int ivarid, fvarid;
    int ivar_dims[VAR_RANK];
    int fvar_dims[VAR_RANK];
    int ivar_data[NVALS];
    float fvar_data[NVALS];
    int r, i;
    char* file_name = FILE_NAME;

    printf("*** Creating chunkable test file %s...\n", file_name);

    if ((ret=nc_create(FILE_NAME, mode, &ncid))) LERR;
    for(r = 0; r < VAR_RANK; r++) {
	if ((ret=nc_def_dim(ncid, dim_names[r], dim_lens[r], &ivar_dims[r]))) LERR;
	fvar_dims[VAR_RANK - 1 - r] = ivar_dims[r];
    }
    if ((ret=nc_def_var(ncid, IVAR_NAME, NC_INT, VAR_RANK, ivar_dims, &ivarid))) LERR;

    /* fvar is unchanged */
    if ((ret=nc_def_var(ncid, FVAR_NAME, NC_FLOAT, VAR_RANK, fvar_dims, &fvarid))) LERR;
    if ((ret=nc_enddef (ncid))) LERR;

    /* Fill in the data */
    for(i=0; i < NVALS; i++) {
	ivar_data[i] = i;
    }
    if ((ret=nc_put_var(ncid, ivarid, ivar_data))) LERR;

    /* fvar is unchanged */
    for(i=0; i < NVALS; i++) {
        fvar_data[i] = NVALS - i;
    }
    if ((ret=nc_put_var(ncid, fvarid, fvar_data))) LERR;

    if ((ret=nc_close(ncid))) LERR;

    SUMMARIZE_ERR;

}
