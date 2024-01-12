// Some common defines for the HDF5 SWMR mode tests

#ifndef NC_TEST_SWMR_COMMON_H
#define NC_TEST_SWMR_COMMON_H

#include "config.h"

#ifdef HAVE_UNISTD_H
#include "unistd.h"
#endif
#ifdef _WIN32
#include <Windows.h>
#endif

#include <stdio.h>

// Portability macro for sleep in seconds
#ifdef _WIN32
#define SLEEP(x) Sleep((x) * 1000)
#else
#define SLEEP(x) sleep((x))
#endif

#define FILE_NAME "test_hdf5_swmr_file.nc"
#define TIME_DIM_NAME "time"
#define BEAM_DIM_NAME "beam"
#define DEPTH_VAR_NAME "depth"
#define NUM_DIMS 1
#define TOTAL_LENGTH 10000

// netCDF function error code
static int status = 0;

/* This macro prints an error message with line number and name of
 * test program. */
#define ERR do {                                                        \
        fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
        fprintf(stderr, "Sorry! Unexpected result, %s, line: %d\n",     \
                __FILE__, __LINE__);                                    \
        fflush(stderr);                                                 \
        return 2;                                                       \
    } while (0)

/* This macro prints an error message with line number and name of
 * test program, along with netcdf error message */
#define ERR_MSG do {                                                      \
        fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
        fprintf(stderr, "Failed: %s, %s, line: %d\n",     \
                nc_strerror(status), __FILE__, __LINE__);                   \
        fflush(stderr);                                                 \
        return 2;                                                       \
    } while (0)


#endif /* NC_TEST_SWMR_COMMON_H */
