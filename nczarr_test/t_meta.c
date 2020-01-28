/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ztest.h"

#define SETUP
#define DEBUG

#define FILE "testmeta.ncz"

#define DATA1 "/data1"
#define DATA1LEN 25

/* Forward */
static int testcreate(void);
static int testdim1(void);
static int testvar1(void);

struct Test tests[] = {
{"create", testcreate},
{"dim1", testdim1},
{"var1", testvar1},
{NULL, NULL}
};


int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    struct Test* test = NULL;

    makeurl(FILE);

    if((stat = setup(argc,argv))) goto done;
    if((stat = findtest(options.cmd, tests,&test))) goto done;

    /* Execute */
    test->test();

done:
    if(stat)
	nc_strerror(stat);
    return (stat ? 1 : 0);    
}

/* Create test netcdf4 file via netcdf.h API*/
static int
testcreate(void)
{
    int stat = NC_NOERR;
    int ncid;

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

/* Create file and add a dimension */
static int
testdim1(void)
{
    int stat = NC_NOERR;
    int ncid, dimid;

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    if((stat = nc_def_dim(ncid, "dim1", (size_t)1, &dimid)))
	goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

/* Create file and add a variable */
static int
testvar1(void)
{
    int stat = NC_NOERR;
    int ncid, varid;

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    if((stat = nc_def_var(ncid, "var1", NC_INT, 0, NULL, &varid)))
	goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}
