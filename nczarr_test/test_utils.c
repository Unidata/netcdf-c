#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netcdf.h>
#include <ncpathmgr.h>
#include <nclist.h>
#include <ncuri.h>
#include <nclog.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#ifdef HAVE_HDF5_H
#include <hdf5.h>
#include <H5DSpublic.h>
#endif

#include "test_utils.h"

Options* options = NULL;
Metadata* meta = NULL;

NClist* capture = NULL;

void
usage(int err)
{
    if(err) {
	fprintf(stderr,"error: (%d) %s\n",err,nc_strerror(err));
    }
    fprintf(stderr,"usage:");
        fprintf(stderr," -d <dimlen>[,<dimlen>*]");
        fprintf(stderr," -c <chunklen>[,<chunklen>*]");
        fprintf(stderr," -s <start>[,<start>*]");
        fprintf(stderr," -e <edges>[,<edges>*]");
        fprintf(stderr," -i <stride>[,<stride>*]");
        fprintf(stderr," -v <int>[,<int>*]");
        fprintf(stderr," -x c|r|w");
        fprintf(stderr," <filename>");
	fprintf(stderr,"\n");	
    fflush(stderr);
    exit(1);
}

static void
CHECKRANK(int r)
{
    if(options->rank == 0)
        options->rank = r;
    else if(r != options->rank) {
        fprintf(stderr,"FAIL: options->rank mismatch\n");
	exit(1);
    }
}

