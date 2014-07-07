/* This is part of the netCDF Package.
   Copyright 2014 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test for data conversion errors.  Converting from a broader type
   to a narrower type should skip the conversion, leaving either
   an undefined or predetermined fill value.  This allows the user
   to query the values and build a mask of original values that cannot
   be properly converted.
*/

#include "nc_tests.h"
#include "netcdf.h"
#include "nc4internal.h"
#include "math.h"
#define TEST_FILLVAL 10
#define TARG_FILLVAL 10

#define EXP_VAL 2
#define ERR_VAL -1
#define ERR_INDEX 5

/*! Test ncbyte to ncubyte
 *
 */
int ncbyte_to_ncuint () {
  void* src = NULL;
  unsigned int *dest = NULL;
  void* src_ptr = NULL;
  unsigned int *dest_ptr = NULL;
  int range_error = 0;
  int curerr = 0;
  int retval = 0;

  printf("Testing NC_BYTE to NC_UINT64.....     ");
  src = (signed char*)malloc(sizeof(signed char)*10);
  src_ptr = src;

  dest = (unsigned long long*)malloc(sizeof(unsigned int)*10);
  dest_ptr = dest;

  for(int i = 0; i < 10; i++) {
    *(signed char*)src_ptr++ = (i == ERR_INDEX ? ERR_VAL : EXP_VAL);
    *(unsigned int*)dest_ptr++ = TEST_FILLVAL;
  }

  retval = nc4_convert_type(src,dest,
			    NC_BYTE,NC_UINT64,
			    sizeof(src),&range_error,
			    NULL,0,0,0);
  dest_ptr = dest;

  /* We expect that illegal values will have been
     skipped over preserving the TEST_FILLVAL, or else
     they will be the expected value. */
  for(int i = 0; i < 10; i++, dest_ptr++) {
    if( (*(unsigned int*)dest_ptr != EXP_VAL ) &&
	(*(unsigned int*)dest_ptr != TEST_FILLVAL) ) {
      curerr = 1;
    }
  }
  if(!curerr) printf("Success\n");
  else printf("Error\n");

  free((signed char*)src);
  free((unsigned int*)dest);

  return curerr;


}

/*! Test ncbyte to ncubyte
 *
 */
int ncbyte_to_ncuint64 () {
  void* src = NULL;
  unsigned long long *dest = NULL;
  void* src_ptr = NULL;
  unsigned long long  *dest_ptr = NULL;
  int range_error = 0;
  int curerr = 0;
  int retval = 0;

  printf("Testing NC_BYTE to NC_UINT64.....     ");
  src = (signed char*)malloc(sizeof(signed char)*10);
  src_ptr = src;

  dest = (unsigned long long*)malloc(sizeof(unsigned long long)*10);
  dest_ptr = dest;

  for(int i = 0; i < 10; i++) {
    *(signed char*)src_ptr++ = (i == ERR_INDEX ? ERR_VAL : EXP_VAL);
    *(unsigned long long*)dest_ptr++ = TEST_FILLVAL;
  }

  retval = nc4_convert_type(src,dest,
			    NC_BYTE,NC_UINT64,
			    sizeof(src),&range_error,
			    NULL,0,0,0);
  dest_ptr = dest;

  /* We expect that illegal values will have been
     skipped over preserving the TEST_FILLVAL, or else
     they will be the expected value. */
  for(int i = 0; i < 10; i++, dest_ptr++) {
    if( (*(unsigned long long*)dest_ptr != EXP_VAL ) &&
	(*(unsigned long long*)dest_ptr != TEST_FILLVAL) ) {
      curerr = 1;
    }
  }
  if(!curerr) printf("Success\n");
  else printf("Error\n");

  free((signed char*)src);
  free((unsigned long long*)dest);

  return curerr;


}

/*! Standard main function for tst_converts3.c
 *
 */
int main(int argc, char **argv) {
  int toterr = 0;


  /* NCBYTE to NCUBYTE */
  toterr = ncbyte_to_ncuint64();

  return toterr;
}
