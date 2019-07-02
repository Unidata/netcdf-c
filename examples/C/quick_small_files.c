/* Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012,
2013, 2014, 2015, 2016, 2017, 2018 University Corporation for
Atmospheric Research/Unidata. See \ref copyright file for more
info. */
/**
 *   @file
 * This example program is part of Unidata's netCDF library for
 * scientific data access.
 *
 * This program will create a large file in netCDF classic
 * format. From the netcdf docs:
 *
 * "If you don't use the unlimited dimension, only one variable can
 * exceed 2 Gbytes in size, but it can be as large as the underlying
 * file system permits. It must be the last variable in the dataset,
 * and the offset to the beginning of this variable must be less than
 * about 2 Gbytes."
 *
 * @author Ed Hartnett
*/

#include <netcdf.h>
#include <stdio.h>
#include <string.h>

/* This macro handles errors by outputting a message to stdout and
   then exiting. */
#define NC_EXAMPLE_ERROR 2 /* This is the exit code for failure. */
#define BAIL(e) do { \
printf("Bailing out in file %s, line %d, error:%s.\n", \
__FILE__, __LINE__, nc_strerror(e)); \
return NC_EXAMPLE_ERROR; \
} while (0)

#define NUMDIMS 1
#define NUMVARS 2
/* This dim len is the max size the first of two fixed size variables
   for an 8-byte type in classic format: int((2*31 - 4) / 8), that is,
   2 GB minus a bit, on an 8 byte boundary. That number is
   268435455. But this is the longest dim len which works. */
#define DIM_LEN 268435440

int
main()
{
   int ncid, spockid, kirkid, dimids[NUMDIMS];
   double val_out = 999.99;
   size_t index[NUMDIMS] = {1};
   int res;

   /* Create the netCDF classic format file. */
   if ((res = nc_create("example.nc", NC_CLOBBER, &ncid)))
      BAIL(res);

   /* Turn off fill mode to speed things up. */
   if ((res = nc_set_fill(ncid, NC_NOFILL, NULL)))
       BAIL(res);

   /* Define dimension. */
   if ((res = nc_def_dim(ncid, "longdim", DIM_LEN, dimids)))
      BAIL(res);

   /* Define two variables. */
   if ((res = nc_def_var(ncid, "spock", NC_DOUBLE, NUMDIMS,
			 dimids, &spockid)))
      BAIL(res);
   if ((res = nc_def_var(ncid, "kirk", NC_DOUBLE, NUMDIMS,
			 dimids, &kirkid)))
      BAIL(res);

   /* We're finished defining metadata. */
   if ((res = nc_enddef(ncid)))
      BAIL(res);

   if ((res = nc_put_var1_double(ncid, spockid, index, &val_out)))
      BAIL(res);

   /* We're done! */
   if ((res = nc_close(ncid)))
      BAIL(res);

   return 0;
}
