/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   @Author Dennis Heimbigner
*/

/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id: main.c,v 1.33 2010/05/26 21:43:36 dmh Exp $ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/main.c,v 1.33 2010/05/26 21:43:36 dmh Exp $ */

#ifndef TEST_NCZARR_UTILS_H
#define TEST_NCZARR_UTILS_H

#include "config.h"
#include <assert.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef _MSC_VER
#include "XGetopt.h"
#endif

#include "nc_tests.h"
#include "err_macros.h"
#include "ncbytes.h"
#include "zincludes.h"


static char* progname = NULL;

struct ITOptions {
    NCZM_IMPL impl;
    char* path;
    char* cloud;
    char* otherfragments;
} itoptions = {NCZM_UNDEF, NULL, NULL, NULL};

static void
test_usage(void)
{
    fprintf(stderr,"usage: <test> [-e <zmapimpl>]\n");
    exit(1);
}

/* strip off leading path; result is malloc'd */
static char *
ubasename(char *logident)
{
    char* sep;

    sep = strrchr(logident,'/');
#ifdef MSDOS
    if(sep == NULL) sep = strrchr(logident,'\\');
#endif
    if(sep == NULL) return logident;
    sep++; /* skip past the separator */
    return sep;
}

static void
setimpl(const char* name)
{
    if(strcasecmp(name,"s3")==0) itoptions.impl = NCZM_S3;
    else if(strcasecmp(name,"nz4")==0) itoptions.impl = NCZM_NC4;
    else if(strcasecmp(name,"nzf")==0) itoptions.impl = NCZM_FILE;
    else test_usage();
}

static const char*
implname(void)
{
    switch (itoptions.impl) {
    case NCZM_S3: return "s3";
    case NCZM_NC4: return "nz4";
    case NCZM_FILE: return "nzf";
    default: test_usage();
    }
    return NULL;
}

static void
buildpath(const char* target,NCZM_IMPL impl)
{
    NCbytes* buf = ncbytesnew();
    switch(itoptions.impl) {
    case NCZM_NC4:
    case NCZM_FILE:
	ncbytescat(buf,"file://");
	ncbytescat(buf,target);
	ncbytescat(buf,".");
	ncbytescat(buf,implname());
	ncbytescat(buf,"#mode=nczarr");
	ncbytescat(buf,",");
	ncbytescat(buf,implname());
	break;
    case NCZM_S3:
	ncbytescat(buf,itoptions.cloud);
	if(itoptions.cloud[strlen(itoptions.cloud)-1] != '/')
	    ncbytescat(buf,"/");
	ncbytescat(buf,target);
	ncbytescat(buf,"#mode=nczarr");
	ncbytescat(buf,",");
	ncbytescat(buf,implname());
	break;
    default: test_usage();
    }
    if(itoptions.otherfragments != NULL) {
        ncbytescat(buf,",");
        ncbytescat(buf,itoptions.otherfragments);
    }
    itoptions.path = ncbytesextract(buf);
    ncbytesfree(buf);
}

void
processoptions(int argc, char** argv, const char* base_file_name)
{
    int c;
    
    if(argc == 1) test_usage();    
    progname = nulldup(ubasename(argv[0]));

    while ((c = getopt(argc, argv, "e:c:F:")) != EOF)
      switch(c) {
	case 'e': /* zmap choice */
	    setimpl(optarg);
	    break;
	case 'c': /* cloud appliance url prefix*/
	    itoptions.cloud = strdup(optarg);
	    break;
	case 'F': /* fragments */
	    itoptions.otherfragments = strdup(optarg);
	    break;
	case '?':
  	    test_usage();
	    break;
      }

    argc -= optind;
    argv += optind;

    if(itoptions.impl == NCZM_UNDEF) itoptions.impl = NCZM_FILE;
    if(itoptions.impl == NCZM_S3 && itoptions.cloud == NULL) test_usage();

    buildpath(base_file_name,itoptions.impl);

}

void
clearoptions(void)
{
    nullfree(itoptions.path);
    nullfree(itoptions.cloud);
    nullfree(itoptions.otherfragments);
    nullfree(progname);
}

#endif /*TEST_NCZARR_UTILS_H*/
