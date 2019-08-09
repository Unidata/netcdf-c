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
    printf("Testing NC list functions with no open files...");
    {
        /* These are cases where there are no files, or NULL
         * params. */
        if (count_NCList()) ERR;
        free_NCList();
        if (find_in_NCList_by_name("nope")) ERR;
        if (iterate_NCList(0, NULL)) ERR;
    }
    SUMMARIZE_ERR;
    printf("Testing adding to NC list...");
    {
        /* NC *ncp; */
    /* int ret; */

        /* /\* Create the NC* instance and insert its dispatcher and model. *\/ */
        /* if ((ret = new_NC(dispatcher, path, cmode, &model, &ncp))) ERR; */

        /* /\* Add to list of known open files and define ext_ncid. *\/ */
        /* add_to_NCList(ncp); */
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