int
getoptions(int* argcp, char*** argvp)
{
    int ret = NC_NOERR;
    int i,c;
    const char* p;

    /* initialize */
    if(options == NULL) {
        if((options = calloc(1,sizeof(Options))) == NULL)
	    {ret = NC_ENOMEM; goto done;}
    }
    /* Set defaults */
    options->mode = 0; /* classic netcdf-3 */

    while ((c = getopt(*argcp, *argvp, "34c:d:e:hi:m:n:p:s:v:D:O:T:X:")) != EOF) {
	switch(c) {
	case '3':
	    options->mode = 0;
	    break;
	case '4':
	    options->mode = NC_NETCDF4;
	    break;
	case 'c':
	    CHECKRANK(parsevector(optarg,options->chunks));
	    options->flags |= HAS_CHUNKS;
	    break;
	case 'd':
	    CHECKRANK(parsevector(optarg,options->dimlens));
	    options->flags |= HAS_DIMLENS;
	    break;
	case 'e':
	    CHECKRANK(parsevector(optarg,options->edges));
	    options->flags |= HAS_EDGES;
	    break;
	case 'h': usage(0); break;
	case 'i':
	    CHECKRANK(parsevector(optarg,options->stride));
	    options->flags |= HAS_STRIDE;
	    break;
	case 'm':
	    CHECKRANK(parsevector(optarg,options->max));
	    options->flags |= HAS_MAX;
	    break;
	case 'n':
	    CHECKRANK(atoi(optarg));
	    break;
	case 'p':
	    CHECKRANK(parsevector(optarg,options->stop));
	    options->flags |= HAS_STOP;
	    break;
	case 's':
	    CHECKRANK(parsevector(optarg,options->start));
	    options->flags |= HAS_START;
	    break;
	case 'v':
	    if(strcmp(optarg,"n")==0)
	        options->data = 0x7fffffff;
	    else
	        options->data = atoi(optarg);
	    options->flags |= HAS_DATA;
	    break;
	case 'D': 
	    options->debug = (unsigned)atoi(optarg);
	    break;
	case 'O': 
	    for(p=optarg;*p;p++) {
	        switch (*p) {
	        case 'c': options->create = Create; break;
	        case 'r': options->op = Read; break;
    	        case 'w': options->op = Write; break;
	        case 'x': options->op = Extend; break;
    	        case 'o': options->op = Odom; break;
	        case 'W': options->wholechunk = 1; break;
		default: fprintf(stderr,"Unknown operation '%c'\n",*p); exit(1);
	        }
	    } break;
	case 'T':
	    nctracelevel(atoi(optarg));
	    break;	
	case 'X': 
	    if(strcmp(optarg,"opt")==0) {
	        options->optimize = 1;
            } else if(strncmp(optarg,"wd",2)==0) {
		options->wdebug = atoi(optarg+2);
	    }
	    break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   exit(1);
	}
    }

    /* get file argument */
    *argcp -= optind;
    *argvp += optind;

    if(*argcp > 0) {
	char* p = NC_shellUnescape((*argvp)[0]);
        strcpy(options->file,filenamefor(p));
	nullfree(p);
    }

    /* Figure out the FORMATX for this file */
    if(options->file) {
	NCURI* uri = NULL;
	ncuriparse(options->file,&uri);
	if(uri == NULL) { /* not a url */
	    switch (options->mode) {
	    default: /* fall thru to default */
	    case 0: options->formatx = NC_FORMATX_NC3; break;
    	    case NC_NETCDF4: options->formatx = NC_FORMATX_NC4; break;
	    }
	} else {
	    options->formatx = NC_FORMATX_NCZARR; /* assume */
	    ncurifree(uri);
	}
    }
    if(options->debug) {
	const char* fmt = "unknown";
	switch(options->formatx) {
	case NC_FORMATX_NC3: fmt = "NC3"; break;
	case NC_FORMATX_NC4: fmt = "NC4"; break;
	case NC_FORMATX_NCZARR: fmt = "NCZARR"; break;
	default: break;
	}
	fprintf(stderr,"Formatx: %s\n",fmt);
    }

#ifndef _WIN32
    if(options->wdebug) {
	char s[64];
	snprintf(s,sizeof(s),"%u",options->wdebug);
        setenv("NCZ_WDEBUG",s,1);
    }
    if(options->optimize) {
	unsetenv("NCZ_NOOPTIMIZE");
    } else {
        setenv("NCZ_NOOPTIMIZE","1",1);
    }
#endif

    /* Default some vectors */
    if(!(options->flags & HAS_STRIDE)) {
        for(i=0;i<NC_MAX_VAR_DIMS;i++) {options->stride[i] = 1;}
	options->flags |= HAS_STRIDE;
    }

    /* Computed Defaults */
    if((options->flags & HAS_EDGES) && (options->flags & HAS_STOP)) {
        fprintf(stderr,"cannot specify both edges and stop\n");
	ERR(NC_EINVAL);
    }
    if(  !(options->flags & HAS_EDGES)
       && (options->flags & HAS_DIMLENS)
       && (options->flags & HAS_STRIDE)) {
        for(i=0;i<options->rank;i++)
            options->edges[i] = (options->dimlens[i]+options->stride[i]-1)/options->stride[i];
	options->flags |= HAS_EDGES;
    }
    if(  !(options->flags & HAS_STOP)
       && (options->flags & HAS_START)
       && (options->flags & HAS_EDGES)
       && (options->flags & HAS_STRIDE)) {
        for(i=0;i<options->rank;i++)
            options->stop[i] = options->start[i] + (options->edges[i] * options->stride[i]);
	options->flags |= HAS_STOP;
    }
    if(  !(options->flags & HAS_EDGES)
       && (options->flags & HAS_STRIDE)
       && (options->flags & HAS_STOP)) {
        for(i=0;i<options->rank;i++)
            options->edges[i] = ((options->stop[i]+(options->stride[i]-1)) / (options->stride[i]));
	options->flags |= HAS_EDGES;
    }	
    
    if(  !(options->flags & HAS_MAX)
       && (options->flags & HAS_STOP)) {
        for(i=0;i<NC_MAX_VAR_DIMS;i++) {options->max[i] = options->stop[i];}
	options->flags |= HAS_MAX;
    }

    if(options->create == Create) {
	if((ret=getmetadata(1)))
	    ERR(ret);
	if(meta->ncid && (ret = nc_close(meta->ncid)))
	    ERR(ret);
    }

    if(options->debug) {
#ifdef USE_HDF5
        H5Eset_auto2(H5E_DEFAULT,(H5E_auto2_t)H5Eprint1,stderr);
#endif
    }

done:
    return ret;
}

