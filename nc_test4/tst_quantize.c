/* This is part of the netCDF package.
   Copyright 2021 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test quantization of netcdf-4 variables. Quantization is the
   zeroing-out of bits in float or double data beyond a desired
   precision.

   Ed Hartnett, 8/19/21
   Dennis Heimbigner, 1/16/22
*/

#include <math.h> /* Define fabs(), powf(), round() */
#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"

#define TEST "tst_quantize"
#define FILE_NAME "tst_quantize.nc"
#define NDIM1 1
#define DIM_NAME_1 "meters_along_canal"
#define DIM_LEN_3 3
#define DIM_LEN_1 1
#define DIM_LEN_5 5
#define DIM_LEN_8 8
#define VAR_NAME_1 "Amsterdam_houseboat_location"
#define VAR_NAME_2 "Amsterdam_street_noise_decibels"
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
    sprintf(pf_str, "0x%llx", (unsigned long long)du.u);
    return pf_str;
}

int
main(int argc, char **argv)
{
#ifdef TESTNCZARR
    const char* template = NULL;
    char file_url[4096];
    
    if(argc == 1)
        {fprintf(stderr,"usage: test_quantize <zarr-url-template>\n"); exit(1);}

    template = argv[1];

    snprintf(file_url,sizeof(file_url),template,FILE_NAME);

#undef FILE_NAME
#define FILE_NAME file_url
#endif

#define NUM_MODE_TESTS 2
#define NUM_QUANTIZE_MODES 3	    
    int mode = NC_NETCDF4|NC_CLOBBER;
    int m;
    int q, quantize_mode[NUM_QUANTIZE_MODES] = {NC_QUANTIZE_BITGROOM, NC_QUANTIZE_GRANULARBR,
						NC_QUANTIZE_BITROUND};
    
    printf("\n*** Testing netcdf-4 variable quantization functions.\n");
    for (m = 0; m < NUM_MODE_TESTS; m++)
    {
	if (!m)
	    printf("**** testing with NC_NETCDF4...\n");
	else
	{
	    printf("**** testing with NC_NETCDF4|NC_CLASSIC_MODEL...\n");
	    mode |= NC_CLASSIC_MODEL;
	}
	    
	printf("\t**** testing quantization setting and error conditions...\n");
	{
	    int ncid, dimid, varid1, varid2;
	    int quantize_mode_in, nsd_in;

	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);
#ifndef TESTNCZARR
		/* Create a netcdf classic file with one var. Attempt
		 * quantization. It will not work. */
		if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
		if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_3, &dimid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3) != NC_ENOTNC4) ERR;
		if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in) != NC_ENOTNC4) ERR;
		if (nc_close(ncid)) ERR;
#endif

		/* Create a netcdf-4 file with two vars. Attempt
		 * quantization. It will work, eventually... */
		if (nc_create(FILE_NAME, mode, &ncid)) ERR;
		if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_3, &dimid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

		/* Bad varid. */
		if (nc_def_var_quantize(ncid, NC_GLOBAL, quantize_mode[q], NSD_3) != NC_EGLOBAL) ERR;
		if (nc_def_var_quantize(ncid, varid2 + 1, quantize_mode[q], NSD_3) != NC_ENOTVAR) ERR;
		/* Invalid values. */
		if (nc_def_var_quantize(ncid, varid1, NUM_QUANTIZE_MODES + 1, NSD_3) != NC_EINVAL) ERR;
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], -1) != NC_EINVAL) ERR;
		if (quantize_mode[q] == NC_QUANTIZE_BITROUND)
		{
		    if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NC_QUANTIZE_MAX_FLOAT_NSB + 1) != NC_EINVAL) ERR;
		}
		else
		{
		    if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NC_QUANTIZE_MAX_FLOAT_NSD + 1) != NC_EINVAL) ERR;
		}
		if (nc_def_var_quantize(ncid, varid2, NUM_QUANTIZE_MODES + 1, 3) != NC_EINVAL) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], -1) != NC_EINVAL) ERR;
		if (quantize_mode[q] == NC_QUANTIZE_BITROUND)
		{
		    if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NC_QUANTIZE_MAX_DOUBLE_NSB + 1) != NC_EINVAL) ERR;
		}
		else
		{
		    if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NC_QUANTIZE_MAX_DOUBLE_NSD + 1) != NC_EINVAL) ERR;
		}
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], 0) != NC_EINVAL) ERR;

		/* This will work. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		if (quantize_mode_in != quantize_mode[q]) ERR;
		if (nsd_in != NSD_3) ERR;

		/* Wait, I changed my mind! Let's turn off quantization. */
		if (nc_def_var_quantize(ncid, varid1, NC_NOQUANTIZE, 0)) ERR;
		if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		if (quantize_mode_in != NC_NOQUANTIZE) ERR;
		if (nsd_in != 0) ERR;

		/* Changed my mind again, turn it on. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;

		/* I changed my mind again! Turn it off! */
		if (nc_def_var_quantize(ncid, varid1, NC_NOQUANTIZE, 0)) ERR;
		if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		if (quantize_mode_in != NC_NOQUANTIZE) ERR;
		if (nsd_in != 0) ERR;

		/* Changed my mind again, turn it on. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;

		/* This also will work for double. */
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_9)) ERR;
		if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		if (quantize_mode_in != quantize_mode[q]) ERR;
		if (nsd_in != NSD_9) ERR;

		/* End define mode. */
		if (nc_enddef(ncid)) ERR;

		/* This will not work, it's too late! */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3) != NC_ELATEDEF) ERR;

		/* Close the file. */
		if (nc_close(ncid)) ERR;

		/* Open the file and check. */
		if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
		/* Don't assume the varid !!! */
		if (nc_inq_varid(ncid, VAR_NAME_1, &varid1)) ERR;
		if (nc_inq_varid(ncid, VAR_NAME_2, &varid2)) ERR;
		if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		if (quantize_mode_in != quantize_mode[q]) ERR;
		if (nsd_in != NSD_3) ERR;
		if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		if (quantize_mode_in != quantize_mode[q]) ERR;
		if (nsd_in != NSD_9) ERR;
		if (nc_close(ncid)) ERR;
	    }
	}
	SUMMARIZE_ERR;

