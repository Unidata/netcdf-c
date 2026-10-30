/* This is part of the netCDF package. Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test that nc_open on a netCDF-4/HDF5 file copes with extended
   attributes (xattrs) when it looks for a DAOS container.

   The xattr calls are replaced by fakes below so the test does not
   depend on file system support for xattrs. The fakes make
   getxattr fail for one attribute (it returns -1), return an
   unterminated value for another, and return an empty value for a
   third. nc_open must still succeed and must go on to examine the
   attributes that follow the failing one. Run under valgrind or a
   sanitizer, this also detects leaks and out of bounds reads in the
   xattr handling.
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include "netcdf.h"
#ifdef USE_HDF5
#include <hdf5.h>
#endif

/* The DAOS check is only compiled in for HDF5 1.12 and later. */
#if defined(__linux__) && defined(HAVE_SYS_XATTR_H) && defined(USE_HDF5) && H5_VERSION_GE(1,12,0)

#define FILE_NAME "tst_isdaos.nc"

static const char xlist[] = "user.err\0user.full\0user.empty\0user.last\0";
static const char xfull[4] = {'a','b','c','d'}; /* deliberately not NUL terminated */

static int queried_last = 0;

ssize_t
listxattr(const char* path, char* list, size_t size)
{
    if(size == 0) return (ssize_t)sizeof(xlist);
    if(size < sizeof(xlist)) {errno = ERANGE; return -1;}
    memcpy(list, xlist, sizeof(xlist));
    return (ssize_t)sizeof(xlist);
}

ssize_t
getxattr(const char* path, const char* name, void* value, size_t size)
{
    const char* v = NULL;
    size_t len = 0;
    if(strcmp(name,"user.err") == 0) {errno = ENODATA; return -1;}
    if(strcmp(name,"user.full") == 0) {v = xfull; len = sizeof(xfull);}
    else if(strcmp(name,"user.empty") == 0) {len = 0;}
    else if(strcmp(name,"user.last") == 0) {v = "xyz"; len = 3; queried_last = 1;}
    else {errno = ENODATA; return -1;}
    if(size == 0) return (ssize_t)len;
    if(size < len) {errno = ERANGE; return -1;}
    if(len > 0) memcpy(value, v, len);
    return (ssize_t)len;
}

int
main(int argc, char **argv)
{
    int ncid, stat;

    printf("\n*** Testing nc_open with xattrs that cannot be read.\n");
    printf("*** creating file...");
    if((stat = nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid))) goto fail;
    if((stat = nc_close(ncid))) goto fail;
    printf("ok\n");

    printf("*** opening file...");
    queried_last = 0;
    if((stat = nc_open(FILE_NAME, NC_NOWRITE, &ncid))) goto fail;
    if((stat = nc_close(ncid))) goto fail;
    printf("ok\n");

    printf("*** checking that xattrs after the failing one were examined...");
    if(!queried_last) {
        printf("FAIL: xattr scan stopped at the failing xattr\n");
        return 1;
    }
    printf("ok\n");

    remove(FILE_NAME);
    printf("*** Tests successful!\n");
    return 0;
fail:
    printf("FAIL: %s\n", nc_strerror(stat));
    return 1;
}

#else /* xattr fakes not supported here */

int
main(int argc, char **argv)
{
    printf("\n*** Skipping xattr test: needs Linux, sys/xattr.h and HDF5 1.12 or later.\n");
    return 0;
}

#endif
