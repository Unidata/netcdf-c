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

/* Raw encode/decode */
struct NCjson; /*Opaque*/
DECLSPEC int NCraw_encode(size_t nparams, const unsigned* params, struct NCjson** jparamsp);
DECLSPEC int NCraw_decode(const NCjson* jraw, size_t* nparamsp, unsigned** paramsp);

#endif /*H5MISC_H*/
