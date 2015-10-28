/* This program adds two new variables to an existing netCDF file.
 * It is used to test if netCDF can correctly calculate the file offsets
 * for the two new variables, in particular for files that align the
 * fix-size variables to a boundary larger than 4 bytes, for instance
 * a file created by PnetCDF with defaut alignment of 512 bytes.
 *
 * To check the file offsets for all variables, adding a printf statement is
 * necessary in netcdf-4.3.3.1/libsrc/nc3internal.c around line 222,
 * for example,
 * printf("var %s: begin=%ld len=%ld end=%ld\n",(*vpp)->name->cp, (*vpp)->begin, (*vpp)->len, (*vpp)->begin+(*vpp)->len);
 *
 * Check the first new variable's begin to see whether it is less than
 * the last variable's offset in the existing file.
 */
#include <nc_tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILENAME "tst_pnetcdf.nc"

#define NVARS 6
#define NX 5


int main(int argc, char** argv) {
  int err, ncid, varid[2], dimid[2], rank, nprocs;
  MPI_Comm comm=MPI_COMM_SELF;
  MPI_Info info=MPI_INFO_NULL;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("\n*** Testing bug fix for adding a new variable to an existing file.\n");

    printf("***** Reopening file...\t");

    err = nc_open_par(FILENAME, NC_WRITE|NC_PNETCDF, comm, info &ncid); ERR;
    if(err) {

    }
    printf("success.\n");

    printf("***** Entering redefinition mode...\t");
    err = nc_redef(ncid); ERR;
    printf("success.\n");

    printf("***** Running tests...\t");
    /* add 2 new dimensions */
    err = nc_def_dim(ncid, "new_dim_1", 5, &dimid[0]); ERR;
    err = nc_def_dim(ncid, "new_dim_2", 4, &dimid[1]); ERR;

    /* add 2 new dimensions */
    err = nc_def_var(ncid, "new_var1", NC_INT,   2, dimid, &varid[0]); ERR;
    err = nc_def_var(ncid, "new_var2", NC_FLOAT, 2, dimid, &varid[1]); ERR;
    err = nc_enddef(ncid); ERR;

    err = nc_close(ncid); ERR;

    printf("success.\n");

 fn_exit:

    MPI_Finalize();
    SUMMARIZE_ERR;
    FINAL_RESULTS;

    return 0;


}
