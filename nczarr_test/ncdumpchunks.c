#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#include "netcdf.h"
#include "ncpathmgr.h"
#include "nclog.h"

#ifdef HAVE_HDF5_H
#include <hdf5.h>
#include <H5DSpublic.h>
#endif

#ifdef ENABLE_NCZARR
#include "zincludes.h"
#endif

#undef DEBUG

/* Short Aliases */
#ifdef HDF5_SUPPORTS_PAR_FILTERS
#define H5
#endif
#ifdef ENABLE_NCZARR
#define NZ
#endif

typedef struct Format {
    int format;
    char file_name[NC_MAX_NAME];
    char var_name[NC_MAX_NAME];
    int fillvalue;
    int debug;
    int rank;
    size_t dimlens[NC_MAX_VAR_DIMS];
    size_t chunklens[NC_MAX_VAR_DIMS];
    size_t chunkcounts[NC_MAX_VAR_DIMS];
    size_t chunkprod;
    size_t dimprod;
    nc_type xtype;
} Format;

typedef struct Odometer {
  size_t rank; /*rank */
  size_t start[NC_MAX_VAR_DIMS];
  size_t stop[NC_MAX_VAR_DIMS];
  size_t max[NC_MAX_VAR_DIMS]; /* max size of ith index */
  size_t index[NC_MAX_VAR_DIMS]; /* current value of the odometer*/
} Odometer;

#define floordiv(x,y) ((x) / (y))
#define ceildiv(x,y) (((x) % (y)) == 0 ? ((x) / (y)) : (((x) / (y)) + 1))

static char* captured[4096];
static int ncap = 0;

extern int nc__testurl(const char*,char**);

Odometer* odom_new(size_t rank, const size_t* stop, const size_t* max);
void odom_free(Odometer* odom);
int odom_more(Odometer* odom);
int odom_next(Odometer* odom);
size_t* odom_indices(Odometer* odom);
size_t odom_offset(Odometer* odom);
const char* odom_print(Odometer* odom);

static void
usage(int err)
{
     if(err != 0) {
	 fprintf(stderr,"Error: (%d) %s\n",err,nc_strerror(err));
     }
     fprintf(stderr,"usage: ncdumpchunks -v <var> <file> \n");
     fflush(stderr);
     exit(1);
}


const char*
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
    captured[ncap++] = strdup(svec);
    return captured[ncap-1];
}

