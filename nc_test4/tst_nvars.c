/* This test was contributed by Jeff Whitaker to illustrate
   a bug introduced in netcdf-c 4.3.1-rc5.

   nc_inq_nvars should return 1.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#define FILE_NAME "ref_tst_nvars.nc"
#define GRP_NAME "phony_group"

void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
        (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

int
main()
{

   int ncid, varid, grpid, numvars, retval;

   if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
   check_err(retval,__LINE__,__FILE__);

   if ((retval = nc_inq_grp_ncid(ncid, GRP_NAME, &grpid)))
   check_err(retval,__LINE__,__FILE__);

   if ((retval = nc_inq_nvars(grpid, &numvars)))
   check_err(retval,__LINE__,__FILE__);

   (void)fprintf(stdout,"number of vars %d\n", numvars);

   
   return (numvars != 1);
}
