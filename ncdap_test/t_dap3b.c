/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netcdf.h>
#define FILE_NAME "http://nomads.ncdc.noaa.gov:80/dods/SEAWINDS/clm/uvclm95to05"
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

#undef DEBUG

int
main()
{
    int ncid, varid;
    int retval,i;
    size_t indx;
    double timevals[2][12];
    double expected[2][2] = {{14.0, 14.0},{45.0, 45.0}};

    memset((void*)timevals,0,sizeof(timevals));
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid, "time", &varid)))
       ERR(retval);
    for(indx=0;indx<2;indx++) {
        for(i=0;i<2;i++) {
            if ((retval = nc_get_var1_double(ncid, varid, &indx, &timevals[indx][i])))
	        ERR(retval);
#ifdef DEBUG
printf("expected[%d][%d]=%g timevals[%d][%d]=%g\n",
indx,i,expected[indx][i],indx,i,timevals[indx][i]);
#endif
	    if(expected[indx][i] != timevals[indx][i]) {
		printf("*** FAIL: expected[%d][%d]=%g timevals[%d][%d]=%g\n",
				indx,i,expected[indx][i],indx,i,timevals[indx][i]);
		exit(1);
	    }
	}
    }
    if ((retval = nc_close(ncid)))
       ERR(retval);
    printf("*** PASS\n");
    return 0;
}
