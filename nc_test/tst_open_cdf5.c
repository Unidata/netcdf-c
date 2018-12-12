/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILE_NAME "bad_cdf5_begin.nc"

int main(int argc, char *argv[])
{
    char *fname=FILE_NAME;
    int err, nerrs=0, ncid;

    if (argc == 2) fname = argv[1];

    err = nc_open(fname, NC_NOWRITE, &ncid);
    if (err != NC_ENOTNC) {
        printf("Error: nc_open() expect NC_ENOTNC but got (%s)\n",
               nc_strerror(err));
        nerrs++;
    }
    else if (err == NC_NOERR) /* close file */
        nc_close(ncid);

    return (nerrs > 0);
}
