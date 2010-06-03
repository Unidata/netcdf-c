/*
 Copyright 2010 University Corporation for Atmospheric
 Research/Unidata. See COPYRIGHT file for more info.

 This file defines most of the netcdf API in terms of the dispatch
 functions along with a few functions that are overlays over the
 dispatch functions.

 "$Id: nc4.c,v 1.1 2010/06/01 15:46:50 ed Exp $" 
*/

#include "dispatch.h"

/* When you read string type the library will allocate the storage
 * space for the data. This storage space must be freed, so pass the
 * pointer back to this function, when you're done with the data, and
 * it will free the string memory. */
int
nc_free_string(size_t len, char **data)
{
   int i;
   for (i = 0; i < len; i++)
      free(data[i]);
   return NC_NOERR;
}

/* When you read VLEN type the library will actually allocate the
 * storage space for the data. This storage space must be freed, so
 * pass the pointer back to this function, when you're done with the
 * data, and it will free the vlen memory. */
int
nc_free_vlen(nc_vlen_t *vl)
{
   free(vl->p);
   return NC_NOERR;
}

/* Free an array of vlens given the number of elements and an
 * array. */ 
int
nc_free_vlens(size_t len, nc_vlen_t vlens[])
{
   int ret;
   size_t i;

   for(i = 0; i < len; i++) 
      if ((ret = nc_free_vlen(&vlens[i])))
	 return ret;

   return NC_NOERR;
}

/* Wrappers for nc_inq_var_all */

int
nc_inq_var_deflate(int ncid, int varid, int *shufflep, int *deflatep, int *deflate_levelp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      shufflep, /*shufflep*/
      deflatep, /*deflatep*/
      deflate_levelp, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      NULL, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}

int
nc_inq_var_szip(int ncid, int varid, int *options_maskp, int *pixels_per_blockp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      NULL, /*endianp*/
      options_maskp, /*optionsmaskp*/
      pixels_per_blockp /*pixelsp*/
      );
}

int
nc_inq_var_fletcher32(int ncid, int varid, int *fletcher32p)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      fletcher32p, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      NULL, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}

int
nc_inq_var_chunking(int ncid, int varid, int *contiguousp, size_t *chunksizesp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      contiguousp, /*contiguousp*/
      chunksizesp, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      NULL, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}

int
nc_inq_var_fill(int ncid, int varid, int *no_fill, void *fill_value)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      no_fill, /*nofillp*/
      fill_value, /*fillvaluep*/
      NULL, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}

int
nc_inq_var_endian(int ncid, int varid, int *endianp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      endianp, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}
