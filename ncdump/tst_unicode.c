/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is a very simple example which writes a netCDF file with
   Unicode names encoded with UTF-8.

   $Id: tst_unicode.c,v 1.12 2008/10/20 01:48:08 ed Exp $
*/
#include <nc_tests.h>
#include "err_macros.h"
#include <stdlib.h>
#include <stdio.h>
#include "netcdf.h"
#include "ncpathmgr.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#undef DEBUG

/* The data file we will create. */
static const unsigned char prefix[] = {
't','s','t','_','u','t','f','8','_',
'\xe6', '\xb5', '\xb7',
'\0'
};

/* Other meta-data */
#define UNITS "units"
#define NDIMS 1
#define UTF8_BYTES 18

static unsigned char name_utf8[] = {
       0xCE, 0x9A,	  /* GREEK CAPITAL LETTER KAPPA  : 2-bytes utf8 */
       0xCE, 0xB1,	  /* GREEK SMALL LETTER LAMBDA   : 2-bytes utf8 */
       0xCE, 0xBB,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
       0xCE, 0xB7,	  /* GREEK SMALL LETTER ETA      : 2-bytes utf8 */
       0xCE, 0xBC,	  /* GREEK SMALL LETTER MU       : 2-bytes utf8 */
       0xE1, 0xBD, 0xB3,  /* GREEK SMALL LETTER EPSILON
                             WITH TONOS                  : 3-bytes utf8 */
       0xCF, 0x81,	  /* GREEK SMALL LETTER RHO      : 2-bytes utf8 */
       0xCE, 0xB1, 0x00	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
   };

/* Name used for dimension, variable, and attribute value */
#define UNAME ((char *) name_utf8)
#define UNAMELEN (sizeof name_utf8)

       /* Note, name was normalized before storing, so retrieved name
	  won't match original unnormalized name.  Check that we get
	  normalized version, instead.  */

       /* NFC normalized UTF-8 for Unicode 8-character "Hello" in Greek */
static unsigned char norm_utf8[] = {
	   0xCE, 0x9A,	  /* GREEK CAPITAL LETTER KAPPA  : 2-bytes utf8 */
	   0xCE, 0xB1,	  /* GREEK SMALL LETTER LAMBDA   : 2-bytes utf8 */
	   0xCE, 0xBB,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
	   0xCE, 0xB7,	  /* GREEK SMALL LETTER ETA      : 2-bytes utf8 */
	   0xCE, 0xBC,	  /* GREEK SMALL LETTER MU       : 2-bytes utf8 */
	   0xCE, 0xAD,    /* GREEK SMALL LETTER EPSILON WITH TONOS
			                                 : 2-bytes utf8 */
	   0xCF, 0x81,	  /* GREEK SMALL LETTER RHO      : 2-bytes utf8 */
	   0xCE, 0xB1,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
	   0x00
       };
#define NNAME ((char *) norm_utf8)
#define NNAMELEN (sizeof norm_utf8)

static int
check(int err, int line, const char* file)
{
    if(err != 0) {
        fprintf(stderr,"ERR %s.%d (%d) %s\n",file,line,err,nc_strerror(err));
        fflush(stderr);
    }
    return err;    
}

#define CHECK(err) {if((ret=check(err,__LINE__,__FILE__))) goto done;}

static int
test(int flags, const char* model)
{
   int ret = NC_NOERR;
   int ncid, dimid, varid;
   int dimids[NDIMS];
   char name_in[UNAMELEN + 1], strings_in[UNAMELEN + 1];
   nc_type att_type;
   size_t att_len;
   char filename[4096];

   /* Construct the file name */
   snprintf(filename,sizeof(filename),"%s_%s.nc",prefix,model);

   printf("\n*** Testing UTF-8: %s model\n",model);
   printf("*** creating UTF-8 test file |%s|...", filename);

   CHECK(nc_create(filename, flags, &ncid));

   /* Define dimension with Unicode UTF-8 encoded name */
   CHECK(nc_def_dim(ncid, UNAME, UTF8_BYTES, &dimid));
   dimids[0] = dimid;

   /* Define variable with same name */
   CHECK(nc_def_var(ncid, UNAME, NC_CHAR, NDIMS, dimids, &varid));

   /* Create string attribute with same value */
   CHECK(nc_put_att_text(ncid, varid, UNITS, UNAMELEN, UNAME));

   CHECK(nc_enddef(ncid));

   /* Write string data, UTF-8 encoded, to the file */
   CHECK(nc_put_var_text(ncid, varid, UNAME));

   CHECK(nc_close(ncid));

   /* Check it out. */

   /* Reopen the file. */
   CHECK(nc_open(filename, NC_NOWRITE, &ncid));
   CHECK(nc_inq_varid(ncid, UNAME, &varid));
   CHECK(nc_inq_varname(ncid, varid, name_in));
   {
       if (strncmp(NNAME, name_in, NNAMELEN) != 0)
          {CHECK(NC_EBADNAME);}
   }
   CHECK(nc_inq_att(ncid, varid, UNITS, &att_type, &att_len));
   CHECK(att_type != NC_CHAR || att_len != UNAMELEN);
   CHECK(nc_get_att_text(ncid, varid, UNITS, strings_in));
   strings_in[att_len] = '\0';	/* null terminate, because nc_get_att_text doesn't */
   if (strncmp(UNAME, strings_in, UNAMELEN) != 0)
       {CHECK(NC_EBADNAME);}
   CHECK(nc_close(ncid));
done:
   return ret; 
}  

int
main(int argc, char **argv)
{
   /* Run the utf8 test both for netcdf-4 and netcdf-3 */
   if(test(0,"classic")) ERR;
#ifdef USE_HDF5
   if(test(NC_NETCDF4,"enhanced")) ERR;
#endif

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
