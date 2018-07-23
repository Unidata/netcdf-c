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
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
