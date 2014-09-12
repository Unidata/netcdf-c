#ifndef NCCOMPRESS_H
#define NCCOMPRESS_H

#define H5Z_FILTER_BZIP2 NC_COMPRESS_BZIP2

/*
Turn on specified compression for a variable's plist
*/
extern int nccompress_set(int algorithm, hid_t plistid, const nc_compression_t*);

/* 
Register compress filter with the library
*/
extern int nccompress_register(int id, const nc_compression_t*);

#endif /*NCCOMPRESS_H*/
