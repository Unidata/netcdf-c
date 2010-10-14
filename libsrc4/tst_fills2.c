/* This is part of the netCDF package. Copyright 2008 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with default fill values for variables of each type.

   $Id$
*/

#include <nc_tests.h>
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

#define FILE_NAME "tst_fills.nc" 

int
main(int argc, char **argv) 
{			/* create tst_classic_fills.nc */
   printf("\n*** Testing fill values.\n");
   printf("*** testing very simple scalar string var...");
   {
#define STRING_VAR_NAME "Petition_of_Right"
#define NDIMS_STRING 1

      int  ncid, varid;
      int varid_in;
      const char *data_out[NDIMS_STRING] = {
	 "Humbly show unto our Sovereign Lord the King, the Lords "
	 "Spiritual and Temporal, and Commons in Parliament assembles, "
	 "that whereas it is declared and enacted by a statute made in "
	 "the time of the reign of King Edward I, commonly called "
	 "Stratutum de Tellagio non Concedendo, that no tallage or "
	 "aid shall be laid or levied by the king or his heirs in this "
	 "realm, without the good will and assent of the archbishops, "
	 "bishops, earls, barons, knights, burgesses, and other the "
	 "freemen of the commonalty of this realm; and by authority "
	 "of parliament holden in the five-and-twentieth year of the "
	 "reign of King Edward III, it is declared and enacted, that "
	 "from thenceforth no person should be compelled to make any "
	 "loans to the king against his will, because such loans were "
	 "against reason and the franchise of the land; and by other "
	 "laws of this realm it is provided, that none should be charged "
	 "by any charge or imposition called a benevolence, nor by such "
	 "like charge; by which statutes before mentioned, and other the "
	 "good laws and statutes of this realm, your subjects have inherited "
	 "this freedom, that they should not be compelled to contribute "
	 "to any tax, tallage, aid, or other like charge not set by "
	 "common consent, in parliament."
      };
      char *data_in[NDIMS_STRING];
      size_t index1[NDIMS_STRING] = {0};

      /* Create file with a 1D string var. Set its fill value to the
       * empty string. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_var(ncid, STRING_VAR_NAME, NC_STRING, 0, NULL, &varid)) ERR;

      /* Check it out. */
      if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR;
      if (varid_in != varid) ERR;

      /* Write one string, leaving some blank records which will then
       * get the fill value. */
      if (nc_put_var1_string(ncid, varid_in, index1, data_out)) ERR;

      /* Get the string, check it, and free it. */
      if (nc_get_var_string(ncid, varid_in, data_in)) ERR;
      if (strcmp(data_in[0], data_out[0])) ERR;
      if (nc_free_string(1, data_in)) ERR;

/*       /\* First there should be fill values, then the data value we */
/*        * wrote. *\/ */
/*       for (i = 0; i < DATA_START; i++) */
/* 	 if (strcmp(data_in[i], "")) ERR; */
/*       if (strcmp(data_in[DATA_START], data_out[0])) ERR; */

/*       /\* Close everything up. Don't forget to free the string! *\/ */
/*       if (nc_free_string(DATA_START + 1, data_in)) ERR; */
      if (nc_close(ncid)) ERR;

      /* Now re-open file, read data, and check values again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Get the string, check it, and free it. */
/*       if (nc_get_var_string(ncid, varid_in, data_in)) ERR; */
/*       if (strcmp(data_in[0], data_out[0])) ERR; */
/*       if (nc_free_string(1, data_in)) ERR; */

      /* Close the file. */
      if (nc_close(ncid)) ERR; 
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
