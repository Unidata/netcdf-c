/* This is part of the netCDF package. Copyright 2005-2019 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test list functions in nclistmgr.c.

   Ed Hartnett, 8/10/19
*/

#include "config.h"
#include <nc_tests.h>
#include "nc.h"
#include "err_macros.h"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf internal NC list functions.\n");
   {

       printf("Testing NC list functions...");
       {
       }
       SUMMARIZE_ERR;
   }
   FINAL_RESULTS;
}
