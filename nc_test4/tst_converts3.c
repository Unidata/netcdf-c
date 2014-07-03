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
#define DEF_FILLVAL 1
#define TARG_FILLVAL 2

/*! Test ncbyte to ncubyte
 *
 */
int ncbyte_to_ncubyte () {
  void* src = NULL;
  void* dest = NULL;
  void* src_ptr = NULL;
  void* dest_ptr = NULL;
  int range_error = 0;
  int curerr = 0;
  int retval = 0;
  printf("Testing NC_BYTE to NC_UBYTE.....     ");
  src = (signed char*)malloc(sizeof(signed char)*10);
  src_ptr = src;

  dest = (unsigned char*)malloc(sizeof(unsigned char)*10);
  dest_ptr = dest;

  for(int i = 0; i < 10; i++) {
    *(signed char*)src_ptr++ = (i == 5 ? -1 : 2);
    *(unsigned char*)dest_ptr++ = DEF_FILLVAL;
  }

  retval = nc4_convert_type(src,dest,
			    NC_BYTE,NC_UBYTE,
			    sizeof(src),&range_error,
			    NULL,0,0,0);
  dest_ptr = dest;
  for(int i = 0; i < 10; i++, dest_ptr++) {
    if( (*(unsigned char*)dest_ptr != 2 ) &&
	(*(unsigned char*)dest_ptr != DEF_FILLVAL) ) {
      curerr = 1;
    }
  }
  if(!curerr) printf("Success\n");
  else printf("Error\n");

  free((signed char*)src);
  free((unsigned char*)dest);

  return curerr;

}

/*! Test ncbyte to ncushort
 *
 */
int ncbyte_to_ncushort() {
  void* src = NULL;
  void* dest = NULL;
  void* src_ptr = NULL;
  void* dest_ptr = NULL;
  int range_error = 0;
  int curerr = 0;
  int retval = 0;
  printf("Testing NC_BYTE to NC_USHORT.....     ");
  src = (signed char*)malloc(sizeof(signed char)*10);
  src_ptr = src;

  dest = (unsigned short*)malloc(sizeof(unsigned short)*10);
  dest_ptr = dest;

  for(int i = 0; i < 10; i++) {
    *(signed char*)src_ptr++ = (i == 5 ? -1 : 2);
    *(unsigned short*)dest_ptr++ = DEF_FILLVAL;
  }

  retval = nc4_convert_type(src,dest,
			    NC_BYTE,NC_USHORT,
			    sizeof(src),&range_error,
			    NULL,0,0,0);
  dest_ptr = dest;
  for(int i = 0; i < 10; i++, dest_ptr++) {
    if( (*(unsigned char*)dest_ptr != 2 ) &&
	(*(unsigned short*)dest_ptr != DEF_FILLVAL) ) {
      curerr = 1;
    }
  }
  if(!curerr) printf("Success\n");
  else printf("Error\n");

  free((signed char*)src);
  free((unsigned short*)dest);

  return curerr;

}

/*! Test ncbyte to ncubyte
 *
 */
int ncbyte_to_ncuint () {
  void* src = NULL;
  void* dest = NULL;
  void* src_ptr = NULL;
  void* dest_ptr = NULL;
  int range_error = 0;
  int curerr = 0;
  int retval = 0;
  printf("Testing NC_BYTE to NC_UINT.....     ");
  src = (signed char*)malloc(sizeof(signed char)*10);
  src_ptr = src;

  dest = (unsigned int*)malloc(sizeof(unsigned int)*10);
  dest_ptr = dest;

  for(int i = 0; i < 10; i++) {
    *(signed char*)src_ptr++ = (i == 5 ? -1 : 2);
    *(unsigned int*)dest_ptr++ = DEF_FILLVAL;
  }

  retval = nc4_convert_type(src,dest,
			    NC_BYTE,NC_UINT,
			    sizeof(src),&range_error,
			    NULL,0,0,0);
  dest_ptr = dest;
  for(int i = 0; i < 10; i++, dest_ptr++) {
    if( (*(unsigned int*)dest_ptr != 2 ) &&
	(*(unsigned int*)dest_ptr != DEF_FILLVAL) ) {
      curerr = 1;
    }
  }
  if(!curerr) printf("Success\n");
  else printf("Error\n");

  free((signed char*)src);
  free((unsigned int*)dest);

  return curerr;

}

/*! Standard main function for tst_converts3.c
 *
 */
int main(int argc, char **argv) {
  int toterr = 0;


  /* NC_BYTE to NC_UBYTE */
  //toterr += ncbyte_to_ncubyte();

  /* NC_BYTE to NC_USHORT */
  //toterr += ncbyte_to_ncushort();

  /* NC_BYTE to NC_UINT */
  //toterr += ncbyte_to_ncuint();

  return toterr;
}
