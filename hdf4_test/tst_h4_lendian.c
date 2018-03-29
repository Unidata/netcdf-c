/*! Testing for proper read of little-endian variables in an hdf4 file.
 *
 * Added to debug issue NCF-332. Based on code submitted by
 * https://github.com/Unidata/netcdf-c/issues/113.
 */

#include <config.h>
#include <unistd.h>
#include <nc_tests.h>
#include "mfhdf.h"
#include "err_macros.h"

#define DIM1 5
#define DIM0 5
#define RANK 2
#define FILENAME "tst_h4_lendian.h4"
#define SDSNAME "data"

/* Read the file. Return -1 if endianness is not little. */
int read_hdf_file(int dtype)
{
   int ncid = 0;
   int le_int16_varid = 0;
   int ed = 0;

   if (nc_open(FILENAME, NC_NETCDF4 | NC_NOWRITE, &ncid)) ERR;
   if (nc_inq_varid(ncid,SDSNAME,&le_int16_varid)) ERR;
   if (nc_inq_var_endian(ncid,le_int16_varid,&ed)) ERR;
   if (nc_close(ncid)) ERR;
   
   if (ed != NC_ENDIAN_LITTLE)
      return -1;

   return 0;
}

int create_hdf_file(int dtype)
{
   int32 sd_id, sds_id, sd_index = 0;
   int32 start[2] = {0, 0}, edges[2] = {DIM1, DIM0};
   int16 array_data[DIM0][DIM1];
   intn i, j, count;

   /* populate data array */
   count = 0;
   for (j = 0; j < DIM0; j++)
      for (i = 0; i < DIM1; i++)
         array_data[j][i] = count++;

   if ((sd_id = SDstart(FILENAME, DFACC_CREATE)) == -1) ERR;
   if ((sds_id = SDcreate(sd_id, SDSNAME, dtype, RANK, edges)) == -1) ERR;
   if (SDendaccess(sds_id)) ERR;
   if (SDend(sd_id)) ERR;

   if ((sd_id = SDstart(FILENAME, DFACC_WRITE)) == -1) ERR;
   if ((sds_id = SDselect(sd_id, sd_index)) == -1) ERR;
   if (SDwritedata(sds_id, start, NULL, edges, (VOIDP)array_data)) ERR;
   if (SDendaccess(sds_id)) ERR;
   if (SDend(sd_id)) ERR;

   return 0;
}


/* Create and then read the HDF4 test file. */
int test_read_write(int dtype)
{
   if (create_hdf_file(dtype)) ERR;
   return read_hdf_file(dtype);
}

int main()
{
   printf("\n***Test reading from an hdf4 file with a little-endian datatype...\n");
   printf("*** testing reading...");
   {
      if (test_read_write(DFNT_LINT8)) ERR;
      if (test_read_write(DFNT_LUINT8)) ERR;
      if (test_read_write(DFNT_LINT16)) ERR;
      if (test_read_write(DFNT_LUINT16)) ERR;
      if (test_read_write(DFNT_LINT32)) ERR;
      if (test_read_write(DFNT_LUINT32)) ERR;
      if (test_read_write(DFNT_LFLOAT32)) ERR;
      if (test_read_write(DFNT_LFLOAT64)) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** testing for True Negatives. these will return -1...");
   {
      /* True Negatives. */
      if (test_read_write(DFNT_INT8) != -1) ERR;
      if (test_read_write(DFNT_UINT8) != -1) ERR;
      if (test_read_write(DFNT_INT16) != -1) ERR;
      if (test_read_write(DFNT_UINT16) != -1) ERR;
      if (test_read_write(DFNT_INT32) != -1) ERR;
      if (test_read_write(DFNT_UINT32) != -1) ERR;
      if (test_read_write(DFNT_FLOAT32) != -1) ERR;
      if (test_read_write(DFNT_FLOAT64) != -1) ERR;
   }
   SUMMARIZE_ERR;

   FINAL_RESULTS;
}
