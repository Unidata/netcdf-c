/* This is part of the netCDF package.
   Copyright 2021 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test quantization of netcdf-4 variables. Quantization is the
   zeroing-out of bits in float or double data beyond a desired
   precision.

   Ed Hartnett, 8/19/21
*/

#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"

#define TEST "tst_quantize"
#define FILE_NAME "tst_quantize.nc"
#define NDIMS1 1
#define DIM_NAME_1 "meters_along_canal"
#define DIM_LEN_3 3
#define DIM_LEN_1 1
#define VAR_NAME_1 "Amsterdam_houseboat_location"
#define VAR_NAME_2 "Amsterdam_street_noise_decibels"
#define NSD_3 3
#define NSD_9 9

int
main(int argc, char **argv)
{
    printf("\n*** Testing netcdf-4 variable quantization functions.\n");
    printf("**** testing quantization setting and error conditions...");
    {
	int ncid, dimid, varid1, varid2;
	int quantize_mode_in, nsd_in;

	/* Create a netcdf classic file with one var. Attempt
	 * quantization. It will not work. */
	if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
	if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_3, &dimid)) ERR;
	if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIMS1, &dimid, &varid1)) ERR;
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NSD_3) != NC_ENOTNC4) ERR;
	if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in) != NC_ENOTNC4) ERR;
	if (nc_close(ncid)) ERR;

	/* Create a netcdf-4 file with two vars. Attempt
	 * quantization. It will work, eventually... */
	if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
	if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_3, &dimid)) ERR;
	if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIMS1, &dimid, &varid1)) ERR;
	if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIMS1, &dimid, &varid2)) ERR;

	/* Bad varid. */
	if (nc_def_var_quantize(ncid, NC_GLOBAL, NC_QUANTIZE_BITGROOM, NSD_3) != NC_EGLOBAL) ERR;
	if (nc_def_var_quantize(ncid, varid2 + 1, NC_QUANTIZE_BITGROOM, NSD_3) != NC_ENOTVAR) ERR;
	/* Invalid values. */
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM + 1, NSD_3) != NC_EINVAL) ERR;
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, -1) != NC_EINVAL) ERR;
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NC_QUANTIZE_MAX_FLOAT_NSD + 1) != NC_EINVAL) ERR;
	if (nc_def_var_quantize(ncid, varid2, NC_QUANTIZE_BITGROOM + 1, 3) != NC_EINVAL) ERR;
	if (nc_def_var_quantize(ncid, varid2, NC_QUANTIZE_BITGROOM, -1) != NC_EINVAL) ERR;
	if (nc_def_var_quantize(ncid, varid2, NC_QUANTIZE_BITGROOM, NC_QUANTIZE_MAX_DOUBLE_NSD + 1) != NC_EINVAL) ERR;

	/* This will work. */
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NSD_3)) ERR;
	if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
	if (quantize_mode_in != NC_QUANTIZE_BITGROOM) ERR;
	if (nsd_in != NSD_3) ERR;

	/* Wait, I changed my mind! Let's turn off quantization. */
	if (nc_def_var_quantize(ncid, varid1, NC_NOQUANTIZE, 0)) ERR;
	if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
	if (quantize_mode_in != NC_NOQUANTIZE) ERR;
	if (nsd_in != 0) ERR;

	/* Changed my mind again, turn it on. */
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NSD_3)) ERR;

	/* I changed my mind again! Turn it off! */
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, 0)) ERR;
	if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
	if (quantize_mode_in != NC_NOQUANTIZE) ERR;
	if (nsd_in != 0) ERR;

	/* Changed my mind again, turn it on. */
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NSD_3)) ERR;

	/* This also will work for double. */
	if (nc_def_var_quantize(ncid, varid2, NC_QUANTIZE_BITGROOM, NSD_9)) ERR;
	if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
	if (quantize_mode_in != NC_QUANTIZE_BITGROOM) ERR;
	if (nsd_in != NSD_9) ERR;

	/* End define mode. */
	if (nc_enddef(ncid)) ERR;

	/* This will not work, it's too late! */
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NSD_3) != NC_ELATEDEF) ERR;

	/* Close the file. */
	if (nc_close(ncid)) ERR;

	/* Open the file and check. */
	if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
	if (nc_inq_var_quantize(ncid, 0, &quantize_mode_in, &nsd_in)) ERR;
	if (quantize_mode_in != NC_QUANTIZE_BITGROOM) ERR;
	if (nsd_in != NSD_3) ERR;
	if (nc_inq_var_quantize(ncid, 1, &quantize_mode_in, &nsd_in)) ERR;
	if (quantize_mode_in != NC_QUANTIZE_BITGROOM) ERR;
	if (nsd_in != NSD_9) ERR;
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

