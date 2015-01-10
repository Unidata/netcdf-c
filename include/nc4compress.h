#ifndef NCCOMPRESS_H
#define NCCOMPRESS_H

#include <hdf5.h>
#include <fpzip.h>

#define CD_NELEMS_ZIP 1
#define CD_NELEMS_SZIP 2
#define CD_NELEMS_BZIP2 1
#define CD_NELEMS_FPZIP 64
#define CD_NELEMS_ZFP 64

/* It should be possible to overlay this
on the params[] to extract the values.
This should match the union comment in netcdf.h.
*/
typedef union {
    unsigned int params[NC_COMPRESSION_MAX_PARAMS];/*arbitrary 32 bit values*/
    struct {unsigned int level;} zip;
    struct {unsigned int level;} bzip2;
    struct {
        unsigned int options_mask;
	unsigned int bits_per_pixel;
        unsigned int pixels_per_block;
	unsigned int pixels_per_scanline;
    } szip;
    struct {
	int isdouble;
	int prec; /* number of bits of precision (zero = full) */
	int rank;
	size_t chunksizes[(NC_COMPRESSION_MAX_PARAMS-3)/2];
    } fpzip;
    struct {
      int isdouble;
      int prec;
      double rate;
      double tolerance;
      int rank;
      size_t chunksizes[(NC_COMPRESSION_MAX_PARAMS-5)/2];
    } zfp; 
} nc_compression_t;

/*
Turn on specified compression for a variable's plist
*/
extern int nccompress_set(const char* algorithm, hid_t plistid, unsigned int*);

extern int nccompress_inq_parameters(H5Z_filter_t filter, hid_t, size_t, unsigned int*,char*,unsigned int*);

/* 
Register all compression filters with the library
*/
extern int nccompress_register_all(void);

#endif /*NCCOMPRESS_H*/
