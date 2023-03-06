/* This is part of the netCDF package. Copyright 2005-2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Runs benchmarks on different chunking sizes.

   Russ Rew, Ed Hartnett, Dennis Heimbigner
*/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>		/* for sysconf */
#endif
#include <assert.h>

#include "nc_tests.h"		/* The ERR macro is here... */
#include "netcdf.h"

#include "bm_utils.h"
#include "timer_utils.h"

#undef NOREPEAT
#undef NOCONTIG

/*
 * The following timing macros can be used by including the necessary
 * declarations with
 *
 *     TIMING_DECLS(seconds)
 *
 * and surrounding sections of code to be timed with the "statements"
 *
 *     TIMING_START
 *     [code to be timed goes here]
 *     TIMING_END(seconds)
 *
 * The macros assume the user has stored a description of what is
 * being timed in a 100-char string time_mess, and has included
 * <sys/times.h> and <sys/resource.h>.  The timing message printed by
 * TIMING_END is not terminated by a new-line, to permit appending
 * additional text to that line, so user must at least printf("\n")
 * after that.
 */

#define TIMING_DECLS(seconds)						       \
	long TMreps;		/* counts repetitions of timed code */ \
	long TMrepeats;		/* repetitions needed to exceed 0.1 second */ \
	Nanotime bnano,enano,delta;	/* start/stop times in nanoseconds */ \
        char time_mess[100]; \
        double seconds; \
	NCT_inittimer();

#ifndef NOREPEAT
#define TIMING_START \
	TMrepeats = 1; \
	do { \
	    NCT_marktime(&bnano); \
	    for(TMreps=0; TMreps < TMrepeats; TMreps++) {

#define TIMING_END(time_mess,seconds)				\
            } \
	    NCT_marktime(&enano); \
	    NCT_elapsedtime(&bnano,&enano,&delta); \
	    TMrepeats *= 2; \
	} while (NCT_nanoseconds(delta) < 100000000 ); \
	seconds = ((double)NCT_nanoseconds(delta)) / (1000000000.0 * TMreps); \
	printf("%-45.45s %#08.6F sec", time_mess, seconds);

#else /*NOREPEAT*/

#define TIMING_START \
	do { \
fprintf(stderr,"TIME_START\n"); \
	    NCT_marktime(&bnano); \
	    {

#define TIMING_END(time_mess,seconds) \
	    } \
	    NCT_marktime(&enano); \
	    NCT_elapsedtime(&bnano,&enano,&delta); \
        } while (0); \
fprintf(stderr,"TIME_END\n"); \
	seconds = ((double)NCT_nanoseconds(delta)) / (1000000000.0 * TMreps); \
	printf("%-45.45s %#08.6F sec", time_mess, seconds);

#endif /*NOREPEAT*/


#define NC_COMPRESSED 1

/* This macro prints an error message with line number and name of
 * test program. */
#define ERR1(n) do {						  \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d - %s\n", \
	__FILE__, __LINE__, nc_strerror(n));			 \
return n; \
} while (0)

void *
emalloc(size_t bytes) {
    size_t *memory;
    memory = malloc(bytes);
    if(memory == 0) {
	printf("malloc failed\n");
	exit(2);
    }
    return memory;
}

typedef enum Tag {tag_contiguous=0, tag_chunked=1, tag_compressed=2} Tag;

static const char* tagnames[] = {"contiguous", "chunked   ", "compressed"};
static const char* rwnames[] = {"write", "read "};

static float test(Tag tag, int read, int ncid, int varid, int rank, int index, size_t* dims, size_t* chunks, unsigned* data);
static void testtrio(int read, int ncid, int index, unsigned* data, const char* slab);

/* Globals */
static int  ncid;  /* netCDF id */
static int dim1id, dim2id, dim3id;
static int varid_g;		  /* varid for contiguous */
static int varid_k;		  /* varid for chunked */
static int varid_x;		  /* varid for compressed */
static unsigned *varxy, *varxz, *varyz;    /* 2D memory slabs used for I/O */
static size_t szyz=0, szxz=0, szxy=0;

static const char*
dataname(void* data)
{
    if(data == varyz) return "yz";
    if(data == varxz) return "xz";
    if(data == varxy) return "xy";
    return "?";
}

