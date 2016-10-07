/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test the netcdf-4 metadata building process.
*/

#include "t_dmr.h"

int
main(int argc, char** argv)
{
    int ret = NC_NOERR;

    setup(2,argc,argv);

#ifdef DEBUG
    fprintf(stderr,"t_dmrmeta %s -> %s\n",infile,outfile);
#endif

    if((ret = NCD4_parse(metadata))) goto done;
    if((ret = NCD4_metabuild(metadata,ncid))) goto done;

done:
    return cleanup(ret);
}

