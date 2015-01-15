#ifndef NCCOMPRESS_H
#define NCCOMPRESS_H

#include <hdf5.h>
#include <fpzip.h>

/* Define an enum for all potentially supported compressors */
typedef enum NC_compress_enum {
NC_NOZIP = -1,
NC_ZIP   = 0,
NC_SZIP  = 1,
NC_BZIP2 = 2,
NC_FPZIP = 3,
NC_ZFP   = 4,
NC_COMPRESSORS = NC_ZFP+1,
} NC_compress_enum;

/* These must all be <= NC_COMPRESSION_MAX_PARAMS in netcdf.h */
#define NC_NELEMS_ZIP 1
#define NC_NELEMS_SZIP 2
#define NC_NELEMS_BZIP2 1
#define NC_NELEMS_FPZIP 36
#define NC_NELEMS_ZFP 40


/* Define a struct for holding info about each compressor */
typedef struct NC_compressor_info {
    const char* name; /* canonical name for the compressor */
    int nelems; /* size of the compression parameters */
} NC_compressor_info;

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
	size_t chunksizes[NC_COMPRESSION_MAX_DIMS];
    } fpzip;
    struct {
        int isdouble;
        int prec;
        double rate;
        double tolerance;
        int rank;
	size_t chunksizes[NC_COMPRESSION_MAX_DIMS];
    } zfp; 
} nc_compression_t;

/*
Turn on specified compression for a variable's plist
*/
extern int nccompress_set(const char* algorithm, hid_t plistid, int, unsigned int*);

extern int nccompress_inq_parameters(H5Z_filter_t filter, hid_t, int, unsigned int*,char*,int*,unsigned int*);

/* 
Register all compression filters with the library
*/
extern int nccompress_register_all(void);

/* get compressor info by enum */
extern const NC_compressor_info* nc_compressor_by_index(NC_compress_enum index);

/* Index by name */
extern NC_compress_enum nc_compressor_by_name(const char* name);

#endif /*NCCOMPRESS_H*/
