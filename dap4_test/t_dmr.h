/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/* Define various things common to all the t_dmr*.c testers */
#undef DEBUG
#undef DUMP

#include "d4includes.h"

#ifdef DEBUG
#include "ezxml.h"
#endif

static NCbytes* input = NULL;
static NCbytes* output = NULL;
static NCD4meta* metadata = NULL;
static char* infile = NULL;
static char* outfile = NULL;
static int ncid = 0;

static int
readfile(const char* filename, NCbytes* content)
{
    FILE* stream;
    char part[1024];

    stream = fopen(filename,"r");
    if(stream == NULL) return errno;
    for(;;) {
	size_t count = fread(part, 1, sizeof(part), stream);
	if(count <= 0) break;
	ncbytesappendn(content,part,count);
	if(ferror(stream)) {fclose(stream); return NC_EIO;}
	if(feof(stream)) break;
    }
    ncbytesnull(content);
    fclose(stream);
    return NC_NOERR;
}

static void
fail(int code)
{
    if(code != NC_NOERR)
	fprintf(stderr,"***Fail: %s\n",nc_strerror(code));
    exit((code==NC_NOERR?EXIT_SUCCESS:EXIT_FAILURE));
}

static void
setup(int expected, int argc, char** argv)
{
    int ret = NC_NOERR;
    if((argc - 1) < expected) {
	fprintf(stderr, "too few arguments\n");
	fail(NC_NOERR);
    }
    infile = argv[1];
    outfile = NULL;
    input = ncbytesnew();
    output = ncbytesnew();
    if((ret = readfile(infile,input))) fail(ret);

#ifdef DUMP
    NCD4_dumpbytes(ncbyteslength(input),ncbytescontents(input),0);
#endif

    if((metadata=NCD4_newmeta(NCD4_CSUM_ALL,ncbyteslength(input),ncbytescontents(input)))==NULL)
	fail(NC_ENOMEM);
    /* Create a fake NCD4INFO */
    {
	NCD4INFO* controller = (NCD4INFO*)calloc(1,sizeof(NCD4INFO));
	if(controller == NULL)
	    fail(NC_ENOMEM);
        metadata->controller = controller;
	controller->controls.translation = NCD4_TRANSNC4;
    }
    if(NCD4_isdmr(metadata->serial.rawdata)) {
	char* dmr = (char*)metadata->serial.rawdata;
	NCD4_setdmr(metadata,dmr);
    } else {
	if((ret=NCD4_dechunk(metadata))) fail(ret);
#ifdef DEBUG
	{
	int swap = (metadata->serial.hostbigendian != metadata->serial.remotebigendian);
	void* d = metadata->serial.dap;
	size_t sz = metadata->serial.dapsize;
	fprintf(stderr,"====================\n");
	fprintf(stderr,"%s\n",metadata->serial.dmr);
	fprintf(stderr,"----------\n");
	NCD4_dumpbytes(sz,d,swap);
	fprintf(stderr,"====================\n");
	fflush(stderr);
	}
#endif
    }
    if(expected > 1) {
        outfile = argv[2];
        if((ret = nc_create(outfile,NC_CLOBBER|NC_NETCDF4,&ncid))) fail(ret);
    }

#ifdef DEBUG
    {
	char* tree;
	ezxml_t dom = ezxml_parse_str(ncbytescontents(input),ncbyteslength(input));
	if(dom == NULL) exit(1);
	tree = ezxml_toxml(dom);
	fprintf(stderr,"////////////////////\n");
	fprintf(stderr,"%s\n",tree);
	fprintf(stderr,"////////////////////\n");
    }
#endif
    {
	const char* slevel = getenv("d4loglevel");
	int level;
	if(slevel != NULL && sscanf(slevel,"%d",&level) == 1) {
            nc_set_log_level(level);
	}
    }
}

int
cleanup(int ret)
{
    if(outfile != NULL) {
        if(ret != NC_NOERR)
            ret = nc_abort(ncid);
        else
            ret = nc_close(ncid);
    }	
    NCD4_reclaimMeta(metadata);
    ncbytesfree(input);
    ncbytesfree(output);
    if(ret)
	fail(ret);
    else
        exit(EXIT_SUCCESS);
    return 0;
}

#if 0
static void
printxml(const char* input)
{
    char* tree;
    ezxml_t dom = ezxml_parse_str(input,strlen(input));
    if(dom == NULL) exit(1);
    tree = ezxml_toxml(dom);
    fprintf(stderr,"////////////////////\n");
    fprintf(stderr,"%s\n",tree);
    fprintf(stderr,"////////////////////\n");
}
#endif
