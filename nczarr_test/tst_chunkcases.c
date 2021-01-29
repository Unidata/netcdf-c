#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#include <stdio.h>
#endif
#include <string.h>
#include <netcdf.h>
#include <ncpathmgr.h>
#include <nclist.h>

#ifdef HAVE_HDF5_H
#include <hdf5.h>
#include <H5DSpublic.h>
#endif

#ifdef ENABLE_NCZARR
#include "zincludes.h"
#endif


#include "tst_utils.h"

static unsigned chunkprod;
static unsigned dimprod;
static int* data = NULL;
static size_t datasize = 0;

static int setupwholechunk(void);
static int reportwholechunk(void);
static void zutest_print(int sort, ...);

static int
writedata(void)
{
    int ret = NC_NOERR;
    int i;

    for(i=0;i<dimprod;i++) data[i] = i;
 
    if(options->wholechunk)
        setupwholechunk();

    if(options->debug >= 1) {
        fprintf(stderr,"write: dimlens=%s chunklens=%s\n",
            printvector(options->rank,options->dimlens),printvector(options->rank,options->chunks));
    }
    if(options->wholechunk) {
        fprintf(stderr,"write var: wholechunk\n");
        if((ret = nc_put_vars(meta->ncid,meta->varid,options->start,options->count,(ptrdiff_t*)options->stride,data)))
	    ERR(ret);
    } else {
        fprintf(stderr,"write vars: start=%s count=%s stride=%s\n",
            printvector(options->rank,options->start),printvector(options->rank,options->count),printvector(options->rank,options->stride));
        if((ret = nc_put_vars(meta->ncid,meta->varid,options->start,options->count,(ptrdiff_t*)options->stride,data)))
	    ERR(ret);
    }

    if(options->wholechunk) {
	if((ret=reportwholechunk()))
	    ERR(ret);
    }

    return 0;
}

static int
readdata(void)
{
    int ret = NC_NOERR;
    int i;
    
    memset(data,0,datasize);

    if(options->wholechunk) {
        setupwholechunk();
    }

    if(options->debug >= 1)
        fprintf(stderr,"read: dimlens=%s chunklens=%s\n",
            printvector(options->rank,options->dimlens),printvector(options->rank,options->chunks));
    fprintf(stderr,"read vars: start=%s count=%s stride=%s",
                printvector(options->rank,options->start),
		printvector(options->rank,options->count),
		printvector(options->rank,options->stride));
    if(options->wholechunk)
        fprintf(stderr," wholechunk");
    fprintf(stderr,"\n");
    if((ret = nc_get_vars(meta->ncid,meta->varid,options->start,options->count,(ptrdiff_t*)options->stride,data)))
        ERR(ret);

    for(i=0;i<dimprod;i++) {
        printf("[%d] %d\n",i,data[i]);
    }
    if(options->wholechunk)
	reportwholechunk();
    
    return 0;
}

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

static int wholechunkcalls = 0;
static struct ZUTEST zutester;

static int
setupwholechunk(void)
{
    int ret = NC_NOERR;

    wholechunkcalls = 0;
    
#ifdef ENABLE_NCZARR
    /* Set the printer */
    zutester.tests = UTEST_WHOLECHUNK;
    zutester.print = zutest_print;
    zutest = &zutester; /* See zdebug.h */
#endif
    return ret;
}


static void
zutest_print(int sort, ...)
{
    va_list ap;    
    struct Common* common = NULL;    
    size64_t* chunkindices;

    NC_UNUSED(common);

    va_start(ap,sort);
    
    switch (sort) {
    default: break; /* ignore */
    case UTEST_WHOLECHUNK:
	common = va_arg(ap,struct Common*);
	chunkindices = va_arg(ap,size64_t*);
	if(options->debug >= 1)
	    fprintf(stderr,"wholechunk: indices=%s\n",printvector64(common->rank,chunkindices));
	wholechunkcalls++;
	break;
    }
    va_end(ap);
}

static int
reportwholechunk(void)
{
    int ret = NC_NOERR;
#ifdef ENABLE_NCZARR
    if(options->debug > 0)
        fprintf(stderr,"wholechunkcalls=%d\n",wholechunkcalls);
    if(wholechunkcalls != 1)
        return NC_EINVAL;
#endif
    return ret;
}




int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int i;

    if((stat=getoptions(&argc,&argv))) goto done;
    
    if(options->formatx != NC_FORMATX_NCZARR && options->wholechunk) {
	fprintf(stderr,"Format is not NCZarr; -OW_ ignored\n");
	options->wholechunk = 0;
    }

    switch (options->op) {
    case Read:
        if((stat = getmetadata(0)))
            ERR(stat);
        if (argc == 0) {fprintf(stderr, "no input file specified\n");exit(1);}
	break;
    case Write:
        if((stat = getmetadata(1)))
            ERR(stat);
        if (argc == 0) {fprintf(stderr, "no output file specified\n");exit(1);}
	break;
    default:
	break; /* do not need a file */
    }

    dimprod = 1;
    chunkprod = 1;
    for(i=0;i<options->rank;i++) {dimprod *= options->dimlens[i]; chunkprod *= options->chunks[i];}

    datasize = dimprod*sizeof(int);
    if((data = calloc(1,datasize)) == NULL)
        {fprintf(stderr,"out of memory\n"); exit(1);}

    switch (options->op) {
    case Read: readdata(); break;
    case Write: writedata(); break;
    case Odom: genodom(); break;
    default:
	fprintf(stderr,"Unknown operation\n");
	exit(1);
    }
done:
    if(data) free(data);
    cleanup();
    return 0;
}
