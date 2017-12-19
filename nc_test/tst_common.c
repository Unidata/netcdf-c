/* This is part of the netCDF package. Copyright 2005-2007 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Common code shared by several tests.

   Ed Hartnett, 12/19/17
*/

#include "config.h"
#include <nc_tests.h>

/**
 * Determine how many formats are available, and what they are. 
 *
 * @param num_formats Pointer that gets the number of available formats.
 * @param format Array that gets the available formats.
 *
 * @author Ed Hartnett
 */
void
determine_test_formats(int *num_formats, int *format)
{
   int ind = 0;
   int num;

   /* Check inputs. */
   assert(num_formats && format);

   /* We always have classic and 64-bit offset */
   num = 2;
   format[ind++] = NC_FORMAT_CLASSIC;
   format[ind++] = NC_FORMAT_64BIT_OFFSET;

   /* Do we have netCDF-4 and netCDF-4 classic? */
#ifdef USE_NETCDF4
   num += 2;
   format[ind++] = NC_FORMAT_NETCDF4;
   format[ind++] = NC_FORMAT_NETCDF4_CLASSIC;
#endif /* USE_NETCDF4 */

   /* Do we have CDF5? */
#ifdef ENABLE_CDF5
   num++;
   format[ind++] = NC_FORMAT_CDF5;
#endif /* ENABLE_CDF5 */

   *num_formats = num;
}