/* compare contiguous, chunked, and compressed performance */
int
main(int argc, char *argv[])
{
    int  stat;  /* return status */
    int i, j, k;
    int mm;
    int deflate_level = 0;	/* default compression level, 9 is
				 * better and slower.  If negative,
				 * turn on shuffle filter also. */
    int shuffle = NC_NOSHUFFLE;
    size_t cache_size_def;
    size_t cache_hash_def;
    float cache_pre_def;
    size_t cache_size = 0;	    /* use library default */
    size_t cache_hash = 0;	    /* use library default */
    float cache_pre = -1.0f;	    /* use library default */
    char* path = NULL;
    int fill = -1;

    size_t* dims = NULL; /* alias */
    size_t* chunks = NULL; /* alias */


    /* rank (number of dimensions) for each variable */
#   define RANK 3

    /* variable shapes */
    int var_dims[RANK];

    NCCHECK(bm_getoptions(&argc,&argv,&bmoptions));
    NCCHECK(nc4_buildpath(&bmoptions,&path));

    if(bmoptions.debug) {
        reportoptions(&bmoptions);
        reportmetaoptions(&bmoptions.meta);
    }

    if(bmoptions.meta.dims.count != RANK)
        ERR1(NC_EINVAL);
    if(bmoptions.meta.dims.count != bmoptions.meta.chunks.count)
        ERR1(NC_EINVAL);
    dims = bmoptions.meta.dims.list;
    chunks = bmoptions.meta.chunks.list;

    if(bmoptions.meta.cachesize > 0) {
    /* get cache defaults, then set cache parameters that are not default */
    if((stat = nc_get_chunk_cache(&cache_size_def, &cache_hash_def,
				   &cache_pre_def)))
	ERR1(stat);
    if(cache_size == 0)
	cache_size = cache_size_def;
    if(cache_hash == 0)
	cache_hash = cache_hash_def;
    if(cache_pre == -1.0f)
	cache_pre = cache_pre_def;
    if((stat = nc_set_chunk_cache(cache_size, cache_hash, cache_pre)))
	ERR1(stat);
    printf("cache: %3.2f MBytes  %ld objs  %3.2f preempt, ",
	   cache_size/1.e6, cache_hash, cache_pre);
    }
    
    if(bmoptions.debug) {
        if(deflate_level == 0) {
  	    printf("compression level: uncompressed");
        } else {
  	    printf("compression level: %d          ", deflate_level);
	}
    }
    if(shuffle == 1) {
	printf(", shuffled");
    }
    printf("\n\n");

    /* initialize 2D slabs for writing along each axis with phony data */
    szyz =       1 * dims[1] * dims[2];
    szxz = dims[0] *       1 * dims[2];
    szxy = dims[0] * dims[1] *       1;
    if(bmoptions.debug)
	fprintf(stderr,"|yz|=%d |xz|=%d |xy|=%d\n",(int)szyz,(int)szxz,(int)szxy);
    varyz = (unsigned *) emalloc(sizeof(unsigned) * szyz);
    varxz = (unsigned *) emalloc(sizeof(unsigned) * szxz);
    varxy = (unsigned *) emalloc(sizeof(unsigned) * szxy);
    
    mm = 0;
    for(j = 0; j < dims[1]; j++) {
	for(k = 0; k < dims[2]; k++) {
	    varyz[mm++] = k + dims[2]*j;
	}
    }
    mm = 0;
    for(i = 0; i < dims[0]; i++) {
	for(k = 0; k < dims[2]; k++) {
	    varxz[mm++] = k + dims[2]*i;
	}
    }
    mm = 0;
    for(i = 0; i < dims[0]; i++) {
	for(j = 0; j < dims[1]; j++) {
	    varxy[mm++] = j + dims[1]*i;
	}
    }

    if((stat = nc_create(path, NC_NETCDF4, &ncid)))
	ERR1(stat);
    /* define dimensions */
    if((stat = nc_def_dim(ncid, "dim1", dims[0], &dim1id)))
	ERR1(stat);
    if((stat = nc_def_dim(ncid, "dim2", dims[1], &dim2id)))
	ERR1(stat);
    if((stat = nc_def_dim(ncid, "dim3", dims[2], &dim3id)))
	ERR1(stat);

    /* define variables */
    var_dims[0] = dim1id;
    var_dims[1] = dim2id;
    var_dims[2] = dim3id;
    if((stat = nc_def_var(ncid, "var_contiguous", NC_INT, RANK,
			   var_dims, &varid_g)))
	ERR1(stat);
    if((stat = nc_def_var(ncid, "var_chunked", NC_INT, RANK,
			   var_dims, &varid_k)))
	ERR1(stat);
    if((stat = nc_def_var(ncid, "var_compressed", NC_INT, RANK,
			   var_dims, &varid_x)))
	ERR1(stat);

    if((stat = nc_def_var_fill(ncid, varid_g, NC_FILL, &fill)))
	ERR1(stat);

    if((stat = nc_def_var_fill(ncid, varid_k, NC_FILL, &fill)))
	ERR1(stat);

    if((stat = nc_def_var_fill(ncid, varid_x, NC_FILL, &fill)))
	ERR1(stat);

#ifndef NOCONTIG
    if((stat = nc_def_var_chunking(ncid, varid_g, NC_CONTIGUOUS, 0)))
	ERR1(stat);
#else
    if((stat = nc_def_var_chunking(ncid, varid_g, NC_CHUNKED, dims)))
	ERR1(stat);
#endif

    if((stat = nc_def_var_chunking(ncid, varid_k, NC_CHUNKED, chunks)))
	ERR1(stat);

    if((stat = nc_def_var_chunking(ncid, varid_x, NC_CHUNKED, chunks)))
	ERR1(stat);

    if (bmoptions.meta.deflatelevel != 0) {
	if((stat = nc_def_var_deflate(ncid, varid_x, shuffle, NC_COMPRESSED, bmoptions.meta.deflatelevel)))
	    ERR1(stat);
    }

    /* leave define mode */
    if((stat = nc_enddef (ncid)))
	ERR1(stat);

    printf("Parameters: path=%s format=%s\n",path,formatname(&bmoptions));
    printf("   mode   |  R/W  |   dims    |    chunked  |     time     |    speedup   |\n");
    printf("--------------------------------------------------------------------------\n");

    /* write each variable one yz slab at a time */
    testtrio(0,ncid,0,varyz,"yz");

    /* write each variable one xz slab at a time */
    testtrio(0,ncid,1,varxz,"xz");

    /* write each variable one xy slab at a time */
    testtrio(0,ncid,2,varxy,"xy");

    /* read each variable one yz slab at a time */
    testtrio(1,ncid,0,varyz,"yz");

    /* read each variable one xz slab at a time */
    testtrio(1,ncid,1,varxz,"xz");

    /* read each variable one xy slab at a time */
    testtrio(1,ncid,2,varxy,"xy");

    if((stat = nc_close(ncid)))
	ERR1(stat);

    nullfree(path);
    clearoptions(&bmoptions);

    return 0;
}

