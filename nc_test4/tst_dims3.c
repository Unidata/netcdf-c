/* This is part of the netCDF package. Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 dimensions inheritance, and dims with and without
   coordinate variables.

   Ed Hartnett
*/

#include <config.h>
#include <nc_tests.h>
#include "err_macros.h"

#define NDIM1 1
#define NDIM2 2
#define DATALEN2 2
#define VAR_NAME "var"
#define DIM_NAME1 "y"
#define DIM_NAME2 "x"
#define NY 3
#define NX 3
#define DIM_LEN 2
#define FILE_NAME "tst_dims3.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 dimensions even more.\n");
   printf("*** testing netcdf-4 dimension inheritance...");
   {
#define RANK_time 1
#define GRP_NAME  "G"
#define GRP2_NAME "G2"
#define TIME_NAME "time"
#define VAR2_NAME "z"
#define TIME_RANK 1
#define NUM_TIMES 2
#define LEV_NAME "level"
#define VRT_NAME "vert_number"
#define LEV_NUM  3
#define LEV_RANK 1
#define VRT_RANK 1
#define VAR2_RANK 2
#define NUM_VRT 3
      int ncid, grpid;
      int time_dim, time_dim_in;
      int time_var, z_var;
      size_t len;
      int time_data[NUM_TIMES] = {1, 2} ;
      size_t time_startset[TIME_RANK] = {0} ;
      size_t time_countset[TIME_RANK] = {NUM_TIMES} ;

      /* Create file with unlimited dim and associated coordinate
       * variable in root group, another variable that uses unlimited
       * dim in subgroup. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, GRP_NAME, &grpid)) ERR;
      if (nc_def_dim(ncid, TIME_NAME, NC_UNLIMITED, &time_dim)) ERR;
      if (nc_def_var(ncid, TIME_NAME, NC_INT, TIME_RANK, &time_dim,
		     &time_var)) ERR;
      if (nc_def_var(grpid, VAR2_NAME, NC_INT, TIME_RANK, &time_dim,
		     &z_var)) ERR;
      if (nc_enddef(ncid)) ERR;

      /* Assign data to time variable, creating two times */
      if (nc_put_vara(ncid, time_dim, time_startset, time_countset,
		      time_data)) ERR;

      /* Check the dim len from the root group */
      if (nc_inq_dimlen(ncid, time_dim, &len)) ERR;
      if (len != NUM_TIMES) ERR;

      /* Check the dim len from the sub group */
      if (nc_inq_dimlen(grpid, time_dim, &len)) ERR;
      if (len != NUM_TIMES) ERR;
      if (nc_close(ncid)) ERR;

      /* Now check how many times there are from the subgroup */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_ncid(ncid, GRP_NAME, &grpid)) ERR;
      if (nc_inq_dimid(ncid, TIME_NAME, &time_dim)) ERR;

      /* Check the dim len from the root group */
      if (nc_inq_dimlen(ncid, time_dim, &len)) ERR;
      if (len != NUM_TIMES) ERR;

      /* Check the dim len from the sub group */
      if (nc_inq_dimlen(grpid, time_dim, &len)) ERR;
      if (len != NUM_TIMES) ERR;

      /* Find the dimension by name. */
      if (nc_inq_dimid(grpid, TIME_NAME, &time_dim_in)) ERR;
      if (time_dim_in != time_dim) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing a scalar coordinate dimension...");
   {
      int ncid, dimid, varid;
      float data = 42.5;

      /* Create a scalar coordinate dimension. The only reason that
       * the user can ever possibly have for doing this is just
       * because they like to make life difficult for poor, poor
       * netCDF programmers, trapped in this horrible place, in a
       * Rocky Mountain valley, drenched in sunlight, with a stream
       * quietly gurgling, deer feeding on the grasses, and all those
       * damn birds chirping! */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR_RET;
      if (nc_def_dim(ncid, "scalar", 0, &dimid)) ERR_RET;
      if (nc_def_var(ncid, "scalar", NC_FLOAT, 0, &dimid, &varid)) ERR_RET;
      if (nc_put_var_float(ncid, varid, &data)) ERR_RET;
      if (nc_close(ncid))
	ERR_RET;
   }
   SUMMARIZE_ERR;
   printf("*** testing defining dimensions and coord variables in different orders in root group...");
   {
       int ncid, grpid, grp2id;
       int time_dimid, lev_dimid, g2lev_dimid, g2vrt_dimid;
       int time_dimid_in, lev_dimid_in, g2lev_dimid_in, g2vrt_dimid_in;
       int time_varid, lev_varid, gvar2_varid, g2lev_varid, g2vrt_varid;
       int var2_dims[VAR2_RANK];
      /* Create test for fix of bug that resulted in two dimensions
       * having the same dimid, which violates the Pauli exclusion
       * principle for dimensions. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR_RET;
      if (nc_def_grp(ncid, GRP_NAME, &grpid)) ERR;
      if (nc_def_dim(ncid, TIME_NAME, NC_UNLIMITED, &time_dimid)) ERR_RET;
      if (nc_def_dim(ncid, LEV_NAME, LEV_NUM, &lev_dimid)) ERR_RET;
      var2_dims[0] = time_dimid;
      var2_dims[1] = lev_dimid;
      if (nc_def_var(grpid, VAR2_NAME, NC_FLOAT, VAR2_RANK, var2_dims, &gvar2_varid)) ERR;
      /* define coord vars in opposite order of coord dims */
      if (nc_def_var(ncid, LEV_NAME, NC_FLOAT, LEV_RANK, &lev_dimid, &lev_varid)) ERR;
      if (nc_def_var(ncid, TIME_NAME, NC_FLOAT, TIME_RANK, &time_dimid, &time_varid)) ERR;

      if (nc_def_grp(ncid, GRP2_NAME, &grp2id)) ERR;
      if (nc_def_dim(grp2id, LEV_NAME, LEV_NUM, &g2lev_dimid)) ERR_RET;
      if (nc_def_dim(grp2id, VRT_NAME, NUM_VRT, &g2vrt_dimid)) ERR_RET;
      if (nc_def_var(grp2id, LEV_NAME, NC_FLOAT, LEV_RANK, &g2lev_dimid, &g2lev_varid)) ERR;
      if (nc_def_var(grp2id, VRT_NAME, NC_FLOAT, VRT_RANK, &g2vrt_dimid, &g2vrt_varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Re-open, in which dimids may get reassigned */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dimid(ncid, TIME_NAME, &time_dimid_in)) ERR;
      if (nc_inq_dimid(ncid, LEV_NAME, &lev_dimid_in)) ERR;
      if (nc_inq_ncid(ncid, GRP2_NAME, &grp2id)) ERR;
      if (nc_inq_dimid(grp2id, LEV_NAME, &g2lev_dimid_in)) ERR;
      if (nc_inq_dimid(grp2id, VRT_NAME, &g2vrt_dimid_in)) ERR;
      /* dimids must still all be distinct */
      if (time_dimid_in == lev_dimid_in ||
	  time_dimid_in == g2lev_dimid_in ||
	  time_dimid_in == g2vrt_dimid_in ||
	  lev_dimid_in == g2lev_dimid_in ||
	  lev_dimid_in == g2vrt_dimid_in ||
	  g2lev_dimid_in == g2vrt_dimid_in) ERR;

      if (nc_close(ncid))
	ERR_RET;
   }
   SUMMARIZE_ERR;
   printf("*** testing defining dimensions and coord variables in different orders in subgroup...");
   {
       int ncid, grpid, grp2id;
       int time_dimid, lev_dimid, g2lev_dimid, g2vrt_dimid;
       int time_dimid_in, lev_dimid_in, g2lev_dimid_in, g2vrt_dimid_in;
       int time_varid, lev_varid, gvar2_varid, g2lev_varid, g2vrt_varid;
       int var2_dims[VAR2_RANK];
      /* Create test for fix of bug inside a subgroup that results in two dimensions
       * having the same dimid. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR_RET;
      if (nc_def_grp(ncid, GRP_NAME, &grpid)) ERR;
      if (nc_def_dim(ncid, TIME_NAME, NC_UNLIMITED, &time_dimid)) ERR_RET;
      if (nc_def_dim(ncid, LEV_NAME, LEV_NUM, &lev_dimid)) ERR_RET;
      var2_dims[0] = time_dimid;
      var2_dims[1] = lev_dimid;
      if (nc_def_var(grpid, VAR2_NAME, NC_FLOAT, VAR2_RANK, var2_dims, &gvar2_varid)) ERR;
      if (nc_def_var(ncid, TIME_NAME, NC_FLOAT, TIME_RANK, &time_dimid, &time_varid)) ERR;
      if (nc_def_var(ncid, LEV_NAME, NC_FLOAT, LEV_RANK, &lev_dimid, &lev_varid)) ERR;

      if (nc_def_grp(ncid, GRP2_NAME, &grp2id)) ERR;
      if (nc_def_dim(grp2id, LEV_NAME, LEV_NUM, &g2lev_dimid)) ERR_RET;
      if (nc_def_dim(grp2id, VRT_NAME, NUM_VRT, &g2vrt_dimid)) ERR_RET;
      /* define coord vars in opposite order of coord dims */
      if (nc_def_var(grp2id, VRT_NAME, NC_FLOAT, VRT_RANK, &g2vrt_dimid, &g2vrt_varid)) ERR;
      if (nc_def_var(grp2id, LEV_NAME, NC_FLOAT, LEV_RANK, &g2lev_dimid, &g2lev_varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Re-open, in which dimids may get reassigned */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dimid(ncid, TIME_NAME, &time_dimid_in)) ERR;
      if (nc_inq_dimid(ncid, LEV_NAME, &lev_dimid_in)) ERR;
      if (nc_inq_ncid(ncid, GRP2_NAME, &grp2id)) ERR;
      if (nc_inq_dimid(grp2id, LEV_NAME, &g2lev_dimid_in)) ERR;
      if (nc_inq_dimid(grp2id, VRT_NAME, &g2vrt_dimid_in)) ERR;
      /* dimids must still all be distinct */
      if (time_dimid_in == lev_dimid_in ||
	  time_dimid_in == g2lev_dimid_in ||
	  time_dimid_in == g2vrt_dimid_in ||
	  lev_dimid_in == g2lev_dimid_in ||
	  lev_dimid_in == g2vrt_dimid_in ||
	  g2lev_dimid_in == g2vrt_dimid_in) ERR;

      if (nc_close(ncid))
	ERR_RET;
   }
   SUMMARIZE_ERR;
   printf("*** testing var and unlim dim with same name, but not related...");
   {
       /* This test code based on test code from Jeff Whitaker. See
        * https://github.com/Unidata/netcdf4-python/issues/975 and
        * https://github.com/Unidata/netcdf-c/issues/1496. */
       int ncid, timesubset_id, time_id, timevar_id, dummyvar_id;
       size_t start[1] = {0};
       size_t count[1] = {1};
       double data[1] = {TEST_VAL_42};
       size_t len;
       double data_in;

       if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
       if (nc_def_dim(ncid, "time", NC_UNLIMITED, &time_id)) ERR;
       if (nc_def_dim(ncid, "time_subset", 50, &timesubset_id)) ERR;

       /* Define vars. */
       if (nc_def_var(ncid, "time", NC_DOUBLE, 1, &timesubset_id, &timevar_id)) ERR;
       if (nc_def_var(ncid, "dummy", NC_DOUBLE, 1, &time_id, &dummyvar_id)) ERR;
       if (nc_enddef(ncid)) ERR;

       /* Write some data. */
       if (nc_put_vara(ncid, dummyvar_id, start, count, data)) ERR;

       /* Close the file. */
       if (nc_close(ncid)) ERR;

       /* Reopen file and check. */
       if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
       if (nc_inq_dim(ncid, 0, NULL, &len)) ERR;
       if (len != 1) ERR;
       if (nc_get_vara_double(ncid, 1, start, count, &data_in)) ERR;
       if (data_in != TEST_VAL_42) ERR;
       if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** testing very simple case writing data with 2 unlim dims...");
   {
       /* This test code based on test code from Jeff Whitaker. See
        * https://github.com/Unidata/netcdf4-python/issues/1413. This
        * is a simplified version. */
       int varid1, varid, ncid;
       int dimids[NDIM2];
       size_t start[1] = {0};
       size_t count[1] = {NY};
       int ydata[NY];
       int y, i;
       size_t len_in;

       /* Fill Y data array. */
       for (i = 0; i < NY; ++i)
           ydata[i] = i;

       /* Create a file. */
       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

       /* Define two unlimited dims. */
       if (nc_def_dim(ncid, DIM_NAME1, NC_UNLIMITED, &dimids[0])) ERR;
       if (nc_def_dim(ncid, DIM_NAME2, NC_UNLIMITED, &dimids[1])) ERR;

       /* Define coord var for first dim Y. */
       if (nc_def_var(ncid, DIM_NAME1, NC_INT, 1, &dimids[0], &varid1)) ERR;

       /* Define a data var with dims Y,X. */
       if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIM2, dimids, &varid)) ERR;

       /* Write data to coordinate var to extend Y. */
       if (nc_put_vara_int(ncid, varid1, start, count, ydata)) ERR;
       if (nc_close(ncid)) ERR;

       /* Reopen the file and check. */
       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
       if (nc_inq_dimlen(ncid, 0, &len_in)) ERR;
       if (len_in != NY) ERR;
       if (nc_inq_dimlen(ncid, 1, &len_in)) ERR;
       if (len_in != 0) ERR;

       /* Read and check each value of the coordinate var. */
       for (y = 0; y < NY; y++)
       {
           size_t index = y;
           int data_in;

           if (nc_get_var1_int(ncid, varid1, &index, &data_in)) ERR;
           if (data_in != y) ERR;
       }

       if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing simple case writing data with 2 unlim dims...");
   {
       /* This test code based on test code from Jeff Whitaker. See
        * https://github.com/Unidata/netcdf4-python/issues/1413. This
        * is a simplified version. */
       int varid1, varid, ncid;
       int dimids[NDIM2];
       size_t start[1] = {0};
       size_t count[1] = {NY};
       size_t start2[NDIM2] = {0, 2};
       size_t count2[NDIM2] = {1, 1};
       int ydata[NY];
       int data = TEST_VAL_42;
       int x, y, i;

       /* Fill Y data array. */
       for (i = 0; i < NY; ++i)
           ydata[i] = i;

       /* Create a file. */
       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

       /* Define two unlimited dims. */
       if (nc_def_dim(ncid, DIM_NAME1, NC_UNLIMITED, &dimids[0])) ERR;
       if (nc_def_dim(ncid, DIM_NAME2, NC_UNLIMITED, &dimids[1])) ERR;

       /* Define coord var for first dim Y. */
       if (nc_def_var(ncid, DIM_NAME1, NC_INT, 1, &dimids[0], &varid1)) ERR;

       /* Define a data var with dims Y,X. */
       if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIM2, dimids, &varid)) ERR;

       /* Write data to coordinate var to extend Y. */
       if (nc_put_vara_int(ncid, varid1, start, count, ydata)) ERR;
       if (nc_sync(ncid)) ERR;

       /* write a single data point to the 2d variable */
       if (nc_put_vara_int(ncid, varid, start2, count2, &data)) ERR;
       if (nc_close(ncid)) ERR;

       /* Reopen the file and check. */
       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

       /* Read and check each value with nc_get_var1_int(). */
       for (x = 0; x < NX; x++)
       {
           for (y = 0; y < NY; y++)
           {
               size_t index[NDIM2] = {y, x};
               int data_in;

               if (nc_get_var1_int(ncid, varid, index, &data_in)) ERR;
               if (y == start2[0] && x == start2[1])
               {
                   if (data_in != data) ERR;
               }
               else
               {
                   if (data_in != NC_FILL_INT) ERR;
               }
           }
       }

       /* Read and check each row with nc_get_vara_int(). */
       for (y = 0; y < NY; y++)
       {
           size_t start_row[NDIM2] = {y, 0};
           size_t count_row[NDIM2] = {1, NX};
           int data_in[NX];

           if (nc_get_vara_int(ncid, varid, start_row, count_row,
                               data_in)) ERR;
           if (y == start2[0])
           {
               /* This row has data. */
               for (x = 0; x < NX; x++)
                   if (data_in[x] != (x == start2[1] ? TEST_VAL_42 : NC_FILL_INT)) ERR;
           }
           else
           {
               /* This row is all fill. */
               for (x = 0; x < NX; x++)
               {
                   /* printf("y %d x %d data %d\n", y, x, data_in[x]); */
                   if (data_in[x] != NC_FILL_INT) ERR;
               }
           }
       }

       if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing dimlen issue reported in 2357 by Jeff W...");
   {
       /* This test code based on test code from Jeff Whitaker. See
        * https://github.com/Unidata/netcdf-c/issues/2357. This is a
        * simplified version. */
       int varid, ncid;
       int dimids[NDIM2];
       size_t start[NDIM2] = {0, 0}, count[NDIM2] = {DATALEN2, 1}, dimlen;
       int data[DATALEN2] = {42, 42};
       
       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
       if (nc_def_dim(ncid, "x", DATALEN2, &dimids[0])) ERR;
       if (nc_def_dim(ncid, "t", NC_UNLIMITED, &dimids[1])) ERR;
       if (nc_def_var(ncid, "v", NC_INT, NDIM2, dimids, &varid)) ERR;
       if (nc_put_vara_int(ncid, varid, start, count, data)) ERR;
       start[1] = 1;
       if (nc_put_vara_int(ncid, varid, start, count, data)) ERR;
       if (nc_close(ncid)) ERR;

       /* Reopen file and add more data. */
       if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
       start[1] = 2;
       if (nc_put_vara_int(ncid, varid, start, count, data)) ERR;
       if (nc_inq_dimlen(ncid, dimids[1], &dimlen)) ERR;
       if (dimlen != 3) ERR;
       if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing dimlen with NC_UNLIMITED reopen and rewrite...");
   {
       int varid, ncid;
       int dimid;
       size_t start[NDIM1] = {0}, count[NDIM1] = {DATALEN2}, dimlen;
       int data[DATALEN2] = {42, 42};
       int data2 = 99;

       /* Create a file with unlimited dim and one var. */
       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
       if (nc_def_dim(ncid, "Number_of_beaches", NC_UNLIMITED, &dimid)) ERR;
       if (nc_def_var(ncid, "Paros", NC_INT, NDIM1, &dimid, &varid)) ERR;
       if (nc_put_vara_int(ncid, varid, start, count, data)) ERR;
       if (nc_inq_dimlen(ncid, dimid, &dimlen)) ERR;
       if (dimlen != DATALEN2) ERR;
       if (nc_close(ncid)) ERR;

       /* Reopen file and change first element of array. */
       if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
       count[0] = 1;
       if (nc_put_vara_int(ncid, varid, start, count, &data2)) ERR;
       if (nc_inq_dimlen(ncid, dimid, &dimlen)) ERR;
       if (dimlen != 2) ERR;
       if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing dimlen issue reported in 2357 by Jeff W...");
   {
       int i, dimidx, dimidt, varid, ncid;
       int dimids[NDIM2];
       size_t start[NDIM2], count[NDIM2], dimlen;
       int data[DATALEN2];
       
       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
       if (nc_def_dim(ncid, "x", DATALEN2, &dimidx)) ERR;
       if (nc_def_dim(ncid, "t", NC_UNLIMITED, &dimidt)) ERR;
       dimids[0] = dimidt; /* note order is now reversed */
       dimids[1] = dimidx;
       if (nc_def_var(ncid, "v", NC_INT, NDIM2, dimids, &varid)) ERR;
       start[0]=0;
       start[1]=0;
       count[0]=1;
       count[1]=DATALEN2;
       for (i = 0; i < DATALEN2; i++)
   	   data[i] = 1;
       if (nc_put_vara_int(ncid, varid, start, count, data)) ERR;
       start[0]=1;
       start[1]=0;
       count[0]=1;
       count[1]=DATALEN2;
       for (i = 0; i < DATALEN2; i++)
   	   data[i] = 2;
       if (nc_put_vara_int(ncid, varid, start, count, data)) ERR;
       if (nc_close(ncid)) ERR;

       /* Reopen the file. */
       if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
       if (nc_inq_varid(ncid, "v", &varid)) ERR;
       if (nc_inq_dimid(ncid, "t", &dimidt)) ERR;
       start[0]=0;
       start[1]=0;
       count[0]=1;
       count[1]=DATALEN2;
       for (i = 0; i < DATALEN2; i++)
   	   data[i] = 0;
       if (nc_put_vara_int(ncid, varid, start, count, data)) ERR;
       if (nc_inq_dimlen(ncid, dimidt, &dimlen)) ERR;
       if (dimlen != DATALEN2) ERR;
       if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
