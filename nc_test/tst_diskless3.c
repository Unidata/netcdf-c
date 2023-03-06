/* This is part of the netCDF package. Copyright 2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test small files.

   $Id: tst_small.c,v 1.15 2008/10/20 01:48:08 ed Exp $
*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include "netcdf.h"
#include "nc_tests.h"
#include "err_macros.h"

#undef DBG

/* Test extending an existing file */

#define NCFILENAME "tst_mmap3.nc"

#define ATT_NAME "Atom"
#define MAX_LEN 7

#define ONE_DIM 1
#define MAX_RECS 10

#define DIM1_NAME "Time"
#define DIM2_NAME "DataStrLen"
#define DIM3_NAME "X"
#define DIM3_LEN 30
#define VAR_NAME "Times"
#define VAR_NAME2 "var2"
#define VAR_NAME3 "var3"
#define STR_LEN 19
#define NUM_VALS 2
#define NDIMS 2
#define TITLE " OUTPUT FROM WRF V2.0.3.1 MODEL"
#define ATT_NAME2 "TITLE"
#define ATT_NAME3 "MISC"

#define NUM_VARS 2

static int status = NC_NOERR;

/* Control flags  */
static int persist, usenetcdf4, mmap, diskless, file, openfile;
static char* filename = NCFILENAME;
static int diskmode = 0;

#ifdef DBG

static int
ERR_report_local(int stat, const char* file, int line)
{
    fflush(stdout);
    fprintf(stderr, "Sorry! Unexpected result, %s, line: %d; status=(%d)%s\n",
            file,line,stat,nc_strerror(stat));
    fflush(stderr);
    return 1;
}

#undef ERRSTAT
#define ERRSTAT(stat) {err+=ERR_report_local(stat,__FILE__,__LINE__);}

static int
catch(void)
{
        fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
        err++;                                                          \
        fprintf(stderr, "Sorry! Unexpected result, %s, line: %d\n",     \
                __FILE__, __LINE__);                                    \
        fflush(stderr);                                                 \
        return 2;                                                       \
}

#undef ERR
#define ERR return catch()

#endif

static char*
smode(int mode)
{
    static char ms[8192];
    ms[0] = '\0';
    if(mode & NC_NETCDF4)
	strcat(ms,"NC_NETCDF4");
    else
	strcat(ms,"NC_NETCDF3");
    if(mode & NC_DISKLESS)
	strcat(ms,"|NC_DISKLESS");
    if(mode & NC_WRITE)
	strcat(ms,"|NC_WRITE");
    if(mode & NC_NOCLOBBER)
	strcat(ms,"|NC_NOCLOBBER");
    else
	strcat(ms,"|NC_CLOBBER");
    if(mode & NC_INMEMORY)
	strcat(ms,"|NC_INMEMORY");
    if(mode & NC_PERSIST)
	strcat(ms,"|NC_PERSIST");
    if(mode & NC_MMAP)
	strcat(ms,"|NC_MMAP");
    return ms;
}

/* Test a file with two record vars, which grow, and has attributes added. */
static int
test_two_growing_with_att(const char *testfile)
{
   int ncid, dimid, varid[NUM_VARS];
   char data[MAX_RECS], data_in;
   char att_name[NC_MAX_NAME + 1];
   size_t start[ONE_DIM], count[ONE_DIM], index[ONE_DIM], len_in;
   int v, r;

   /* Create a file with one unlimited dimensions, and one var. */
   if((status=nc_create(testfile, diskmode|NC_CLOBBER, &ncid))) ERRSTAT(status);
   if((status=nc_def_dim(ncid, DIM1_NAME, NC_UNLIMITED, &dimid))) ERRSTAT(status);
   if((status=nc_def_var(ncid, VAR_NAME, NC_CHAR, 1, &dimid, &varid[0]))) ERRSTAT(status);
   if((status=nc_def_var(ncid, VAR_NAME2, NC_CHAR, 1, &dimid, &varid[1]))) ERRSTAT(status);
   if((status=nc_enddef(ncid))) ERRSTAT(status);

   /* Create some phoney data. */
   for (data[0] = 'a', r = 1; r < MAX_RECS; r++)
      data[r] = data[r - 1] + 1;

   for (r = 0; r < MAX_RECS; r++)
   {
      count[0] = 1;
      start[0] = r;
      sprintf(att_name, "a_%d", data[r]);
      for (v = 0; v < NUM_VARS; v++)
      {
	 if((status=nc_put_vara_text(ncid, varid[v], start, count, &data[r]))) ERRSTAT(status);
	 if((status=nc_redef(ncid))) ERRSTAT(status);
	 if((status=nc_put_att_text(ncid, varid[v], att_name, 1, &data[r]))) ERRSTAT(status);
	 if((status=nc_enddef(ncid))) ERRSTAT(status);
      }

      /* verify */
      if((status=nc_inq_dimlen(ncid, 0, &len_in))) ERRSTAT(status);
      if (len_in != r + 1)
	ERR;
      index[0] = r;
      for (v = 0; v < NUM_VARS; v++)
      {
	 if((status=nc_get_var1_text(ncid, varid[v], index, &data_in))) ERRSTAT(status);
	 if (data_in != data[r])
		ERR;
      }
   } /* Next record. */
   if((status=nc_close(ncid))) ERRSTAT(status);
   return 0;
}