static float
test(Tag tag, int reading, int ncid, int varid, int rank, int index, size_t* dims, size_t* chunks, unsigned* data)
{
    int stat = NC_NOERR;
    int i;
    size_t start[NC_MAX_VAR_DIMS];
    size_t count[NC_MAX_VAR_DIMS];
    float time;
    TIMING_DECLS(TMsec) ;

    /* do each variable one slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = dims[0];
    count[1] = dims[1];
    count[2] = dims[2];

    count[index] = 1;    

    if(chunks != NULL)
        sprintf(time_mess,"%s %s %3ld %3ld %3ld  %3ld %3ld %3ld", tagnames[tag], rwnames[reading],
	        count[0], count[1], count[2], chunks[0], chunks[1], chunks[2]);
    else
        sprintf(time_mess,"%s %s %3ld %3ld %3ld", tagnames[tag], rwnames[reading],
		count[0], count[1], count[2]);

    TIMING_START ;
    for(i = 0; i < dims[index]; i++) {
	start[index] = i;
#if 0
if(reading){
    int k,x;
    size_t prod = 1;
    size_t st = 0;
    for(k=0;k<RANK;k++) {prod *= count[k]; st *= dims[k]; st += start[k];}
    fprintf(stderr,"AAA: begin: i=%d data=%s soffset=%d prod=%d\n",i,dataname(data),(int)st,(int)prod);
    fprintf(stderr,"\tdims=%s\n",bm_printvector(RANK,dims));
    fprintf(stderr,"\tvara=%s\n",bm_varasprint(RANK,start,count,NULL));
    for(k=0;k<prod;k++)
#if 0
        fprintf(stderr,"AAA: data[%d]=%d\n",st+k,((int*)data)[k]);
#else
        x = ((int*)data)[k];
#endif
    fprintf(stderr,"AAA: done\n");
}
#endif /*0*/

	if(reading) {
	    if(bmoptions.debug)
		fprintf(stderr,"bm: read: dataname=%s vara=%s\n",dataname(data),bm_varasprint(RANK,start,count,NULL));
	    stat = nc_get_vara(ncid, varid, start, count, (void*)data);
	} else {
//	    if(bmoptions.debug)
//		fprintf(stderr,"bm: write: dataname=%s vara=%s\n",dataname(data),bm_varasprint(RANK,start,count,NULL));
	    stat = nc_put_vara(ncid, varid, start, count, (void*)data);
	}
	if(stat) ERR1(stat);

    }
    if(bmoptions.debug)
	fprintf(stderr,"bm: write: done\n");
    TIMING_END(time_mess,TMsec) ;
    time = TMsec;
    return time;
}

static void
testtrio(int reading, int ncid, int index, unsigned* data, const char* slab)
{
    float ratio, contig_time, chunked_time, compressed_time;
    size_t* dims = bmoptions.meta.dims.list;
    size_t* chunks = bmoptions.meta.chunks.list;

    contig_time = test(tag_contiguous,reading,ncid, varid_g, RANK, index, dims, NULL, data);
    printf("\n");

    if(bmoptions.debug)
	fprintf(stderr,"testtrio: case=%s slab=%s index=%d\n","chunked",slab,index);
    chunked_time = test(tag_chunked,reading,ncid,varid_k, RANK, index, dims, chunks, data);
    ratio = contig_time/chunked_time;
    if(ratio >= 1.0) printf(" %5.2g x faster\n", ratio); else printf(" %5.2g x slower\n", 1.0/ratio);
    if(bmoptions.debug)
	fprintf(stderr,"testtrio: case=%s done\n","chunked");

    if (bmoptions.meta.deflatelevel != 0) {
        compressed_time = test(tag_compressed, reading, ncid, varid_x, RANK, index, dims, chunks, data);
        ratio = contig_time/compressed_time;
        if(ratio >= 1.0) printf(" %5.2g x faster\n", ratio); else printf(" %5.2g x slower\n", 1.0/ratio);
    }
}

