/* This is part of the netCDF package.
   Copyright 2017 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test PIO parallel IO library use.
   @author Ed Hartnett
*/

#include <config.h>
#include <stdlib.h>
#include <nc_tests.h>
#include "err_macros.h"

/* #define FILE_NAME "tst_pio.nc" */
/* #define S1_PACKED_NAME "s1_packed_compound_type_with_boring_name" */
/* #define S1_NAME "s1_compound_type_unwhimsiclaly_named" */
/* #define I_NAME "i_of_little_quirkiness" */
/* #define J_NAME "j_with_no_originality" */
/* #define DIM_NAME "intentionally_unimaginatevely_named_dimension" */
/* #define DIM_LEN 1 */
/* #define VAR_NAME "deliberately_boring_variable" */

/* typedef struct g1_c_t { */
/*    float x; */
/*    double y; */
/* } g1_c_t; */
/* typedef struct g2_d_t { */
/*    g1_c_t s1; */
/* } g2_d_t; */

int
main(int argc, char **argv)
{
   printf("\n*** Testing PIO parallel IO library use.\n");
   printf("*** testing file create...");
   {
      /* int ncid, typeid, varid; */
      /* size_t nfields; */
      /* int dimid; */
      /* int ndims, nvars, natts, unlimdimid; */
      /* char name_in[NC_MAX_NAME + 1]; */
      /* size_t size; */
      /* nc_type xtype, field_xtype; */
      /* int dimids[] = {0}; */
      /* int field_ndims, field_sizes[NC_TESTS_MAX_DIMS]; */
      /* size_t offset; */
      /* int i; */

      /* struct s1_packed */
      /* { */
      /*    short i; */
      /*    long long j; */
      /* }; */
      /* /\* This packing extension works with GNU compilers... *\/ */
      /* /\* } __attribute__ ((__packed__));*\/ */
      /* struct s1 */
      /* { */
      /*    short i; */
      /*    long long j; */
      /* }; */
      /* struct s1_packed *data; */
      /* struct s1 *data_in; */

      /* if (!(data = calloc(sizeof(struct s1_packed), DIM_LEN))) ERR; */
      /* if (!(data_in = calloc(sizeof(struct s1), DIM_LEN))) ERR; */

      /* /\* Create some phony data. *\/ */
      /* for (i = 0; i < DIM_LEN; i++) */
      /* { */
      /*    data[i].i = 100; */
      /*    data[i].j = 1000000000000LL; */
      /* } */

      /* /\* Create a file with a compound type. Write a little data. *\/ */
      /* if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */
      /* if (nc_def_compound(ncid, sizeof(struct s1_packed), S1_PACKED_NAME, &typeid)) ERR; */
      /* if (nc_inq_compound(ncid, typeid, name_in, &size, &nfields)) ERR; */
      /* if (size != sizeof(struct s1_packed) || strcmp(name_in, S1_PACKED_NAME) || nfields) ERR; */
      /* if (nc_insert_compound(ncid, typeid, I_NAME, NC_COMPOUND_OFFSET(struct s1_packed, i), */
      /*   		     NC_SHORT)) ERR; */
      /* if (nc_insert_compound(ncid, typeid, J_NAME, NC_COMPOUND_OFFSET(struct s1_packed, j), */
      /*   		     NC_INT64)) ERR; */
      /* if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR; */
      /* if (nc_def_var(ncid, VAR_NAME, typeid, 1, dimids, &varid)) ERR; */
      /* if (nc_put_var(ncid, varid, data)) ERR; */
      /* if (nc_close(ncid)) ERR; */

      /* /\* Open the file and take a peek. *\/ */
      /* if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR; */
      /* if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR; */
      /* if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR; */

      /* /\* Check the var and its type. *\/ */
      /* if (nc_inq_var(ncid, 0, name_in, &xtype, &ndims, dimids, &natts)) ERR; */
      /* if (strcmp(name_in, VAR_NAME) || ndims != 1 || natts != 0 || dimids[0] != 0) ERR; */
      /* if (nc_inq_compound(ncid, xtype, name_in, &size, &nfields)) ERR; */
      /* if (nfields != 2 || size != sizeof(struct s1) || strcmp(name_in, S1_PACKED_NAME)) ERR; */
      /* if (nc_inq_compound_field(ncid, xtype, 0, name_in, &offset, &field_xtype, &field_ndims, field_sizes)) ERR; */
      /* if (field_ndims) ERR; */
      /* if (strcmp(name_in, I_NAME) || offset != NC_COMPOUND_OFFSET(struct s1, i) || */
      /*     (field_xtype != NC_SHORT || field_ndims != 0)) ERR; */
      /* if (nc_inq_compound_field(ncid, xtype, 1, name_in, &offset, &field_xtype, &field_ndims, */
      /*   			field_sizes)) ERR; */
      /* if (strcmp(name_in, J_NAME) || offset != NC_COMPOUND_OFFSET(struct s1, j) || */
      /*     field_xtype != NC_INT64) ERR; */

      /* /\* Now check the data. *\/ */
      /* if (nc_get_var(ncid, varid, data_in)) ERR; */
      /* for (i = 0; i < DIM_LEN; i++) */
      /*    if (data[i].i != data_in[i].i || data[i].j != data_in[i].j) ERR; */
      /* if (nc_close(ncid)) ERR; */

      /* free(data); */
      /* free(data_in); */
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