int
verifyoptions(Options* options)
{
    int ret = NC_NOERR;
    /* Check that we have or can compute relevant values */
    if(options->create == Create) {
        if(!(options->flags & HAS_DIMLENS)) {
            fprintf(stderr,"dimlens option not computable\n");
	    ERR(NC_EINVAL);
	}
        if(!(options->flags & HAS_CHUNKS)) {
            fprintf(stderr,"chunklens option not computable\n");
	    ERR(NC_EINVAL);
        }
        if(!(options->flags & HAS_STRIDE)) {
            fprintf(stderr,"stride option not computable\n");
	    ERR(NC_EINVAL);
        }
    }
    if(options->op == Read || options->op == Write) {
        if(!(options->flags & HAS_STOP)) {
            fprintf(stderr,"stop option not computable\n");
	    ERR(NC_EINVAL);
        }
        if(!(options->flags & HAS_EDGES)) {
            fprintf(stderr,"edges option not computable\n");
    	    ERR(NC_EINVAL);
        }
        if(!(options->flags & HAS_MAX)) {
            fprintf(stderr,"max option not computable\n");
	    ERR(NC_EINVAL);
        }
    }
    return ret;
}

int
getmetadata(int create)
{
    int ret = NC_NOERR;
    char dname[NC_MAX_NAME];
    int i;

    if(meta == NULL) {
	if((meta = calloc(1,sizeof(Metadata)))==NULL)
	    {ret = NC_ENOMEM; goto done;}
	/* Non-zero defaults */
	meta->fill = -1;
    }

    if(create) {
	if((options->flags & (HAS_DIMLENS | HAS_CHUNKS)) != (HAS_DIMLENS | HAS_CHUNKS)) {
	    fprintf(stderr,"dimlens or chunks not specified\n");
	    ret = NC_EINVAL;
	    goto done;
	}
        if((ret = nc_create(options->file,options->mode|NC_CLOBBER,&meta->ncid))) goto done;
        for(i=0;i<options->rank;i++) {
            snprintf(dname,sizeof(dname),"d%d",i);
	    if(options->dimlens[i] == 0)
                ret = nc_def_dim(meta->ncid,dname,NC_UNLIMITED,&meta->dimids[i]);
	    else
                ret = nc_def_dim(meta->ncid,dname,options->dimlens[i],&meta->dimids[i]);
	    if(ret) goto done;
        }
        if((ret = nc_def_var(meta->ncid,"v",NC_INT,options->rank,meta->dimids,&meta->varid))) goto done;
        if((ret = nc_def_var_fill(meta->ncid,meta->varid,0,&meta->fill))) goto done;
        if(options->formatx == NC_FORMATX_NC4 || options->formatx == NC_FORMATX_NCZARR) {
            if((ret = nc_def_var_chunking(meta->ncid,meta->varid,NC_CHUNKED,options->chunks))) goto done;
        }
        if((ret = nc_enddef(meta->ncid))) goto done;
    } else {/*Open*/
        if((ret = nc_open(options->file,options->mode|NC_WRITE,&meta->ncid))) goto done;
        for(i=0;i<options->rank;i++) {
            snprintf(dname,sizeof(dname),"d%d",i);
            if((ret = nc_inq_dimid(meta->ncid,dname,&meta->dimids[i]))) goto done;
            if((ret = nc_inq_dimlen(meta->ncid,meta->dimids[i],&options->dimlens[i]))) goto done;
        }
	options->flags |= HAS_DIMLENS;
        if((ret = nc_inq_varid(meta->ncid,"v",&meta->varid))) goto done;
        if(options->formatx == NC_FORMATX_NC4 || options->formatx == NC_FORMATX_NCZARR) {
	    int storage = -1;
	    /* Get chunk sizes also */
            if((ret = nc_inq_var_chunking(meta->ncid,meta->varid,&storage,options->chunks))) goto done;
	    if(storage != NC_CHUNKED) {ret = NC_EBADCHUNK; goto done;}
	    options->flags |= HAS_CHUNKS;
	}
    }
    
done:
    return ret;
}

