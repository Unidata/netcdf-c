#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "ncpathmgr.h"
#include "nclist.h"

#ifdef USE_HDF5
#include <hdf5.h>
#include <H5DSpublic.h>
#endif

#ifdef NETCDF_ENABLE_NCZARR
#include "zincludes.h"
#endif

#include "test_utils.h"

#define NDATA MAX_DATA
static int data[NDATA];

static size_t chunkprod;
static size_t dimprod;
static size_t datasize = 0;

static int
writedata(void)
{
    int ret = NC_NOERR;
    int i;

    for(i=0;i<NDATA;i++) data[i] = (options->data == 0x7fffffff ? i: options->data);
 
    if(options->debug >= 1) {
	fprintf(stderr,"write: dimlens=%s chunklens=%s\n",
	    printvector(options->rank,options->dimlens),printvector(options->rank,options->chunks));
    }
    fprintf(stderr,"write vars: start=%s edges=%s stride=%s\n",
    printvector(options->rank,options->start),printvector(options->rank,options->edges),printvector(options->rank,options->stride));
    if((ret = nc_put_vars(meta->ncid,meta->varid,options->start,options->edges,(ptrdiff_t*)options->stride,data)))
	ERR(ret);
    return 0;
}

static int
readdata(void)
{
    int ret = NC_NOERR;
    size_t i;
    
    memset(data,0,datasize);

    if(options->debug >= 1)
	fprintf(stderr,"read: dimlens=%s chunklens=%s\n",
	    printvector(options->rank,options->dimlens),printvector(options->rank,options->chunks));
    fprintf(stderr,"read vars: start=%s edges=%s stride=%s",
		printvector(options->rank,options->start),
		printvector(options->rank,options->edges),
		printvector(options->rank,options->stride));
    fprintf(stderr,"\n");
    if((ret = nc_get_vars(meta->ncid,meta->varid,options->start,options->edges,(ptrdiff_t*)options->stride,data)))
	ERR(ret);

    for(i=0;i<dimprod;i++) {
	printf("[%u] %d\n",(unsigned)i,data[i]);
    }
    return 0;
}

#if 0
static int
genodom(void)
{
    int i,ret = NC_NOERR;
    Odometer* odom = odom_new(options->rank, options->start, options->stop, options->stride, options->max);
    if(odom == NULL) {ret = NC_ENOMEM; goto done;}
    if(options->debug > 1)
	fprintf(stderr,"genodom: odom = %s\n",odom_print(odom));
    /* Iterate the odometer */
    for(i=0;odom_more(odom);odom_next(odom),i++) {
	printf("[%02d] %s\n",i,(i==0?odom_print(odom):odom_printshort(odom)));
    }	     
done:
    odom_free(odom);
    return ret;
}
#endif

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    size_t i;
    
    if((stat=getoptions(&argc,&argv))) goto done;
    if((stat=verifyoptions(options))) goto done;
    
    if(meta->ncid == 0) {
	fprintf(stderr,"File not found: %s\n",options->file);
	ERR(NC_EACCESS);
    }
    
    if(options->create == Create) {
	if((stat = getmetadata(1))) ERR(stat);
	if(meta->ncid && (stat = nc_close(meta->ncid))) ERR(stat);
    }
    if((stat = getmetadata(0)))
	ERR(stat);
    
    dimprod = 1;
    chunkprod = 1;
    for(i=0;i<options->rank;i++) {dimprod *= options->dimlens[i]; chunkprod *= options->chunks[i];}

    datasize = dimprod*sizeof(int);

    switch (options->op) {
    case Read: readdata(); break;
    case Write: writedata(); break;
    default: break;
    }
    if(meta->ncid) {if((stat = nc_close(meta->ncid))) ERR(stat);}

done:
    cleanup();
    return 0;
}