/* Test a diskless file with one new var (dimensioned)  */
static int
test_plus_one(const char *testfile)
{
   int i;
   int ncid, dimid, varid;
   size_t count[1];
   size_t start[1];
   int idata[DIM3_LEN];
   int ndims, nvars, natts, unlimdimid;

   /* Open file */
   if((status=nc_open(testfile, diskmode, &ncid))) ERRSTAT(status);
   if((status=nc_redef(ncid))) ERRSTAT(status);
   if((status=nc_def_dim(ncid, DIM3_NAME, DIM3_LEN, &dimid))) ERRSTAT(status);
   if((status=nc_def_var(ncid, VAR_NAME3, NC_INT, 1, &dimid, &varid))) ERRSTAT(status);
   if((status=nc_enddef(ncid))) ERRSTAT(status);

   /* Write one record of var data */
   for(i=0;i<DIM3_LEN;i++)
	idata[i] = i;
   count[0] = DIM3_LEN;
   start[0] = 0;
   if((status=nc_put_vara_int(ncid, varid, start, count, idata))) ERRSTAT(status);

   /* We're done! */
   if((status=nc_close(ncid))) ERRSTAT(status);

   /* Reopen the file and check it. */
   if((status=nc_open(testfile, diskmode & ~NC_WRITE, &ncid))) ERRSTAT(status);
   if((status=nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid))) ERRSTAT(status);
   if (ndims != 2 && nvars != 3 && natts != 0 && unlimdimid != 0) ERRSTAT(status);

   if((status=nc_inq_varid(ncid,VAR_NAME3,&varid))) ERRSTAT(status);

   memset(idata,0,sizeof(idata));
   if((status=nc_get_var_int(ncid, varid, idata))) ERRSTAT(status);
   for(i=0;i<DIM3_LEN;i++) {
	if(idata[i] != i)
	    ERR;
   }   
   if((status=nc_close(ncid))) ERRSTAT(status);
   return 0;
}

static void
parse(int argc, char** argv)
{
    int i;

    /* Set defaults */
    persist = 0;
    usenetcdf4 = 0;
    mmap = 0;
    diskless = 0;
    file = 0;
    diskmode = 0;
    openfile = 0;

    for(i=1;i<argc;i++) {
	if(strcmp(argv[i],"diskless")==0) diskless=1;
	else if(strcmp(argv[i],"mmap")==0) mmap=1;
	else if(strcmp(argv[i],"file")==0) file=1;
	else if(strcmp(argv[i],"persist")==0) persist=1;
	else if(strcmp(argv[i],"open")==0) openfile=1;
	else if(strcmp(argv[i],"create")==0) openfile=0;
	else if(strncmp(argv[i],"file:",strlen("file:"))==0) {
	    filename = argv[i];
	    filename += strlen("file:");
	}
	/* ignore anything not recognized */
    }

    if(diskless && mmap) {
	fprintf(stderr,"NC_DISKLESS and NC_MMAP are mutually exclusive\n");
	exit(1);
    }

}

static void
buildmode(void)
{
    diskmode = 0;
    if(diskless)
        diskmode |= NC_DISKLESS;
    if(mmap)
        diskmode |= NC_MMAP;
    if(persist)
        diskmode |= NC_PERSIST;
    if(openfile)
        diskmode |= NC_NOCLOBBER|NC_WRITE;
}

int
main(int argc, char **argv)
{
    parse(argc,argv);

    if(!diskless && !mmap && !file) {
	fprintf(stderr,"file or diskless or mmap must be specified\n");
	exit(1);
    }

    buildmode();

    printf("\n*** Testing %s: file=%s mode=%s\n",
	(openfile?"open+modify":"create"),filename,smode(diskmode));
    fflush(stdout);

    /* case NC_FORMAT_CLASSIC: only test this format */
    nc_set_default_format(NC_FORMAT_CLASSIC, NULL);

    if(openfile) {
        printf("*** testing diskless file with new variable...");
#ifdef DBG
	openfile = 0;
	buildmode();
        test_two_growing_with_att(filename);
	openfile = 1;
	buildmode();
#endif
	test_plus_one(filename);
    } else {/* create */
        printf("*** testing diskless file with two growing record "
	     "variables, with attributes added...");
        test_two_growing_with_att(filename);
    }
    SUMMARIZE_ERR;

   FINAL_RESULTS;
}
