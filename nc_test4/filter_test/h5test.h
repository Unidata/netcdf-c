#include "bzlib.h"

/* use an integer greater than 256 to be id of the registered filter. */
#define H5Z_FILTER_TEST 32768

/* Define the test cases */

typedef enum H5testcase {
TC_NONE = 0,
TC_ENDIAN = 1,
} H5testcase;

const H5Z_class2_t H5Z_TEST[1]; 

/* declare a filter function */
extern size_t H5Z_filter_test(unsigned flags,size_t cd_nelmts,const unsigned cd_values[],
                    size_t nbytes,size_t *buf_size,void**buf);

