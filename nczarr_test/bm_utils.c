/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/*! \file Utility functions for tests. */

#include "config.h"
#include "netcdf.h"
#include "nc_tests.h"
#include "time.h"
#include "sys/time.h"
#include "bm_utils.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

static struct option options[] = {
{"treedepth", 1, NULL, OPT_TREEDEPTH},
{"ngroups", 1, NULL, OPT_NGROUPS},
{"ngroupattrs", 1, NULL, OPT_NGROUPATTRS},
{"ndims", 1, NULL, OPT_NDIMS},
{"ntypes", 1, NULL, OPT_NTYPES},
{"nvars", 1, NULL, OPT_NVARS},
{"varrank", 1, NULL, OPT_VARRANK},
{"nvarattrs", 1, NULL, OPT_NVARATTRS},
{"pathtemplate", 1, NULL, OPT_PATH},
{"format", 1, NULL, OPT_FORMAT},
{"f", 1, NULL, OPT_FILE},
{"X", 1, NULL, OPT_X},
{"D", 0, NULL, OPT_DEBUG},
{"W", 1, NULL, OPT_WDEBUG},
{"dims", 1, NULL, OPT_DIMS},
{"chunks", 1, NULL, OPT_CHUNKS},
{"cachesize", 1, NULL, OPT_CACHESIZE},
{"deflatelevel", 1, NULL, OPT_DEFLATELEVEL},
{NULL, 0, NULL, 0}
};

struct BMOptions bmoptions;

static char* dumpintlist(struct IntList* list);
static int parseintlist(const char* s0, struct IntList* list);

#if 0
/** Subtract the `struct timeval' values X and Y, storing the result in
   RESULT.  Return 1 if the difference is negative, otherwise 0.  This
   function from the GNU documentation. */
int
nc4_timeval_subtract (struct timeval *result, struct timeval *x, struct timeval*y)
{
   /* Perform the carry for the later subtraction by updating Y. */
   if (x->tv_usec < y->tv_usec) {
      int nsec = (y->tv_usec - x->tv_usec) / MILLION + 1;
      y->tv_usec -= MILLION * nsec;
      y->tv_sec += nsec;
   }
   if (x->tv_usec - y->tv_usec > MILLION) {
      int nsec = (x->tv_usec - y->tv_usec) / MILLION;
      y->tv_usec += MILLION * nsec;
      y->tv_sec -= nsec;
   }

   /* Compute the time remaining to wait.
      `tv_usec' is certainly positive. */
   result->tv_sec = x->tv_sec - y->tv_sec;
   result->tv_usec = x->tv_usec - y->tv_usec;

   /* Return 1 if result is negative. */
   return x->tv_sec < y->tv_sec;
}
#endif

int
nc4_buildpath(struct BMOptions* o, char** pathp)
{
    char* path = NULL;
    size_t len;

    len = strlen(o->filename);
    len++;
    len += strlen(formatname(o));
    len += strlen(o->pathtemplate);

    if((path=calloc(1,len+1))==NULL)
        {return NC_ENOMEM;}

    snprintf(path,len,o->pathtemplate,o->filename,formatname(o));
#ifdef DEBUG
fprintf(stderr,"buildpath: file=%s\n",path);
#endif
    if(pathp) {*pathp = path; path = NULL;}
    nullfree(path);
    return NC_NOERR;
}