void
cleanup(void)
{
    if(meta) {
        if(meta->ncid) {nc_close(meta->ncid); meta->ncid = 0;}
    }
    nclistfreeall(capture);
    nullfree(meta);
    nullfree(options);
}

int
parsevector(const char* s0, size_t* vec)
{
    char* s = strdup(s0);
    char* p = NULL;
    int i, done;

    if(s0 == NULL || vec == NULL) abort();

    for(done=0,p=s,i=0;!done;) {
	char* q;
	q = p;
        p = strchr(q,',');
        if(p == NULL) {p = q+strlen(q); done=1;}
        *p++ = '\0';
	if(strcasecmp(q,"u")==0)
            vec[i++] = 0; /* signals unlimited */
	else
            vec[i++] = (size_t)atol(q);
    }
    if(s) free(s);
    return i;
}

int
parsedata(const char* s0, int* data)
{
    char* s = strdup(s0);
    char* p = NULL;
    int i, done;


    if(s0 == NULL || data == NULL) abort();

    for(done=0,p=s,i=0;!done;) {
	char* q;
	q = p;
        p = strchr(q,',');
        if(p == NULL) {p = q+strlen(q); done=1;}
        *p++ = '\0';
        data[i++] = (size_t)atoi(q);
    }
    if(s) free(s);
    return i;
}

const char*
printvector(int rank, const size_t* vec)
{
    size64_t v64[NC_MAX_VAR_DIMS];
    int r;
    for(r=0;r<rank;r++) v64[r]= (size64_t)vec[r];
    return printvector64(rank,v64);
}

const char*
printvector64(int rank, const size64_t* vec)
{
    char s[NC_MAX_VAR_DIMS*3+1];
    int i;
    char* ss = NULL;

    s[0] = '\0';
    for(i=0;i<rank;i++) {
        char e[16];
	snprintf(e,sizeof(e),"%02u",(unsigned)vec[i]);
	if(i > 0) strcat(s,",");
	strcat(s,e);
    }
    if(capture == NULL) capture = nclistnew();
    ss = strdup(s);
    nclistpush(capture,ss);
    return ss;
}

