/* This is part of the netCDF package. Copyright 2026 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Regression test for GitHub issue #2738:
   "Attempting to create a variable using a compound with size larger
   or equal than 2**16 bytes fails"

   Root cause: var_create_dataset() in nc4hdf.c called H5Pset_fill_value
   for compound types. HDF5 stores the fill value in the object header,
   which has a fixed size limit. Compound types >= 64 KB cause
   H5Dcreate2 to fail with "object header message is too large".

   Fix: skip H5Pset_fill_value for NC_COMPOUND (and NC_VLEN). NetCDF-C
   handles fill values for these types via the provide_fill path in
   NC4_get_vars, so HDF5 does not need to store them.

   Edward Hartnett 4/19/26
*/

#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "tst_large_compound.nc"

/* ------------------------------------------------------------------ */
/* Test 1: compound type exactly 64 KB (the boundary case).           */
/* ------------------------------------------------------------------ */
static int
test_compound_64k(void)
{
    int ncid, typeid, dimid, varid;
    /* 256 fields of 256 chars each = 65536 bytes total */
    const size_t field_size  = 256;
    const int    nfields     = 256;
    const size_t type_size   = (size_t)nfields * field_size;
    char field_name[32];
    int dimsizes[1];
    int i, no_fill;
    size_t dimlen;
    void *data = NULL;
    void *rdata = NULL;

    if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_compound(ncid, type_size, "compound_64k", &typeid)) ERR;

    dimsizes[0] = (int)field_size;
    for (i = 0; i < nfields; i++)
    {
        snprintf(field_name, sizeof(field_name), "f%d", i);
        if (nc_insert_array_compound(ncid, typeid, field_name,
                                     (size_t)i * field_size,
                                     NC_CHAR, 1, dimsizes)) ERR;
    }

    if (nc_def_dim(ncid, "x", 3, &dimid)) ERR;
    if (nc_def_var(ncid, "v", typeid, 1, &dimid, &varid)) ERR;

    /* Fill mode should be on by default. */
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (no_fill) ERR;

    /* Turn fill mode off and verify. */
    if (nc_def_var_fill(ncid, varid, NC_NOFILL, NULL)) ERR;
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (!no_fill) ERR;

    /* Turn fill mode back on and verify. */
    if (nc_def_var_fill(ncid, varid, NC_FILL, NULL)) ERR;
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (no_fill) ERR;

    /* Write zeroed data. */
    data = calloc(3, type_size);
    if (!data) ERR;
    if (nc_put_var(ncid, varid, data)) { free(data); ERR; }
    free(data);

    if (nc_close(ncid)) ERR;

    /* Read it back and verify. */
    if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
    if (nc_inq_dimlen(ncid, dimid, &dimlen)) ERR;
    if (dimlen != 3) ERR;
    rdata = calloc(3, type_size);
    if (!rdata) ERR;
    if (nc_get_var(ncid, varid, rdata)) { free(rdata); ERR; }
    free(rdata);
    if (nc_close(ncid)) ERR;

    return 0;
}

/* ------------------------------------------------------------------ */
/* Test 2: compound type larger than 64 KB.                           */
/* ------------------------------------------------------------------ */
static int
test_compound_larger_than_64k(void)
{
    int ncid, typeid, dimid, varid;
    /* 512 fields of 256 chars = 131072 bytes (128 KB) */
    const size_t field_size = 256;
    const int    nfields    = 512;
    const size_t type_size  = (size_t)nfields * field_size;
    char field_name[32];
    int dimsizes[1];
    int i, no_fill;
    void *data = NULL;

    if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_compound(ncid, type_size, "compound_128k", &typeid)) ERR;

    dimsizes[0] = (int)field_size;
    for (i = 0; i < nfields; i++)
    {
        snprintf(field_name, sizeof(field_name), "f%d", i);
        if (nc_insert_array_compound(ncid, typeid, field_name,
                                     (size_t)i * field_size,
                                     NC_CHAR, 1, dimsizes)) ERR;
    }

    if (nc_def_dim(ncid, "x", 2, &dimid)) ERR;
    if (nc_def_var(ncid, "v", typeid, 1, &dimid, &varid)) ERR;

    /* Fill mode should be on by default. */
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (no_fill) ERR;

    /* Turn fill mode off and verify. */
    if (nc_def_var_fill(ncid, varid, NC_NOFILL, NULL)) ERR;
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (!no_fill) ERR;

    /* Turn fill mode back on and verify. */
    if (nc_def_var_fill(ncid, varid, NC_FILL, NULL)) ERR;
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (no_fill) ERR;

    data = calloc(2, type_size);
    if (!data) ERR;
    if (nc_put_var(ncid, varid, data)) { free(data); ERR; }
    free(data);

    if (nc_close(ncid)) ERR;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Test 3: small compound type works.                                 */
