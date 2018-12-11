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

#define ERR { \
    if (err != NC_NOERR) { \
        printf("Error at %s line %d: %s\n", __FILE__, __LINE__, \
               nc_strerror(err)); \
        nerrs++; \
    } \
}

#define EXP_ERR(exp) { \
    if (err != exp) { \
        printf("Error at %s:%d: expect %d but got %d\n", \
               __FILE__, __LINE__, exp,err); \
        nerrs++; \
    } \
}

static int default_format;

static int
create_check(char *fname, int cmode, int exp_format)
{
    int nerrs=0, err, exp_err=NC_NOERR, ncid, format;
    char *exp_str;

    switch (exp_format) {
        case NC_FORMAT_CLASSIC:      exp_str="NC_FORMAT_CLASSIC";      break;
        case NC_FORMAT_64BIT_OFFSET: exp_str="NC_FORMAT_64BIT_OFFSET"; break;
        case NC_FORMAT_64BIT_DATA:   exp_str="NC_FORMAT_64BIT_DATA";   break;
        case NC_FORMAT_NETCDF4:      exp_str="NC_FORMAT_NETCDF4";      break;
        case NC_FORMAT_NETCDF4_CLASSIC:
                                     exp_str="NC_FORMAT_NETCDF4_CLASSIC";break;
        default: break;
    }

#ifndef USE_HDF5
    if (cmode & NC_NETCDF4)
        exp_err = NC_ENOTBUILT;
#endif
#ifndef ENABLE_CDF5
    if (cmode & NC_64BIT_DATA)
        exp_err = NC_ENOTBUILT;
#endif

    /* create a file */
    cmode |= NC_CLOBBER;
    err = nc_create(fname, cmode, &ncid); EXP_ERR(exp_err)
    if (err == NC_ENOTBUILT) return nerrs;

    err = nc_close(ncid); ERR

    /* open the file and check its format */
    err = nc_open(fname, NC_NOWRITE, &ncid); ERR
    err = nc_inq_format(ncid, &format); ERR
    if (format != exp_format) {
        char *f_str="", *d_str="";
        switch (format) {
            case NC_FORMAT_CLASSIC:      f_str = "NC_FORMAT_CLASSIC";
                                         break;
            case NC_FORMAT_64BIT_OFFSET: f_str = "NC_FORMAT_64BIT_OFFSET";
                                         break;
            case NC_FORMAT_64BIT_DATA:   f_str = "NC_FORMAT_64BIT_DATA";
                                         break;
            case NC_FORMAT_NETCDF4:      f_str = "NC_FORMAT_NETCDF4";
                                         break;
            case NC_FORMAT_NETCDF4_CLASSIC: f_str = "NC_FORMAT_NETCDF4_CLASSIC";
                                         break;
            default: break;
        }
        switch (default_format) {
            case NC_FORMAT_CLASSIC:      d_str = "NC_FORMAT_CLASSIC";
                                         break;
            case NC_FORMAT_64BIT_OFFSET: d_str = "NC_FORMAT_64BIT_OFFSET";
                                         break;
            case NC_FORMAT_64BIT_DATA:   d_str = "NC_FORMAT_64BIT_DATA";
                                         break;
            case NC_FORMAT_NETCDF4:      d_str = "NC_FORMAT_NETCDF4";
                                         break;
            case NC_FORMAT_NETCDF4_CLASSIC: d_str = "NC_FORMAT_NETCDF4_CLASSIC";
                                         break;
            default: break;
        }

        printf("Error at %s line %d: default is %s and expect %s but got %s\n",
               __FILE__, __LINE__, d_str, exp_str, f_str);
        nerrs++;
    }
    err = nc_close(ncid); ERR
    return nerrs;
}

