#include <netcdf.h>
#include <stdio.h>
#include <nc_tests.h>


void
check_err(const int stat, const int line, const char *file) {
  if (stat != NC_NOERR) {
    (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
    fflush(stderr);
    exit(1);
  }
}



int main(int argc, char *argv[])
{
  int status;
  const char *path = "diskless.nc";
  int mode = NC_DISKLESS;
  int ncid;

  /* Create an in memory file */
  status = nc_create(path, mode, &ncid);
  check_err(status, __LINE__, __FILE__);
  /* Open the same memory file somewhere else. */
  /* This now fails because the file is not found */
  status = nc_open(path, mode, &ncid);
  check_err(status, __LINE__, __FILE__);
  /* Maybe keep a hashtable of path->ncid and check when a diskless file is opened */

  /* Close the file */
  status = nc_close(ncid);
  check_err(status, __LINE__, __FILE__);

  SUMMARIZE_ERR;
  FINAL_RESULTS;

}
