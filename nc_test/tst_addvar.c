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


//#define ERR {if(err!=NC_NOERR){printf("Error at line=%d: %s\n", __LINE__, nc_strerror(err));}}

int createfile() {
  int i, j, rank, nprocs, ncid, cmode, varid[NVARS], dimid[2], *buf;
  int err = 0;
  size_t start[2], count[2];
  char str[32];

  cmode = NC_CLOBBER;
  if (nc_create(FILENAME, cmode, &ncid)) ERR_RET;

  /* define dimension */
  if (nc_def_dim(ncid, "Y", NC_UNLIMITED, &dimid[0])) ERR;
  if (nc_def_dim(ncid, "X", NX,           &dimid[1])) ERR;

  /* Odd numbers are fixed variables, even numbers are record variables */
  for (i=0; i<NVARS; i++) {
    if (i%2) {
      sprintf(str,"fixed_var_%d",i);
      if (nc_def_var(ncid, str, NC_INT, 1, dimid+1, &varid[i])) ERR;
    }
    else {
      sprintf(str,"record_var_%d",i);
      if (nc_def_var(ncid, str, NC_INT, 2, dimid, &varid[i])) ERR;
    }
  }
  if (nc_enddef(ncid)) ERR;

  /* write all variables */
  buf = (int*) malloc(NX * sizeof(int));
  for (i=0; i<NVARS; i++) {
    for (j=0; j<NX; j++) buf[j] = i*10 + j;
    if (i%2) {
      start[0] = 0; count[0] = NX;
      if (nc_put_vara_int(ncid, varid[i], start, count, buf)) ERR;
    }
    else {
      start[0] = 0; start[1] = 0;
      count[0] = 1; count[1] = NX;
      if (nc_put_vara_int(ncid, varid[i], start, count, buf)) ERR;
    }
  }

  if (nc_close(ncid)) ERR;
}

int main(int argc, char** argv) {
    int err, ncid, varid[2], dimid[2];

    printf("\n*** Testing bug fix for adding a new variable to an existing file.\n");

    printf("***** Reopening file...\t");

    err = nc_open(FILENAME, NC_WRITE, &ncid); ERR;
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

    return 0;
}
