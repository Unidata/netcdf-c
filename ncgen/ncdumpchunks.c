#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netcdf.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#if 0
#define FILE_NAME "tst_vars.nc"
#define VAR "v1"

#define RANK 3
static const size_t dimlens[RANK] = {4,4,2};
static const size_t chunksize[RANK] = {2,2,2};
static const size_t chunkcount[RANK] = {2,2,1};
static const size_t CHUNKPROD = 8;
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
void setoffset(Odometer* odom, size_t* chunksizes, hsize_t* offset);

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

int
main(int argc, char** argv)
{
    int i,stat = NC_NOERR;
    hid_t fileid, grpid, datasetid;
    int* chunkdata = NULL; /*[CHUNKPROD];*/
#ifdef HDF5_SUPPORTS_PAR_FILTERS
    int r;
    hid_t dxpl_id = H5P_DEFAULT; /*data transfer property list */
    unsigned int filter_mask = 0;
#endif
    const char* file_name = NULL;
    const char* var_name = NULL;
    int ncid, varid, dimids[NC_MAX_VAR_DIMS];
    int rank, vtype, storage;
    size_t dimlens[NC_MAX_VAR_DIMS];
    size_t chunklens[NC_MAX_VAR_DIMS];
    size_t chunkcounts[NC_MAX_VAR_DIMS];
    size_t chunkprod;
    Odometer* odom = NULL;
    hsize_t offset[NC_MAX_VAR_DIMS];

    if(argc < 3)
	usage(0);
    file_name = argv[1];
    var_name = argv[2];

    /* Get the info about the var */
    if((stat=nc_open(file_name,0,&ncid))) usage(stat);
    if((stat=nc_inq_varid(ncid,var_name,&varid))) usage(stat);
    if((stat=nc_inq_var(ncid,varid,NULL,&vtype,&rank,dimids,NULL))) usage(stat);
    if(rank == 0) usage(NC_EDIMSIZE);
    if((stat=nc_inq_var_chunking(ncid,varid,&storage,chunklens))) usage(stat);
    if(storage != NC_CHUNKED) usage(NC_EBADCHUNK);

    chunkprod = 1;
    for(i=0;i<rank;i++) {
	if((stat=nc_inq_dimlen(ncid,dimids[i],&dimlens[i]))) usage(stat);
	chunkcounts[i] = ceildiv(dimlens[i],chunklens[i]);
	chunkprod *= chunklens[i];
    }
    if((stat=nc_close(ncid))) usage(stat);

    if ((fileid = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) usage(NC_EHDFERR);
    if ((grpid = H5Gopen(fileid, "/", H5P_DEFAULT)) < 0) usage(NC_EHDFERR);
    if ((datasetid = H5Dopen1(grpid, var_name)) < 0) usage(NC_EHDFERR);

    if((odom = odom_new(rank,chunkcounts,dimlens))==NULL) usage(NC_ENOMEM);

    if((chunkdata = calloc(sizeof(int),chunkprod))==NULL) usage(NC_ENOMEM);

    while(odom_more(odom)) {
        setoffset(odom,chunklens,offset);
#ifdef DEBUG
	fprintf(stderr,"(");
	for(i=0;i<rank;i++)
            fprintf(stderr,"%s%lu",(i > 0 ? "," : ""),(unsigned long)offset[i]);
	fprintf(stderr,")\n");
        fflush(stderr);
#endif
	memset(chunkdata,0,sizeof(int)*chunkprod);
#ifdef HDF5_SUPPORTS_PAR_FILTERS
        if(H5Dread_chunk(datasetid, dxpl_id, offset, &filter_mask, chunkdata) < 0) abort();
	for(r=0;r<rank;r++)
            printf("[%lu/%lu]",(unsigned long)odom->index[r],(unsigned long)offset[r]);
        printf(" =");
	for(r=0;r<chunkprod;r++)
	    printf(" %02d", chunkdata[r]);
        printf("\n");
	fflush(stdout);
#endif
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

void
setoffset(Odometer* odom, size_t* chunksizes, hsize_t* offset)
{
    int i;
    for(i=0;i<odom->rank;i++)
        offset[i] = odom->index[i] * chunksizes[i];
}
