/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   An over-length attribute-name key in a hand-authored .zattrs must not
   cause an out-of-bounds write in NCZ_inq_attname. Test that this is properly rejected.

   @author Ward Fisher
*/

#include <config.h>
#include <nc_tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir((p), 0700)
#endif

#include "netcdf.h"

#define ERR(r) do { \
    fprintf(stderr, "fail: line %d: (%d) %s\n", __LINE__, (r), nc_strerror((r))); \
    return 2; \
} while (0)

#define STORE_DIR   "tst_ghsa_rw7h_7ph9_2q8f.zarr"
#define ZGROUP_PATH STORE_DIR "/.zgroup"
#define ZATTRS_PATH STORE_DIR "/.zattrs"
#define STORE_URL   "file://" STORE_DIR "#mode=nczarr,zarr"
#define OVERSIZE    5000

static int
write_text_file(const char *path, const char *buf, size_t len)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL) return 1;
    if (fwrite(buf, 1, len, f) != len) { fclose(f); return 1; }
    return fclose(f) == 0 ? 0 : 1;
}

static void
cleanup(void)
{
    remove(ZATTRS_PATH);
    remove(ZGROUP_PATH);
#ifdef _WIN32
    _rmdir(STORE_DIR);
#else
    rmdir(STORE_DIR);
#endif
}

int
main(void)
{
    static const char zgroup[] = "{\"zarr_format\":2}";
    char *zattrs = NULL;
    size_t zattrs_len;
    int ncid = -1;
    int stat;
    char namebuf[NC_MAX_NAME + 1];

    cleanup();

    if (MKDIR(STORE_DIR) != 0) {
        fprintf(stderr, "fail: could not create %s\n", STORE_DIR);
        return 2;
    }

    if (write_text_file(ZGROUP_PATH, zgroup, sizeof(zgroup) - 1) != 0) {
        fprintf(stderr, "fail: could not write %s\n", ZGROUP_PATH);
        cleanup();
        return 2;
    }

    zattrs_len = 1 + 1 + OVERSIZE + 1 + 1 + 1 + 1;
    if ((zattrs = (char *)malloc(zattrs_len)) == NULL) {
        cleanup();
        return 2;
    }
    zattrs[0] = '{';
    zattrs[1] = '"';
    memset(zattrs + 2, 'A', OVERSIZE);
    zattrs[2 + OVERSIZE + 0] = '"';
    zattrs[2 + OVERSIZE + 1] = ':';
    zattrs[2 + OVERSIZE + 2] = '1';
    zattrs[2 + OVERSIZE + 3] = '}';

    if (write_text_file(ZATTRS_PATH, zattrs, zattrs_len) != 0) {
        fprintf(stderr, "fail: could not write %s\n", ZATTRS_PATH);
        free(zattrs);
        cleanup();
        return 2;
    }
    free(zattrs);
    zattrs = NULL;

    printf("*** Test: GHSA-rw7h-7ph9-2q8f OOB write in NCZ_inq_attname\n");

    stat = nc_open(STORE_URL, NC_NOWRITE, &ncid);
    if (stat == NC_EMAXNAME) {
        printf("*** PASS: nc_open rejected over-length .zattrs key (NC_EMAXNAME)\n");
        cleanup();
        return 0;
    }
    if (stat != NC_NOERR) {
        fprintf(stderr, "fail: unexpected nc_open status\n");
        ERR(stat);
    }

    stat = nc_inq_attname(ncid, NC_GLOBAL, 0, namebuf);
    (void)nc_close(ncid);

    if (stat == NC_EMAXNAME) {
        printf("*** PASS: nc_inq_attname rejected over-length name (NC_EMAXNAME)\n");
        cleanup();
        return 0;
    }

    fprintf(stderr,
            "fail: OOB-write regression: nc_inq_attname returned %d (%s)\n",
            stat, nc_strerror(stat));
    cleanup();
    return 2;
}