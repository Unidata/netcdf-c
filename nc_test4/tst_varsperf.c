/*
Create a netcdf-4 file with
a large variable for the purpose
of doing performance test on the
new NC4_get/put_vars functions.

WARNING: do not attempt to run this
under windows because of the use
of gettimeofday().

*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include "netcdf.h"


#define FILE "bigvars.nc"
#define VAR "bigvar"
#define NDIMS 2
#define DIM0 "d0"
#define DIM1 "d1"
#define DIMSIZE0 1024
#define DIMSIZE1 1024
#define TOTALSIZE (DIMSIZE0*DIMSIZE1)

#define CHECK(expr) assert((expr) == NC_NOERR)

static int data[TOTALSIZE];
static int read[TOTALSIZE];

extern void nc4_set_default_vars(int);

void
buildfile(void)
{
    int ncid, varid;
    int dimids[NDIMS];
    int* p;
    int index;

    CHECK(nc_create(FILE, NC_NETCDF4, &ncid));

    CHECK(nc_def_dim(ncid,DIM0,(size_t)DIMSIZE0,&dimids[0]));
    CHECK(nc_def_dim(ncid,DIM1,(size_t)DIMSIZE1,&dimids[1]));
    CHECK(nc_def_var(ncid,VAR,NC_INT,NDIMS,dimids,&varid));
    
    CHECK(nc_enddef(ncid));

    for(p=data,index=0;index<TOTALSIZE;index++)
	*p++ = index;	

    CHECK(nc_put_var_int(ncid,varid,data));

    CHECK(nc_close(ncid));
}

long long
readfile(void)
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
    CHECK(nc_open(FILE, NC_NETCDF4, &ncid));

    CHECK(nc_inq_dimid(ncid,DIM0,&dimids[0]));
    CHECK(nc_inq_dimid(ncid,DIM1,&dimids[1]));
    CHECK(nc_inq_dim(ncid,dimids[0],NULL,&dimsizes[0]));
    CHECK(nc_inq_dim(ncid,dimids[1],NULL,&dimsizes[1]));
    assert(dimsizes[0] == DIMSIZE0);
    assert(dimsizes[1] == DIMSIZE1);

    CHECK(nc_inq_varid(ncid,VAR,&varid));
    CHECK(nc_inq_varndims(ncid,varid,&ndims));
    assert(ndims == NDIMS);
    CHECK(nc_inq_vardimid(ncid,varid,vardims));
    for(i=0;i<NDIMS;i++)
        assert(vardims[i] == dimids[i]);

    /* Do the timed read */
    for(i=0;i<NDIMS;i++) {
	start[i] = 0;
	count[i] = dimsizes[i]/2;
	stride[i] = 2;
    }

    memset(read,0,sizeof(read));
    gettimeofday(&starttime,NULL);
    CHECK(nc_get_vars(ncid,varid,start,count,stride,read));
    gettimeofday(&endtime,NULL);

    CHECK(nc_close(ncid));

    /* Verify read -- Note: NDIMS dependent */
    {
	int d0, d1;
	i = 0;
	for(d0=0;d0<DIMSIZE0;d0+=stride[0]) {
            for(d1=0;d1<DIMSIZE1;d1+=stride[1]) {
		size_t dataindex = (d0 * DIMSIZE0) + d1;
		size_t readindex = i;
		assert(data[dataindex] == read[readindex]);
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

    buildfile();
    nc4_set_default_vars(1);
    defaultdelta = readfile();
    nc4_set_default_vars(0);
    nc4delta = readfile();

    /* Print results to the millisec */
    factor = defaultdelta / nc4delta;    
    printf("NCDEFAULT time=%lld NC4 time=%lld Speedup=%lld\n",
		defaultdelta, nc4delta, factor);
    return 0;
}