Odometer*
odom_new(size_t rank, const size_t* start, const size_t* stop, const size_t* stride, const size_t* max)
{
     size_t i;
     Odometer* odom = NULL;
     if((odom = calloc(1,sizeof(Odometer))) == NULL)
	 return NULL;
     odom->rank = rank;
     for(i=0;i<rank;i++) {
	 odom->start[i] = start[i];
	 odom->stop[i] = stop[i];
 	 odom->stride[i] = stride[i];
	 odom->max[i] = (max?max[i]:stop[i]);
         odom->edges[i] = (odom->stop[i]+odom->stride[i]-1)/odom->stride[i];
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
	 odom->index[i] += odom->stride[i];
	 if(odom->index[i] < odom->stop[i]) break;
	 if(i == 0) return 0; /* leave the 0th entry if it overflows */
	 odom->index[i] = odom->start[i]; /* reset this position */
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
     size_t i;

     offset = 0;
     for(i=0;i<odom->rank;i++) {
	 offset *= odom->max[i];
	 offset += odom->index[i];
     }
     return offset;
}

const char*
odom_print1(Odometer* odom, int isshort)
{
    static char s[4096];
    static char tmp[4096];
    const char* sv;
    
    s[0] = '\0';
    strcat(s,"{");
    if(!isshort) {
        snprintf(tmp,sizeof(tmp),"rank=%u",(unsigned)odom->rank); strcat(s,tmp);    
        strcat(s," start=("); sv = printvector(odom->rank,odom->start); strcat(s,sv); strcat(s,")");
        strcat(s," stop=("); sv = printvector(odom->rank,odom->stop); strcat(s,sv); strcat(s,")");
        strcat(s," stride=("); sv = printvector(odom->rank,odom->stride); strcat(s,sv); strcat(s,")");
        strcat(s," max=("); sv = printvector(odom->rank,odom->max); strcat(s,sv); strcat(s,")");
        strcat(s," edges=("); sv = printvector(odom->rank,odom->edges); strcat(s,sv); strcat(s,")");
    }
    snprintf(tmp,sizeof(tmp)," offset=%u",(unsigned)odom_offset(odom)); strcat(s,tmp);
    strcat(s," indices=("); sv = printvector(odom->rank,odom->index); strcat(s,sv); strcat(s,")");
    strcat(s,"}");
    return s;
}

const char*
odom_print(Odometer* odom)
{
    return odom_print1(odom,0);
}

const char*
odom_printshort(Odometer* odom)
{
    return odom_print1(odom,1);
}

static const char* urlexts[] = {"nzf", "zip", "nz4", NULL};

nc_type
gettype(const char* name)
{
    if(strcasecmp(name,"byte")==0) return NC_BYTE;
    if(strcasecmp(name,"ubyte")==0) return NC_UBYTE;
    if(strcasecmp(name,"short")==0) return NC_SHORT;
    if(strcasecmp(name,"ushort")==0) return NC_USHORT;
    if(strcasecmp(name,"int")==0) return NC_INT;
    if(strcasecmp(name,"uint")==0) return NC_UINT;
    if(strcasecmp(name,"int64")==0) return NC_INT64;
    if(strcasecmp(name,"uint64")==0) return NC_UINT64;
    if(strcasecmp(name,"float")==0) return NC_FLOAT;
    if(strcasecmp(name,"double")==0) return NC_DOUBLE;
    return NC_NAT;
}

size_t
gettypesize(nc_type t)
{
    switch (t) {
    case NC_BYTE: return sizeof(char);
    case NC_UBYTE: return sizeof(unsigned char);
    case NC_SHORT: return sizeof(short);
    case NC_USHORT: return sizeof(unsigned short);
    case NC_INT: return sizeof(int);
    case NC_UINT: return sizeof(unsigned int);
    case NC_INT64: return sizeof(long long int);
    case NC_UINT64: return sizeof(unsigned long long int);
    case NC_FLOAT: return sizeof(float);
    case NC_DOUBLE: return sizeof(double);
    default: break;
    }
    return 0;
}

const char*
gettypename(nc_type t)
{
    switch (t) {
    case NC_BYTE: return "byte";
    case NC_UBYTE: return "ubyte";
    case NC_SHORT: return "short";
    case NC_USHORT: return "ushort";
    case NC_INT: return "int";
    case NC_UINT: return "uint";
    case NC_INT64: return "int64";
    case NC_UINT64: return "uint64";
    case NC_FLOAT: return "float";
    case NC_DOUBLE: return "double";
    default: break;
    }
    return NULL;
}

const char*
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

static char s3testurl[8192];
static char* s3testurlp = NULL;

const char*
ncz_gets3testurl(void)
{
    char* s;
    if(s3testurlp == NULL) {
	s3testurl[0] = '\0';
	strcat(s3testurl,"https://");
        s = getenv("NCZARR_S3_TEST_HOST");
	if(s == NULL) s = "stratus.ucar.edu";
	strcat(s3testurl,s);
	strcat(s3testurl,"/");
        s = getenv("NCZARR_S3_TEST_BUCKET");
	if(s == NULL) s = "unidata-netcdf-zarr-testing";
	strcat(s3testurl,s);
	s3testurlp = s3testurl;
    }
    return s3testurlp;
}

void
ncz_report(int err, int lineno)
{
    fprintf(stderr,"Error: %d: %s\n", lineno, nc_strerror(err));
    exit(1);
}

