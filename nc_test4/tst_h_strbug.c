/* This is part of the netCDF package.  Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program tests fixes for bugs reported with accessing
   fixed-length scalar string variables and variable-length scalar
   string attributes from HDF5 files through the netCDF-4 API.

   Here's a HDF5 sample programs:
   http://hdf.ncsa.uiuc.edu/training/other-ex5/sample-programs/strings.c
*/

#include <config.h>
#include <nc_tests.h>
#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_strbug.h5"
#define VS_ATT_NAME "vsatt"
#define FS_ATT_NAME "fsatt"
#define VS_VAR_NAME "vsvar"
#define FS_VAR_NAME "fsvar"
#define FSTR_LEN 20

int
main()
{
    char *vsdata    = "variable-length string";
    char fsdata[]   = "fixed-length string";
    int i;
    char ch;

    printf("\n*** Creating file for checking fix to bugs in accessing strings from HDF5 non-netcdf-4 file.\n");
    {
	hid_t fileid, spaceid, vstypeid, fstypeid, vsattid, fsattid, vsdsetid, fsdsetid;
	hid_t class;
	size_t type_size = FSTR_LEN;

	if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
	
	/* Create variable-length and fixed-length string types. */
	if ((vstypeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
	if (H5Tset_size(vstypeid, H5T_VARIABLE) < 0) ERR;
	
	if ((fstypeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
	if (H5Tset_size(fstypeid, type_size) < 0) ERR;

	/* Create new file, using default properties. */
	if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
	
	/* Create scalar datasets of variable- and fixed-length strings. */
	if ((vsdsetid = H5Dcreate (fileid, VS_VAR_NAME, vstypeid, spaceid, 
				   H5P_DEFAULT)) < 0) ERR;
	if (H5Dwrite (vsdsetid, vstypeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &vsdata)) ERR;
	if ((fsdsetid = H5Dcreate (fileid, FS_VAR_NAME, fstypeid, spaceid, 
				   H5P_DEFAULT)) < 0) ERR;
	if (H5Dwrite (fsdsetid, fstypeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &fsdata)) ERR;
	
	/* Write scalar global attributes of these types. */
	if ((vsattid = H5Acreate(fileid, VS_ATT_NAME, vstypeid, spaceid, 
				 H5P_DEFAULT)) < 0) ERR;
	if (H5Awrite(vsattid, vstypeid, &vsdata) < 0) ERR;
	if ((fsattid = H5Acreate(fileid, FS_ATT_NAME, fstypeid, spaceid, 
				 H5P_DEFAULT)) < 0) ERR;
	if (H5Awrite(fsattid, fstypeid, &fsdata) < 0) ERR;
	
	/* Close up. */
	if (H5Dclose(vsdsetid) < 0) ERR;
	if (H5Dclose(fsdsetid) < 0) ERR;
	if (H5Aclose(vsattid) < 0) ERR;
	if (H5Aclose(fsattid) < 0) ERR;
	if (H5Sclose(spaceid) < 0) ERR;
	if (H5Tclose(vstypeid) < 0) ERR;
	if (H5Tclose(fstypeid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;
    }
    
    {
	hid_t fileid, spaceid, vstypeid, fstypeid, vsattid, fsattid, vsdsetid, fsdsetid;
	hid_t class;
	size_t type_size_in;
	htri_t is_str;
	char fsdata_in[FSTR_LEN];
	char *vsdata_in;		/* gets allocated */
	
	/* Now reopen the file. */
	if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
	
	/* Check out the vstring attribute. */
	if ((vsattid = H5Aopen_name(fileid, VS_ATT_NAME)) < 0) ERR;
	if ((vstypeid = H5Aget_type(vsattid)) < 0) ERR;
	if ((spaceid = H5Aget_space(vsattid)) < 0) ERR;
	
	/* Given this type id, how would we know this is a variable-length string
	 * attribute? */
	if ((class = H5Tget_class(vstypeid)) < 0) ERR;
	if (class != H5T_STRING) ERR;
	if (!(type_size_in = H5Tget_size(vstypeid))) ERR;
	if (type_size_in != sizeof (char *)) ERR;
	if (!(is_str = H5Tis_variable_str(vstypeid))) ERR;
	
	/* Make sure this is a scalar. */
	if (H5Sget_simple_extent_type(spaceid) != H5S_SCALAR) ERR;
	
	/* Read the attribute. */
	if (H5Aread(vsattid, vstypeid, &vsdata_in) < 0) ERR;
	
	/* Check the data. */
	if (strcmp(vsdata, vsdata_in)) ERR;
	
	/* Free our memory. */
	free(vsdata_in);
	
	/* Check out the fstring attribute. */
	if ((fsattid = H5Aopen_name(fileid, FS_ATT_NAME)) < 0) ERR;
	if ((fstypeid = H5Aget_type(fsattid)) < 0) ERR;
	if ((spaceid = H5Aget_space(fsattid)) < 0) ERR;
	
	/* Given this type id, how would we know this is a fixed-length string
	 * attribute? */
	if ((class = H5Tget_class(fstypeid)) < 0) ERR;
	if (class != H5T_STRING) ERR;
	if (!(type_size_in = H5Tget_size(fstypeid))) ERR;
	if (type_size_in != FSTR_LEN) ERR;
	if ((is_str = H5Tis_variable_str(fstypeid))) ERR;
	
	/* Make sure this is a scalar. */
	if (H5Sget_simple_extent_type(spaceid) != H5S_SCALAR) ERR;
	
	/* Read the attribute. */
	if (H5Aread(fsattid, fstypeid, &fsdata_in) < 0) ERR;
	
	/* Check the data. */
	if (strcmp(fsdata, fsdata_in)) ERR;
	
	/* Check out the vstring dataset. */
	if((vsdsetid = H5Dopen(fileid, VS_VAR_NAME)) < 0) ERR;
	if ((vstypeid = H5Dget_type(vsdsetid)) < 0) ERR;
	if ((spaceid = H5Dget_space(vsdsetid)) < 0) ERR;
	
	/* Given this type id, how would we know this is a variable-length string
	 * dataset? */
	if ((class = H5Tget_class(vstypeid)) < 0) ERR;
	if (class != H5T_STRING) ERR;
	if (!(type_size_in = H5Tget_size(vstypeid))) ERR;
	if (type_size_in != sizeof (char *)) ERR;
	if (!(is_str = H5Tis_variable_str(vstypeid))) ERR;
	
	/* Make sure this is a scalar. */
	if (H5Sget_simple_extent_type(spaceid) != H5S_SCALAR) ERR;
	
	/* Read the dataset. */
	if (H5Dread(vsdsetid, vstypeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &vsdata_in) < 0) ERR;
	
	/* Check the data. */
	if (strcmp(vsdata, vsdata_in)) ERR;
	
	/* Free our memory. */
	free(vsdata_in);
	
	/* Check out the fstring dataset. */
	if((fsdsetid = H5Dopen(fileid, FS_VAR_NAME)) < 0) ERR;
	if ((fstypeid = H5Dget_type(fsdsetid)) < 0) ERR;
	if ((spaceid = H5Dget_space(fsdsetid)) < 0) ERR;
	
	/* Given this type id, how would we know this is a fixed-length string
	 * dataset? */
	if ((class = H5Tget_class(fstypeid)) < 0) ERR;
	if (class != H5T_STRING) ERR;
	if (!(type_size_in = H5Tget_size(fstypeid))) ERR;
	if (type_size_in != FSTR_LEN) ERR;
	if (is_str = H5Tis_variable_str(fstypeid)) ERR;
	
	/* Make sure this is a scalar. */
	if (H5Sget_simple_extent_type(spaceid) != H5S_SCALAR) ERR;
	
	/* Read the dataset. */
	if (H5Dread(fsdsetid, fstypeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &fsdata_in) < 0) ERR;
	
	/* Check the data. */
	if (strcmp(fsdata, fsdata_in)) ERR;
	
	/* Close HDF5 stuff. */
	if (H5Aclose(vsattid) < 0) ERR;
	if (H5Aclose(fsattid) < 0) ERR;
	if (H5Dclose(vsdsetid) < 0) ERR;
	if (H5Dclose(fsdsetid) < 0) ERR;
	if (H5Sclose(spaceid) < 0) ERR;
	if (H5Tclose(vstypeid) < 0) ERR;
	if (H5Tclose(fstypeid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;
    }

    printf("*** Checking reading variable-length HDF5 string var through netCDF-4 API...");
    {
	int ncid, varid, ndims;
	nc_type type;
	char *data_in;
	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	if (nc_inq_varid(ncid, VS_VAR_NAME, &varid)) ERR;
	if (nc_inq_vartype(ncid, varid, &type)) ERR;
	if (type != NC_STRING) ERR;
	if (nc_inq_varndims(ncid, varid, &ndims )) ERR;
	if (ndims != 0) ERR;
	if (nc_get_var_string(ncid, varid, &data_in)) ERR;
	if (strcmp(vsdata, data_in));
	if (nc_free_string(1, &data_in)) ERR;
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** Checking reading fixed-length HDF5 string var through netCDF-4 API...");
    {
    	int ncid, varid, ndims;
    	nc_type type;
    	char *data_in;
    	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
    	if (nc_inq_varid(ncid, FS_VAR_NAME, &varid)) ERR;
    	if (nc_inq_vartype(ncid, varid, &type)) ERR;
    	if (type != NC_STRING) ERR;
    	if (nc_inq_varndims(ncid, varid, &ndims )) ERR;
    	if (ndims != 0) ERR;
    	if (nc_get_var_string(ncid, varid, &data_in)) ERR;
    	if (strcmp(fsdata, data_in));
	if (nc_free_string(1, &data_in)) ERR;
    	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** Checking reading variable-length HDF5 string att through netCDF-4 API...");
    {
	int ncid, varid, ndims;
	nc_type type;
	size_t len;
	char *data_in;
	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	if (nc_inq_att(ncid, NC_GLOBAL, VS_ATT_NAME, &type, &len)) ERR;
	if (type != NC_STRING) ERR;
	if (len != 1) ERR;
        if (nc_get_att_string(ncid, NC_GLOBAL, VS_ATT_NAME, &data_in));
	if (strcmp(vsdata, data_in));
	if (nc_free_string(1, &data_in)) ERR;
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** Checking reading fixed-length HDF5 string att through netCDF-4 API...");
    {
	int ncid, varid, ndims;
	nc_type type;
	size_t len;
	char *data_in;
	if (!(data_in = malloc(FSTR_LEN))) ERR;
	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	if (nc_inq_att(ncid, NC_GLOBAL, FS_ATT_NAME, &type, &len)) ERR;
	if (type != NC_CHAR) ERR;
	if (len != FSTR_LEN) ERR;
        if (nc_get_att_string(ncid, NC_GLOBAL, FS_ATT_NAME, &data_in));
	if (strcmp(fsdata, data_in));
	if (nc_free_string(1, &data_in)) ERR;
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    /* printf("*** Checking reading variable-length HDF5 strings var through netCDF-4 API..."); */
    /* { */
    /* 	int ncid, varid, ndims; */
    /* 	nc_type type; */
    /* 	char *data_in[1]; */
    /* 	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; */
    /* 	if (nc_inq_varid(ncid, VS_VAR_NAME, &varid)) ERR; */
    /* 	if (nc_inq_vartype(ncid, varid, &type)) ERR; */
    /* 	if (type != NC_STRING) ERR; */
    /* 	if (nc_inq_varndims(ncid, varid, &ndims )) ERR; */
    /* 	if (ndims != 0) ERR; */
    /* 	if (nc_get_var_string(ncid, varid, data_in)) ERR; */
    /* 	if (strcmp(vsdata, data_in[0])); */
    /* 	if (nc_free_string(1, data_in)) ERR; */
    /* 	if (nc_close(ncid)) ERR; */
    /* } */
    /* SUMMARIZE_ERR; */

    FINAL_RESULTS;
}
