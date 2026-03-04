/* This is part of the netCDF package.
   Copyright 2024 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Unit test for NC_hashmap alloc/free cycle.
   Regression test for https://github.com/Unidata/netcdf-c/issues/2665

   This test compiles nchashmap.c directly so it does not depend on
   internal symbols being exported from netcdf.dll on Windows.
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nchashmap.h"

#define ERR do { fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); nerrs++; } while(0)
#define SUMMARIZE_ERR do { \
    if (nerrs) fprintf(stderr, "FAILED: %d errors\n", nerrs); \
    else printf("ok\n"); \
} while(0)

static int nerrs = 0;

#define HM_NENTRIES 200
#define HM_NROUNDS  100

int
main(void)
{
    NC_hashmap *hm;
    char key[64];
    uintptr_t val;
    int i, round;

    printf("\n*** Testing NC_hashmapnew()/NC_hashmapfree() cycle (issue #2665)...");

    for (round = 0; round < HM_NROUNDS; round++)
    {
        hm = NC_hashmapnew(0);
        if (!hm) ERR;

        for (i = 0; i < HM_NENTRIES; i++)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (!NC_hashmapadd(hm, (uintptr_t)i, key, strlen(key))) ERR;
        }

        if (NC_hashmapcount(hm) != HM_NENTRIES) ERR;

        for (i = 0; i < HM_NENTRIES; i++)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (!NC_hashmapget(hm, key, strlen(key), &val)) ERR;
            if (val != (uintptr_t)i) ERR;
        }

        for (i = 0; i < HM_NENTRIES; i += 2)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (!NC_hashmapremove(hm, key, strlen(key), &val)) ERR;
            if (val != (uintptr_t)i) ERR;
        }

        if (NC_hashmapcount(hm) != HM_NENTRIES / 2) ERR;

        for (i = 0; i < HM_NENTRIES; i += 2)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (NC_hashmapget(hm, key, strlen(key), &val)) ERR;
        }

        for (i = 1; i < HM_NENTRIES; i += 2)
        {
            snprintf(key, sizeof(key), "key_%04d", i);
            if (!NC_hashmapget(hm, key, strlen(key), &val)) ERR;
            if (val != (uintptr_t)i) ERR;
        }

        NC_hashmapfree(hm);
    }

    hm = NC_hashmapnew(4);
    if (!hm) ERR;
    for (i = 0; i < HM_NENTRIES; i++)
    {
        snprintf(key, sizeof(key), "key_%04d", i);
        if (!NC_hashmapadd(hm, (uintptr_t)i, key, strlen(key))) ERR;
    }
    if (NC_hashmapcount(hm) != HM_NENTRIES) ERR;
    NC_hashmapfree(hm);

    SUMMARIZE_ERR;
    return nerrs ? 1 : 0;
}