EXTERNL int
bm_getoptions(int* argcp, char*** argvp, struct BMOptions* opt)
{
    int stat = NC_NOERR;
    int tag;
    int argc = *argcp;
    char** argv = *argvp;
    const char* nczarr_s3_test_url = "https://stratus.ucar.edu/unidata-netcdf-zarr-testing";

    memset((void*)opt,0,sizeof(struct BMOptions));
    if(argc <= 1) return NC_NOERR;
    while ((tag = getopt_long_only(argc, argv, "", options, NULL)) >= 0) {
#ifdef DEBUG
fprintf(stderr,"arg=%s value=%s\n",argv[optind-1],optarg);
#endif
        switch (tag) {
	    case OPT_TREEDEPTH:
		opt->meta.treedepth = atoi(optarg);
		break;
	    case OPT_NGROUPS:
		opt->meta.ngroups = atoi(optarg);
		break;
	    case OPT_NGROUPATTRS:
		opt->meta.ngroupattrs = atoi(optarg);
		break;
	    case OPT_NDIMS:
		opt->meta.ndims = atoi(optarg);
		break;
	    case OPT_NTYPES:
		opt->meta.ntypes = atoi(optarg);
		break;
	    case OPT_NVARS:
		opt->meta.nvars = atoi(optarg);
		break;
	    case OPT_VARRANK:
		opt->meta.varrank = atoi(optarg);
		break;
	    case OPT_NVARATTRS:
		opt->meta.nvarattrs = atoi(optarg);
		break;
	    case OPT_FILE:
	        opt->filename = strdup(optarg);
	        break;
  	    case OPT_PATH:
	        opt->pathtemplate = strdup(optarg);
	        break;
	    case OPT_FORMAT:
		if(strcasecmp(optarg,"file")==0) {
		    opt->format = NC_FORMATX_NCZARR;
		    opt->impl = NCZM_FILE;
		}  else if(strcasecmp(optarg,"zip")==0) {
		    opt->format = NC_FORMATX_NCZARR;
		    opt->impl = NCZM_ZIP;
		} else if(strcasecmp(optarg,"s3")==0) {
		    opt->format = NC_FORMATX_NCZARR;
		    opt->impl = NCZM_S3;
		} else 
		    {fprintf(stderr,"illegal format\n"); return NC_EINVAL;}
		break;
  	    case OPT_X:
		opt->xvalue |= atoi(optarg);
	        break;
  	    case OPT_DEBUG:
		opt->debug = 1;
	        break;
  	    case OPT_WDEBUG:
		opt->wdebug = atoi(optarg);
	        break;
  	    case OPT_CACHESIZE:
		opt->meta.cachesize = atoi(optarg);
	        break;
  	    case OPT_DEFLATELEVEL:
		opt->meta.deflatelevel = atoi(optarg);
	        break;
  	    case OPT_DIMS:
		if((stat=parseintlist(optarg,&opt->meta.dims))) return stat;
	        break;
  	    case OPT_CHUNKS:
		if((stat=parseintlist(optarg,&opt->meta.chunks))) return stat;
	        break;
   	    case ':':
	        fprintf(stderr,"missing argument\n");
	        return NC_EINVAL;
            case '?':
	    default:
	        fprintf(stderr,"unknown option\n");
	        return NC_EINVAL;
            }
    }

    if(opt->filename == NULL) return NC_ENOOBJECT;

    switch(opt->format) {
    case NC_FORMATX_NCZARR:
        if(opt->pathtemplate == NULL) {
	    size_t ptlen;
	    switch (opt->impl) {
	    case NCZM_S3:
		ptlen = strlen(nczarr_s3_test_url) + strlen("/netcdf-c/%s.%s#mode=nczarr,s3");
	        opt->pathtemplate = (char*)calloc(1,ptlen+1);
		if(opt->pathtemplate == NULL) return NC_ENOMEM;
		strlcat(opt->pathtemplate,nczarr_s3_test_url,ptlen+1);
		strlcat(opt->pathtemplate,"/netcdf-c/%s.%s#mode=nczarr,s3",ptlen+1);
		break;
	    case NCZM_FILE:
		opt->pathtemplate = strdup("file://%s.%s#mode=nczarr,file");
	    case NCZM_ZIP:
	        opt->pathtemplate = strdup("file://%s.%s#mode=nczarr,zip");
	    default: abort();
	    }
	}
	break;
    case NC_FORMATX_NC4:
        if(opt->pathtemplate == NULL) opt->pathtemplate = strdup("%s.%s");
	break;
    default:
	fprintf(stderr, "no template specified\n"); return NC_EINVAL;
    }

    argc -= optind;
    argv += optind;

#ifndef _WIN32
    if(opt->wdebug) {
	char s[64];
	snprintf(s,sizeof(s),"%u",opt->wdebug);
        setenv("NCZ_WDEBUG",s,1);
    }
#endif

    *argcp = argc;
    *argvp = argv;
    return NC_NOERR;
}

EXTERNL void
clearoptions(struct BMOptions* o)
{
    nullfree(o->pathtemplate);
    nullfree(o->filename);
    nullfree(o->meta.dims.list);
    nullfree(o->meta.chunks.list);
}

EXTERNL const char*
formatname(const struct BMOptions* o)
{
    switch (o->format) {
    case NC_FORMATX_NCZARR:
	switch (o->impl) {
	case NCZM_S3: return "s3";
	case NCZM_FILE: return "file";
	case NCZM_ZIP: return "zip";
	default: abort();
	}
	break;
    case NC_FORMATX_NC4: /* fall thru */
	return "nz4";
    default:
	abort();
    }
    return NULL;
}


