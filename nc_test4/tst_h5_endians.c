/*! Test for NCF-331. Added May 11, 2015.
 * Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
 * 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
 * 2015, 2016, 2017, 2018
 * University Corporation for Atmospheric Research/Unidata.
 *
 * See \ref copyright file for more info.
 *  See the following links for more information:
 *
 *  o Issue on GitHub: https://github.com/Unidata/netcdf-c/issues/112
 *  o Issue in JIRA:   https://bugtracking.unidata.ucar.edu/browse/NCF-331
 *
 * Test contributed by Jeff Whitaker
 */

#include <nc_tests.h>
#include <string.h>
#include "nc_logging.h"

#undef BE_DEBUG

#ifdef TESTNCZARR
#include "test_utils.h"
#define FILE_NAME_NC "file://tmp_h5_endians.file#mode=nczarr,file"
#else
#define FILE_NAME_NC "tst_h5_endians.nc"
#endif

static float
f32swap(float x)
{
  union {
    unsigned char bytes[4];
    float f;
  } u;
  unsigned char c;
  u.f = x;
  c = u.bytes[0]; u.bytes[0] = u.bytes[3]; u.bytes[3] = c;
  c = u.bytes[1]; u.bytes[1] = u.bytes[2]; u.bytes[2] = c;  
  return u.f;
}

static double
f64swap(double x)
{
  union {
    unsigned char bytes[8];
    double d;
  } u;
  unsigned char c;
  u.d = x;
  c = u.bytes[0]; u.bytes[0] = u.bytes[7]; u.bytes[7] = c;
  c = u.bytes[1]; u.bytes[1] = u.bytes[6]; u.bytes[6] = c;
  c = u.bytes[2]; u.bytes[2] = u.bytes[5]; u.bytes[5] = c;
  c = u.bytes[3]; u.bytes[3] = u.bytes[4]; u.bytes[4] = c;  
  return u.d;
}

#define NDIM 10
#define NLON 20
#define DIM_NAME "x"
#define DIM_LEN 4
#define GRP_NAME "grp"
#define LE_FLOAT_VARNAME "fl_le"
#define BE_FLOAT_VARNAME "fl_be"
#define LE_INT_VARNAME "int_le"
#define BE_INT_VARNAME "int_be"
#define LE_DBL_VARNAME "dbl_le"
#define BE_DBL_VARNAME "dbl_be"


