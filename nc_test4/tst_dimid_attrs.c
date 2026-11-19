/* This is part of the netCDF package.
   Copyright 2026 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Regression test for issue #2477: scale-only _Netcdf4Dimid writes. */
#include <nc_tests.h>
#include "err_macros.h"
#include <hdf5.h>
#include <H5DSpublic.h>

static int
check_int_attribute(hid_t dataset, const char *name, int count,
                    const int *expected)
{
    hid_t attr, space;
    int values[2];

    if ((attr = H5Aopen(dataset, name, H5P_DEFAULT)) < 0) ERR;
    if ((space = H5Aget_space(attr)) < 0) ERR;
    if (count < 1 || count > 2 ||
        H5Sget_simple_extent_npoints(space) != count) ERR;
    if (H5Aread(attr, H5T_NATIVE_INT, values) < 0) ERR;
    if (memcmp(values, expected, (size_t)count * sizeof(int))) ERR;
    if (H5Sclose(space) < 0) ERR;
    if (H5Aclose(attr) < 0) ERR;
    return 0;
}

static int
check_storage(const char *path, const char *dataset_name,
              const char **scale_names, int nscales,
              const int *dimids, const int *axes)
{
    hid_t file, dataset;
    int i;

    if ((file = H5Fopen(path, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
    if ((dataset = H5Dopen2(file, dataset_name, H5P_DEFAULT)) < 0) ERR;
    if (H5DSis_scale(dataset) != 0) ERR;
    /* Hidden attributes require an HDF5 check, not nc_inq_varnatts(). */
    if (H5Aexists(dataset, "_Netcdf4Dimid") != 0)
    {
        fprintf(stderr, "%s: ordinary dataset %s has _Netcdf4Dimid\n",
                path, dataset_name);
        err++;
    }
    if (check_int_attribute(dataset, "_Netcdf4Coordinates", 2, axes))
        return 2;
    if (H5Dclose(dataset) < 0) ERR;

    if ((dataset = H5Dopen2(file, "scalar", H5P_DEFAULT)) < 0) ERR;
    if (H5DSis_scale(dataset) != 0 ||
        H5Aexists(dataset, "_Netcdf4Dimid") != 0) ERR;
    if (H5Dclose(dataset) < 0) ERR;

    /* A blanket suppression of dimension IDs must fail this test. */
    for (i = 0; i < nscales; i++)
    {
        if ((dataset = H5Dopen2(file, scale_names[i], H5P_DEFAULT)) < 0) ERR;
        if (H5DSis_scale(dataset) != 1) ERR;
        if (check_int_attribute(dataset, "_Netcdf4Dimid", 1, &dimids[i]))
            return 2;
        if (H5Dclose(dataset) < 0) ERR;
    }
    if (H5Fclose(file) < 0) ERR;
    return 0;
}

static int
check_case(const char *path, int reversed)
{
    const char *names[3] = {"lev", "len", NULL};
    const char *data_name = reversed ? "sample" : "len";
    const char *dataset_name = reversed ? "sample" : "_nc4_non_coord_len";
    int ncid, dims[3], axes[2], varid, dataid, scalarid;
    int data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int got[8], got_axes[2], natts, dimid, rank;
    int i, pass, nscales = reversed ? 3 : 2;

    if (nc_create(path, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
    if (reversed)
    {
        names[0] = "lat";
        names[1] = "lon";
        names[2] = "lev";
        if (nc_def_dim(ncid, "lat", 2, &dims[0])) ERR;
        if (nc_def_dim(ncid, "lon", 3, &dims[1])) ERR;
        if (nc_def_dim(ncid, "lev", 4, &dims[2])) ERR;
        /* Reversed coordinate creation requests explicit scale IDs. */
        if (nc_def_var(ncid, "lon", NC_INT, 1, &dims[1], &varid)) ERR;
        if (nc_def_var(ncid, "lat", NC_INT, 1, &dims[0], &varid)) ERR;
        axes[0] = dims[0];
        axes[1] = dims[2];
    }
    else
    {
        if (nc_def_dim(ncid, "lev", 2, &dims[0])) ERR;
        if (nc_def_dim(ncid, "len", 4, &dims[1])) ERR;
        axes[0] = dims[0];
        axes[1] = dims[1];
        /* A multidimensional character coordinate is the other trigger. */
        if (nc_def_var(ncid, "lev", NC_CHAR, 2, axes, &varid)) ERR;
    }
    if (nc_def_var(ncid, data_name, NC_INT, 2, axes, &dataid)) ERR;
    if (nc_def_var(ncid, "scalar", NC_INT, 0, NULL, &scalarid)) ERR;
    /* Exercise existing-dataset writes after initial metadata creation. */
    if (nc_enddef(ncid)) ERR;
    if (nc_put_var_int(ncid, dataid, data)) ERR;
    if (nc_sync(ncid)) ERR;
    if (nc_close(ncid)) ERR;

    for (pass = 0; pass < 2; pass++)
    {
        if (pass)
        {
            if (nc_open(path, NC_WRITE, &ncid)) ERR;
            if (nc_close(ncid)) ERR;
        }
        if (nc_open(path, NC_NOWRITE, &ncid)) ERR;
        if (nc_inq_varid(ncid, data_name, &dataid)) ERR;
        if (nc_inq_varndims(ncid, dataid, &rank)) ERR;
        if (rank != 2) ERR;
        if (nc_inq_vardimid(ncid, dataid, got_axes)) ERR;
        if (nc_inq_varnatts(ncid, dataid, &natts)) ERR;
        if (nc_get_var_int(ncid, dataid, got)) ERR;
        if (memcmp(data, got, sizeof(data)) ||
            got_axes[0] != axes[0] || got_axes[1] != axes[1] || natts) ERR;
        for (i = 0; i < nscales; i++)
        {
            if (nc_inq_dimid(ncid, names[i], &dimid)) ERR;
            if (dimid != dims[i]) ERR;
        }
        if (nc_close(ncid)) ERR;
        if (check_storage(path, dataset_name, names, nscales, dims, axes))
            return 2;
    }
    return 0;
}

int
main(void)
{
    printf("*** Checking dimension IDs with a character coordinate...");
    check_case("tst_dimid_attrs_char.nc", 0);
    SUMMARIZE_ERR;
    printf("*** Checking dimension IDs with reversed coordinates...");
    check_case("tst_dimid_attrs_order.nc", 1);
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}