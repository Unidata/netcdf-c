/* This is part of the netCDF package.  Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program excersizes HDF5 variable length array code.

   $Id: tst_h_vl2.c,v 1.5 2010/06/01 15:34:52 ed Exp $
*/
#include <nc_tests.h>
#include <hdf5.h>
#include <nc_logging.h>

#define FILE_NAME "tst_gh221.nc"
#define DIM_LEN 3;
#define DIM_NAME "x"
#define VLNE_NAME "vltest"
#define VAR_NAME1 "v"
#define VAR_NAME2 "w"

int main() {

  printf("Testing access to unset entrines in VLEN variable\n");
  {
    int ncid, typeid, dimid;
    nc_vlen_t data[DIM_LEN], data_in[DIM_LEN];

    size_t size_in;
    nc_type base_nc_type_in;
    int *phony, class_in;
    size_t len_in;
    int i, j;

    /* Create phony data. */
    for (i=0; i<DIM_LEN; i++) {
        if (!(phony = malloc(sizeof(int) * (i+1))))
          return NC_ENOMEM;
        for (j = 0; j < i + 1; j++)
          phony[j] = PHONY_VAL;
        data[i].p = phony;
        data[i].len = i+1;
      }

    /* Create File */
    if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

    /* Create Dimension */
    if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;

    /* Create ragged array type. */
    if (nc_def_vlen(ncid, VLEN_NAME, NC_FLOAT, &typeid)) ERR;

    /* Create a variable of typeid. */
    if (nc_def_var(ncid, VAR_NAME1, typeid, 1, &dimid, &varid);

    /* Close File. */
    if (nc_close(ncid)) ERR;



}