int main(int argc, char *argv[])
{
    char *fname="tst_default_format.nc";
    int err, nerrs=0, ncid, cmode;

    if (argc == 2) fname = argv[1];

    default_format = NC_FORMAT_CLASSIC;

    /* check illegal cmode */
    cmode = NC_64BIT_OFFSET | NC_64BIT_DATA;
    err = nc_create(fname, cmode, &ncid);
    if (err != NC_EINVAL) {
        printf("Error at %s line %d: expect NC_EINVAL but got %d\n",
               __FILE__, __LINE__, err);
        nerrs++;
    }

    /* check illegal cmode */
    cmode = NC_NETCDF4 | NC_64BIT_OFFSET;
    err = nc_create(fname, cmode, &ncid);
    if (err != NC_EINVAL) {
        printf("Error at %s line %d: expect NC_EINVAL but got %d\n",
               __FILE__, __LINE__, err);
        nerrs++;
    }

    /* create a file in CDF1 format */
    cmode = 0;
    nerrs += create_check(fname, cmode, NC_FORMAT_CLASSIC);

    /* create a file in CDF2 format */
    cmode = NC_64BIT_OFFSET;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_OFFSET);

    /* create a file in CDF5 format */
    cmode = NC_64BIT_DATA;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_DATA);

    /* set default file format to NC_FORMAT_64BIT_OFFSET ------------------*/
    default_format = NC_FORMAT_64BIT_OFFSET;
    err = nc_set_default_format(default_format, NULL); ERR

    /* create a file in default format */
    cmode = 0;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_OFFSET);

    /* create a file in CDF5 format (this should ignore default) */
    cmode = NC_64BIT_DATA;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_DATA);

#ifndef ENABLE_CDF5
    err = nc_set_default_format(NC_FORMAT_64BIT_DATA, NULL); EXP_ERR(NC_ENOTBUILT)
#else
    /* set default file format to NC_FORMAT_64BIT_DATA --------------------*/
    default_format = NC_FORMAT_64BIT_DATA;
    err = nc_set_default_format(default_format, NULL); ERR

    /* create a file in default format */
    cmode = 0;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_DATA);

    /* create a file in CDF2 format (this should ignore default) */
    cmode = NC_64BIT_OFFSET;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_OFFSET);
#endif

#ifndef USE_HDF5
    err = nc_set_default_format(NC_FORMAT_NETCDF4, NULL); EXP_ERR(NC_ENOTBUILT)
    err = nc_set_default_format(NC_FORMAT_NETCDF4_CLASSIC, NULL); EXP_ERR(NC_ENOTBUILT)
    nerrs += create_check(fname, NC_NETCDF4, NC_FORMAT_NETCDF4);
    nerrs += create_check(fname, NC_NETCDF4|NC_CLASSIC_MODEL, NC_FORMAT_NETCDF4_CLASSIC);
#else
    /* set default file format to NC_FORMAT_NETCDF4 -----------------------*/
    default_format = NC_FORMAT_NETCDF4;
    err = nc_set_default_format(default_format, NULL); ERR

    /* create a file in default format */
    cmode = 0;
    nerrs += create_check(fname, cmode, NC_FORMAT_NETCDF4);

    /* create a file in CDF2 format (this should ignore default) */
    cmode = NC_64BIT_OFFSET;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_OFFSET);

    /* set default file format to NC_FORMAT_NETCDF4_CLASSIC ---------------*/
    default_format = NC_FORMAT_NETCDF4_CLASSIC;
    err = nc_set_default_format(default_format, NULL); ERR

    /* create a file in default format */
    cmode = 0;
    nerrs += create_check(fname, cmode, NC_FORMAT_NETCDF4_CLASSIC);

    /* create a file in NETCDF4 format (this should ignore default) */
    cmode = NC_NETCDF4;
    nerrs += create_check(fname, cmode, NC_FORMAT_NETCDF4);

    /* create a file in CDF2 format (this should ignore default) */
    cmode = NC_64BIT_OFFSET;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_OFFSET);

    /* set default file format to NC_FORMAT_NETCDF4 -----------------------*/
    default_format = NC_FORMAT_NETCDF4;
    err = nc_set_default_format(default_format, NULL); ERR

    /* create a file in default format */
    cmode = 0;
    nerrs += create_check(fname, cmode, NC_FORMAT_NETCDF4);

    /* create a file in NETCDF4 format (this should ignore default) */
    cmode = NC_NETCDF4 | NC_CLASSIC_MODEL;
    nerrs += create_check(fname, cmode, NC_FORMAT_NETCDF4_CLASSIC);

    /* create a file in CDF2 format (this should ignore default) */
    cmode = NC_64BIT_OFFSET;
    nerrs += create_check(fname, cmode, NC_FORMAT_64BIT_OFFSET);
#endif

    return (nerrs > 0);
}
