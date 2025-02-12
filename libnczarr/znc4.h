/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file
 *
 * @author Dennis Heimbigner
 */

#ifndef ZNC4_H
#define ZNC4_H

/**************************************************/
#if defined(__cplusplus)
extern "C" {
#endif
    
/* Wrap the code for creating the netcdf-4 data model objects from nc4internal.h */
    
extern int ncz4_create_file(NC_FILE_INFO_T** filep);
extern int ncz4_create_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* name,
			   NC_GRP_INFO_T** grpp);
extern int ncz4_create_var(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const char* name, NC_VAR_INFO_T** varp);
extern int ncz4_build_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var,
				nc_type nctype,
				int storage,
				int scalar,
				int endianness,
				size_t maxstrlen,
				int dimsep,
				char order,
			        size_t ndims,
				size64_t* shape,
				size64_t* chunksizes,
				int* dimids,
			        NClist* filters,
				int no_fill,
				void* fill_value);
extern int ncz4_create_dim(NC_FILE_INFO_T* file, NC_GRP_INFO_T* parent, const struct NCZ_DimInfo* dimdef, NC_DIM_INFO_T** dimp);
extern int ncz4_create_filter(NC_FILE_INFO_T* file,
				NCZ_HDF5* hdf5,
				NCZ_Codec* codec,
				NClist* filterlist,
				NCZ_Filter** filterp);

#if defined(__cplusplus)
}
#endif

#endif /* ZNC4_H */
