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
#define VAR_NAME "Hilary_Duff"
#define DIM_NAME "Foggy"
#define DIM_LEN 5
#define TYPE_NAME "Madonna"
#define TYPE_SIZE TEST_VAL_42
#define FIELD_NAME "Britany_Spears"
#define FIELD_OFFSET 9

int
main(int argc, char **argv)
{
    printf("\n*** Testing netcdf nc4internal functions.\n");
    printf("Testing adding new file to nc4internal file lists with "
           "nc4_file_list_add()...");
    {
        NC *ncp;
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

        /* Create the NC* instance and insert its dispatcher */
        if (new_NC(dispatcher, FILE_NAME, mode, &ncp)) ERR;

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
        NC *ncp, *ncp_in, *ncp_in2;
        char *path;
        void *dispatchdata_in;
        int mode = 0, mode_in;
        NC_GRP_INFO_T *grp, *grp2;
        NC_FILE_INFO_T *h5, *h52;

        /* Create the NC* instance and insert its dispatcher */
        if (new_NC(NC3_dispatch_table, FILE_NAME, mode, &ncp)) ERR;

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

        /* Find it again. */
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, &ncp_in, &grp, &h5)) ERR;
        if (ncp_in->ext_ncid != ncp->ext_ncid) ERR;
        if (grp->nc4_info->controller->ext_ncid != ncp->ext_ncid) ERR;
        if (h5->controller->ext_ncid != ncp->ext_ncid) ERR;

        /* Any of the pointer parameters may be NULL. */
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, NULL, NULL, NULL)) ERR;
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, &ncp_in2, NULL, NULL)) ERR;
        if (ncp_in2->ext_ncid != ncp->ext_ncid) ERR;
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, NULL, &grp2, NULL)) ERR;
        if (grp2->nc4_info->controller->ext_ncid != ncp->ext_ncid) ERR;
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, NULL, NULL, &h52)) ERR;
        if (h52->controller->ext_ncid != ncp->ext_ncid) ERR;

        /* There are additional functions which use the NULL
         * parameters of nc4_find_nc_grp_h5(). */
        grp2 = NULL;
        h52 = NULL;
        if (nc4_find_grp_h5(ncp->ext_ncid, NULL, NULL)) ERR;
        if (nc4_find_grp_h5(ncp->ext_ncid, &grp2, NULL)) ERR;
        if (grp2->nc4_info->controller->ext_ncid != ncp->ext_ncid) ERR;
        if (nc4_find_grp_h5(ncp->ext_ncid, NULL, &h52)) ERR;
        if (h52->controller->ext_ncid != ncp->ext_ncid) ERR;
        grp2 = NULL;
        if (nc4_find_nc4_grp(ncp->ext_ncid, NULL)) ERR;
        if (nc4_find_nc4_grp(ncp->ext_ncid, &grp2)) ERR;
        if (grp2->nc4_info->controller->ext_ncid != ncp->ext_ncid) ERR;

        /* Delete the NC_FILE_INFO_T and related storage. */
        if (nc4_file_list_del(ncp->ext_ncid)) ERR;

        /* Delete the ncp from the list. (In fact, just null out its
         * entry in the array of file slots.) */
        del_from_NCList(ncp); /* Will free empty list. */

        /* Now free the NC struct. */
        free_NC(ncp);
    }
    SUMMARIZE_ERR;
    printf("Testing adding new var to nc4internal file...");
    {
        NC *ncp, *ncp_in;
        NC_GRP_INFO_T *grp;
        NC_VAR_INFO_T *var, *var_in;
        NC_FILE_INFO_T *h5;

        /* Create the NC* instance and insert its dispatcher */
        if (new_NC(NC3_dispatch_table, FILE_NAME, 0, &ncp)) ERR;

        /* Add to array of open files nc_filelist and define
         * ext_ncid by left-shifting the index 16 bits. */
        add_to_NCList(ncp);

        /* Create the NC_FILE_INFO_T instance associated empty lists
         * to hold dims, types, groups, and the root group. */
        if (nc4_file_list_add(ncp->ext_ncid, FILE_NAME, 0, NULL)) ERR;

        /* Find the file in the list. */
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, &ncp_in, &grp, &h5)) ERR;
        if (ncp_in->ext_ncid != ncp->ext_ncid) ERR;

        /* Add a var to the varlist. */
        if (nc4_var_list_add(grp, VAR_NAME, 0, &var)) ERR;

        /* Find the var. */
        if (nc4_find_var(grp, VAR_NAME, &var_in)) ERR;
        if (strcmp(var_in->hdr.name, var->hdr.name)) ERR;

        /* Find it again. */
        h5 = NULL;
        grp = NULL;
        var_in = NULL;
        if (nc4_find_grp_h5_var(ncp->ext_ncid, 0, &h5, &grp, &var_in)) ERR;
        if (h5->controller->ext_ncid != ncp->ext_ncid) ERR;
        if (grp->nc4_info->controller->ext_ncid != ncp->ext_ncid) ERR;
        if (strcmp(var_in->hdr.name, var->hdr.name)) ERR;

        /* Delete the NC_FILE_INFO_T and related storage, including
         * all vars, dims, types, etc. */
        if (nc4_file_list_del(ncp->ext_ncid)) ERR;

        /* Delete the ncp from the list. (In fact, just null out its
         * entry in the array of file slots.) */
        del_from_NCList(ncp); /* Will free empty list. */

        /* Now free the NC struct. */
        free_NC(ncp);
    }
    SUMMARIZE_ERR;
    printf("Testing adding new dim to nc4internal file...");
    {
        NC *ncp;
        NC_GRP_INFO_T *grp, *dim_grp;
        NC_DIM_INFO_T *dim, *dim_in;

        /* Create the NC, add it to nc_filelist array, add and init
         * NC_FILE_INFO_T. */
        if (new_NC(NC3_dispatch_table, FILE_NAME, 0, &ncp)) ERR;
        add_to_NCList(ncp);
        if (nc4_file_list_add(ncp->ext_ncid, FILE_NAME, 0, NULL)) ERR;
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, NULL, &grp, NULL)) ERR;

        /* Add a dim. */
        if (nc4_dim_list_add(grp, DIM_NAME, DIM_LEN, 0, &dim)) ERR;

        /* Find the dim. */
        if (nc4_find_dim(grp, 0, &dim_in, &dim_grp)) ERR;
        if (strcmp(dim_in->hdr.name, dim->hdr.name)) ERR;
        if (strcmp(dim_grp->hdr.name, grp->hdr.name)) ERR;
        dim_in = NULL;
        if (nc4_find_dim(grp, 0, &dim_in, NULL)) ERR;
        if (strcmp(dim_in->hdr.name, dim->hdr.name)) ERR;

        /* Release resources. */
        if (nc4_file_list_del(ncp->ext_ncid)) ERR;
        del_from_NCList(ncp);
        free_NC(ncp);
    }
    SUMMARIZE_ERR;
    printf("Testing adding new type to nc4internal file...");
    {
        NC *ncp;
        NC_GRP_INFO_T *grp;
        NC_TYPE_INFO_T *type, *type_in;
        NC_FILE_INFO_T *h5;

        /* Create the NC, add it to nc_filelist array, add and init
         * NC_FILE_INFO_T. */
        if (new_NC(NC3_dispatch_table, FILE_NAME, 0, &ncp)) ERR;
        add_to_NCList(ncp);
        if (nc4_file_list_add(ncp->ext_ncid, FILE_NAME, 0, NULL)) ERR;
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, NULL, &grp, &h5)) ERR;

        /* Add a type. */
        if (nc4_type_list_add(grp, TYPE_SIZE, TYPE_NAME, &type)) ERR;

        /* Add a field to the type. */
        /* if (nc4_field_list_add(type, FIELD_NAME, FIELD_OFFSET, NC_INT, 0, */
        /*                        NULL)) ERR; */

        /* Find it. */
        if (nc4_find_type(h5, type->hdr.id, &type_in)) ERR;
        if (strcmp(type_in->hdr.name, type->hdr.name)) ERR;

        /* Release resources. */
        if (nc4_file_list_del(ncp->ext_ncid)) ERR;
        del_from_NCList(ncp);
        free_NC(ncp);
    }
    SUMMARIZE_ERR;
    printf("Testing changing ncid...");
    {
        NC *ncp;
        NC_GRP_INFO_T *grp;
        NC_FILE_INFO_T *h5;
        int old_ncid;

        /* Create the NC, add it to nc_filelist array, add and init
         * NC_FILE_INFO_T. */
        if (new_NC(NC3_dispatch_table, FILE_NAME, 0, &ncp)) ERR;
        add_to_NCList(ncp);
        if (nc4_file_list_add(ncp->ext_ncid, FILE_NAME, 0, NULL)) ERR;
        if (nc4_find_nc_grp_h5(ncp->ext_ncid, NULL, &grp, &h5)) ERR;

        /* Change the ncid. */
        old_ncid = ncp->ext_ncid;
        if (nc4_file_change_ncid(ncp->ext_ncid, TEST_VAL_42)) ERR;

        /* Can't find old ncid. */
        if (nc4_find_nc_grp_h5(old_ncid, NULL, NULL, NULL) != NC_EBADID) ERR;

        /* Delete it. */
        if (nc4_file_list_del(ncp->ext_ncid)) ERR;
        del_from_NCList(ncp); /* Will free empty list. */
        free_NC(ncp);

        /* Ensure it is no longer in list. */
        /* if (find_in_NCList(ncp->ext_ncid)) ERR; */

    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
