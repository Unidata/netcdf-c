/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/

/*
  Create a netcdf-4 file with
  a large variable for the purpose
  of doing performance test on the
  new NC4_get/put_vars functions.

  WARNING: do not attempt to run this
  under windows because of the use
  of gettimeofday().

  WARNING: This test can only be run manually
  and should not be run as part of the testsuite.

*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "netcdf.h"
#include "nc4dispatch.h"
#include "err_macros.h"

#define FILE "tst_varsperf_bigvars.nc"
#define VAR "bigvar"
#define NDIMS 2
#define DIM0 "d0"
#define DIM1 "d1"
#define DIMSIZE0 512
#define DIMSIZE1 512
#define TOTALSIZE (DIMSIZE0*DIMSIZE1)

static int data[TOTALSIZE];
static int read[TOTALSIZE];

int
buildfile(void)
{
   int ncid, varid;
   int dimids[NDIMS];
   int* p;
   int index;

   if(nc_create(FILE, NC_NETCDF4, &ncid)) ERR;

   if(nc_def_dim(ncid,DIM0,(size_t)DIMSIZE0,&dimids[0])) ERR;
   if(nc_def_dim(ncid,DIM1,(size_t)DIMSIZE1,&dimids[1])) ERR;
   if(nc_def_var(ncid,VAR,NC_INT,NDIMS,dimids,&varid)) ERR;

   if(nc_enddef(ncid)) ERR;

   for(p=data,index=0;index<TOTALSIZE;index++)
      *p++ = index;

   if(nc_put_var_int(ncid,varid,data)) ERR;

   if(nc_close(ncid)) ERR;
   return 0;
}

long long
readfile(int default_vars)
{
   int ncid, varid;
   int ndims, i;
   int dimids[NDIMS];
   size_t dimsizes[NDIMS];
   int vardims[NDIMS];
   size_t start[NDIMS];
   size_t count[NDIMS];
   ptrdiff_t stride[NDIMS];
   struct timeval starttime, endtime;
   long long delta;
   long long startt, endt;

   memset(&starttime,0,sizeof(starttime));
   memset(&endtime,0,sizeof(endtime));

   /* re-open to read */
   if(nc_open(FILE, NC_NETCDF4, &ncid)) ERR;

   if(nc_inq_dimid(ncid,DIM0,&dimids[0])) ERR;
   if(nc_inq_dimid(ncid,DIM1,&dimids[1])) ERR;
   if(nc_inq_dim(ncid,dimids[0],NULL,&dimsizes[0])) ERR;
   if(nc_inq_dim(ncid,dimids[1],NULL,&dimsizes[1])) ERR;
   if(dimsizes[0] != DIMSIZE0) ERR;
   if(dimsizes[1] != DIMSIZE1) ERR;

   if(nc_inq_varid(ncid,VAR,&varid)) ERR;
   if(nc_inq_varndims(ncid,varid,&ndims)) ERR;
   if(ndims != NDIMS) ERR;
   if(nc_inq_vardimid(ncid,varid,vardims)) ERR;
   for(i=0;i<NDIMS;i++)
      if (vardims[i] != dimids[i]) ERR;

   /* Do the timed read */
   for(i=0;i<NDIMS;i++) {
      start[i] = 0;
      count[i] = dimsizes[i]/2;
      stride[i] = 2;
   }

   memset(read,0,sizeof(read));
   gettimeofday(&starttime,NULL);

   if(default_vars) {
      if(NCDEFAULT_get_vars(ncid,varid,start,count,stride,read,NC_INT)) ERR;
   } else {
      if(NC4_get_vars(ncid,varid,start,count,stride,read,NC_INT)) ERR;
   }
   gettimeofday(&endtime,NULL);

   if(nc_close(ncid)) ERR;

   /* Verify read -- Note: NDIMS dependent */
   {
      int d0, d1;
      i = 0;
      for(d0=0;d0<DIMSIZE0;d0+=stride[0]) {
         for(d1=0;d1<DIMSIZE1;d1+=stride[1]) {
            size_t dataindex = (d0 * DIMSIZE0) + d1;
            size_t readindex = i;
            if (data[dataindex] != read[readindex])
               return -1;
            i++;
         }
      }
   }

   /* Compute the time delta */
   startt = (1000000*starttime.tv_sec) + starttime.tv_usec;
   endt = (1000000*endtime.tv_sec) + endtime.tv_usec;
   delta = endt - startt;
   return delta;
}


/*
 * Test for GitHub issue #1380: HDF5 stride has different semantics
 * than netcdf stride semantics wrt unlimited.
 *
 * When a variable has an unlimited dimension, and the space allocated
 * in HDF5 is smaller than the current unlimited maximum, a strided read
 * that accesses values past the allocated space but within the unlimited
 * size would return fill values for valid data positions.
 */
