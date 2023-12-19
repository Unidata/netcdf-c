#ifndef H5MISC_H
#define H5MISC_H

#ifdef _MSC_VER
#ifdef DLL_EXPORT /* define when building the library */
#    define DECLSPEC __declspec(dllexport)
#else
#    define DECLSPEC __declspec(dllimport)
#endif
#else
#    define DECLSPEC extern
#endif

#include "netcdf_filter_build.h"
#include "netcdf_filter_hdf5_build.h"

/* use an integer greater than 256 to be id of the registered filter. */
#define H5Z_FILTER_TEST 32768

/* Define the test cases */

typedef enum H5testcase {
TC_NONE = 0,
TC_PARAMS = 1,
TC_ODDSIZE = 2,
TC_EXPANDED = 3,
} H5testcase;

/* Ensure consistency with test case */
/* All numeric types types */
struct All {
    signed char tbyte;
    unsigned char tubyte;
    signed short tshort;
    unsigned short tushort;
    signed int tint;
    unsigned int tuint;
    float tfloat32;
    signed long long tint64;
    unsigned long long tuint64;
    double tfloat64;
};

/* number of 32 bit unsigned value needed to hold fields of struct All */
#define NPARAMS (10 + 1/*int64*/ + 1/*uint64*/ + 1/*double*/ + 1/*test case number*/)

/* Test values */
static struct All spec = {
(char)-17,		/* signed byte */
(unsigned char)23,	/* unsigned byte */
(signed short)-25,			/* signed short */
(unsigned short)27U,			/* unsigned short */
77,			/* signed int */
93U,			/* unsigned int */
789.0f,			/* float */
-9223372036854775807LL,	/* signed int64 */
18446744073709551615ULL,/* unsigned int64 */
(double)12345678.12345678/* double */
};

/* declare the hdf5 interface */
DECLSPEC H5PL_type_t H5PLget_plugin_type(void);
DECLSPEC const void* H5PLget_plugin_info(void);
DECLSPEC const H5Z_class2_t H5Z_TEST[1]; 

#if 0
/* Declare filter specific functions */
DECLSPEC htri_t H5Z_test_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id);
DECLSPEC size_t H5Z_filter_test(unsigned flags,size_t cd_nelmts,const unsigned cd_values[],
                    size_t nbytes,size_t *buf_size,void**buf);
#endif

DECLSPEC void NC_h5filterspec_fix8(void* mem0, int decode);

/* Shutup compiler */
void* NC_unused_h5misc() {return (void*)&spec;}

#endif /*H5MISC_H*/
