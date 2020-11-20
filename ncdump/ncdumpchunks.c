#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <getopt.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "netcdf.h"
#ifdef HAVE_HDF5_H
#include <hdf5.h>
#include <H5DSpublic.h>
#endif

#ifdef ENABLE_NCZARR
#include "zincludes.h"
#endif

typedef struct Odometer {
  size_t rank; /*rank */
  size_t stop[NC_MAX_VAR_DIMS];
  size_t max[NC_MAX_VAR_DIMS]; /* max size of ith index */
  size_t index[NC_MAX_VAR_DIMS]; /* current value of the odometer*/
} Odometer;

#define floordiv(x,y) ((x) / (y))
#define ceildiv(x,y) (((x) % (y)) == 0 ? ((x) / (y)) : (((x) / (y)) + 1))

Odometer* odom_new(size_t rank, const size_t* stop, const size_t* max);
void odom_free(Odometer* odom);
int odom_more(Odometer* odom);
int odom_next(Odometer* odom);
size_t* odom_indices(Odometer* odom);
size_t odom_offset(Odometer* odom);

static void
usage(int err)
{
     if(err != 0) {
	 fprintf(stderr,"Error: (%d) %s\n",err,nc_strerror(err));
     }
     fprintf(stderr,"usage: ncdumpchunks <file> <var>\n");
     fflush(stderr);
     exit(1);
}

Odometer*
odom_new(size_t rank, const size_t* stop, const size_t* max)
{
     int i;
     Odometer* odom = NULL;
     if((odom = calloc(1,sizeof(Odometer))) == NULL)
	 return NULL;
     odom->rank = rank;
     for(i=0;i<rank;i++) {
	 odom->stop[i] = stop[i];
	 odom->max[i] = max[i];
	 odom->index[i] = 0;
     }
     return odom;
}

void
odom_free(Odometer* odom)
{
     if(odom) free(odom);
}

int
odom_more(Odometer* odom)
{
     return (odom->index[0] < odom->stop[0]);
}

int
odom_next(Odometer* odom)
{
     size_t i;
     for(i=odom->rank-1;i>=0;i--) {
	 odom->index[i]++;
	 if(odom->index[i] < odom->stop[i]) break;
	 if(i == 0) return 0; /* leave the 0th entry if it overflows */
	 odom->index[i] = 0; /* reset this position */
     }
     return 1;
}

/* Get the value of the odometer */
size_t*
odom_indices(Odometer* odom)
{
     return odom->index;
}

size_t
odom_offset(Odometer* odom)
{
     size_t offset;
     int i;

     offset = 0;
     for(i=0;i<odom->rank;i++) {
	 offset *= odom->max[i];
	 offset += odom->index[i];
     }
     return offset;
}

char*
printvector(int rank, size_t* vec)
{
    char svec[NC_MAX_VAR_DIMS*3+1];
    int i;
    svec[0] = '\0';
    for(i=0;i<rank;i++) {
	char s[3+1];
	if(i > 0) strlcat(svec,",",sizeof(svec));
	snprintf(s,sizeof(s),"%u",(unsigned)vec[i]);
	strlcat(svec,s,sizeof(svec));
    }
    return strdup(svec);
}

#ifdef USE_HDF5
void
hdf5_setoffset(Odometer* odom, size_t* chunksizes, hsize_t* offset)
{
     int i;
     for(i=0;i<odom->rank;i++)
	 offset[i] = odom->index[i] * chunksizes[i];
}