EXTERNL void
reportoptions(struct BMOptions* o)
{
    fprintf(stderr,"--format=%d\n",o->format);
    fprintf(stderr,"--pathtemplate=%s\n",o->pathtemplate);
    fprintf(stderr,"--filename=%s\n",o->filename);
    fprintf(stderr,"--X=%d\n",o->xvalue);
    fflush(stderr);
}

EXTERNL void
reportmetaoptions(struct BMMeta* o)
{
    fprintf(stderr,"--treedepth=%d\n",o->treedepth);
    fprintf(stderr,"--ngroups=%d\n",o->ngroups);
    fprintf(stderr,"--ngroupattrs=%d\n",o->ngroupattrs);
    fprintf(stderr,"--ndims=%d\n",o->ndims);
    fprintf(stderr,"--ntypes=%d\n",o->ntypes);
    fprintf(stderr,"--nvars=%d\n",o->nvars);
    fprintf(stderr,"--varrank=%d\n",o->varrank);
    fprintf(stderr,"--nvarattrs=%d\n",o->nvarattrs);
    fprintf(stderr,"--dims={%s}\n",dumpintlist(&o->dims));
    fprintf(stderr,"--chunks={%s}\n",dumpintlist(&o->chunks));
}

static char*
dumpintlist(struct IntList* list)
{
    int i;
    static char s[4096];
    static char sn[64];
    s[0] = '\0';
    for(i=0;i<list->count;i++) {
	snprintf(sn,sizeof(sn),"%lu",(unsigned long)list->list[i]);
	if(i > 0) strlcat(s,",",sizeof(s));
	strlcat(s,sn,sizeof(s));
    }
    return s;
}

static int
parseintlist(const char* s0, struct IntList* intlist)
{
    int stat = NC_NOERR;
    char* s = NULL;
    char* p;
    int count,i;
    size_t* list = NULL;

    if(s0 == NULL || strlen(s0) == 0)
	{stat = NC_EINVAL; goto done;}
    if((s = strdup(s0))==NULL)
	{stat = NC_ENOMEM; goto done;}
    for(count=0,p=s;*p;p++) {
	if(*p == ',') {*p = '\0'; count++;}
	else if(strchr("0123456789",*p) == NULL)
	    {stat = NC_EINVAL; goto done;}
    }
    count++; /* For last entry */
    if((list = malloc(sizeof(size_t)*count))==NULL)
	{stat = NC_ENOMEM; goto done;}
    for(p=s,i=0;i<count;i++,p+=(1+strlen(p))) {
	list[i] = atol(p);
    }
    if(intlist) {
	intlist->count = count;
	intlist->list = list;
	list = NULL;
    }
done:
   nullfree(s);
   nullfree(list);
   return stat;
}

void
nccheck(int stat, int line)
{
    if(stat) {
        fprintf(stderr,"%d: %s\n",line,nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

const char*
bm_printvector(int rank, const size_t* vec)
{
    static char s[NC_MAX_VAR_DIMS*3+1];
    int i;

    s[0] = '\0';
    for(i=0;i<rank;i++) {
        char e[16];
	snprintf(e,sizeof(e),"%02u",(unsigned)vec[i]);
	if(i > 0) strcat(s,",");
	strcat(s,e);
    }
    return s;
}

const char*
bm_printvectorp(int rank, const ptrdiff_t* vec)
{
    size_t v[NC_MAX_VAR_DIMS];
    int i;
    for(i=0;i<rank;i++) v[i] = (size_t)vec[i];
    return bm_printvector(rank,v);
}

const char*
bm_varasprint(int rank, const size_t* start, const size_t* edges, const ptrdiff_t* stride)
{
    static char s[4096];
    static char tmp[4096];
    const char* sv;
    
    s[0] = '\0';
    if(stride == NULL) strcat(s,"vara{"); else strcat(s,"vars{");
    snprintf(tmp,sizeof(tmp),"rank=%u",(unsigned)rank); strcat(s,tmp);    
    strcat(s," start=("); sv = bm_printvector(rank,start); strcat(s,sv); strcat(s,")");
    strcat(s," edges=("); sv = bm_printvector(rank,edges); strcat(s,sv); strcat(s,")");
    if(stride != NULL) {
        strcat(s," stride=("); sv = bm_printvectorp(rank,stride); strcat(s,sv); strcat(s,")");
    }
    strcat(s,"}");
    return s;
}
