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

#define FILE_NAME "tst_nclist.nc"

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
        int ncid;
        NC *ncp, *ncp2;
        int mode = 0;
        int ret;

        /* Create the NC* instance and insert its dispatcher and model. */
        if ((ret = new_NC(NULL, FILE_NAME, mode, &ncp))) ERR;

        /* Nothing to find yet. */
        if (find_in_NCList(TEST_VAL_42)) ERR;

        /* Add to list of known open files and define ext_ncid. To get
         * the ncid, we find the first open index > 1 in the
         * nc_filelist array, which has a size of 65536. Then we
         * left-shift that index 16 bits to put it in the first
         * 2-bytes of the 4-byte ncid. (The other two bytes are
         * reserved for grpid of netCDF-4 groups.) */
        add_to_NCList(ncp);

        /* These won't work! */
        if (find_in_NCList(TEST_VAL_42)) ERR;
        if (find_in_NCList_by_name("nope")) ERR;
        if ((ret = iterate_NCList(2, &ncp2))) ERR;

        /* Find it in the list. */
        if (!(ncp2 = find_in_NCList(ncp->ext_ncid))) ERR;
        if (!(ncp2 = find_in_NCList_by_name(FILE_NAME))) ERR;
        if ((ret = iterate_NCList(1, &ncp2))) ERR;
        if (count_NCList() != 1) ERR;

        /* Won't do anything because list contains an entry. */
        free_NCList();

        /* Delete it. */
        ncid = ncp->ext_ncid;
        del_from_NCList(ncp); /* Will free empty list. */
        free_NC(ncp);

        /* Ensure it is no longer in list. */
        if (find_in_NCList(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    printf("Testing moving in NC list (needed for PIO)...");
    {
        int ncid;
        NC *ncp, *ncp2;
        int mode = 0;
        int ret;

        /* Create the NC* instance and add it to list. */
        if ((ret = new_NC(NULL, FILE_NAME, mode, &ncp))) ERR;
        add_to_NCList(ncp);

        /* Find it in the list. */
        if (!(ncp2 = find_in_NCList(ncp->ext_ncid))) ERR;

        /* Move it. */
        ncid = ncp->ext_ncid;
        if (move_in_NCList(ncp, TEST_VAL_42)) ERR;

        /* Now we won't find old ncid in the list. */
        if (find_in_NCList(ncid)) ERR;

        /* Delete it. */
        ncid = ncp->ext_ncid;
        del_from_NCList(ncp); /* Will free empty list. */
        free_NC(ncp);

        /* Ensure it is no longer in list. */
        if (find_in_NCList(ncid)) ERR;
    }
    SUMMARIZE_ERR;
#ifdef LARGE_FILE_TESTS
    /* This test is slow, only run it on large file test builds. */
    printf("Testing maxing out NC list...");
    {
        NC *ncp;
        int mode = 0;
        NCmodel model;
        int max_num_nc = 65535;
        int i;
        int ret;

        /* Fill the NC list. */
        for (i = 0; i < max_num_nc; i++)
        {
            if ((ret = new_NC(NULL, FILE_NAME, mode, &ncp))) ERR;
            if (add_to_NCList(ncp)) ERR;
        }

        /* Check the count. */
        if (count_NCList() != max_num_nc) ERR;

        /* Try and add another. It will fail. */
        if (add_to_NCList(ncp) != NC_ENOMEM) ERR;

        /* Delete them all. */
        for (i = 0; i < max_num_nc; i++)
        {
            if (iterate_NCList(i + 1, &ncp)) ERR;
            if (!ncp) ERR;
            del_from_NCList(ncp);
            free_NC(ncp);
        }
    }
    SUMMARIZE_ERR;
#endif /* LARGE_FILE_TESTS */

    FINAL_RESULTS;
}
