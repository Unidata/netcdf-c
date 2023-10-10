#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>
#endif

#include "netcdf.h"

#define filename "file://tmp_cachetest.zarr#mode=zarr,file"
#define chunkSize (size_t)(1<<17) /* 128k */
#define numCells (size_t)(50 * chunkSize)
#define numSteps (size_t)360

static float var[numCells];

size_t getPeakRSS(void)
{
  struct rusage rusage;
  getrusage( RUSAGE_SELF, &rusage );
#if defined(__APPLE__) && defined(__MACH__)
  return (size_t)rusage.ru_maxrss;
#else
  return (size_t)(rusage.ru_maxrss * 1024L);
#endif
}

static void
nce(int istat)
{
  if (istat != NC_NOERR)
    {
      fprintf(stderr, "%s\n", nc_strerror(istat));
      exit(-1);
    }
}

int
main(void)
{
  printf("read: chunkSize=%zu, numCells=%zu, numSteps=%zu, filename=%s\n", chunkSize, numCells, numSteps, filename);

  int ncId;
  nce(nc_open(filename, NC_NOWRITE, &ncId));

  int varId;
  nce(nc_inq_varid(ncId, "var", &varId));

  size_t size, nelems;
  float preemption;
  nce(nc_get_var_chunk_cache(ncId, varId, &size, &nelems, &preemption));
  printf("default chunk cache: size=%zu, nelems=%zu, preemption=%g\n", size, nelems, preemption);
  size = 4 * numCells; // one float field at one time step
  nelems = 1000;
  preemption = 0.5;
  nce(nc_set_var_chunk_cache(ncId, varId, size, nelems, preemption));
  printf("set chunk cache: size=%zu, nelems=%zu, preemption=%g\n", size, nelems, preemption);

  {
    for (size_t i = 0; i < numCells; ++i) var[i] = 0.0f;
    for (size_t i = 0; i < numSteps; ++i)
      {
        size_t start[2], count[2];
	start[0] = i; start[1] = 0;
	count[0] = 1; count[1] = numCells;
        nce(nc_get_vara_float(ncId, varId, start, count, var));
      }
  }

  nce(nc_close(ncId));

  {
    size_t mbused = getPeakRSS() / (1024 * 1024);
    printf("Max mem: %zu MB\n", mbused);
    if(mbused > 100) {
      fprintf(stderr,"*** Failed: used: %luMB expected: < 100MB\n",mbused);
      return (1);
    }
  }

  return 0;
}