#define NX_BIG 100
#define NY_BIG 100
#define NTYPES 9
#define VAR_NAME "Amsterdam_coffeeshop_location"
#define X_NAME "distance_from_center"
#define Y_NAME "distance_along_canal"
#define NDIM2 2

	printf("\t**** testing quantization handling of non-floats...\n");
	{
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid;
		int dimid[NDIM2];
		int varid;
		int nsd_in, quantize_mode_in;
		int nsd_out = 3;
		char file_name[NC_MAX_FILENAME + 1];
		int xtype[NTYPES] = {NC_CHAR, NC_SHORT, NC_INT, NC_BYTE, NC_UBYTE,
				     NC_USHORT, NC_UINT, NC_INT64, NC_UINT64};
		int t;

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);
		for (t = 0; t < NTYPES; t++)
		{
		    sprintf(file_name, "%s_quantize_%d_type_%d.nc", TEST, quantize_mode[q], xtype[t]);
#ifdef TESTNCZARR
		    {
			char url[NC_MAX_FILENAME + 1];
			snprintf(url,sizeof(url),template,file_name);
			strcpy(file_name,url);
		    }
#endif
		    /* Create file. */
		    if (nc_create(file_name, NC_NETCDF4, &ncid)) ERR;
		    if (nc_def_dim(ncid, X_NAME, NX_BIG, &dimid[0])) ERR;
		    if (nc_def_dim(ncid, Y_NAME, NY_BIG, &dimid[1])) ERR;
		    if (nc_def_var(ncid, VAR_NAME, xtype[t], NDIM2, dimid, &varid)) ERR;

		    /* Quantzie returns NC_EINVAL because this is not
		     * an NC_FLOAT or NC_DOULBE. */
		    if (nc_def_var_quantize(ncid, varid, quantize_mode[q], nsd_out) != NC_EINVAL) ERR;
		    if (nc_close(ncid)) ERR;

		    /* Check file. */
		    {
			if (nc_open(file_name, NC_NETCDF4, &ncid)) ERR;
			if (nc_inq_varid(ncid,VAR_NAME,&varid)) ERR;
			if (nc_inq_var_quantize(ncid, varid, &quantize_mode_in, &nsd_in))
			    ERR;
			if (quantize_mode_in) ERR;
			if (nc_close(ncid)) ERR;
		    }
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t**** testing quantization of scalars...\n");
	{
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid, varid1, varid2;
		int quantize_mode_in, nsd_in;
		float float_data[DIM_LEN_1] = {1.1111111};
		double double_data[DIM_LEN_1] = {1.111111111111};

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);

		/* Create a netcdf-4 file with two scalar vars. */
		if (nc_create(FILE_NAME, mode, &ncid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, 0, NULL, &varid1)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, 0, NULL, &varid2)) ERR;

		/* Turn on quantize for both vars. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_3)) ERR;

		/* For classic mode, we must call enddef. */
		if (m)
		    if (nc_enddef(ncid)) ERR;

		/* Write some data. */
		if (nc_put_var_float(ncid, varid1, float_data)) ERR;
		if (nc_put_var_double(ncid, varid2, double_data)) ERR;

		/* Close the file. */
		if (nc_close(ncid)) ERR;

		{
		    float float_in;
		    double double_in;
		    union FU fin;
		    int nsd_att_in;
		    union DU dfin;
		    /* union FU fout; */
		    /* union DU dfout; */

		    /* Open the file and check metadata. */
		    if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
		    if (nc_inq_varid(ncid, VAR_NAME_1, &varid1)) ERR;
		    if (nc_inq_varid(ncid, VAR_NAME_2, &varid2)) ERR;
		    if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;
		    if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;

		    /* Each var now has an attribute describing the quantize settings. */
		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			if (nc_get_att_int(ncid, 0, NC_QUANTIZE_BITGROOM_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			if (nc_get_att_int(ncid, 1, NC_QUANTIZE_BITGROOM_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			if (nc_get_att_int(ncid, 0, NC_QUANTIZE_GRANULARBR_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			if (nc_get_att_int(ncid, 1, NC_QUANTIZE_GRANULARBR_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			break;
		    case NC_QUANTIZE_BITROUND:
			if (nc_get_att_int(ncid, 0, NC_QUANTIZE_BITROUND_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			if (nc_get_att_int(ncid, 1, NC_QUANTIZE_BITROUND_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			break;
		    default:
			ERR;
		    }

		    /* Check the data. */
		    if (nc_get_var(ncid, varid1, &float_in)) ERR;
		    if (nc_get_var(ncid, varid2, &double_in)) ERR;
		    /* fout.f = float_data[0]; */
		    /* dfout.d = double_data[0]; */
		    fin.f = float_in;
		    dfin.d = double_in;
		    /* printf ("\nfloat_data: %10f   : 0x%x  float_data_in: %10f   : 0x%x\n", */
		    /*         float_data[0], fout.u, float_data[0], fin.u); */
		    /* printf ("\ndouble_data: %15g   : 0x%16lx  double_data_in: %15g   : 0x%lx\n", */
		    /*          double_data[0], dfout.u, double_data[0], dfin.u); */

		    /* Check the results, slightly different for each quantize algorithm. */
		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			if (fin.u != 0x3f8e3000) ERR;
			if (dfin.u != 0x3ff1c60000000000) ERR;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			if (fin.u != 0x3f8e0000) ERR;
			if (dfin.u != 0x3ff1c00000000000) ERR;
			break;
		    case NC_QUANTIZE_BITROUND:
			if (fin.u != 0x3f900000) ERR;
			if (dfin.u != 0x3ff2000000000000) ERR;
			break;
		    default:
			ERR;
		    }

		    /* Close the file again. */
		    if (nc_close(ncid)) ERR;
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t**** testing quantization of one value...\n");
	{
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid, dimid, varid1, varid2;
		int quantize_mode_in, nsd_in;
		float float_data[DIM_LEN_1] = {1.1111111};
		double double_data[DIM_LEN_1] = {1.111111111111};

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);

		/* Create a netcdf-4 file with two vars. */
		if (nc_create(FILE_NAME, mode, &ncid)) ERR;
		if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_1, &dimid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

		/* Turn on quantize for both vars. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_3)) ERR;

		/* For classic mode, we must call enddef. */
		if (m)
		    if (nc_enddef(ncid)) ERR;

		/* Write some data. */
		if (nc_put_var_float(ncid, varid1, float_data)) ERR;
		if (nc_put_var_double(ncid, varid2, double_data)) ERR;

		/* Close the file. */
		if (nc_close(ncid)) ERR;

		{
		    float float_in;
		    double double_in;
		    union FU fin;
		    union DU dfin;
		    /* union FU fout; */
		    /* union DU dfout; */

		    /* Open the file and check metadata. */
		    if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_1,&varid1)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_2,&varid2)) ERR;
		    if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;
		    if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;

		    /* Check the data. */
		    if (nc_get_var(ncid, varid1, &float_in)) ERR;
		    if (nc_get_var(ncid, varid2, &double_in)) ERR;
		    /* fout.f = float_data[0]; */
		    /* dfout.d = double_data[0]; */
		    fin.f = float_in;
		    dfin.d = double_in;
		    /* printf ("\nfloat_data: %10f   : 0x%x  float_data_in: %10f   : 0x%x\n", */
		    /*         float_data[0], fout.u, float_data[0], fin.u); */
		    /* printf ("\ndouble_data: %15g   : 0x%16lx  double_data_in: %15g   : 0x%lx\n", */
		    /*         double_data[0], dfout.u, double_data[0], dfin.u); */

		    /* Check the results, slightly different for each quantize algorithm. */
		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			if (fin.u != 0x3f8e3000) ERR;
			if (dfin.u != 0x3ff1c60000000000) ERR;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			if (fin.u != 0x3f8e0000) ERR;
			if (dfin.u != 0x3ff1c00000000000) ERR;
			break;
		    case NC_QUANTIZE_BITROUND:
			if (fin.u != 0x3f900000) ERR;
			if (dfin.u != 0x3ff2000000000000) ERR;
			break;
		    default:
			ERR;
		    }

		    /* Close the file again. */
		    if (nc_close(ncid)) ERR;
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t**** testing more quantization values...\n");
	{
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid, dimid, varid1, varid2;
		int quantize_mode_in, nsd_in;
		float float_data[DIM_LEN_5] = {1.11111111, 1.0, 9.99999999, 12345.67, .1234567};
		double double_data[DIM_LEN_5] = {1.1111111, 1.0, 9.999999999, 1234567890.12345, 123456789012345.0};
		int x;

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);
		
		/* Create a netcdf-4 file with two vars. */
		if (nc_create(FILE_NAME, mode, &ncid)) ERR;
		if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_5, &dimid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

		/* Turn on quantize for both vars. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_3)) ERR;

		/* For classic mode, we must call enddef. */
		if (m)
		    if (nc_enddef(ncid)) ERR;

		/* Write some data. */
		if (nc_put_var_float(ncid, varid1, float_data)) ERR;
		if (nc_put_var_double(ncid, varid2, double_data)) ERR;

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
		    union FU fout;
		    union FU xpect[NUM_QUANTIZE_MODES][DIM_LEN_5];
		    union DU dfin;
		    union DU dfout;
		    union DU double_xpect[NUM_QUANTIZE_MODES][DIM_LEN_5];
		    NC_UNUSED(fout); NC_UNUSED(dfout);

		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			xpect[0][0].u = 0x3f8e3000;
			xpect[0][1].u = 0x3f800fff;
			xpect[0][2].u = 0x41200000;
			xpect[0][3].u = 0x4640efff;
			xpect[0][4].u = 0x3dfcd000;
			double_xpect[0][0].u = 0x3ff1c60000000000;
			double_xpect[0][1].u = 0x3ff001ffffffffff;
			double_xpect[0][2].u = 0x4023fe0000000000;
			double_xpect[0][3].u = 0x41d265ffffffffff;
			double_xpect[0][4].u = 0x42dc120000000000;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			xpect[1][0].u = 0x3f8e0000;
			xpect[1][1].u = 0x3f800000;
			xpect[1][2].u = 0x41200000;
			xpect[1][3].u = 0x46410000;
			xpect[1][4].u = 0x3dfc0000;
			double_xpect[1][0].u = 0x3ff1c00000000000;
			double_xpect[1][1].u = 0x3ff0000000000000;
			double_xpect[1][2].u = 0x4024000000000000;
			double_xpect[1][3].u = 0x41d2600000000000;
			double_xpect[1][4].u = 0x42dc200000000000;
			break;
		    case NC_QUANTIZE_BITROUND:
			xpect[2][0].u = 0x3f900000;
			xpect[2][1].u = 0x3f800000;
			xpect[2][2].u = 0x41200000;
			xpect[2][3].u = 0x46400000;
			xpect[2][4].u = 0x3e000000;
			double_xpect[2][0].u = 0x3ff2000000000000;
			double_xpect[2][1].u = 0x3ff0000000000000;
			double_xpect[2][2].u = 0x4024000000000000;
			double_xpect[2][3].u = 0x41d2000000000000;
			double_xpect[2][4].u = 0x42dc000000000000;
			break;
		    default:
			ERR;
		    }

		    /* Open the file and check metadata. */
		    if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_1,&varid1)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_2,&varid2)) ERR;
		    if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;
		    if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;

		    /* Check the data. */
		    if (nc_get_var(ncid, varid1, float_in)) ERR;
		    if (nc_get_var(ncid, varid2, double_in)) ERR;
		    /* printf("\n"); */
		    for (x = 0; x < DIM_LEN_5; x++)
		    {
			fout.f = float_data[x];
			fin.f = float_in[x];
			/* printf ("float_data: %10f   : 0x%x  float_data_in: %10f   : 0x%x\n", */
			/*         float_data[x], fout.u, float_data[x], fin.u); */
			if (fin.u != xpect[q][x].u) ERR;
			dfout.d = double_data[x];
			dfin.d = double_in[x];
			/* printf("double_data: %15g   : 0x%16lx  double_data_in: %15g   : 0x%16lx\n", */
			/* 		       double_data[x], dfout.u, double_data[x], dfin.u); */
			if (dfin.u != double_xpect[q][x].u) ERR;
		    }

		    /* Close the file again. */
		    if (nc_close(ncid)) ERR;
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t**** testing quantization of one value with type conversion...\n");
	{
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid, dimid, varid1, varid2;
		int quantize_mode_in, nsd_in;
		float float_data[DIM_LEN_1] = {1.1111111};
		double double_data[DIM_LEN_1] = {1.111111111111};

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);
		
		/* Create a netcdf-4 file with two vars. */
		if (nc_create(FILE_NAME, mode, &ncid)) ERR;
		if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_1, &dimid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

		/* Turn on quantize for both vars. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_3)) ERR;

		/* For classic mode, we must call enddef. */
		if (m)
		    if (nc_enddef(ncid)) ERR;

		/* Write some double data to float var. */
		if (nc_put_var_double(ncid, varid1, double_data)) ERR;

		/* Write some float data to double var. */
		if (nc_put_var_float(ncid, varid2, float_data)) ERR;

		/* Close the file. */
		if (nc_close(ncid)) ERR;

		{
		    float float_in;
		    double double_in;
		    union FU fin;
		    union DU dfin;
		    union FU fout;
		    union DU dfout;
		    int nsd_att_in;
		    NC_UNUSED(fout); NC_UNUSED(dfout);

		    /* Open the file and check metadata. */
		    if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_1,&varid1)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_2,&varid2)) ERR;
		    if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;
		    if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;

		    /* Each var now has an attribute describing the quantize settings. */
		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			if (nc_get_att_int(ncid, 0, NC_QUANTIZE_BITGROOM_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			if (nc_get_att_int(ncid, 1, NC_QUANTIZE_BITGROOM_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			if (nc_get_att_int(ncid, 0, NC_QUANTIZE_GRANULARBR_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			if (nc_get_att_int(ncid, 1, NC_QUANTIZE_GRANULARBR_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			break;
		    case NC_QUANTIZE_BITROUND:
			if (nc_get_att_int(ncid, 0, NC_QUANTIZE_BITROUND_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			if (nc_get_att_int(ncid, 1, NC_QUANTIZE_BITROUND_ATT_NAME, &nsd_att_in)) ERR;
			if (nsd_att_in != NSD_3) ERR;
			break;
		    default:
			ERR;
		    }

		    /* Check the data. */
		    if (nc_get_var(ncid, varid1, &float_in)) ERR;
		    if (nc_get_var(ncid, varid2, &double_in)) ERR;
		    fout.f = (float)double_data[0];
		    fin.f = float_in;
		    dfout.d = float_data[0];
		    dfin.d = double_in;
		    /* printf ("\ndouble_data: %15g   : 0x%x  float_data_in: %10f   : 0x%x\n", */
		    /*         double_data[0], fout.u, float_in, fin.u); */
		    /* printf ("\nfloat_data: %15g   : 0x%16lx  double_data_in: %15g   : 0x%lx\n", */
		    /*         float_data[0], dfout.u, double_in, dfin.u); */

		    
		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			if (fin.u != 0x3f8e3000) ERR;
			if (dfin.u != 0x3ff1c60000000000) ERR;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			if (fin.u != 0x3f8e0000) ERR;
			if (dfin.u != 0x3ff1c00000000000) ERR;
			break;
		    case NC_QUANTIZE_BITROUND:
			if (fin.u !=0x3f900000 ) ERR;
			if (dfin.u != 0x3ff2000000000000) ERR;
			break;
		    default:
			ERR;
		    }

		    /* Close the file again. */
		    if (nc_close(ncid)) ERR;
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t**** testing more quantization values with type conversion...\n");
	{
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid, dimid, varid1, varid2;
		int quantize_mode_in, nsd_in;
		float float_data[DIM_LEN_5] = {1.11111111, 1.0, 9.99999999, 12345.67, .1234567};
		double double_data[DIM_LEN_5] = {1.1111111, 1.0, 9.999999999, 1234567890.12345, 123456789012345.0};
		int x;

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);
		
		/* Create a netcdf-4 file with two vars. */
		if (nc_create(FILE_NAME, mode, &ncid)) ERR;
		if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_5, &dimid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

		/* Turn on quantize for both vars. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_3)) ERR;

		/* For classic mode, we must call enddef. */
		if (m)
		    if (nc_enddef(ncid)) ERR;

		/* Write some data. */
		if (nc_put_var_double(ncid, varid1, double_data)) ERR;
		if (nc_put_var_float(ncid, varid2, float_data)) ERR;

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
		    union FU xpect[NUM_QUANTIZE_MODES][DIM_LEN_5];
		    union DU dfin;
		    union FU fout;
		    union DU dfout;
		    union DU double_xpect[NUM_QUANTIZE_MODES][DIM_LEN_5];
		    NC_UNUSED(fout); NC_UNUSED(dfout);

		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			xpect[0][0].u = 0x3f8e3000;
			xpect[0][1].u = 0x3f800fff;
			xpect[0][2].u = 0x41200000;
			xpect[0][3].u = 0x4e932fff;
			xpect[0][4].u = 0x56e09000;
			double_xpect[0][0].u = 0x3ff1c60000000000;
			double_xpect[0][1].u = 0x3ff001ffffffffff;
			double_xpect[0][2].u = 0x4024000000000000;
			double_xpect[0][3].u = 0x40c81dffffffffff;
			double_xpect[0][4].u = 0x3fbf9a0000000000;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			xpect[1][0].u = 0x3f8e0000;
			xpect[1][1].u =0x3f800000 ;
			xpect[1][2].u = 0x41200000;
			xpect[1][3].u = 0x4e930000;
			xpect[1][4].u = 0x56e10000;
			double_xpect[1][0].u = 0x3ff1c00000000000;
			double_xpect[1][1].u = 0x3ff0000000000000;
			double_xpect[1][2].u = 0x4024000000000000;
			double_xpect[1][3].u = 0x40c8200000000000;
			double_xpect[1][4].u = 0x3fbf800000000000;
			break;
		    case NC_QUANTIZE_BITROUND:
			xpect[2][0].u = 0x3f900000;
			xpect[2][1].u = 0x3f800000;
			xpect[2][2].u = 0x41200000;
			xpect[2][3].u = 0x4e900000;
			xpect[2][4].u = 0x56e00000;
			double_xpect[2][0].u = 0x3ff2000000000000;
			double_xpect[2][1].u = 0x3ff0000000000000;
			double_xpect[2][2].u = 0x4024000000000000;
			double_xpect[2][3].u = 0x40c8000000000000;
			double_xpect[2][4].u = 0x3fc0000000000000;
			break;
		    default:
			ERR;
		    }
			
		    /* Open the file and check metadata. */
		    if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_1,&varid1)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_2,&varid2)) ERR;
		    if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;
		    if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;

		    /* Check the data. */
		    if (nc_get_var(ncid, varid1, float_in)) ERR;
		    if (nc_get_var(ncid, varid2, double_in)) ERR;
		    /* printf("\n"); */
		    for (x = 0; x < DIM_LEN_5; x++)
		    {
			fout.f = float_data[x];
			fin.f = float_in[x];
			/* printf ("float_data: %10f   : 0x%x  float_data_in: %10f   : 0x%x\n", */
			/*         float_data[x], fout.u, float_data[x], fin.u); */
			dfout.d = double_data[x];
			dfin.d = double_in[x];
			/* printf("double_data: %15g   : 0x%16lx  double_data_in: %15g   : 0x%16lx\n", */
			/*        double_data[x], dfout.u, double_data[x], dfin.u); */
			if (fin.u != xpect[q][x].u) ERR;
			if (dfin.u != double_xpect[q][x].u) ERR;
		    }

		    /* Close the file again. */
		    if (nc_close(ncid)) ERR;
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t**** testing more quantization values with default fill values...\n");
	{
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid, dimid, varid1, varid2;
		int quantize_mode_in, nsd_in;
		float float_data[DIM_LEN_5] = {1.11111111, NC_FILL_FLOAT, 9.99999999, 12345.67, NC_FILL_FLOAT};
		double double_data[DIM_LEN_5] = {1.1111111, NC_FILL_DOUBLE, 9.999999999, 1234567890.12345, NC_FILL_DOUBLE};
		int x;

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);
		
		/* Create a netcdf-4 file with two vars. */
		if (nc_create(FILE_NAME, mode, &ncid)) ERR;
		if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_5, &dimid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

		/* Turn on quantize for both vars. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_3)) ERR;

		/* For classic mode, we must call enddef. */
		if (m)
		    if (nc_enddef(ncid)) ERR;

		/* Write some data. */
		if (nc_put_var_float(ncid, varid1, float_data)) ERR;
		if (nc_put_var_double(ncid, varid2, double_data)) ERR;

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
		    union FU xpect[NUM_QUANTIZE_MODES][DIM_LEN_5];
		    union DU dfin;
		    union FU fout;
		    union DU dfout;
		    union DU double_xpect[NUM_QUANTIZE_MODES][DIM_LEN_5];
		    NC_UNUSED(fout); NC_UNUSED(dfout);

		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			xpect[0][0].u = 0x3f8e3000;
			xpect[0][1].u = 0x7cf00000;
			xpect[0][2].u = 0x41200000;
			xpect[0][3].u = 0x4640efff;
			xpect[0][4].u = 0x7cf00000;
			double_xpect[0][0].u = 0x3ff1c60000000000;
			double_xpect[0][1].u = 0x479e000000000000;
			double_xpect[0][2].u = 0x4023fe0000000000;
			double_xpect[0][3].u = 0x41d265ffffffffff;
			double_xpect[0][4].u = 0x479e000000000000;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			xpect[1][0].u = 0x3f8e0000;
			xpect[1][1].u = 0x7cf00000;
			xpect[1][2].u = 0x41200000;
			xpect[1][3].u = 0x46410000;
			xpect[1][4].u = 0x7cf00000;
			double_xpect[1][0].u = 0x3ff1c00000000000;
			double_xpect[1][1].u = 0x479e000000000000;
			double_xpect[1][2].u = 0x4024000000000000;
			double_xpect[1][3].u = 0x41d2600000000000;
			double_xpect[1][4].u = 0x479e000000000000;
			break;
		    case NC_QUANTIZE_BITROUND:
			xpect[2][0].u = 0x3f900000;
			xpect[2][1].u = 0x7cf00000;
			xpect[2][2].u = 0x41200000;
			xpect[2][3].u = 0x46400000;
			xpect[2][4].u = 0x7cf00000;
			double_xpect[2][0].u = 0x3ff2000000000000;
			double_xpect[2][1].u = 0x479e000000000000;
			double_xpect[2][2].u = 0x4024000000000000;
			double_xpect[2][3].u = 0x41d2000000000000;
			double_xpect[2][4].u = 0x479e000000000000;
			break;
		    default:
			ERR;
		    }

		    /* Open the file and check metadata. */
		    if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_1,&varid1)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_2,&varid2)) ERR;
		    if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;
		    if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;

		    /* Check the data. */
		    if (nc_get_var(ncid, varid1, float_in)) ERR;
		    if (nc_get_var(ncid, varid2, double_in)) ERR;
		    /* printf("\n"); */
		    for (x = 0; x < DIM_LEN_5; x++)
		    {
			fout.f = float_data[x];
			fin.f = float_in[x];
			dfout.d = double_data[x];
			dfin.d = double_in[x];
			/* printf ("float_data: %10f   : 0x%x  float_data_in: %10f   : 0x%x\n", */
			/*         float_data[x], fout.u, float_data[x], fin.u); */
			/* printf("double_data: %15g   : 0x%16lx  double_data_in: %15g   : 0x%16lx\n", */
			/*        double_data[x], dfout.u, double_data[x], dfin.u); */
			if (fin.u != xpect[q][x].u) ERR;
			if (dfin.u != double_xpect[q][x].u) ERR;
		    }

		    /* Close the file again. */
		    if (nc_close(ncid)) ERR;
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t**** testing more quantization values with custom fill values...\n");
	{
#define CUSTOM_FILL_FLOAT 99.99999
#define CUSTOM_FILL_DOUBLE -99999.99999
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid, dimid, varid1, varid2;
		int quantize_mode_in, nsd_in;
		float float_data[DIM_LEN_5] = {1.11111111, CUSTOM_FILL_FLOAT, 9.99999999, 12345.67, CUSTOM_FILL_FLOAT};
		double double_data[DIM_LEN_5] = {1.1111111, CUSTOM_FILL_DOUBLE, 9.999999999, 1234567890.12345, CUSTOM_FILL_DOUBLE};
		float custom_fill_float = CUSTOM_FILL_FLOAT;
		double custom_fill_double = CUSTOM_FILL_DOUBLE;
		int x;

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);
		
		/* Create a netcdf-4 file with two vars. */
		if (nc_create(FILE_NAME, mode, &ncid)) ERR;
		if (nc_def_dim(ncid, DIM_NAME_1, DIM_LEN_5, &dimid)) ERR;
		if (nc_def_var(ncid, VAR_NAME_1, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_put_att_float(ncid, varid1, _FillValue, NC_FLOAT, 1, &custom_fill_float)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;
		if (nc_put_att_double(ncid, varid2, _FillValue, NC_DOUBLE, 1, &custom_fill_double)) ERR;

		/* Turn on quantize for both vars. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_3)) ERR;

		/* For classic mode, we must call enddef. */
		if (m)
		    if (nc_enddef(ncid)) ERR;

		/* Write some data. */
		if (nc_put_var_float(ncid, varid1, float_data)) ERR;
		if (nc_put_var_double(ncid, varid2, double_data)) ERR;

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
		    union FU xpect[NUM_QUANTIZE_MODES][DIM_LEN_5];
		    union DU dfin;
		    union FU fout;
		    union DU dfout;
		    union DU double_xpect[NUM_QUANTIZE_MODES][DIM_LEN_5];
		    NC_UNUSED(fout); NC_UNUSED(dfout);
		    
		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			xpect[0][0].u = 0x3f8e3000;
			xpect[0][1].u = 0x42c7ffff;
			xpect[0][2].u = 0x41200000;
			xpect[0][3].u = 0x4640efff;
			xpect[0][4].u = 0x42c7ffff;
			double_xpect[0][0].u = 0x3ff1c60000000000;
			double_xpect[0][1].u = 0xc0f869fffff583a5;
			double_xpect[0][2].u = 0x4023fe0000000000;
			double_xpect[0][3].u = 0x41d265ffffffffff;
			double_xpect[0][4].u = 0xc0f869fffff583a5;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			xpect[1][0].u = 0x3f8e0000;
			xpect[1][1].u = 0x42c7ffff;
			xpect[1][2].u = 0x41200000;
			xpect[1][3].u = 0x46410000;
			xpect[1][4].u = 0x42c7ffff;
			double_xpect[1][0].u = 0x3ff1c00000000000;
			double_xpect[1][1].u = 0xc0f869fffff583a5;
			double_xpect[1][2].u = 0x4024000000000000;
			double_xpect[1][3].u = 0x41d2600000000000;
			double_xpect[1][4].u = 0xc0f869fffff583a5;
			break;
		    case NC_QUANTIZE_BITROUND:
			xpect[2][0].u = 0x3f900000;
			xpect[2][1].u = 0x42c7ffff;
			xpect[2][2].u = 0x41200000;
			xpect[2][3].u = 0x46400000;
			xpect[2][4].u = 0x42c7ffff;
			double_xpect[2][0].u = 0x3ff2000000000000;
			double_xpect[2][1].u = 0xc0f869fffff583a5;
			double_xpect[2][2].u = 0x4024000000000000;
			double_xpect[2][3].u = 0x41d2000000000000;
			double_xpect[2][4].u = 0xc0f869fffff583a5;
			break;
		    default:
			ERR;
		    }

		    /* Open the file and check metadata. */
		    if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_1,&varid1)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_2,&varid2)) ERR;
		    if (nc_inq_var_quantize(ncid, varid1, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;
		    if (nc_inq_var_quantize(ncid, varid2, &quantize_mode_in, &nsd_in)) ERR;
		    if (quantize_mode_in != quantize_mode[q] || nsd_in != NSD_3) ERR;

		    /* Check the data. */
		    if (nc_get_var(ncid, varid1, float_in)) ERR;
		    if (nc_get_var(ncid, varid2, double_in)) ERR;
		    /* printf("\n"); */
		    for (x = 0; x < DIM_LEN_5; x++)
		    {
			fout.f = float_data[x];
			fin.f = float_in[x];
			dfout.d = double_data[x];
			dfin.d = double_in[x];
			/* printf ("float_data: %10f   : 0x%x  float_data_in: %10f   : 0x%x\n", */
			/*         float_data[x], fout.u, float_data[x], fin.u); */
			/* printf("double_data: %15g   : 0x%16lx  double_data_in: %15g   : 0x%16lx\n", */
			/*        double_data[x], dfout.u, double_data[x], dfin.u); */
			if (fin.u != xpect[q][x].u) ERR;
			if (dfin.u != double_xpect[q][x].u) ERR;
		    }

		    /* Close the file again. */
		    if (nc_close(ncid)) ERR;
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t*** Checking BitGroom values with type conversion between ints and floats...\n");
	{
	    for (q = 0; q < NUM_QUANTIZE_MODES; q++)
	    {
		int ncid;
		int dimid;
		int varid1, varid2;
		unsigned char uc = 99;
		signed char sc = -99;
		unsigned short us = 9999;
		signed short ss = -9999;
		unsigned int ui = 9999999;
		signed int si = -9999999;
		unsigned long long int ull = 999999999;
		signed long long int sll = -999999999;
		size_t index;

		printf("\t\t**** testing quantize algorithm %d...\n", quantize_mode[q]);
		
		/* Create file. */
		if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

		/* Create dims. */
		if (nc_def_dim(ncid, X_NAME, DIM_LEN_8, &dimid)) ERR;

		/* Create the variables. */
		if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
		if (nc_def_var(ncid, VAR_NAME_2, NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

		/* Set up quantization. */
		if (nc_def_var_quantize(ncid, varid1, quantize_mode[q], NSD_3)) ERR;
		if (nc_def_var_quantize(ncid, varid2, quantize_mode[q], NSD_3)) ERR;

		/* For classic mode, we must call enddef. */
		if (m)
		    if (nc_enddef(ncid)) ERR;

		/* Write data. */
		index = 0;
		if (nc_put_var1_uchar(ncid, varid1, &index, &uc)) ERR;
		if (nc_put_var1_uchar(ncid, varid2, &index, &uc)) ERR;
		index = 1;
		if (nc_put_var1_schar(ncid, varid1, &index, &sc)) ERR;
		if (nc_put_var1_schar(ncid, varid2, &index, &sc)) ERR;
		index = 2;
		if (nc_put_var1_ushort(ncid, varid1, &index, &us)) ERR;
		if (nc_put_var1_ushort(ncid, varid2, &index, &us)) ERR;
		index = 3;
		if (nc_put_var1_short(ncid, varid1, &index, &ss)) ERR;
		if (nc_put_var1_short(ncid, varid2, &index, &ss)) ERR;
		index = 4;
		if (nc_put_var1_uint(ncid, varid1, &index, &ui)) ERR;
		if (nc_put_var1_uint(ncid, varid2, &index, &ui)) ERR;
		index = 5;
		if (nc_put_var1_int(ncid, varid1, &index, &si)) ERR;
		if (nc_put_var1_int(ncid, varid2, &index, &si)) ERR;
		index = 6;
		if (nc_put_var1_ulonglong(ncid, varid1, &index, &ull)) ERR;
		if (nc_put_var1_ulonglong(ncid, varid2, &index, &ull)) ERR;
		index = 7;
		if (nc_put_var1_longlong(ncid, varid1, &index, &sll)) ERR;
		if (nc_put_var1_longlong(ncid, varid2, &index, &sll)) ERR;

		/* Close the file. */
		if (nc_close(ncid)) ERR;

		{
		    float float_data_in[DIM_LEN_8];
		    double double_data_in[DIM_LEN_8];
		    int x;

		    /* Now reopen the file and check. */
		    if (nc_open(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME,&varid1)) ERR;
		    if (nc_inq_varid(ncid,VAR_NAME_2,&varid2)) ERR;

		    /* Read the data. */
		    if (nc_get_var_float(ncid, varid1, float_data_in)) ERR;
		    if (nc_get_var_double(ncid, varid2, double_data_in)) ERR;

		    union FU xpect[NUM_QUANTIZE_MODES][DIM_LEN_8];
		    union DU double_xpect[NUM_QUANTIZE_MODES][DIM_LEN_8];
		    
		    /* This test comes up with different answers to this than
		     * the corresponding bitgroom filter test, but that's
		     * OK. In netcdf-c quantization is applied as the data are
		     * written by the user, but in HDF5 filters, the bitgroom
		     * filter is applied to all data values as they are
		     * written to disk. See
		     * https://github.com/ccr/ccr/issues/194 for a full
		     * explanation. */
		    switch (quantize_mode[q])
		    {
		    case NC_QUANTIZE_BITGROOM:
			xpect[0][0].u = 0x42c60000;
			xpect[0][1].u = 0xc2c60000;
			xpect[0][2].u = 0x461c3000;
			xpect[0][3].u = 0xc61c3000;
			xpect[0][4].u = 0x4b189000;
			xpect[0][5].u = 0xcb189000;
			xpect[0][6].u = 0x4e6e6000;
			xpect[0][7].u = 0xce6e6000;
			double_xpect[0][0].u = 0x4058c00000000000;
			double_xpect[0][1].u = 0xc058c00000000000;
			double_xpect[0][2].u = 0x40c3860000000000;
			double_xpect[0][3].u = 0xc0c3860000000000;
			double_xpect[0][4].u = 0x4163120000000000;
			double_xpect[0][5].u = 0xc163120000000000;
			double_xpect[0][6].u = 0x41cdcc0000000000;
			double_xpect[0][7].u = 0xc1cdcc0000000000;
			break;
		    case NC_QUANTIZE_GRANULARBR:
			xpect[1][0].u = 0x42c60000;
			xpect[1][1].u = 0xc2c60000;
			xpect[1][2].u = 0x461c4000;
			xpect[1][3].u = 0xc61c4000;
			xpect[1][4].u = 0x4b18a000;
			xpect[1][5].u = 0xcb18a000;
			xpect[1][6].u = 0x4e6e6000;
			xpect[1][7].u = 0xce6e6000;
			double_xpect[1][0].u = 0x4058c00000000000;
			double_xpect[1][1].u = 0xc058c00000000000;
			double_xpect[1][2].u = 0x40c3880000000000;
			double_xpect[1][3].u = 0xc0c3880000000000;
			double_xpect[1][4].u = 0x4163140000000000;
			double_xpect[1][5].u = 0xc163140000000000;
			double_xpect[1][6].u = 0x41cdcc0000000000;
			double_xpect[1][7].u = 0xc1cdcc0000000000;
			break;
		    case NC_QUANTIZE_BITROUND:
			xpect[2][0].u = 0x42c00000;
			xpect[2][1].u = 0xc2c00000;
			xpect[2][2].u = 0x46200000;
			xpect[2][3].u = 0xc6200000;
			xpect[2][4].u = 0x4b200000;
			xpect[2][5].u = 0xcb200000;
			xpect[2][6].u = 0x4e700000;
			xpect[2][7].u = 0xce700000;
			double_xpect[2][0].u = 0x4058000000000000;
			double_xpect[2][1].u = 0xc058000000000000;
			double_xpect[2][2].u = 0x40c4000000000000;
			double_xpect[2][3].u = 0xc0c4000000000000;
			double_xpect[2][4].u = 0x4164000000000000;
			double_xpect[2][5].u = 0xc164000000000000;
			double_xpect[2][6].u = 0x41ce000000000000;
			double_xpect[2][7].u = 0xc1ce000000000000;
			break;
		    default:
			ERR;
		    }

		    for (x = 0; x < DIM_LEN_8; x++)
		    {
			union FU fin;
			union DU dfin;
			fin.f = float_data_in[x];
			dfin.d = double_data_in[x];
			/* printf ("%d float_data_in : %08.8f   : 0x%x expected %08.8f   : 0x%x\n",  */
			/*  	x, float_data_in[x], fin.u, xpect[q][x].f, xpect[q][x].u);  */
			/* printf ("%d double_data_in : %15g   : 0x%lx expected %15g   : 0x%lx\n", */
			/* 	x, double_data_in[x], dfin.u, double_xpect[q][x].d, double_xpect[q][x].u); */
			if (fin.u != xpect[q][x].u) ERR;
			if (dfin.u != double_xpect[q][x].u) ERR;
		    }

		    /* Close the file. */
		    if (nc_close(ncid)) ERR;
		}
	    }
	}
	SUMMARIZE_ERR;
	printf("\t**** Nice, simple example of using BitGroom plus zlib...");
	{
#define DIM_LEN_SIMPLE 100
#define EPSILON .1
	    int ncid;
	    int dimid;
	    int varid1, varid2;
	    float *float_data;
	    double *double_data;
	    int i;

	    /* Set up some data to write. */
	    if (!(float_data = malloc(DIM_LEN_SIMPLE * sizeof(float))))
		ERR;
	    if (!(double_data = malloc(DIM_LEN_SIMPLE * sizeof(double))))
		ERR;
	    for (i = 0; i < DIM_LEN_SIMPLE; i++)
	    {
		float_data[i] = 1.5 * i;
		double_data[i] = 1.5 * i;
	    }

	    /* Create the file. */
	    if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

	    /* Add one dimension. */
	    if (nc_def_dim(ncid, "dim1", DIM_LEN_SIMPLE, &dimid)) ERR;

	    /* Create two variables, one float, one double. Quantization
	     * may only be applied to floating point data. */
	    if (nc_def_var(ncid, "var1", NC_FLOAT, NDIM1, &dimid, &varid1)) ERR;
	    if (nc_def_var(ncid, "var2", NC_DOUBLE, NDIM1, &dimid, &varid2)) ERR;

	    /* Set up quantization. This will not make the data any
	     * smaller, unless compression is also turned on. In this
	     * case, we will set 3 significant digits. */
	    if (nc_def_var_quantize(ncid, varid1, NC_QUANTIZE_BITGROOM, NSD_3)) ERR;
	    if (nc_def_var_quantize(ncid, varid2, NC_QUANTIZE_BITGROOM, NSD_3)) ERR;

#ifdef TESTNCZARR
#ifdef ENABLE_NCZARR_FILTERS
	    /* Set up zlib compression. This will work better because the
	     * data are quantized, yielding a smaller output file. We will
	     * set compression level to 1, which is usually the best
	     * choice. */
	    if (nc_def_var_deflate(ncid, varid1, 0, 1, 1)) ERR;
#endif
#endif
	    /* For classic mode, we must call enddef. */
	    if (m)
		if (nc_enddef(ncid)) ERR;

	    /* Write the data. */
	    if (nc_put_var_float(ncid, varid1, float_data)) ERR;
	    if (nc_put_var_double(ncid, varid2, double_data)) ERR;

	    /* Close the file. */
	    if (nc_close(ncid)) ERR;

	    /* Check the resulting file for correctness. */
	    {
		float float_data_in[DIM_LEN_SIMPLE];
		double double_data_in[DIM_LEN_SIMPLE];

		/* Now reopen the file and check. */
		if (nc_open(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
		if (nc_inq_varid(ncid,"var1",&varid1)) ERR;
		if (nc_inq_varid(ncid,"var2",&varid2)) ERR;

		/* Read the data. */
		if (nc_get_var_float(ncid, varid1, float_data_in)) ERR;
		if (nc_get_var_double(ncid, varid2, double_data_in)) ERR;

		for (i = 0; i < DIM_LEN_SIMPLE; i++)
		{
		    if (fabs(float_data_in[i] - float_data[i]) > EPSILON)
			ERR;
		    if (fabs(double_data_in[i] - double_data[i]) > EPSILON)
			ERR;
		}

		/* Close the file. */
		if (nc_close(ncid)) ERR;
	    }

	    /* Free resources. */
	    free(float_data);
	    free(double_data);
	}
	SUMMARIZE_ERR;
    }
    FINAL_RESULTS;
}