void
cleanup(void)
{
    int i;
    for(i=0;i<ncap;i++)
        if(captured[i]) free(captured[i]);
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
	 odom->start[i] = 0;
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

const char*
odom_print(Odometer* odom)
{
    static char s[4096];
    static char tmp[4096];
    const char* sv;
    
    s[0] = '\0';
    snprintf(tmp,sizeof(tmp),"{rank=%u",(unsigned)odom->rank);
    strcat(s,tmp);    
    strcat(s," start=("); sv = printvector(odom->rank,odom->start); strcat(s,sv); strcat(s,")");
    strcat(s," stop=("); sv = printvector(odom->rank,odom->stop); strcat(s,sv); strcat(s,")");
    strcat(s," max=("); sv = printvector(odom->rank,odom->max); strcat(s,sv); strcat(s,")");
    snprintf(tmp,sizeof(tmp)," offset=%u",(unsigned)odom_offset(odom)); strcat(s,tmp);
    strcat(s," indices=("); sv = printvector(odom->rank,odom->index); strcat(s,sv); strcat(s,")");
    strcat(s,"}");
    return s;
}

#ifdef DEBUG
char*
chunk_key(int format->rank, size_t* indices)
{
    char key[NC_MAX_VAR_DIMS*3+1];
    int i;
    key[0] = '\0';
    for(i=0;i<format->rank;i++) {
	char s[3+1];
	if(i > 0) strlcat(key,".",sizeof(key));
	snprintf(s,sizeof(s),"%u",(unsigned)indices[i]);
	strlcat(key,s,sizeof(key));
    }
    return strdup(key);
}
#endif

void
setoffset(Odometer* odom, size_t* chunksizes, size_t* offset)
{
     int i;
     for(i=0;i<odom->rank;i++)
	 offset[i] = odom->index[i] * chunksizes[i];
}

static void
printindent(size_t indent)
{
    while(indent-- > 0) printf(" ");
}

static void
printchunk(Format* format, int* chunkdata, size_t indent)
{
    int k[3];
    int rank = format->rank;
    unsigned cols[3], pos;
    size_t* chl = format->chunklens;

    memset(cols,0,sizeof(cols));

    if(format->xtype == NC_UBYTE) rank = 0;
    switch (rank) {
    case 1:
        cols[0] = 1;
        cols[1] = 1;
        cols[2] = chl[0];	
	break;
    case 2:
        cols[0] = 1;
        cols[1] = chl[0];
        cols[2] = chl[1];	
	break;
    case 3:
        cols[0] = chl[0];
        cols[1] = chl[1];
        cols[2] = chl[2];	
	break;
    default:	
	cols[0] = 1;
	cols[1] = 1;
	cols[2] = format->chunkprod;
	break;
    }
//		offset = (((k0*chl[0])+k1)*chl[1])+k2;
    pos = 0;
    for(k[0]=0;k[0]<cols[0];k[0]++) {
        if(k[0] > 0) printindent(indent);
	k[1] = 0; k[2] = 0; /* reset */
        for(k[1]=0;k[1]<cols[1];k[1]++) {
	    k[2] = 0;
	    if(k[1] > 0) printf(" |");
	    for(k[2]=0;k[2]<cols[2];k[2]++) {
		if(format->xtype == NC_UBYTE) {
		    int l;
		    unsigned char* bchunkdata = (unsigned char*)(&chunkdata[pos]);
		    for(l=0;l<sizeof(int);l++) {
                        printf(" %02u", bchunkdata[l]);
		    }
		} else {
                    printf(" %02d", chunkdata[pos]);
		}
		pos++;
	    }
	}
	printf("\n");
    }

#if 0
    for(k=0;k<format->chunkprod;k++) {
	if(k > 0 && k % cols == 0) printf(" |");
        printf(" %02d", chunkdata[k]);
    }
    printf("\n");
#endif
}


int
dump(Format* format)
{
    void* chunkdata = NULL; /*[CHUNKPROD];*/
    Odometer* odom = NULL;
    int r;
    size_t offset[NC_MAX_VAR_DIMS];
    int holechunk = 0;
    char sindices[64];
#ifdef H5
    int i;
    hid_t fileid, grpid, datasetid;
    hid_t dxpl_id = H5P_DEFAULT; /*data transfer property list */
    unsigned int filter_mask = 0;
    hsize_t hoffset[NC_MAX_VAR_DIMS];
#endif
#ifdef NZ
    int stat = NC_NOERR;
    size64_t zindices[NC_MAX_VAR_DIMS];
    int ncid, varid;
#endif

#ifdef H5
    if(format->debug) {
        H5Eset_auto2(H5E_DEFAULT,(H5E_auto2_t)H5Eprint1,stderr);
    }
#endif

    memset(offset,0,sizeof(offset));
#ifdef H5
    memset(hoffset,0,sizeof(hoffset));
#endif

     switch (format->format) {
#ifdef H5
     case NC_FORMATX_NC_HDF5:
	if ((fileid = H5Fopen(format->file_name, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) usage(NC_EHDFERR);
        if ((grpid = H5Gopen1(fileid, "/")) < 0) usage(NC_EHDFERR);
        if ((datasetid = H5Dopen1(grpid, format->var_name)) < 0) usage(NC_EHDFERR);
	break; 
#endif
#ifdef NZ
     case NC_FORMATX_NCZARR:
	if((stat=nc_open(format->file_name,0,&ncid))) usage(stat);
	if((stat=nc_inq_varid(ncid,format->var_name,&varid))) usage(stat);
	break;	
#endif
     default: usage(NC_EINVAL);
     }

     if((odom = odom_new(format->rank,format->chunkcounts,format->dimlens))==NULL) usage(NC_ENOMEM);

     if((chunkdata = calloc(sizeof(int),format->chunkprod))==NULL) usage(NC_ENOMEM);

     printf("rank=%d dims=(%s) chunks=(%s)\n",format->rank,printvector(format->rank,format->dimlens),
                                                           printvector(format->rank,format->chunklens));

     while(odom_more(odom)) {
        setoffset(odom,format->chunklens,offset);

#ifdef DEBUG
	fprintf(stderr,"odom=%s\n",odom_print(odom));
	fprintf(stderr,"offset=(");
	for(i=0;i<format->rank;i++)
	    fprintf(stderr,"%s%lu",(i > 0 ? "," : ""),(unsigned long)offset[i]);
	fprintf(stderr,")\n");
	fflush(stderr);
#endif

	if(format->debug) {
	    fprintf(stderr,"chunk: %s\n",printvector(format->rank,offset));
	}

	holechunk = 0;
        switch (format->format) {
#ifdef H5
	case NC_FORMATX_NC_HDF5: {
	    for(i=0;i<format->rank;i++) hoffset[i] = (hsize_t)offset[i];
	    if(H5Dread_chunk(datasetid, dxpl_id, hoffset, &filter_mask, chunkdata) < 0)
	        holechunk = 1;
	} break;
#endif
#ifdef NZ
	case NC_FORMATX_NCZARR:
	    for(r=0;r<format->rank;r++) zindices[r] = (size64_t)odom->index[r];
            switch (stat=NCZ_read_chunk(ncid, varid, zindices, chunkdata)) {
	    case NC_NOERR: break;
	    case NC_EEMPTY: holechunk = 1; break;
	    default: usage(stat);
	    }
	    break;
#endif
	default: usage(NC_EINVAL);
	}
	if(holechunk) {
	    /* Hole chunk: use fillvalue */
	    size_t i = 0;
	    int* idata = (int*)chunkdata;
	    for(i=0;i<format->chunkprod;i++)
	        idata[i] = format->fillvalue;
	}
	sindices[0] = '\0';
	for(r=0;r<format->rank;r++) {
	    char sstep[64];
	    snprintf(sstep,sizeof(sstep),"[%lu/%lu]",(unsigned long)odom->index[r],(unsigned long)offset[r]);
	    strcat(sindices,sstep);
	}
	strcat(sindices," =");
	printf("%s",sindices);
	printchunk(format,chunkdata,strlen(sindices));
	fflush(stdout);
	odom_next(odom);
     }

     /* Close up. */
    switch (format->format) {
#ifdef H5
    case NC_FORMATX_NC_HDF5:
        if (H5Dclose(datasetid) < 0) abort();
        if (H5Gclose(grpid) < 0) abort();
        if (H5Fclose(fileid) < 0) abort();
	break;
#endif
#ifdef NZ
     case NC_FORMATX_NCZARR:
	if((stat=nc_close(ncid))) usage(stat);
	break;
#endif
    default: usage(NC_EINVAL);
    }
    /* Cleanup */
    free(chunkdata);
    odom_free(odom);
    return 0;
}

static const char* urlexts[] = {"file", "zip", NULL};

static const char*
filenamefor(const char* f0)
{
    static char result[4096];
    const char** extp;
    char* p;

    strcpy(result,f0); /* default */
    if(nc__testurl(f0,NULL)) goto done;
    /* Not a URL */
    p = strrchr(f0,'.'); /* look at the extension, if any */
    if(p == NULL) goto done; /* No extension */
    p++;
    for(extp=urlexts;*extp;extp++) {
        if(strcmp(p,*extp)==0) break;
    }
    if(*extp == NULL) goto done; /* not found */
    /* Assemble the url */
    strcpy(result,"file://");
    strcat(result,f0); /* core path */
    strcat(result,"#mode=nczarr,");
    strcat(result,*extp);
done:
    return result;
}

int
main(int argc, char** argv)
{
    int i,stat = NC_NOERR;
    Format format;
    int ncid, varid, dimids[NC_MAX_VAR_DIMS];
    int vtype, storage;
    int mode;
    int c;

    memset(&format,0,sizeof(format));

    /* Init some format fields */
    format.xtype = NC_INT;

    while ((c = getopt(argc, argv, "bv:DT:")) != EOF) {
    switch(c) {
	case 'b':
	    format.xtype = NC_UBYTE;
	    break;
	case 'v':
	    strcpy(format.var_name,optarg);
	    break;
	case 'D':
	    format.debug = 1;
	    break;
	case 'T':
	    nctracelevel(atoi(optarg));
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

    {
        char* s = NC_shellUnescape(argv[0]);
	strcpy(format.file_name,filenamefor(s));
	nullfree(s);
    }

    if(strlen(format.file_name) == 0) {
	fprintf(stderr, "no input file specified\n");
	exit(1);
    }

    if(strlen(format.var_name) == 0) {
	fprintf(stderr, "no input var specified\n");
	exit(1);
    }

    /* Get info about the file type */
    if((stat=nc_open(format.file_name,0,&ncid))) usage(stat);

    if((stat=nc_inq_format_extended(ncid,&format.format,&mode))) usage(stat);

    /* Get the info about the var */
    if((stat=nc_inq_varid(ncid,format.var_name,&varid))) usage(stat);
    if((stat=nc_inq_var(ncid,varid,NULL,&vtype,&format.rank,dimids,NULL))) usage(stat);
    if(format.rank == 0) usage(NC_EDIMSIZE);
    if((stat=nc_inq_var_chunking(ncid,varid,&storage,format.chunklens))) usage(stat);
    if(storage != NC_CHUNKED) usage(NC_EBADCHUNK);
    if((stat=nc_get_att(ncid,varid,"_FillValue",&format.fillvalue))) usage(stat);

    for(i=0;i<format.rank;i++) {
	 if((stat=nc_inq_dimlen(ncid,dimids[i],&format.dimlens[i]))) usage(stat);
	 format.chunkcounts[i] = ceildiv(format.dimlens[i],format.chunklens[i]);
    }

    if((stat=nc_close(ncid))) usage(stat);

    /* Precompute */
    for(format.chunkprod=1,i=0;i<format.rank;i++) format.chunkprod *= format.chunklens[i];
    for(format.dimprod=1,i=0;i<format.rank;i++) format.dimprod *= format.dimlens[i];

    dump(&format);

    cleanup();

    return 0;
}