/* ------------------------------------------------------------------ */
static int
test_compound_small(void)
{
    int ncid, typeid, dimid, varid;
    int no_fill;
    struct { int x; float y; } write_data[4], read_data[4];
    int i;

    for (i = 0; i < 4; i++) { write_data[i].x = i; write_data[i].y = (float)i * 1.5f; }

    if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_compound(ncid, sizeof(write_data[0]), "small_cmp", &typeid)) ERR;
    if (nc_insert_compound(ncid, typeid, "x", offsetof(typeof(write_data[0]), x), NC_INT)) ERR;
    if (nc_insert_compound(ncid, typeid, "y", offsetof(typeof(write_data[0]), y), NC_FLOAT)) ERR;
    if (nc_def_dim(ncid, "x", 4, &dimid)) ERR;
    if (nc_def_var(ncid, "v", typeid, 1, &dimid, &varid)) ERR;

    /* Fill mode should be on by default. */
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (no_fill) ERR;

    /* Turn fill mode off and verify. */
    if (nc_def_var_fill(ncid, varid, NC_NOFILL, NULL)) ERR;
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (!no_fill) ERR;

    /* Turn fill mode back on and verify. */
    if (nc_def_var_fill(ncid, varid, NC_FILL, NULL)) ERR;
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (no_fill) ERR;

    if (nc_put_var(ncid, varid, write_data)) ERR;
    if (nc_close(ncid)) ERR;

    if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
    if (nc_get_var(ncid, varid, read_data)) ERR;
    for (i = 0; i < 4; i++)
    {
        if (read_data[i].x != i) ERR;
        if (read_data[i].y != (float)i * 1.5f) ERR;
    }
    if (nc_close(ncid)) ERR;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Test 4: fill mode is preserved across file close/reopen.           */
/*                                                                     */
/* Regression test for a bug introduced by the fix for issue #2738:   */
/* nc4hdf.c stopped calling H5Pset_fill_value for compound types to   */
/* avoid "object header too large" errors. As a side effect, when the */
/* file was reopened, hdf5open.c found no user-defined fill value in  */
/* the HDF5 dataset property and set no_fill=1 for every compound     */
/* variable, even those that were created with fill mode on.          */
/* ------------------------------------------------------------------ */
static int
test_compound_fill_mode_persist(void)
{
    int ncid, typeid, dimid, varid;
    int no_fill;
    struct { int x; float y; } write_data[4];
    int i;

    for (i = 0; i < 4; i++) { write_data[i].x = i; write_data[i].y = (float)i * 1.5f; }

    if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_compound(ncid, sizeof(write_data[0]), "persist_cmp", &typeid)) ERR;
    if (nc_insert_compound(ncid, typeid, "x", offsetof(typeof(write_data[0]), x), NC_INT)) ERR;
    if (nc_insert_compound(ncid, typeid, "y", offsetof(typeof(write_data[0]), y), NC_FLOAT)) ERR;
    if (nc_def_dim(ncid, "d", 4, &dimid)) ERR;
    if (nc_def_var(ncid, "v", typeid, 1, &dimid, &varid)) ERR;

    /* Do NOT call nc_def_var_fill - fill mode should default to on. */
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (no_fill) ERR;

    if (nc_put_var(ncid, varid, write_data)) ERR;
    if (nc_close(ncid)) ERR;

    /* Reopen and check that fill mode is still on (no_fill == 0).
       Before the bug was fixed, hdf5open.c set no_fill=1 for every
       compound variable because H5Pfill_value_defined() did not
       return H5D_FILL_VALUE_USER_DEFINED (since nc4hdf.c intentionally
       skips H5Pset_fill_value for compound types). */
    if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
    if (nc_inq_varid(ncid, "v", &varid)) ERR;
    if (nc_inq_var_fill(ncid, varid, &no_fill, NULL)) ERR;
    if (no_fill) ERR;
    if (nc_close(ncid)) ERR;

    return 0;
}

int
main(int argc, char **argv)
{
    printf("\n*** Testing large compound types (issue #2738).\n");

    printf("*** testing small compound type...");
    if (test_compound_small()) ERR;
    SUMMARIZE_ERR;

    printf("*** testing compound type exactly 64 KB...");
    if (test_compound_64k()) ERR;
    SUMMARIZE_ERR;

    printf("*** testing compound type larger than 64 KB (128 KB)...");
    if (test_compound_larger_than_64k()) ERR;
    SUMMARIZE_ERR;

    printf("*** testing compound fill mode persists across file reopen...");
    if (test_compound_fill_mode_persist()) ERR;
    SUMMARIZE_ERR;

    FINAL_RESULTS;
}