int
hdf5(const char* file_name, const char* var_name, int debug,
     int rank, size_t* dimlens, size_t* chunklens, size_t* chunkcounts
     )
{
     int i;
     hid_t fileid, grpid, datasetid;
     int* chunkdata = NULL; /*[CHUNKPROD];*/
     size_t chunkprod;
     Odometer* odom = NULL;
     hsize_t offset[NC_MAX_VAR_DIMS];
#ifdef HDF5_SUPPORTS_PAR_FILTERS
     int r;
     hid_t dxpl_id = H5P_DEFAULT; /*data transfer property list */
     unsigned int filter_mask = 0;
#endif
 
     if(debug) {
        H5Eset_auto2(H5E_DEFAULT,(H5E_auto2_t)H5Eprint,stderr);
    }
     
     chunkprod = 1;
     for(i=0;i<rank;i++)
	 chunkprod *= chunklens[i];

     if ((fileid = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) usage(NC_EHDFERR);
     if ((grpid = H5Gopen1(fileid, "/")) < 0) usage(NC_EHDFERR);
     if ((datasetid = H5Dopen1(grpid, var_name)) < 0) usage(NC_EHDFERR);

     if((odom = odom_new(rank,chunkcounts,dimlens))==NULL) usage(NC_ENOMEM);

     if((chunkdata = calloc(sizeof(int),chunkprod))==NULL) usage(NC_ENOMEM);

     printf("rank=%d dims=(%s) chunks=(%s)\n",rank,printvector(rank,dimlens),printvector(rank,chunklens));

     while(odom_more(odom)) {
 	 hdf5_setoffset(odom,chunklens,offset);
#ifdef DEBUG
	 fprintf(stderr,"(");
	 for(i=0;i<rank;i++)
	     fprintf(stderr,"%s%lu",(i > 0 ? "," : ""),(unsigned long)offset[i]);
	 fprintf(stderr,")\n");
	 fflush(stderr);
#endif
	 memset(chunkdata,0,sizeof(int)*chunkprod);
	 if(H5Dread_chunk(datasetid, dxpl_id, offset, &filter_mask, chunkdata) < 0) abort();
	 for(r=0;r<rank;r++)
	     printf("[%lu/%lu]",(unsigned long)odom->index[r],(unsigned long)offset[r]);
	 printf(" =");
	 for(r=0;r<chunkprod;r++)
	     printf(" %02d", chunkdata[r]);
	 printf("\n");
	 fflush(stdout);
	 odom_next(odom);
     }

     /* Close up. */
     if (H5Dclose(datasetid) < 0) abort();
     if (H5Gclose(grpid) < 0) abort();
     if (H5Fclose(fileid) < 0) abort();

     /* Cleanup */
     free(chunkdata);
     odom_free(odom);
     return 0;
}
#endif

#ifdef ENABLE_NCZARR

static void
nczarr_setoffset(Odometer* odom, size_t* chunksizes, size64_t* offset)
{
     int i;
     for(i=0;i<odom->rank;i++)
	 offset[i] = odom->index[i] * chunksizes[i];
}

char*
chunk_key(int rank, size_t* indices)
{
    char key[NC_MAX_VAR_DIMS*3+1];
    int i;
    key[0] = '\0';
    for(i=0;i<rank;i++) {
	char s[3+1];
	if(i > 0) strlcat(key,".",sizeof(key));
	snprintf(s,sizeof(s),"%u",(unsigned)indices[i]);
	strlcat(key,s,sizeof(key));
    }
    return strdup(key);
}

int
nczarr(const char* file_name, const char* var_name, int debug,
       int rank, size_t* dimlens, size_t* chunklens, size_t* chunkcounts
       )
{
    int r,stat = NC_NOERR;
    int* chunkdata = NULL; /*[CHUNKPROD];*/
    size_t chunkprod;
    Odometer* odom = NULL;
    size64_t zindices[NC_MAX_VAR_DIMS];
    size64_t offset[NC_MAX_VAR_DIMS];
    int ncid, varid;

    NC_UNUSED(debug);

    if((stat=nc_open(file_name,0,&ncid))) usage(stat);
    if((stat=nc_inq_varid(ncid,var_name,&varid))) usage(stat);

    chunkprod = 1;
    for(r=0;r<rank;r++)
	chunkprod *= chunklens[r];
    if((odom = odom_new(rank,chunkcounts,dimlens))==NULL) usage(NC_ENOMEM);
    
    if((chunkdata = calloc(sizeof(int),chunkprod))==NULL) usage(NC_ENOMEM);

    printf("rank=%d dims=(%s) chunks=(%s)\n",rank,printvector(rank,dimlens),printvector(rank,chunklens));

    while(odom_more(odom)) {
	memset(chunkdata,0,sizeof(int)*chunkprod);
        nczarr_setoffset(odom,chunklens,offset);
	for(r=0;r<rank;r++)
	    zindices[r] = (size64_t)odom->index[r];
        if((stat=NCZ_read_chunk(ncid, varid, zindices, chunkdata) < 0)) usage(stat);
	for(r=0;r<rank;r++)
            printf("[%lu/%lu]",(unsigned long)odom->index[r],(unsigned long)offset[r]);
	printf(" =");
	for(r=0;r<chunkprod;r++)
	    printf(" %02d", chunkdata[r]);
	printf("\n");
	fflush(stdout);
	odom_next(odom);
     }

     /* Cleanup */
     free(chunkdata);
     odom_free(odom);
     if((stat=nc_close(ncid))) usage(stat);
     return 0;
}
#endif

int
main(int argc, char** argv)
{
    int i,stat = NC_NOERR;
    const char* file_name = NULL;
    const char* var_name = NULL;
    int ncid, varid, dimids[NC_MAX_VAR_DIMS];
    int rank, vtype, storage;
    size_t dimlens[NC_MAX_VAR_DIMS];
    size_t chunklens[NC_MAX_VAR_DIMS];
    size_t chunkcounts[NC_MAX_VAR_DIMS];
    int debug = 0;
    int format,mode;
    int c;

    while ((c = getopt(argc, argv, "dv:")) != EOF) {
	switch(c) {
	case 'd':
	    debug = 1;
	    break;
	case 'v':
	    var_name = strdup(optarg);
	    break;
	case '?':
	   fprintf(stderr,"unknown option: '%c'\n",c);
	   exit(1);
	}
    }

    /* get file argument */
    argc -= optind;
    argv += optind;

    if (argc == 0) {
	fprintf(stderr, "no input file specified\n");
	exit(1);
    }

    file_name = strdup(argv[0]);

    if(file_name == NULL || strlen(file_name) == 0) {
	fprintf(stderr, "no input file specified\n");
	exit(1);
    }

    if(var_name == NULL || strlen(var_name) == 0) {
	fprintf(stderr, "no input var specified\n");
	exit(1);
    }

    /* Get info about the file type */
    if((stat=nc_open(file_name,0,&ncid))) usage(stat);

    if((stat=nc_inq_format_extended(ncid,&format,&mode))) usage(stat);

    /* Get the info about the var */
    if((stat=nc_inq_varid(ncid,var_name,&varid))) usage(stat);
    if((stat=nc_inq_var(ncid,varid,NULL,&vtype,&rank,dimids,NULL))) usage(stat);
    if(rank == 0) usage(NC_EDIMSIZE);
    if((stat=nc_inq_var_chunking(ncid,varid,&storage,chunklens))) usage(stat);
    if(storage != NC_CHUNKED) usage(NC_EBADCHUNK);

    for(i=0;i<rank;i++) {
	 if((stat=nc_inq_dimlen(ncid,dimids[i],&dimlens[i]))) usage(stat);
	 chunkcounts[i] = ceildiv(dimlens[i],chunklens[i]);
    }

    if((stat=nc_close(ncid))) usage(stat);

    switch (format) {	  
#ifdef USE_HDF5
    case NC_FORMATX_NC_HDF5:
        hdf5(file_name, var_name, debug, rank, dimlens, chunklens, chunkcounts);
	 break;
#endif
#ifdef ENABLE_NCZARR
    case NC_FORMATX_NCZARR:
        nczarr(file_name, var_name, debug, rank, dimlens, chunklens, chunkcounts);
	 break;
#endif
    default:
	 usage(NC_EINVAL);
	 break;
    }

    return 0;
}

