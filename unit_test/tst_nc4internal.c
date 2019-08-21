/* This is part of the netCDF package. Copyright 2005-2019 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test list functions in nc4internal.c.

   Ed Hartnett, 8/21/19
*/

#include "config.h"
#include <nc_tests.h>
#include "nc.h"
#include "nc4internal.h"
#include "ncdispatch.h"
#include "err_macros.h"

/* An integer value to use in testing. */
#define TEST_VAL_42 42

#define FILE_NAME "tst_nc4internal.nc"

int
main(int argc, char **argv)
{
    printf("\n*** Testing netcdf nc4internal functions.\n");
    printf("Testing adding new file to nc4internal file lists with "
           "nc4_file_list_add()...");
    {
        NC *ncp;
        NCmodel model;
        char *path;
        /* The NC3_dispatch_table is defined in nc3dispatch.c and
         * externed in ncdispatch.h. But it will be 0 because we have
         * not yet called NC3_initialize(). */
        const void *dispatcher = NC3_dispatch_table;
        void *dispatchdata_in;
        int mode = 0, mode_in;

        /* This won't work because there is no NC in the NC list which
         * has an ncid of TEST_VAL_42. */
        if (nc4_file_list_add(TEST_VAL_42, FILE_NAME, 0, NULL) != NC_EBADID) ERR;

        /* Create the NC* instance and insert its dispatcher and
         * model. */
        if (new_NC(dispatcher, FILE_NAME, mode, &model, &ncp)) ERR;

        /* Add to array of open files nc_filelist and define
         * ext_ncid by left-shifting the index 16 bits. */
        add_to_NCList(ncp);

        /* Create the NC_FILE_INFO_T instance associated empty lists
         * to hold dims, types, groups, and the root group. */
        if (nc4_file_list_add(ncp->ext_ncid, FILE_NAME, mode, NULL)) ERR;

        /* Find the file in the list. */
        if (!(path = malloc(NC_MAX_NAME + 1))) ERR;
        if (nc4_file_list_get(ncp->ext_ncid, &path, &mode_in, &dispatchdata_in)) ERR;
        if (strcmp(path, FILE_NAME)) ERR;
        free(path);
        if (mode_in != mode) ERR;

        /* This won't work. */
        if (nc4_file_list_del(TEST_VAL_42) != NC_EBADID) ERR;

        /* Delete the NC_FILE_INFO_T and related storage. */
        if (nc4_file_list_del(ncp->ext_ncid)) ERR;

        /* Delete the ncp from the list. (In fact, just null out its
         * entry in the array of file slots.) */
        del_from_NCList(ncp); /* Will free empty list. */

        /* Now free the NC struct. */
        free_NC(ncp);
    }
    SUMMARIZE_ERR;
    printf("Testing adding new file to nc4internal file lists with "
           "nc4_nc4f_list_add()...");
    {
        NC *ncp;
        NCmodel model;
        char *path;
        void *dispatchdata_in;
        int mode = 0, mode_in;

        /* Create the NC* instance and insert its dispatcher and
         * model. */
        if (new_NC(NC3_dispatch_table, FILE_NAME, mode, &model, &ncp)) ERR;

        /* Add to array of open files nc_filelist and define
         * ext_ncid by left-shifting the index 16 bits. */
        add_to_NCList(ncp);

        /* Create the NC_FILE_INFO_T instance associated empty lists
         * to hold dims, types, groups, and the root group. */
        if (nc4_nc4f_list_add(ncp, FILE_NAME, mode)) ERR;

        /* Find the file in the list. */
        if (!(path = malloc(NC_MAX_NAME + 1))) ERR;
        if (nc4_file_list_get(ncp->ext_ncid, &path, &mode_in, &dispatchdata_in)) ERR;
        if (strcmp(path, FILE_NAME)) ERR;
        free(path);
        if (mode_in != mode) ERR;

        /* This won't work. */
        if (nc4_file_list_del(TEST_VAL_42) != NC_EBADID) ERR;

        /* Delete the NC_FILE_INFO_T and related storage. */
        if (nc4_file_list_del(ncp->ext_ncid)) ERR;

        /* Delete the ncp from the list. (In fact, just null out its
         * entry in the array of file slots.) */
        del_from_NCList(ncp); /* Will free empty list. */

        /* Now free the NC struct. */
        free_NC(ncp);
    }
    SUMMARIZE_ERR;
    /* printf("Testing changing ncid..."); */
    /* { */
    /*     NC *ncp, *ncp2; */
    /*     int mode = 0; */
    /*     NCmodel model; */
    /*     int ret; */

    /*     /\* Create the NC* instance and insert its dispatcher and model. *\/ */
    /*     if ((ret = new_NC(NULL, FILE_NAME, mode, &model, &ncp))) ERR; */

    /*     /\* Add to list of known open files and define ext_ncid. *\/ */
    /*     add_to_NCList(ncp); */

    /*     /\* Find it in the list. *\/ */
    /*     if (!(ncp2 = find_in_NCList(ncp->ext_ncid))) ERR; */
    /*     if (!(ncp2 = find_in_NCList_by_name(FILE_NAME))) ERR; */
    /*     if ((ret = iterate_NCList(1, &ncp2))) ERR; */
    /*     if (count_NCList() != 1) ERR; */

    /*     /\* Change the ncid. *\/ */
    /*     if (nc4_file_change_ncid(ncp->ext_ncid, TEST_VAL_42)) ERR; */

    /*     /\* Delete it. *\/ */
    /*     del_from_NCList(ncp); /\* Will free empty list. *\/ */
    /*     free_NC(ncp); */

    /*     /\* Ensure it is no longer in list. *\/ */
    /*     /\* if (find_in_NCList(ncp->ext_ncid)) ERR; *\/ */

    /* } */
    /* SUMMARIZE_ERR; */
    FINAL_RESULTS;
}
