/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#else
#include <getopt.h>
#endif

#include "netcdf.h"
#include "ncpathmgr.h"
#include "netcdf_filter.h"

static const char* USAGE =
"ncfilteravail [-t <format>] <filtername>|<filterid> n"
"Options\n"
"  -h help"
"  -t the formatx name: nc4|nczarr (optional)\n"
"\n"
"  <filtername> the filter name \n"
"  <filterid> the filter id number \n"
"\n"
"Default format is nc4 if built, then nczarr if built else error.\n"
;

#undef DEBUG

struct Options {
    char* format;
    char* filtername;
    unsigned filterid;
    char* url;
} faoptions;

static const struct Filter {
    char* name;
    unsigned id;
} known_filters[] = {
{"deflate",1},
{"szip",4},
{"bzip2",307},
{"zstd",32015},
{"zstandard",32015},
{"blosc",32001},
{NULL,0}
};


static void usage(const char* msg);

static void
usage(const char* msg)
{
    if(msg != NULL) fprintf(stderr,"%s\n",msg);
    fprintf(stderr,"%s",USAGE);
    if(msg == NULL) exit(0); else exit(1);
}

static int
getformat(const char* sformat)
{
    if(sformat != NULL && strlen(sformat) > 0) {
        if(strcmp(sformat,"nc4")==0) return NC_FORMATX_NC4;
        if(strcmp(sformat,"nczarr")==0) return NC_FORMATX_NCZARR;
    }
#ifdef USE_HDF5
    return NC_FORMATX_NC4;
#else
#  ifdef ENABLE_NCZARR
    return NC_FORMATX_NCZARR;
#  endif
#endif
    return NC_FORMATX_UNDEFINED;
}


static int
makeurl(struct Options* faopt)
{
    int stat = NC_NOERR;
    int format;
    char url[4096];
    int pid;

#if defined(_WIN32) || defined(__MINGW32__)
    pid = (int)_getpid();
#else
    pid = (int)getpid();
#endif
    switch (format = getformat(faopt->format)) {
    case NC_FORMATX_NC4:
	snprintf(url,sizeof(url),"tmp_%d.nc",pid);
	break;
    case NC_FORMATX_NCZARR:
	snprintf(url,sizeof(url),"file://tmp_%d.file#mode=zarr,file",pid);
	break;
    default: stat = NC_EINVAL; break;
    }
    if(stat == NC_NOERR) faopt->url = strdup(url);
    return stat;
}

static int
makefilterid(struct Options* faopt)
{
    int stat = NC_NOERR;
    const struct Filter* f = NULL;
    for(f=known_filters;f->name;f++) {
        if(strcmp(f->name,faopt->filtername)==0) {
	    faopt->filterid = f->id;
	    goto done;
	}
    }
    /* See if it is a number */
    if(1!=sscanf(faopt->filtername,"%u",&faopt->filterid))
	{stat = NC_EINVAL; goto done;}
done:
    return stat;
}

int
main(int argc, char** argv)
{
    int c, ncid, stat = NC_NOERR;
    int yesno = 0;

    memset((void*)&faoptions,0,sizeof(faoptions));

    while ((c = getopt(argc, argv, "t:")) != EOF) {
	switch(c) {
	case 'h': usage(""); break;
	case 'x': faoptions.format = strdup(optarg); break;
	case '?':
	   usage("unknown option");
	   break;
	}
    }

    argc -= optind;
    argv += optind;

    /* If no filter arguments left or more than one, print usage message. */
    if (argc == 0)
       usage("no filter specified");
    if (argc > 1)
       usage("more than one filter specified");
    faoptions.filtername = strdup(argv[0]);

    if(makeurl(&faoptions)) {yesno = 0; goto done;}
    if(makefilterid(&faoptions)) {yesno = 0; goto done;}
    if((stat = nc_create(faoptions.url,NC_NETCDF4,&ncid))) goto done;
    switch(stat = nc_inq_filter_avail(ncid,faoptions.filterid)) {
    case NC_NOERR: yesno = 1; break;
    case NC_ENOFILTER: yesno = 0; stat = NC_NOERR; break;
    default: break;
    }
    if((stat = nc_abort(ncid))) goto done;

done:
    printf("%s",(yesno?"yes":"no"));
    if(stat) fprintf(stderr,"error: (%d) %s\n",stat,nc_strerror(stat));
    return (stat?1:0);
}

