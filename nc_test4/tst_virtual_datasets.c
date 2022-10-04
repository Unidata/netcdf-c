#include <hdf5.h>
#include <hdf5_hl.h>
#include <nc_tests.h>
#include "err_macros.h"

#define FILE_NAME_A "tst_virtual_a.nc"
#define FILE_NAME_B "tst_virtual_b.nc"
#define VARIABLE_SIZE 5
#define VARIABLE_NAME "v"

static void
create_dataset_a() {
    hsize_t dims[1] = {VARIABLE_SIZE};
    float data[VARIABLE_SIZE];
    size_t i;
    for(i = 0; i < VARIABLE_SIZE; ++i) {
        data[i] = (float)i;
    }

    hid_t file = H5Fcreate(FILE_NAME_A, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hid_t space = H5Screate_simple(1, dims, NULL);
    hid_t dset = H5Dcreate2(file, VARIABLE_NAME, H5T_IEEE_F32LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    H5Dwrite(dset, H5T_IEEE_F32LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    H5DSset_scale(dset, VARIABLE_NAME);

    H5Dclose(dset);
    H5Sclose(space);
    H5Fclose(file);
}

static void
create_dataset_b() {
    hsize_t dims[1] = {VARIABLE_SIZE};

    hid_t file = H5Fcreate(FILE_NAME_B, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hid_t src_space = H5Screate_simple(1, dims, NULL);
    hid_t space = H5Screate_simple(1, dims, NULL);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_virtual(dcpl, space, FILE_NAME_A, VARIABLE_NAME, src_space);
    hid_t dset = H5Dcreate2(file, VARIABLE_NAME, H5T_IEEE_F32LE, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5DSset_scale(dset, VARIABLE_NAME);

    H5Dclose(dset);
    H5Pclose(dcpl);
    H5Sclose(space);
    H5Sclose(src_space);
    H5Fclose(file);
}

int read_back_contents(const char* filename) {
    int ncid, varid, ndims;
    size_t i;
    float data[VARIABLE_SIZE];

    char var_name[NC_MAX_NAME+1];
    int dimids_var[1], var_type, natts;

    printf("\treading back %s.\n", filename);

    if (nc_open(filename, 0, &ncid)) ERR;
    if (nc_inq_varid(ncid, VARIABLE_NAME, &varid)) ERR;
    if (nc_inq_var(ncid, varid, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
    if (nc_get_var_float(ncid, varid, data)) ERR;
    printf("\t %s: ", var_name);
    for (i = 0; i < VARIABLE_SIZE; ++i) {
        printf("%f, ", data[i]);
    }
    printf("\n");
    if (nc_close(ncid)) ERR;
    SUMMARIZE_ERR;
    return 0;
}

int
main(int argc, char **argv)
{

    printf("\n*** Testing reading of virtual datasets.\n");

    create_dataset_a();
    create_dataset_b();

    int status;
    if((status = read_back_contents(FILE_NAME_A))) return status;
    if((status = read_back_contents(FILE_NAME_B))) return status;
}
