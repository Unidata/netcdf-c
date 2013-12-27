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
  int ncid1;
  int ncid2;
  int dimid1;
  int dimid2;
  char name[NC_MAX_NAME];
  size_t len;

  /* Create an in memory file */
  status = nc_create(path, mode, &ncid1);
  check_err(status, __LINE__, __FILE__);

  /* Add a dimension */
  status = nc_def_dim(ncid1, "dimension1", 10, &dimid1);
  check_err(status, __LINE__, __FILE__);

  status = nc_enddef(ncid1);
  check_err(status, __LINE__, __FILE__);

  /* Open the same memory file somewhere else. */
  status = nc_open(path, mode, &ncid2);
  check_err(status, __LINE__, __FILE__);

  /* Should be the same file */
  assert(ncid1 == ncid2);

  /* Read the id of  dimension1 */
  status = nc_inq_dimid(ncid2, "dimension1", &dimid2);
  check_err(status, __LINE__, __FILE__);

  /* Should be the same dimension */
  assert(dimid1 == dimid2);

  /* Let's check the name and the length */
  status = nc_inq_dim(ncid2, dimid2, name, &len);
  check_err(status, __LINE__, __FILE__);

  assert(len == 10);
  assert(strcmp(name, "dimension1") == 0);

  /* Close the file */
  status = nc_close(ncid1);
  check_err(status, __LINE__, __FILE__);

  /* File 2 should also be closed now. */
  status = nc_close(ncid2);
  /* so status should be a bad id error, if we close it twice */
  assert(status == NC_EBADID);

  SUMMARIZE_ERR;
  FINAL_RESULTS;

}
