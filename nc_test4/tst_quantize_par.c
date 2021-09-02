/* This is part of the netCDF package.
   Copyright 2021 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test quantization of netcdf-4 variables with parallel
   I/O. Quantization is the zeroing-out of bits in float or double
   data beyond a desired precision.

   Ed Hartnett, 9/2/21
*/

#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"

#define TEST "tst_quantize"
#define FILE_NAME "tst_quantize_par.nc"
#define NDIM1 1
#define DIM_NAME_1 "DIM_1"
#define DIM_LEN_5 5
#define DIM_LEN_20 20
#define VAR_NAME_1 "VAR_1"
#define VAR_NAME_2 "VAR_2"
#define NSD_3 3
#define NSD_9 9

/* This var used to help print a float in hex. */
char pf_str[20];

/* This struct allows us to treat float as uint32_t
 * types. */
union FU {
    float f;
    uint32_t u;
};

/* This struct allows us to treat double points as uint64_t
 * types. */
union DU {
    double d;
    uint64_t u;
};

/* This function prints a float as hex. */
char *
pf(float myf)
{
    union {
	float f;
	uint32_t u;
    } fu;
    fu.f = myf;
    sprintf(pf_str, "0x%x", fu.u);
    return pf_str;
}

/* This function prints a double as hex. */
char *
pd(double myd)
{
    union {
	double d;
	uint64_t u;
    } du;
    du.d = myd;
    sprintf(pf_str, "0x%lx", du.u);
    return pf_str;
}

int
main(int argc, char **argv)
{

    int mpi_size, mpi_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* Initialize MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    /* Must be run on exactly 4 processors. */
    if (mpi_size != 4)
    {
        if (mpi_rank == 0)
            printf("Test must be run on 4 processors.\n");
        return 2;
    }

    if (mpi_rank == 0)
    {
        printf("\n*** Testing netcdf-4 variable quantization with parallel I/O.\n");
        printf("**** testing quantization on four processors...");
    }
    {
        int ncid, dimid, varid1, varid2;
        int quantize_mode_in, nsd_in;
        float float_data[DIM_LEN_5] = {1.11111111, 1.0, 9.99999999, 12345.67, .1234567};
        double double_data[DIM_LEN_5] = {1.1111111, 1.0, 9.999999999, 1234567890.12345, 123456789012345.0};
        size_t start[NDIM1], count[NDIM1] = {DIM_LEN_5};
        int x;

        /* Create a netcdf-4 file with two vars. */
        if (nc_create_par(FILE_NAME, NC_NETCDF4|NC_CLOBBER, comm, info, &ncid)) ERR;
        if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_20, &dimid)) ERR;
        if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
        if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

        /* Turn on quantize for both vars. */
        if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NSD_3)) ERR;
        if (nc_def_var_quantize(ncid, varid2, NC_QUANTIZE_BITGROOM, NSD_3)) ERR;

        /* Write some data. Each of the 4 processes writes the same 5
         * values, writing 20 in all. */
        start[0] = mpi_rank * DIM_LEN_5;
        if (nc_put_vara_float(ncid, varid1, start, count, float_data)) ERR;
        if (nc_put_vara_double(ncid, varid2, start, count, double_data)) ERR;

        /* Close the file. */
        if (nc_close(ncid)) ERR;

        {
            float float_in[DIM_LEN_5];
            double double_in[DIM_LEN_5];
            union FU {
                float f;
                uint32_t u;
            };

            union FU fin;
            /* union FU fout; */
            union FU xpect[DIM_LEN_5];
            union DU dfin;
            /* union DU dfout; */
            union DU double_xpect[DIM_LEN_5];

            xpect[0].u = 0x3f8e3000;
            xpect[1].u = 0x3f800fff;
            xpect[2].u = 0x41200000;
            xpect[3].u = 0x4640efff;
            xpect[4].u = 0x3dfcd000;
	    double_xpect[0].u = 0x3ff1c60000000000;
	    double_xpect[1].u = 0x3ff001ffffffffff;
	    double_xpect[2].u = 0x4023fe0000000000;
	    double_xpect[3].u = 0x41d265ffffffffff;
	    double_xpect[4].u = 0x42dc120000000000;

            /* Open the file and check metadata. */
            if (nc_open_par(FILE_NAME, NC_WRITE, comm, info, &ncid)) ERR;
            if (nc_inq_var_quantize(ncid, 0, &quantize_mode_in, &nsd_in)) ERR;
            if (quantize_mode_in != NC_QUANTIZE_BITGROOM || nsd_in != NSD_3) ERR;
            if (nc_inq_var_quantize(ncid, 1, &quantize_mode_in, &nsd_in)) ERR;
            if (quantize_mode_in != NC_QUANTIZE_BITGROOM || nsd_in != NSD_3) ERR;

            /* Check the data. */
            start[0] = mpi_rank * DIM_LEN_5;
            if (nc_get_vara_float(ncid, varid1, start, count, float_in)) ERR;
            if (nc_get_vara_double(ncid, varid2, start, count, double_in)) ERR;
            
            /* printf("\n"); */
            for (x = 0; x < DIM_LEN_5; x++)
            {
                /* fout.f = float_data[x]; */
                fin.f = float_in[x];
                /* printf ("float_data: %10f   : 0x%x  float_data_in: %10f   : 0x%x\n", */
                /*         float_data[x], fout.u, float_data[x], fin.u); */
                if (fin.u != xpect[x].u) ERR;
                /* dfout.d = double_data[x];		 */
	        dfin.d = double_in[x];
                /* printf("double_data: %15g   : 0x%16lx  double_data_in: %15g   : 0x%16lx\n", */
	        /*        double_data[x], dfout.u, double_data[x], dfin.u); */
                if (dfin.u != double_xpect[x].u) ERR;
            }

            /* Close the file again. */
            if (nc_close(ncid)) ERR;
        }
    }

    /* Shut down MPI. */
    MPI_Finalize();

    if (mpi_rank == 0)
    {
        SUMMARIZE_ERR;
        FINAL_RESULTS;
    }
    return 0;
}
