/*********************************************************************
 * Copyright 1993-2018, UCAR/Unidata
 * See COPYRIGHT file for copying and redistribution conditions.
 *
 * Test driver for netCDF implementation.  This program performs tests
 * against the netCDF specification for all user-level functions in an
 * implementation of the netCDF library.  Must be invoked from a
 * directory in which the invoker has write permission.
 *
 * Glenn Davis, Russ Rew, Ed Hartnett
 *********************************************************************/

#include <config.h>
#include <stdlib.h>
#include <netcdf.h>
#include <stdio.h>
#include <string.h>
#include "testcdf.h"		/* defines in-memory test netcdf structure */
#include "tests.h"

#define MAX_NUM_FORMATS 5

#undef DEBUG

#ifdef DEBUG
#define CATCH(x) {(x); if(nerrs > 0) breakpoint();}
static void breakpoint(void) {return;}
#else
#define CATCH(x) (x)
#endif

int current_format = 0;

/* Determine how many formats are available, and what they are. */
void
determine_test_formats(int *num_formats, int *format)
{
   int ind = 0;
   int num;

   /* We always have classic and 64-bit offset */
   num = 2;
   format[ind++] = NC_FORMAT_CLASSIC;
   format[ind++] = NC_FORMAT_64BIT_OFFSET;

   /* Do we have netCDF-4 and netCDF-4 classic? */
#ifdef USE_HDF5
   num += 2;
   format[ind++] = NC_FORMAT_NETCDF4_CLASSIC;
   format[ind++] = NC_FORMAT_NETCDF4;
#endif /* USE_HDF5 */

   /* Do we have CDF5? */
#ifdef ENABLE_CDF5
   num++;
   format[ind++] = NC_FORMAT_CDF5;
#endif /* ENABLE_CDF5 */

   *num_formats = num;
}

int
main(int argc, char **argv)
{
   /*EXTERNL int ncopts;	*/	/* netCDF error options */
   char *format_name[MAX_NUM_FORMATS] = {"classic", "64bit_offset", "netcdf4_classic",
                                         "netcdf4", "CDF5"};
   char testfile[NC_MAX_NAME];
   int format[MAX_NUM_FORMATS];
   int num_formats;
   int i, nerrs = 0;

   ncopts &= ~NC_FATAL;	/* make errors nonfatal */
   ncopts &= ~NC_VERBOSE;	/* turn off error messages */

   /* How many formats are we testing? */
   determine_test_formats(&num_formats, format);
   printf("Testing V2 API with %d different netCDF formats.\n", num_formats);

   for (i = 0; i < num_formats; i++)
   {
      current_format = format[i];

      /* Skip netCDF-4 - only netCDF-4 classic will work. */
      if (format[i] == NC_FORMAT_NETCDF4)
         continue;

      /* Come up with a test file name. */
      sprintf(testfile, "nctest_%s.nc", format_name[i]);
      printf("Testing %s with file %s.\n", format_name[i], testfile);

      /* Set the default format. */
      nc_set_default_format(format[i], NULL);

      /* Run all the tests for this format. */
      CATCH(nerrs += test_nccreate(testfile));
      CATCH(nerrs += test_ncopen(testfile));
      CATCH(nerrs += test_ncredef(testfile));
      CATCH(nerrs += test_ncendef(testfile));
      CATCH(nerrs += test_ncclose(testfile));
      CATCH(nerrs += test_ncinquire(testfile));
      CATCH(nerrs += test_ncsync(testfile));
      CATCH(nerrs += test_ncabort(testfile));
      CATCH(nerrs += test_ncdimdef(testfile));
      CATCH(nerrs += test_ncdimid(testfile));
      CATCH(nerrs += test_ncdiminq(testfile));
      CATCH(nerrs += test_ncdimrename(testfile));
      CATCH(nerrs += test_ncvardef(testfile));
      CATCH(nerrs += test_ncvarid(testfile));
      CATCH(nerrs += test_ncvarinq(testfile));
      CATCH(nerrs += test_ncvarputg(testfile));
      CATCH(nerrs += test_ncvarput1(testfile));
      CATCH(nerrs += test_ncvarget1(testfile));
      CATCH(nerrs += test_ncvarput(testfile));
      CATCH(nerrs += test_ncvarget(testfile));
      CATCH(nerrs += test_ncvarputg(testfile));
      CATCH(nerrs += test_ncvargetg(testfile));
      CATCH(nerrs += test_ncrecinq(testfile));
      CATCH(nerrs += test_ncrecput(testfile));
      CATCH(nerrs += test_ncrecget(testfile));
      CATCH(nerrs += test_ncvarrename(testfile));
      CATCH(nerrs += test_ncattput(testfile));
      CATCH(nerrs += test_ncattinq(testfile));
      CATCH(nerrs += test_ncattget(testfile));
      CATCH(nerrs += test_ncattcopy(testfile, "test2.nc"));
      CATCH(nerrs += test_ncattname(testfile));
      CATCH(nerrs += test_ncattrename(testfile));
      CATCH(nerrs += test_ncattdel(testfile));
      CATCH(nerrs += test_nctypelen());

      /* Clean up in-memory struct. */
      {
         int i;

         for (i = 0; i < test.ndims; i++)
            free(test.dims[i].name);

         for (i = 0; i < test.nvars; i++)
         {
            free(test.vars[i].name);
            free(test.vars[i].dims);
         }

         for (i = 0; i < test.natts; i++)
            free(test.atts[i].name);

      }
   }

   fprintf(stderr, "\nTotal number of failures: %d\n", nerrs);

   if (nerrs)
   {
      fprintf(stderr, "nctest FAILURE!!!\n");
      return 2;
   }
   else
      fprintf(stderr, "nctest SUCCESS!!!\n");

   return 0;
}
