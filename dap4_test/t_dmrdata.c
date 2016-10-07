/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the netcdf-4 data building process.
*/

#include "t_dmr.h"

int
main(int argc, char** argv)
{
    int ret = NC_NOERR;

    setup(2,argc,argv);

#ifdef DEBUG
    fprintf(stderr,"t_dmrbuild %s -> %s\n",infile,outfile);
#endif

    if((ret = NCD4_parse(metadata))) goto done;
    if((ret = NCD4_metabuild(metadata,ncid))) goto done;
    if((ret = NCD4_databuild(metadata))) goto done;

done:
    return cleanup(ret);
}

#if 0
static void
printxml(const char* input)
{
    char* tree;
    ezxml_t dom = ezxml_parse_str(input,strlen(input));
    if(dom == NULL) exit(1);
    tree = ezxml_toxml(dom);
abort();
    fprintf(stderr,"////////////////////\n");
    fprintf(stderr,"%s\n",tree);
    fprintf(stderr,"////////////////////\n");
}
#endif
