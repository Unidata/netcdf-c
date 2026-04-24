/* This is part of the netCDF package. Copyright 2026 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Regression test for GitHub issues #2181 and #2160:
   - Crash reading NC_VLEN variable with unlimited dimension
   - Error with "charvlenbug" test (char VLEN with unlimited dim)

   These tests write partial data to a VLEN variable that has an
   unlimited dimension (triggering the fill-value path on read), then
   read back the entire variable and verify both the data values and
   that all memory can be freed without crashing.

   These bugs were actually fixed by Dennis some time ago. This test,
   run under the ASAN, proves that the fixes work.

   Ed Hartnett, 4/15/26
*/

#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include <stdlib.h>
#include <string.h>

#define FILE_INT   "tst_vlen_unlim_int.nc"
#define FILE_CHAR  "tst_vlen_unlim_char.nc"
#define FILE_2D    "tst_vlen_unlim_2d.nc"
#define FILE_2VAR  "tst_vlen_unlim_2var.nc"

/* ------------------------------------------------------------------ */
/* Test 1: 1-D VLEN(int) with unlimited dim, partial write.           */
/* Exercises the fill-value path in NC4_get_vars.                     */
/* ------------------------------------------------------------------ */
static int
test_vlen_int_unlim(void)
{
    int ncid, typeid, dimid, varid;
    int stat;
    nc_vlen_t write_data[3];
    nc_vlen_t read_data[5];  /* 5 slots; only 3 written */
    int buf0[] = {10, 20};
    int buf1[] = {30};
    int buf2[] = {40, 50, 60};
    size_t start, count;
    size_t i;

    write_data[0].len = 2; write_data[0].p = buf0;
    write_data[1].len = 1; write_data[1].p = buf1;
    write_data[2].len = 3; write_data[2].p = buf2;

    /* Write 3 elements starting at index 1 (leave index 0 as fill). */
    if (nc_create(FILE_INT, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_vlen(ncid, "vl", NC_INT, &typeid)) ERR;
    if (nc_def_dim(ncid, "x", NC_UNLIMITED, &dimid)) ERR;
    if (nc_def_var(ncid, "v", typeid, 1, &dimid, &varid)) ERR;
    start = 1; count = 3;
    if (nc_put_vara(ncid, varid, &start, &count, write_data)) ERR;
    if (nc_close(ncid)) ERR;

    /* Read back all 4 elements (0..3). Element 0 is fill. */
    if (nc_open(FILE_INT, NC_NOWRITE, &ncid)) ERR;
    memset(read_data, 0, sizeof(read_data));
    start = 0; count = 4;
    stat = nc_get_vara(ncid, varid, &start, &count, read_data);
    if (stat) ERR;

    /* Verify the 3 written elements. */
    if (read_data[1].len != 2) ERR;
    if (((int*)read_data[1].p)[0] != 10 || ((int*)read_data[1].p)[1] != 20) ERR;
    if (read_data[2].len != 1) ERR;
    if (((int*)read_data[2].p)[0] != 30) ERR;
    if (read_data[3].len != 3) ERR;
    if (((int*)read_data[3].p)[0] != 40) ERR;

    /* Reclaim all read memory including fill element 0. */
    if (nc_reclaim_data(ncid, typeid, read_data, 4)) ERR;
    if (nc_close(ncid)) ERR;

    return 0;
}

/* ------------------------------------------------------------------ */
/* Test 2: 1-D VLEN(char) with unlimited dim (the charvlenbug case).  */
/* ------------------------------------------------------------------ */
static int
test_vlen_char_unlim(void)
{
    int ncid, typeid, dimid, varid;
    nc_vlen_t write_data[2];
    nc_vlen_t read_data[4];
    size_t start, count;

    write_data[0].len = 1; write_data[0].p = "a";
    write_data[1].len = 2; write_data[1].p = "bc";

    /* Write at offset 1; offset 0 stays as fill. */
    if (nc_create(FILE_CHAR, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_vlen(ncid, "str", NC_CHAR, &typeid)) ERR;
    if (nc_def_dim(ncid, "x", NC_UNLIMITED, &dimid)) ERR;
    if (nc_def_var(ncid, "v", typeid, 1, &dimid, &varid)) ERR;
    start = 1; count = 2;
    if (nc_put_vara(ncid, varid, &start, &count, write_data)) ERR;
    if (nc_close(ncid)) ERR;

    /* Read all 3 elements (0..2). */
    if (nc_open(FILE_CHAR, NC_NOWRITE, &ncid)) ERR;
    memset(read_data, 0, sizeof(read_data));
    start = 0; count = 3;
    if (nc_get_vara(ncid, varid, &start, &count, read_data)) ERR;

    if (read_data[1].len != 1) ERR;
    if (((char*)read_data[1].p)[0] != 'a') ERR;
    if (read_data[2].len != 2) ERR;
    if (((char*)read_data[2].p)[0] != 'b') ERR;

    if (nc_reclaim_data(ncid, typeid, read_data, 3)) ERR;
    if (nc_close(ncid)) ERR;

    return 0;
}

/* ------------------------------------------------------------------ */
/* Test 3: 2-D VLEN(int) with unlimited dim first [NC_UNLIMITED][4].  */
/* Mirrors the exact tst_charvlenbug scenario but with int base type. */
/* ------------------------------------------------------------------ */
static int
test_vlen_2d_unlim_first(void)
{
    int ncid, typeid, dimid_u, dimid_b, dimids[2], varid;
    nc_vlen_t write_data[6];
    nc_vlen_t *read_data = NULL;
    size_t start[2], count[2];
    ptrdiff_t stride[2] = {1, 1};
    int b0[] = {1};
    int b1[] = {2, 3};
    int b2[] = {4};
    int b3[] = {5, 6, 7};
    int b4[] = {8};
    int b5[] = {9, 10};
    size_t total, i;
    size_t dimlen_u, dimlen_b;
    int ndims, dimids_r[NC_MAX_VAR_DIMS];

    write_data[0].len = 1; write_data[0].p = b0;
    write_data[1].len = 2; write_data[1].p = b1;
    write_data[2].len = 1; write_data[2].p = b2;
    write_data[3].len = 3; write_data[3].p = b3;
    write_data[4].len = 1; write_data[4].p = b4;
    write_data[5].len = 2; write_data[5].p = b5;

    if (nc_create(FILE_2D, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_vlen(ncid, "vl", NC_INT, &typeid)) ERR;
    if (nc_def_dim(ncid, "a", NC_UNLIMITED, &dimid_u)) ERR;
    if (nc_def_dim(ncid, "b", 4, &dimid_b)) ERR;
    dimids[0] = dimid_u;
    dimids[1] = dimid_b;
    if (nc_def_var(ncid, "v", typeid, 2, dimids, &varid)) ERR;

    /* Write 2 rows x 3 cols starting at {1,0}. */
    start[0] = 1; start[1] = 0;
    count[0] = 2; count[1] = 3;
    if (nc_put_vars(ncid, varid, start, count, stride, write_data)) ERR;
    if (nc_close(ncid)) ERR;

    /* Read back the full variable. */
    if (nc_open(FILE_2D, NC_NOWRITE, &ncid)) ERR;
    if (nc_inq_var(ncid, varid, NULL, NULL, &ndims, dimids_r, NULL)) ERR;
    if (nc_inq_dimlen(ncid, dimids_r[0], &dimlen_u)) ERR;
    if (nc_inq_dimlen(ncid, dimids_r[1], &dimlen_b)) ERR;
    total = dimlen_u * dimlen_b;

    read_data = (nc_vlen_t*)malloc(sizeof(nc_vlen_t) * total);
    if (!read_data) ERR;

    if (nc_get_var(ncid, varid, read_data)) { free(read_data); ERR; }

    /* Verify a sample written element: row 1, col 0 => write_data[0]. */
    if (read_data[dimlen_b + 0].len != 1) { free(read_data); ERR; }
    if (((int*)read_data[dimlen_b + 0].p)[0] != 1) { free(read_data); ERR; }

    /* Reclaim and free. This is the crash point without the fix. */
    if (nc_reclaim_data(ncid, typeid, read_data, total)) { free(read_data); ERR; }
    free(read_data);
    if (nc_close(ncid)) ERR;

    return 0;
}

/* ------------------------------------------------------------------ */
/* Test 4: exact issue #2181 reproduction.                            */
/* Two VLEN variables share the same unlimited dimension.             */
/* Var1 written at offset 1 (count 2), Var2 written at offset 2      */
/* (count 2).  Var2 pushes ulen to 4; Var1's HDF5 fdims is only 3.  */
/* Reading all of Var1 (indices 0-3) requires HDF5 to supply index   */
/* 0 as an internal fill cell — its .p pointer is HDF5-allocated and */
/* must not be freed with free().                                    */
/* ------------------------------------------------------------------ */
static int
test_vlen_two_vars_unlim(void)
{
    int ncid, typeid, dimid, varid1, varid2;
    nc_vlen_t data[4];
    double first[]  = {65, 66};
    double second[] = {67, 68};
    double third[]  = {69, 70};
    double fourth[] = {71, 72};
    size_t start, count;
    ptrdiff_t stride = 1;
    nc_vlen_t *data_read = NULL;
    size_t num_items;
    size_t ulen;

    data[0].p = first;  data[0].len = 2;
    data[1].p = second; data[1].len = 2;
    data[2].p = third;  data[2].len = 2;
    data[3].p = fourth; data[3].len = 2;

    /* Create file with one shared unlimited dim and two VLEN vars. */
    if (nc_create(FILE_2VAR, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
    if (nc_def_vlen(ncid, "RAGGED_DOUBLE", NC_DOUBLE, &typeid)) ERR;
    if (nc_def_dim(ncid, "x", NC_UNLIMITED, &dimid)) ERR;
    if (nc_def_var(ncid, "Var1", typeid, 1, &dimid, &varid1)) ERR;
    if (nc_def_var(ncid, "Var2", typeid, 1, &dimid, &varid2)) ERR;

    /* Write Var1 at offset 1 (extent becomes 3). */
    start = 1; count = 2;
    if (nc_put_vars(ncid, varid1, &start, &count, &stride, data)) ERR;

    /* Write Var2 at offset 2 (extent becomes 4, pushing ulen to 4). */
    start = 2; count = 2;
    if (nc_put_vars(ncid, varid2, &start, &count, &stride, data + 2)) ERR;

    if (nc_close(ncid)) ERR;

    /* Re-open and read all of Var1 (indices 0..3).                   */
    /* Index 0 is within Var1's HDF5 physical extent but was never    */
    /* written — HDF5 returns it as an internal fill.                 */
    /* Index 3 is beyond Var1's physical extent (netCDF fill path).   */
    if (nc_open(FILE_2VAR, NC_NOWRITE, &ncid)) ERR;
    if (nc_inq_dimlen(ncid, dimid, &ulen)) ERR;
    num_items = ulen; /* should be 4 */

    data_read = (nc_vlen_t*)calloc(num_items, sizeof(nc_vlen_t));
    if (!data_read) ERR;

    /* This is the call that crashes without the fix. */
    if (nc_get_var(ncid, varid1, data_read)) { free(data_read); ERR; }

    /* Verify the two written elements. */
    if (data_read[1].len != 2 || data_read[2].len != 2)
        { nc_reclaim_data(ncid, typeid, data_read, num_items); free(data_read); ERR; }
    if (((double*)data_read[1].p)[0] != 65.0)
        { nc_reclaim_data(ncid, typeid, data_read, num_items); free(data_read); ERR; }
    if (((double*)data_read[2].p)[0] != 67.0)
        { nc_reclaim_data(ncid, typeid, data_read, num_items); free(data_read); ERR; }

    /* Reclaim all — this is also the crash point without the fix. */
    if (nc_reclaim_data(ncid, typeid, data_read, num_items))
        { free(data_read); ERR; }
    free(data_read);
    if (nc_close(ncid)) ERR;

    return 0;
}

int
main(int argc, char **argv)
{
    printf("\n*** Testing VLEN with unlimited dimension (issues #2181, #2160).\n");

    printf("*** testing VLEN(int) + unlimited dim, partial write/read...");
    if (test_vlen_int_unlim()) ERR;
    SUMMARIZE_ERR;

    printf("*** testing VLEN(char) + unlimited dim (charvlenbug)...");
    if (test_vlen_char_unlim()) ERR;
    SUMMARIZE_ERR;

    printf("*** testing 2-D VLEN(int) + unlimited first dim...");
    if (test_vlen_2d_unlim_first()) ERR;
    SUMMARIZE_ERR;

    printf("*** testing two VLEN vars on same unlimited dim (issue #2181)...");
    if (test_vlen_two_vars_unlim()) ERR;
    SUMMARIZE_ERR;

    FINAL_RESULTS;
}