int main() {

  int ncid, dimid;
  int le_float_varid;
  int be_float_varid;
  int le_int_varid;
  int be_int_varid;
  int le_dbl_varid;
  int be_dbl_varid;
  int ed;
  int failures = 0;
  int retval;

  printf("* Checking that endianness is properly read from file.\n");
  printf("** Generating test files.\n");
  /*
   * 1. Create a netcdf file with endianness as desired.
   */
  {

    printf("*** Creating a file via netcdf API: %s.\n",FILE_NAME_NC);
    if ((retval = nc_create(FILE_NAME_NC, NC_NETCDF4 | NC_CLOBBER, &ncid)))
	return retval;

    if ((retval = nc_def_dim(ncid, DIM_NAME, NDIM, &dimid)))
	return retval;

    /* Little-Endian Float */
    if ((retval = nc_def_var(ncid, LE_FLOAT_VARNAME, NC_FLOAT, 1, &dimid, &le_float_varid)))
	return retval;
    if ((retval = nc_def_var_endian(ncid, le_float_varid, NC_ENDIAN_LITTLE)))
	return retval;

    /* Big-Endian Float */
    if ((retval = nc_def_var(ncid, BE_FLOAT_VARNAME, NC_FLOAT, 1, &dimid, &be_float_varid)))
	return retval;
    if ((retval = nc_def_var_endian(ncid, be_float_varid, NC_ENDIAN_BIG)))
	return retval;

    /* Little-Endian Int */
    if ((retval = nc_def_var(ncid, LE_INT_VARNAME, NC_INT, 1, &dimid, &le_int_varid)))
	return retval;
    if ((retval = nc_def_var_endian(ncid, le_int_varid, NC_ENDIAN_LITTLE)))
	return retval;

    /* Big-Endian Int */
    if ((retval = nc_def_var(ncid, BE_INT_VARNAME, NC_INT, 1, &dimid, &be_int_varid)))
	return retval;
    if ((retval = nc_def_var_endian(ncid, be_int_varid, NC_ENDIAN_BIG)))
	return retval;

    /* Little-Endian Double */
    if ((retval = nc_def_var(ncid, LE_DBL_VARNAME, NC_DOUBLE, 1, &dimid, &le_dbl_varid)))
	return retval;
    if ((retval = nc_def_var_endian(ncid, le_dbl_varid, NC_ENDIAN_LITTLE)))
	return retval;

    /* Big-Endian Double */
    if ((retval = nc_def_var(ncid, BE_DBL_VARNAME, NC_DOUBLE, 1, &dimid, &be_dbl_varid)))
	return retval;
    if ((retval = nc_def_var_endian(ncid, be_dbl_varid, NC_ENDIAN_BIG)))
	return retval;


    if ((retval = nc_close(ncid)))
	return retval;
  }

  /*
   * 2. Reopen netcdf-generated file, check to see if the endianness attribute
   *    exists.
   */
  printf("** Checking test files.\n");
  {
    ncid = 0;
    le_float_varid = 0;
    be_float_varid = 0;
    le_int_varid = 0;
    be_int_varid = 0;
    le_dbl_varid = 0;
    be_dbl_varid = 0;

    printf("*** %s\n",FILE_NAME_NC);
    if ((retval = nc_open(FILE_NAME_NC, NC_NETCDF4 | NC_NOWRITE, &ncid)))
	return retval;

    if ((retval = nc_inq_varid(ncid,LE_FLOAT_VARNAME,&le_float_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,BE_FLOAT_VARNAME,&be_float_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,LE_INT_VARNAME,&le_int_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,BE_INT_VARNAME,&be_int_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,LE_DBL_VARNAME,&le_dbl_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,BE_DBL_VARNAME,&be_dbl_varid)))
	return retval;

    printf("\tLittle-Endian Float...\t");
    if ((retval = nc_inq_var_endian(ncid,le_float_varid,&ed)))
	return retval;
    if(ed == NC_ENDIAN_LITTLE) printf("passed\n"); else {printf("failed\n"); failures++;}

    printf("\tBig-Endian Float...\t");
    if ((retval = nc_inq_var_endian(ncid,be_float_varid,&ed)))
	return retval;
    if(ed == NC_ENDIAN_BIG) printf("passed\n"); else {printf("failed\n"); failures++;}

    printf("\tLittle-Endian Int...\t");
    if ((retval = nc_inq_var_endian(ncid,le_int_varid,&ed)))
	return retval;
    if(ed == NC_ENDIAN_LITTLE) printf("passed\n"); else {printf("failed\n"); failures++;}

    printf("\tBig-Endian Int...\t");
    if ((retval = nc_inq_var_endian(ncid,be_int_varid,&ed)))
	return retval;
    if(ed == NC_ENDIAN_BIG) printf("passed\n"); else {printf("failed\n"); failures++;}

    printf("\tLittle-Endian Double...\t");
    if ((retval = nc_inq_var_endian(ncid,le_dbl_varid,&ed)))
	return retval;
    if(ed == NC_ENDIAN_LITTLE) printf("passed\n"); else {printf("failed\n"); failures++;}

    printf("\tBig-Endian Double...\t");
    if ((retval = nc_inq_var_endian(ncid,be_dbl_varid,&ed)))
	return retval;
    if(ed == NC_ENDIAN_BIG) printf("passed\n"); else {printf("failed\n"); failures++;}

    if ((retval = nc_close(ncid)))
	return retval;
  }

  /*
   * 3. Reopen netcdf-generated file, write data and reread.
   */
  printf("** Read/Write test files.\n");
  {
    ncid = 0;
    le_float_varid = 0;
    be_float_varid = 0;
    le_int_varid = 0;
    be_int_varid = 0;
    le_dbl_varid = 0;
    be_dbl_varid = 0;

    printf("*** %s\n",FILE_NAME_NC);
    if ((retval = nc_open(FILE_NAME_NC, NC_NETCDF4 | NC_WRITE, &ncid)))
	return retval;

    if ((retval = nc_inq_varid(ncid,LE_FLOAT_VARNAME,&le_float_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,BE_FLOAT_VARNAME,&be_float_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,LE_INT_VARNAME,&le_int_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,BE_INT_VARNAME,&be_int_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,LE_DBL_VARNAME,&le_dbl_varid)))
	return retval;
    if ((retval = nc_inq_varid(ncid,BE_DBL_VARNAME,&be_dbl_varid)))
	return retval;

    {
	int i, failed;
        int idata_in[NDIM];
        float fdata_in[NDIM];
	double ddata_in[NDIM];
        int idata_le_out[NDIM];
        float fdata_le_out[NDIM];
	double ddata_le_out[NDIM];
        int idata_be_out[NDIM];
        float fdata_be_out[NDIM];
        double ddata_be_out[NDIM];

	/* Setup data in/out */
	for(i=0;i<NDIM;i++) {
	    idata_in[i] = (int)i;
    	    fdata_in[i] = (float)i;
	    ddata_in[i] = (double)i;
	    idata_le_out[i] = 0;
    	    fdata_le_out[i] = 0.0f;
	    ddata_le_out[i] = 0.0;
	    idata_be_out[i] = 0;
	    fdata_be_out[i] = 0.0f;
	    ddata_be_out[i] = 0.0;
	}

        printf("\tLittle-Endian Float...\t");
        if ((retval = nc_put_var(ncid,le_float_varid,fdata_in)))
            return retval;
        if ((retval = nc_get_var(ncid,le_float_varid,fdata_le_out)))
            return retval;
	for(failed=0,i=0;i<NDIM;i++) {if(fdata_in[i] != fdata_le_out[i]) {printf("failed\n"); failures++; failed++; break;}}
	if(!failed) printf("passed\n");

        printf("\tBig-Endian Float...\t");
        if ((retval = nc_put_var(ncid,be_float_varid,fdata_in)))
            return retval;
        if ((retval = nc_get_var(ncid,be_float_varid,fdata_be_out)))
            return retval;
	for(i=0;i<NDIM;i++) {
	    float f = f32swap(fdata_be_out[i]);
	    fprintf(stderr,"[%d] %f\n",i,f);
	}
	for(failed=0,i=0;i<NDIM;i++) {if(fdata_in[i] != fdata_be_out[i]) {printf("failed\n"); failures++; failed++; break;}}
	if(!failed) printf("passed\n");

        printf("\tLittle-Endian Int...\t");
        if ((retval = nc_put_var(ncid,le_int_varid,idata_in)))
            return retval;
        if ((retval = nc_get_var(ncid,le_int_varid,idata_le_out)))
            return retval;
	for(failed=0,i=0;i<NDIM;i++) {if(idata_in[i] != idata_le_out[i]) {printf("failed\n"); failures++; failed++; break;}}
	if(!failed) printf("passed\n");

        printf("\tBig-Endian Int...\t");
        if ((retval = nc_put_var(ncid,be_int_varid,idata_in)))
            return retval;
        if ((retval = nc_get_var(ncid,be_int_varid,idata_be_out)))
            return retval;
	for(failed=0,i=0;i<NDIM;i++) {if(idata_in[i] != idata_be_out[i]) {printf("failed\n"); failures++; failed++; break;}}
	if(!failed) printf("passed\n");

        printf("\tLittle-Endian Double...\t");
        if ((retval = nc_put_var(ncid,le_dbl_varid,ddata_in)))
            return retval;
        if ((retval = nc_get_var(ncid,le_dbl_varid,ddata_le_out)))
            return retval;
	for(failed=0,i=0;i<NDIM;i++) {if(ddata_in[i] != ddata_le_out[i]) {printf("failed\n"); failures++; failed++; break;}}
	if(!failed) printf("passed\n");

        printf("\tBig-Endian Double...\t");
        if ((retval = nc_put_var(ncid,be_dbl_varid,ddata_in)))
            return retval;
        if ((retval = nc_get_var(ncid,be_dbl_varid,ddata_be_out)))
            return retval;
	for(i=0;i<NDIM;i++) {
	    double d = f64swap(ddata_be_out[i]);
	    fprintf(stderr,"[%d] %lf\n",i,d);
	}
	for(failed=0,i=0;i<NDIM;i++) {if(ddata_in[i] != ddata_be_out[i]) {printf("failed\n"); failures++; failed++; break;}}
	if(!failed) printf("passed\n");

        if ((retval = nc_close(ncid)))
            return retval;
    }
  }

  /*
   * 4. Regression test for GitHub issue #1802:
   *    Create a file with a big-endian variable, close it WITHOUT writing data,
   *    reopen it, write data, close, reopen read-only and verify the values.
   *    On a little-endian host this exposed the H5Dwrite memory-type bug where
   *    hdf_typeid (on-disk type) was used instead of native_hdf_typeid.
   */
  printf("** Regression test: write-after-close-reopen (issue #1802).\n");
  {
#define FILE_1802 "tst_h5_endians_1802.nc"
#define DIM_1802 "x"
#define DIM_LEN_1802 10
    int i, failed;
    int data_out[DIM_LEN_1802];
    int data_in[DIM_LEN_1802];
    float fdata_out[DIM_LEN_1802];
    float fdata_in[DIM_LEN_1802];
    double ddata_out[DIM_LEN_1802];
    double ddata_in[DIM_LEN_1802];
    int varid_be_int, varid_be_float, varid_be_dbl;
    int varid_le_int, varid_le_float, varid_le_dbl;
    int dim1802;

    for (i = 0; i < DIM_LEN_1802; i++) {
        data_out[i]  = i;
        fdata_out[i] = (float)i;
        ddata_out[i] = (double)i;
        data_in[i]   = -1;
        fdata_in[i]  = -1.0f;
        ddata_in[i]  = -1.0;
    }

    /* Create file, define vars, close WITHOUT writing any data. */
    if ((retval = nc_create(FILE_1802, NC_NETCDF4 | NC_CLOBBER, &ncid)))
        return retval;
    if ((retval = nc_def_dim(ncid, DIM_1802, DIM_LEN_1802, &dim1802)))
        return retval;
    if ((retval = nc_def_var(ncid, "be_int", NC_INT, 1, &dim1802, &varid_be_int)))
        return retval;
    if ((retval = nc_def_var_endian(ncid, varid_be_int, NC_ENDIAN_BIG)))
        return retval;
    if ((retval = nc_def_var(ncid, "be_float", NC_FLOAT, 1, &dim1802, &varid_be_float)))
        return retval;
    if ((retval = nc_def_var_endian(ncid, varid_be_float, NC_ENDIAN_BIG)))
        return retval;
    if ((retval = nc_def_var(ncid, "be_dbl", NC_DOUBLE, 1, &dim1802, &varid_be_dbl)))
        return retval;
    if ((retval = nc_def_var_endian(ncid, varid_be_dbl, NC_ENDIAN_BIG)))
        return retval;
    if ((retval = nc_def_var(ncid, "le_int", NC_INT, 1, &dim1802, &varid_le_int)))
        return retval;
    if ((retval = nc_def_var_endian(ncid, varid_le_int, NC_ENDIAN_LITTLE)))
        return retval;
    if ((retval = nc_def_var(ncid, "le_float", NC_FLOAT, 1, &dim1802, &varid_le_float)))
        return retval;
    if ((retval = nc_def_var_endian(ncid, varid_le_float, NC_ENDIAN_LITTLE)))
        return retval;
    if ((retval = nc_def_var(ncid, "le_dbl", NC_DOUBLE, 1, &dim1802, &varid_le_dbl)))
        return retval;
    if ((retval = nc_def_var_endian(ncid, varid_le_dbl, NC_ENDIAN_LITTLE)))
        return retval;
    if ((retval = nc_close(ncid)))
        return retval;

    /* Reopen, write data, close. */
    if ((retval = nc_open(FILE_1802, NC_WRITE, &ncid)))
        return retval;
    if ((retval = nc_inq_varid(ncid, "be_int",   &varid_be_int)))   return retval;
    if ((retval = nc_inq_varid(ncid, "be_float", &varid_be_float))) return retval;
    if ((retval = nc_inq_varid(ncid, "be_dbl",   &varid_be_dbl)))   return retval;
    if ((retval = nc_inq_varid(ncid, "le_int",   &varid_le_int)))   return retval;
    if ((retval = nc_inq_varid(ncid, "le_float", &varid_le_float))) return retval;
    if ((retval = nc_inq_varid(ncid, "le_dbl",   &varid_le_dbl)))   return retval;
    if ((retval = nc_put_var_int(ncid,    varid_be_int,   data_out)))  return retval;
    if ((retval = nc_put_var_float(ncid,  varid_be_float, fdata_out))) return retval;
    if ((retval = nc_put_var_double(ncid, varid_be_dbl,   ddata_out))) return retval;
    if ((retval = nc_put_var_int(ncid,    varid_le_int,   data_out)))  return retval;
    if ((retval = nc_put_var_float(ncid,  varid_le_float, fdata_out))) return retval;
    if ((retval = nc_put_var_double(ncid, varid_le_dbl,   ddata_out))) return retval;
    if ((retval = nc_close(ncid)))
        return retval;

    /* Reopen read-only, verify all values round-tripped correctly. */
    if ((retval = nc_open(FILE_1802, NC_NOWRITE, &ncid)))
        return retval;
    if ((retval = nc_inq_varid(ncid, "be_int",   &varid_be_int)))   return retval;
    if ((retval = nc_inq_varid(ncid, "be_float", &varid_be_float))) return retval;
    if ((retval = nc_inq_varid(ncid, "be_dbl",   &varid_be_dbl)))   return retval;
    if ((retval = nc_inq_varid(ncid, "le_int",   &varid_le_int)))   return retval;
    if ((retval = nc_inq_varid(ncid, "le_float", &varid_le_float))) return retval;
    if ((retval = nc_inq_varid(ncid, "le_dbl",   &varid_le_dbl)))   return retval;

    printf("\tBig-Endian Int (write-after-reopen)...\t");
    if ((retval = nc_get_var_int(ncid, varid_be_int, data_in))) return retval;
    for (failed=0,i=0;i<DIM_LEN_1802;i++) {if(data_out[i]!=data_in[i]){printf("failed\n");failures++;failed++;break;}}
    if (!failed) printf("passed\n");

    printf("\tBig-Endian Float (write-after-reopen)...\t");
    if ((retval = nc_get_var_float(ncid, varid_be_float, fdata_in))) return retval;
    for (failed=0,i=0;i<DIM_LEN_1802;i++) {if(fdata_out[i]!=fdata_in[i]){printf("failed\n");failures++;failed++;break;}}
    if (!failed) printf("passed\n");

    printf("\tBig-Endian Double (write-after-reopen)...\t");
    if ((retval = nc_get_var_double(ncid, varid_be_dbl, ddata_in))) return retval;
    for (failed=0,i=0;i<DIM_LEN_1802;i++) {if(ddata_out[i]!=ddata_in[i]){printf("failed\n");failures++;failed++;break;}}
    if (!failed) printf("passed\n");

    printf("\tLittle-Endian Int (write-after-reopen)...\t");
    if ((retval = nc_get_var_int(ncid, varid_le_int, data_in))) return retval;
    for (failed=0,i=0;i<DIM_LEN_1802;i++) {if(data_out[i]!=data_in[i]){printf("failed\n");failures++;failed++;break;}}
    if (!failed) printf("passed\n");

    printf("\tLittle-Endian Float (write-after-reopen)...\t");
    if ((retval = nc_get_var_float(ncid, varid_le_float, fdata_in))) return retval;
    for (failed=0,i=0;i<DIM_LEN_1802;i++) {if(fdata_out[i]!=fdata_in[i]){printf("failed\n");failures++;failed++;break;}}
    if (!failed) printf("passed\n");

    printf("\tLittle-Endian Double (write-after-reopen)...\t");
    if ((retval = nc_get_var_double(ncid, varid_le_dbl, ddata_in))) return retval;
    for (failed=0,i=0;i<DIM_LEN_1802;i++) {if(ddata_out[i]!=ddata_in[i]){printf("failed\n");failures++;failed++;break;}}
    if (!failed) printf("passed\n");

    if ((retval = nc_close(ncid)))
        return retval;
  }

  printf("** Failures Returned: [%d]\n",failures);
  return failures;
}