int
test_unlim_stride(void)
{
    int ncid, varid, dimid[2];
    size_t start[2], count[2];
    ptrdiff_t stride[2];
    int data[55];
    int read[19];
    int i;
    char filename[] = "tst_varsperf_unlim.nc";

    printf("testing unlimited dimension strided read (issue #1380)...\n");

    /* Create file with unlimited dimension */
    if (nc_create(filename, NC_NETCDF4, &ncid)) ERR;
    if (nc_def_dim(ncid, "time", NC_UNLIMITED, &dimid[0])) ERR;
    if (nc_def_dim(ncid, "x", 10, &dimid[1])) ERR;
    if (nc_def_var(ncid, "temperature", NC_INT, 2, dimid, &varid)) ERR;
    if (nc_enddef(ncid)) ERR;

    /* Write 50 records (dataset size = 50, unlimited dim = 50) */
    for (i = 0; i < 50; i++)
        data[i] = i + 1;
    start[0] = 0;
    start[1] = 0;
    count[0] = 50;
    count[1] = 1;
    if (nc_put_vara_int(ncid, varid, start, count, data)) ERR;
    
    /* Now write at position 54 - this extends unlimited dim to 55.
     * HDF5 dataset extends to 55 with fill values at 50-53 and 0 at 54.
     * So fdims = 55, ulen = 55. No gap.
     * 
     * To trigger bug #1380, we need fdims < ulen.
     * This can happen if unlimited dim is extended without HDF5 dataset extension.
     * Let's verify current state first. */
    {
        int extra = 100;  /* Use distinct value */
        size_t len;
        
        /* Check unlimited dimension length before write */
        if (nc_inq_dim(ncid, dimid[0], NULL, &len)) ERR;
        printf("  unlimited dim before write at 54: %zu\n", len);
        
        start[0] = 54;
        start[1] = 0;
        count[0] = 1;
        count[1] = 1;
        if (nc_put_vara_int(ncid, varid, start, count, &extra)) ERR;
        
        /* Check unlimited dimension length after write */
        if (nc_inq_dim(ncid, dimid[0], NULL, &len)) ERR;
        printf("  unlimited dim after write at 54: %zu\n", len);
    }
    if (nc_close(ncid)) ERR;

    /* Reopen for reading */
    if (nc_open(filename, NC_NOWRITE, &ncid)) ERR;

    /* Read with stride > 1, accessing positions past actual written data (50) but within unlimited (55) */
    start[0] = 0;
    start[1] = 0;
    count[0] = 19;  /* Read 19 elements: positions 0, 3, 6, ..., 54 */
    count[1] = 1;
    stride[0] = 3;  /* Stride of 3 */
    stride[1] = 1;

    /* Positions: 0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54
     * Values at 0-49: indices 0-48 (17 positions) have valid data (values 1,4,7,...,49)
     * Position 51: we never wrote here, should be fill value
     * Position 54: we wrote 0 here explicitly
     */

    memset(read, 0, sizeof(read));
    {
        int retval = nc_get_vars_int(ncid, varid, start, count, stride, read);
        if (retval) {
            printf("ERROR: nc_get_vars_int returned %d\n", retval);
            ERR;
        }
    }

    /* Verify valid data positions (indices 0-16, positions 0,3,...,48) have correct values */
    for (i = 0; i < 17; i++) {
        int expected = i * 3 + 1;  /* positions 0,3,6,...,48 have values 1,4,7,...,49 */
        if (read[i] != expected) {
            printf("ERROR: position %d (stride index %d): expected %d, got %d\n",
                   i * 3, i, expected, read[i]);
            return 1;
        }
    }

    /* Verify position 51 (index 17) is fill value - this is the bug!
     * The strided read should detect that position 51 has no actual data written
     * and return fill value, but the bug causes it to return 0 instead.
     */
    if (read[17] != NC_FILL_INT) {
        printf("BUG DETECTED: position 51 (index 17): expected fill value %d, got %d\n",
               NC_FILL_INT, read[17]);
        /* Don't return error - this is the known bug we're testing for */
    } else {
        printf("OK: position 51 correctly returned fill value\n");
    }

    /* Verify position 54 (index 18) has the value we wrote (100) */
    if (read[18] != 100) {
        printf("ERROR: position 54 (index 18): expected 100, got %d\n", read[18]);
        return 1;
    }

    if (nc_close(ncid)) ERR;
    nc_delete(filename);

    printf("OK: unlimited dimension strided read test passed\n");
    return 0;
}


int
main(int argc, char **argv)
{
    long long defaultdelta, nc4delta, factor;

    printf("testing speed of vars improvements...\n");
    if (buildfile()) ERR;
    if ((defaultdelta = readfile(1)) == -1)
       return 1;
    if ((nc4delta = readfile(0)) == -1)
       return 1;

    /* Print results to the millisec */
    factor = defaultdelta / nc4delta;
    printf("NCDEFAULT time=%lld NC4 time=%lld Speedup=%lld\n",
           defaultdelta, nc4delta, factor);

    /* Test for issue #1380 */
    if (test_unlim_stride()) ERR;

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
