#ifndef NCCOMPRESS_H
#define NCCOMPRESS_H

#include <hdf5.h>

#define COMPRESSION_MAX_NAME 64
#define COMPRESSION_MAX_PARAMS 64

/*
Turn on specified compression for a variable's plist
*/
extern int nccompress_set(const char* algorithm, hid_t plistid, nc_compression_t*);

//extern const char* nccompress_name_for(int id);
//extern int nccompress_id_for(const char* name);
//extern int nccompress_get_parameters(const char*, hid_t, nc_compression_t*);
//extern int nccompress_inq_parameters(const char*, hid_t, size_t, unsigned int*,nc_compression_t*);
extern int nccompress_inq_parameters(H5Z_filter_t filter, hid_t, size_t, unsigned int*,char*,nc_compression_t*);

/* 
Register alll compression filters with the library
*/
extern int nccompress_register_all(void);

#endif /*NCCOMPRESS_H*/
