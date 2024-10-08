/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
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
#include "netcdf_filter.h"
#include "netcdf_aux.h"
#include "ncplugins.h"

extern int NCstdbinary(void);

static const char* USAGE =
"ncpluginpath [-f global|hdf5|nczarr][-h][-n]"
"Options\n"
"  -f which plugin path to print.\n"
"     global - print the global plugin path\n"
"     hdf5 - print the hdf5 plugin path\n"
"     nczarr - print the nczarr plugin path\n"
"     If -f is not specified, then it defaults to 'global'\n"
"  -h print the usage message\n"
"  -n print the length of the current internal plugin path list\n"
"\n"
;

#undef DEBUG

static void usage(const char* msg);

static void
usage(const char* msg)
{
    if(msg != NULL) fprintf(stderr,"%s\n",msg);
    fprintf(stderr,"%s",USAGE);
    if(msg == NULL) exit(0); else exit(1);
}

static int
getformatx(const char* sformat)
{
    if(sformat != NULL && strlen(sformat) > 0) {
	if(strcmp(sformat,"global")==0) return NC_FORMATX_UNDEFINED;
        if(strcmp(sformat,"hdf5")==0) return NC_FORMATX_NC_HDF5;
        if(strcmp(sformat,"nczarr")==0) return NC_FORMATX_NCZARR;
    }
    return NC_FORMATX_UNDEFINED;
}

#if 0
static const char*
getsource(int formatx)
{
    switch (formatx) {
    case NC_FORMATX_NC_HDF5: return "hdf5";
    case NC_FORMATX_NCZARR: return "nczarr";
    default: break;
    }
    return "global";
}
#endif

static int
getfrom(int formatx, char** textp)
{
    int stat = NC_NOERR;
    NCPluginList dirs = {0,NULL};
    char* text = NULL;
   
    /* Get a plugin path */
    switch (formatx) {
    case 0: /* Global */
        if((stat=nc_plugin_path_get(&dirs))) goto done;
	break;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    case NC_FORMATX_NCZARR:
        if((stat=NCZ_plugin_path_get(&dirs))) goto done;
	break;
#endif
#ifdef USE_HDF5
    case NC_FORMATX_NC_HDF5:
	if((stat=NC4_hdf5_plugin_path_get(&dirs))) goto done;
	break;
#endif
    default: abort();
    }
    if((stat = ncaux_plugin_path_tostring(&dirs,';',&text))) goto done;
    *textp = text; text = NULL;
done:
    nullfree(text);
    ncaux_plugin_path_clear(&dirs);
    return stat;
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;
    int formatx = NC_FORMATX_UNDEFINED;
    char* text = NULL;
    int ndirsflag = 0;

    NCstdbinary(); /* avoid \r\n on windows */

    nc_initialize();

    while ((c = getopt(argc, argv, "f:hn")) != EOF) {
	switch(c) {
	case 'f':
	    formatx = getformatx(optarg);
	    break;
	case 'n':
	    ndirsflag = 1;
	    break;
	case '?':
	   usage("unknown option");
	   break;
	}
    }

    if(ndirsflag) {
	size_t ndirs = 0;
	char sndirs[64];
	if((stat = nc_plugin_path_ndirs(&ndirs))) goto done;
	snprintf(sndirs,sizeof(sndirs),"%zu",ndirs);
	text = strdup(sndirs);
    } else {
        if((stat = getfrom(formatx,&text))) goto done;
    }
    printf("%s",text); /* suppress trailing eol */
done:
    nullfree(text);
    nc_finalize();
    return (stat?1:0);
}

