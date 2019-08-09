/* This is part of the netCDF package. Copyright 2005-2019 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test list functions in nclistmgr.c.

   Ed Hartnett, 8/10/19
*/

#include "config.h"
#include <nc_tests.h>
#include "nc.h"
#include "ncdispatch.h"
#include "err_macros.h"

/* An integer value to use in testing. */
#define TEST_VAL_42 42

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
        /* int ncid; */
        NC *ncp, *ncp2;
        int mode = 0;
        NCmodel model;
        char path[] = {"file.nc"};
        int ret;

        /* Create the NC* instance and insert its dispatcher and model. */
        if ((ret = new_NC(NULL, path, mode, &model, &ncp))) ERR;

        /* Add to list of known open files and define ext_ncid. */
        add_to_NCList(ncp);

        /* These won't work! */
        if (find_in_NCList(TEST_VAL_42)) ERR;
        if (find_in_NCList_by_name("nope")) ERR;
        if ((ret = iterate_NCList(2, &ncp2))) ERR;

        /* Find it in the list. */
        if (!(ncp2 = find_in_NCList(ncp->ext_ncid))) ERR;
        if (!(ncp2 = find_in_NCList_by_name(path))) ERR;
        if ((ret = iterate_NCList(1, &ncp2))) ERR;
        if (count_NCList() != 1) ERR;

        /* Delete it. */
        /* ncid = ncp->ext_ncid; */
        /* free_NC(ncp); */
        /* del_from_NCList(ncp); */
        /* if (find_in_NCList(ncid)) ERR; */

        /* Where is the allocated memory being freed? */
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
