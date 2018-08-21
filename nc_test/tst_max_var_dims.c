#include "tests.h"
#include "err_macros.h"

int main(int argc, char *argv[])
{
    int i, err, nerrs=0, ncid, dimid[NC_MAX_VAR_DIMS+2], varid;

    err = nc_create("tst_max_var_dims.nc", NC_CLOBBER, &ncid); CHECK_ERR(err)
    err = nc_def_dim(ncid, "dim0", NC_UNLIMITED, &dimid[0]); CHECK_ERR(err)
    err = nc_def_dim(ncid, "dim1", 1, &dimid[1]); CHECK_ERR(err)

    for (i=2; i<NC_MAX_VAR_DIMS+2; i++) dimid[i] = dimid[1];

    err = nc_def_var(ncid, "v0", NC_INT, NC_MAX_VAR_DIMS+1, &dimid[0], &varid);
    EXP_ERR(NC_EMAXDIMS,err)

    err = nc_def_var(ncid, "v1", NC_INT, NC_MAX_VAR_DIMS+1, &dimid[1], &varid);
    EXP_ERR(NC_EMAXDIMS,err)

    err = nc_set_fill(ncid, NC_NOFILL, NULL); CHECK_ERR(err)
    err = nc_close(ncid); CHECK_ERR(err)
    return (nerrs > 0);
}