#define NX_BIG 100
#define NY_BIG 100
#define NTYPES 9
#define VAR_NAME "Amsterdam_coffeeshop_location"
#define X_NAME "distance_from_center"
#define Y_NAME "distance_along_canal"
#define NDIM2 2

    printf("**** testing quantization handling of non-floats...");
    {
        int ncid;
        int dimid[NDIM2];
        int varid;
        int nsd_in, quantize_mode;
	int nsd_out = 3;
	char file_name[NC_MAX_NAME + 1];
	int xtype[NTYPES] = {NC_CHAR, NC_SHORT, NC_INT, NC_BYTE, NC_UBYTE,
                             NC_USHORT, NC_UINT, NC_INT64, NC_UINT64};
	int t;

	for (t = 0; t < NTYPES; t++)
	{
	    sprintf(file_name, "%s_bitgroom_type_%d.nc", TEST, xtype[t]);

	    /* Create file. */
	    if (nc_create(file_name, NC_NETCDF4, &ncid)) ERR;
	    if (nc_def_dim(ncid, X_NAME, NX_BIG, &dimid[0])) ERR;
	    if (nc_def_dim(ncid, Y_NAME, NY_BIG, &dimid[1])) ERR;
	    if (nc_def_var(ncid, VAR_NAME, xtype[t], NDIM2, dimid, &varid)) ERR;

	    /* Bitgroom filter returns NC_EINVAL because this is not an
	     * NC_FLOAT or NC_DOULBE. */
	    if (nc_def_var_quantize(ncid, varid, NC_QUANTIZE_BITGROOM, nsd_out) != NC_EINVAL) ERR;
	    if (nc_close(ncid)) ERR;

	    /* Check file. */
	    {
		if (nc_open(file_name, NC_NETCDF4, &ncid)) ERR;
		if (nc_inq_var_quantize(ncid, varid, &quantize_mode, &nsd_in)) ERR;
		if (quantize_mode) ERR;
		if (nc_close(ncid)) ERR;
	    } 
	}
    }
    SUMMARIZE_ERR;
    printf("**** testing quantization values...");
    {
	int ncid, dimid, varid1, varid2;
	int quantize_mode_in, nsd_in;
	float float_data[DIM_LEN_1] = {1.1111111};
	double double_data[DIM_LEN_1] = {1.111111111111};

	/* Create a netcdf-4 file with two vars. */
	if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
	if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_1, &dimid)) ERR;
	if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIMS1, &dimid, &varid1)) ERR;
	if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIMS1, &dimid, &varid2)) ERR;

	/* Turn on quantize for both vars. */
	if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NSD_3)) ERR;
	if (nc_def_var_quantize(ncid, varid2, NC_QUANTIZE_BITGROOM, NSD_9)) ERR;

	/* Write some data. */
	if (nc_put_var_float(ncid, varid1, float_data)) ERR;
	if (nc_put_var_double(ncid, varid2, double_data)) ERR;

	/* Close the file. */
	if (nc_close(ncid)) ERR;

	{
	    float float_in;
	    double double_in;
	    union {
		float f;
		uint32_t u;
	    } fin, fout;
		    
	    /* Open the file and check metadata. */
	    if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
	    if (nc_inq_var_quantize(ncid, 0, &quantize_mode_in, &nsd_in)) ERR;
	    if (quantize_mode_in != NC_QUANTIZE_BITGROOM || nsd_in != NSD_3) ERR;
	    if (nc_inq_var_quantize(ncid, 1, &quantize_mode_in, &nsd_in)) ERR;
	    if (quantize_mode_in != NC_QUANTIZE_BITGROOM || nsd_in != NSD_9) ERR;
	    
	    /* Check the data. */
	    if (nc_get_var(ncid, varid1, &float_in)) ERR;
	    if (nc_get_var(ncid, varid2, &double_in)) ERR;
	    fout.f = float_data[0];
	    fin.f = float_in;
	    printf ("\nfloat_data: %10f   : 0x%x  float_data_in: %10f   : 0x%x\n",
		    float_data[0], fout.u, float_data[0], fin.u);
	    if (fout.u != 0x3f8e38e3) ERR;
	    if (fin.u != 0x3f8e3000) ERR;

	    /* Close the file again. */
	    if (nc_close(ncid)) ERR;
	}
    }
    SUMMARIZE_ERR;

    FINAL_RESULTS;


}
