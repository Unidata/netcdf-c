/*********************************************************************
 *   Copyright 1996-2018, UCAR/Unidata
 *   See COPYRIGHT file for copying and redistribution conditions.
 *   $Id: nc_test.c 2796 2014-10-28 03:40:29Z wkliao $
 *********************************************************************/

#include "tests.h"

#if 1		/* both CRAY MPP and OSF/1 Alpha systems need this */
#include <signal.h>
#endif /* T90 */

/*
 * Test driver for netCDF-3 interface.  This program performs tests against
 * the netCDF-3 specification for all user-level functions in an
 * implementation of the netCDF library.
 *
 * Files:
 * The read-only tests read files:
 *     test.nc (see below)
 *     tests.h (used merely as an example of a non-netCDF file)
 *
 * The write tests
 *     read test.nc (see below)
 *     write scratch.nc (deleted after each test)
 *
 * The file test.nc is created by running nc_test with the -c (create) option.
 * It is described by the following global variables.
 */

#if defined(ENABLE_THREADSAFE)
typedef struct ThreadData {
    int id;		/* (in) Thread id */ 
    int format;
    int nfailsTotal;	/* (out) total fails for this thread */
    pthread_barrier_t* barrier;
} ThreadData;

typedef struct Threadset {
    unsigned nthreads;
    pthread_t* threadset;
} Threadset;

static pthread_barrier_t barrier;

/* Ensure this stays around */
static ThreadData* threaddata = NULL;

#endif

/*
 * global variables (defined by function init_gvars) describing file test.nc
 */

/*
 * The following globals are thread-independent
 */

/*
 * The following globals are thread-dependent
 */
THREADED char dim_name[NDIMS][3];
THREADED size_t dim_len[NDIMS];
THREADED char var_name[NVARS][2+MAX_RANK];
THREADED nc_type var_type[NVARS];
THREADED int var_rank[NVARS];
THREADED int var_dimid[NVARS][MAX_RANK];
THREADED size_t var_shape[NVARS][MAX_RANK];
THREADED size_t var_nels[NVARS];
THREADED int var_natts[NVARS];
THREADED char att_name[NVARS][MAX_NATTS][2];
THREADED char gatt_name[NGATTS][3];
THREADED nc_type att_type[NVARS][NGATTS];
THREADED nc_type gatt_type[NGATTS];
THREADED size_t att_len[NVARS][MAX_NATTS];
THREADED size_t gatt_len[NGATTS];

THREADED int numGatts;  /* number of global attributes */
THREADED int numVars;   /* number of variables */
THREADED int numTypes;  /* number of netCDF data types to test */

THREADED int  nfailsTotal = 0;        /* total number of failures (per-thread) */

/*
 * command-line options
 */
int  verbose;		/* if 1, print details of tests */
int  max_nmpt;		/* max. number of messages per test */
int  nthreads;
int format;

/* Test everything for classic and 64-bit offsetfiles. If netcdf-4 is
 * included, that means another whole round of testing. */
#define NUM_FORMATS (5)

/*
 * Misc. global variables
 */
THREADED int  nfails;		/* number of failures in specific test */
THREADED char testfile[NC_MAX_NAME];
THREADED char scratch[NC_MAX_NAME];  /* writable scratch file */

#if defined(ENABLE_THREADSAFE)
#define printtest(func) print( "*** (%d) testing " #func " ... ",threaddata->id);
#else
#define printtest(func) print( "*** testing " #func " ... ");
#endif

#define NC_TEST(func) \
    printtest(func); \
    nfails = 0;\
    test_ ## func();\
    nfailsTotal += nfails;\
    if (verbose) \
	print("\n"); \
    if ( nfails == 0) \
        print( "ok\n");\
    else\
        print( "\n\t### %d FAILURES TESTING %s! ###\n", nfails, #func)

/* Create the testfile name based on format and thread id */
static void
filenames(const char* base, const char* ext, int tid)
{
    static char filename[8192];
    snprintf(filename,sizeof(testfile),"%s_%d%s",base,tid,ext);
    testfile[0] = '\0';
    strlcat(testfile,filename,sizeof(testfile));
    snprintf(filename,sizeof(filename),"scratch_%d%s",tid,ext);
    scratch[0] = '\0';
    strlcat(scratch,filename,sizeof(testfile));
    if(verbose) {
	fprintf(stderr,"thread: %d testfile=%s scratch=%s\n",tid,testfile,scratch);
    }
}

static const char*
unparseformat(int f)
{
    switch (f) {
    case NC_FORMAT_NETCDF4: return "netcdf-4";
    case NC_FORMAT_CLASSIC: return "netcdf-3";
    case NC_FORMAT_64BIT_OFFSET: return "netcdf-64";
    case NC_FORMAT_64BIT_DATA: return "cdf5";
    case NC_FORMAT_NETCDF4_CLASSIC: return "netcdf-4-classic";
    default: break;
    }
    return NULL;
}

#ifdef ENABLE_THREADSAFE
#define NC_TEST_ARG void* perthreaddata
#else
#define NC_TEST_ARG int format
#endif

void*
nc_test(NC_TEST_ARG)
{
    int i,nf;
    int formatvec[NUM_FORMATS];
    int nformats = NUM_FORMATS;
    int specific_format;

#if defined(ENABLE_THREADSAFE)
    ThreadData* threaddata = (ThreadData*)perthreaddata;
    specific_format = threaddata->format;
#else
    specific_format = format;
#endif

    for (i = 0; i < NUM_FORMATS; i++) formatvec[i] = i+1;
    if(specific_format > 0) {formatvec[0] = specific_format; nformats = 1;}

    fprintf(stderr, "Testing %d different netCDF formats:", nformats);
    for(i=0;i<nformats;i++) fprintf(stderr, " %s", unparseformat(formatvec[i]));
    fprintf(stderr,"\n");

#if defined(ENABLE_THREADSAFE)
    fprintf(stderr,"starting thread: %d\n",threaddata->id); fflush(stderr);
    pthread_barrier_wait(threaddata->barrier);
#endif

    /* Both CRAY MPP and OSF/1 Alpha systems need this.  Some of the
     * extreme test assignments in this program trigger floating point
     * exceptions on CRAY T90
     */
    (void) signal(SIGFPE, SIG_IGN);

    /* If you uncomment the nc_set_log_level line, you will get a lot
     * of debugging info. If you set the number higher, you'll get
     * more. 6 is max, 0 shows only errors. 3 is a good place to
     * start. */
    /*nc_set_log_level(3);*/

    /* Go thru formats and run all tests for each of two (for netCDF-3
     * only builds), or 3 (for netCDF-4 builds) different formats. Do
     * the netCDF-4 format last, however, because, as an additional
     * test, the ../nc_test4/tst_nc_test_file program looks at the
     * output of this program. */
    for (nf=0; nf<nformats; nf++)
    {
       i = formatvec[nf];

       numGatts = 6;
       numVars  = 136;
       numTypes = 6;

       switch (i)
       {
	  case NC_FORMAT_CLASSIC:
	     nc_set_default_format(NC_FORMAT_CLASSIC, NULL);
	     fprintf(stderr, "\n\nSwitching to netCDF classic format.\n");
	     filenames("nc_test_classic",".nc",threaddata->id);
	     break;
	  case NC_FORMAT_64BIT_OFFSET:
	     nc_set_default_format(NC_FORMAT_64BIT_OFFSET, NULL);
	     fprintf(stderr, "\n\nSwitching to 64-bit offset format.\n");
	     filenames("nc_test_64bit",".nc",threaddata->id);
	     break;

	  case NC_FORMAT_CDF5:
#ifdef ENABLE_CDF5
        nc_set_default_format(NC_FORMAT_CDF5, NULL);
	     fprintf(stderr, "\n\nSwitching to 64-bit data format.\n");
	     filenames("nc_test_cdf5",".nc",threaddata->id);
             numGatts = NGATTS;
             numVars  = NVARS;
             numTypes = NTYPES;
	     break;
#else
         continue;
#endif

         /* Repeated test. See https://github.com/Unidata/netcdf-c/issues/556 for more
            information re: the immediate 'continue' */
       case NC_FORMAT_NETCDF4: /* actually it's _CLASSIC. */
         continue; /* loop i */


       case NC_FORMAT_NETCDF4_CLASSIC:
#ifdef USE_HDF5
	     nc_set_default_format(NC_FORMAT_NETCDF4_CLASSIC, NULL);
	     filenames("nc_test_classic",".nc",threaddata->id);
	     fprintf(stderr, "\n\nSwitching to netCDF-4 format (with NC_CLASSIC_MODEL).\n");
	     break;
#else
	     continue; /* loop i */
#endif
	  default:
	     fprintf(stderr, "Unexpected format: %d!\n",i);
	     goto done;
       }

       /* Initialize global variables defining test file */
       init_gvars();

       /* Write the test file, needed for the read-only tests below. */
       write_file(testfile);

	/* delete any existing scratch netCDF file */
       (void) remove(scratch);

	/* Test read-only functions, using pre-generated test-file */
 	NC_TEST(nc_strerror);
	NC_TEST(nc_open);
	NC_TEST(nc_close);
	NC_TEST(nc_inq);
	NC_TEST(nc_inq_dimid);
	NC_TEST(nc_inq_dim);
	NC_TEST(nc_inq_dimlen);
	NC_TEST(nc_inq_dimname);
	NC_TEST(nc_inq_varid);
	NC_TEST(nc_inq_var);
	NC_TEST(nc_inq_natts);
	NC_TEST(nc_inq_ndims);
	NC_TEST(nc_inq_nvars);
	NC_TEST(nc_inq_unlimdim);
	NC_TEST(nc_inq_vardimid);
	NC_TEST(nc_inq_varname);
	NC_TEST(nc_inq_varnatts);
	NC_TEST(nc_inq_varndims);
	NC_TEST(nc_inq_vartype);
	NC_TEST(nc_get_var_text);
	NC_TEST(nc_get_var_uchar);
	NC_TEST(nc_get_var_schar);
	NC_TEST(nc_get_var_short);
	NC_TEST(nc_get_var_int);
	NC_TEST(nc_get_var_long);
	NC_TEST(nc_get_var_float);
	NC_TEST(nc_get_var_double);
	NC_TEST(nc_get_var_ushort);
	NC_TEST(nc_get_var_uint);
	NC_TEST(nc_get_var_longlong);
	NC_TEST(nc_get_var_ulonglong);
	NC_TEST(nc_get_var1_text);
	NC_TEST(nc_get_var1_uchar);
	NC_TEST(nc_get_var1_schar);
	NC_TEST(nc_get_var1_short);
	NC_TEST(nc_get_var1_int);
	NC_TEST(nc_get_var1_long);
	NC_TEST(nc_get_var1_float);
	NC_TEST(nc_get_var1_double);
	NC_TEST(nc_get_var1_ushort);
	NC_TEST(nc_get_var1_uint);
	NC_TEST(nc_get_var1_longlong);
	NC_TEST(nc_get_var1_ulonglong);
	NC_TEST(nc_get_var1);
	NC_TEST(nc_get_vara_text);
	NC_TEST(nc_get_vara_uchar);
	NC_TEST(nc_get_vara_schar);
	NC_TEST(nc_get_vara_short);
	NC_TEST(nc_get_vara_int);
	NC_TEST(nc_get_vara_long);
	NC_TEST(nc_get_vara_float);
	NC_TEST(nc_get_vara_double);
	NC_TEST(nc_get_vara_ushort);
	NC_TEST(nc_get_vara_uint);
	NC_TEST(nc_get_vara_longlong);
	NC_TEST(nc_get_vara_ulonglong);
	NC_TEST(nc_get_vara);
	NC_TEST(nc_get_vars_text);
	NC_TEST(nc_get_vars_uchar);
	NC_TEST(nc_get_vars_schar);
	NC_TEST(nc_get_vars_short);
	NC_TEST(nc_get_vars_int);
	NC_TEST(nc_get_vars_long);
	NC_TEST(nc_get_vars_float);
	NC_TEST(nc_get_vars_double);
	NC_TEST(nc_get_vars_ushort);
	NC_TEST(nc_get_vars_uint);
	NC_TEST(nc_get_vars_longlong);
	NC_TEST(nc_get_vars_ulonglong);
	NC_TEST(nc_get_vars);
	NC_TEST(nc_get_varm_text);
	NC_TEST(nc_get_varm_uchar);
	NC_TEST(nc_get_varm_schar);
	NC_TEST(nc_get_varm_short);
	NC_TEST(nc_get_varm_int);
	NC_TEST(nc_get_varm_long);
	NC_TEST(nc_get_varm_float);
	NC_TEST(nc_get_varm_double);
	NC_TEST(nc_get_varm_ushort);
	NC_TEST(nc_get_varm_uint);
	NC_TEST(nc_get_varm_longlong);
	NC_TEST(nc_get_varm_ulonglong);
	NC_TEST(nc_get_varm);
	NC_TEST(nc_get_att_text);
	NC_TEST(nc_get_att_uchar);
	NC_TEST(nc_get_att_schar);
	NC_TEST(nc_get_att_short);
	NC_TEST(nc_get_att_int);
	NC_TEST(nc_get_att_long);
	NC_TEST(nc_get_att_float);
	NC_TEST(nc_get_att_double);
	NC_TEST(nc_get_att_ushort);
	NC_TEST(nc_get_att_uint);
	NC_TEST(nc_get_att_longlong);
	NC_TEST(nc_get_att_ulonglong);
	NC_TEST(nc_get_att);
	NC_TEST(nc_inq_att);
	NC_TEST(nc_inq_attname);
	NC_TEST(nc_inq_attid);
	NC_TEST(nc_inq_attlen);
	NC_TEST(nc_inq_atttype);

	/* Test write functions */
	NC_TEST(nc_create);
	NC_TEST(nc_redef);
	/* NC_TEST(nc_enddef); *//* redundant */
	NC_TEST(nc_sync);
	NC_TEST(nc_abort);
	NC_TEST(nc_def_dim);
	NC_TEST(nc_rename_dim);
	NC_TEST(nc_def_var);
	NC_TEST(nc_put_var_text);
	NC_TEST(nc_put_var_uchar);
	NC_TEST(nc_put_var_schar);
	NC_TEST(nc_put_var_short);
	NC_TEST(nc_put_var_int);
	NC_TEST(nc_put_var_long);
	NC_TEST(nc_put_var_float);
	NC_TEST(nc_put_var_double);
	NC_TEST(nc_put_var_ushort);
	NC_TEST(nc_put_var_uint);
	NC_TEST(nc_put_var_longlong);
	NC_TEST(nc_put_var_ulonglong);
	NC_TEST(nc_put_var1_text);
	NC_TEST(nc_put_var1_uchar);
	NC_TEST(nc_put_var1_schar);
	NC_TEST(nc_put_var1_short);
	NC_TEST(nc_put_var1_int);
	NC_TEST(nc_put_var1_long);
	NC_TEST(nc_put_var1_float);
	NC_TEST(nc_put_var1_double);
	NC_TEST(nc_put_var1_ushort);
	NC_TEST(nc_put_var1_uint);
	NC_TEST(nc_put_var1_longlong);
	NC_TEST(nc_put_var1_ulonglong);
	NC_TEST(nc_put_var1);
	NC_TEST(nc_put_vara_text);
	NC_TEST(nc_put_vara_uchar);
	NC_TEST(nc_put_vara_schar);
	NC_TEST(nc_put_vara_short);
	NC_TEST(nc_put_vara_int);
	NC_TEST(nc_put_vara_long);
	NC_TEST(nc_put_vara_float);
	NC_TEST(nc_put_vara_double);
	NC_TEST(nc_put_vara_ushort);
	NC_TEST(nc_put_vara_uint);
	NC_TEST(nc_put_vara_longlong);
	NC_TEST(nc_put_vara_ulonglong);
	NC_TEST(nc_put_vara);
	NC_TEST(nc_put_vars_text);
	NC_TEST(nc_put_vars_uchar);
	NC_TEST(nc_put_vars_schar);
	NC_TEST(nc_put_vars_short);
	NC_TEST(nc_put_vars_int);
	NC_TEST(nc_put_vars_long);
	NC_TEST(nc_put_vars_float);
	NC_TEST(nc_put_vars_double);
	NC_TEST(nc_put_vars_ushort);
	NC_TEST(nc_put_vars_uint);
	NC_TEST(nc_put_vars_longlong);
	NC_TEST(nc_put_vars_ulonglong);
	NC_TEST(nc_put_vars);
	NC_TEST(nc_put_varm_text);
	NC_TEST(nc_put_varm_uchar);
	NC_TEST(nc_put_varm_schar);
	NC_TEST(nc_put_varm_short);
	NC_TEST(nc_put_varm_int);
	NC_TEST(nc_put_varm_long);
	NC_TEST(nc_put_varm_float);
	NC_TEST(nc_put_varm_double);
	NC_TEST(nc_put_varm_ushort);
	NC_TEST(nc_put_varm_uint);
	NC_TEST(nc_put_varm_longlong);
	NC_TEST(nc_put_varm_ulonglong);
	NC_TEST(nc_put_varm);
	NC_TEST(nc_rename_var);
	NC_TEST(nc_put_att_text);
	NC_TEST(nc_put_att_uchar);
	NC_TEST(nc_put_att_schar);
	NC_TEST(nc_put_att_short);
	NC_TEST(nc_put_att_int);
	NC_TEST(nc_put_att_long);
	NC_TEST(nc_put_att_float);
	NC_TEST(nc_put_att_ulonglong);
	NC_TEST(nc_put_att_ushort);
	NC_TEST(nc_put_att_uint);
	NC_TEST(nc_put_att_longlong);
	NC_TEST(nc_put_att_ulonglong);
	NC_TEST(nc_put_att);
	NC_TEST(nc_copy_att);
	NC_TEST(nc_rename_att);
	NC_TEST(nc_del_att);
	NC_TEST(nc_against_pnetcdf);
        /* keep below the last test, as it changes the default file format */
	NC_TEST(nc_set_default_format);
    }

done:
#if defined(ENABLE_THREADSAFE)
    threaddata->nfailsTotal = nfailsTotal;
    if(nfailsTotal > 0) {fprintf(stderr,"***Fail: thread=%d nfailsTotal=%d\n",threaddata->id,nfailsTotal); fflush(stderr);}
    printf("stopping thread: %d\n",threaddata->id); fflush(stdout);
    pthread_exit(NULL);
#endif

return NULL;
}     

static int
parseformat(const char* f)
{
    int format = 0;
    if(strcmp(optarg,"cdf5")==0) format = NC_FORMAT_64BIT_DATA;
    else if(strcmp(optarg,"nc4")==0) format = NC_FORMAT_NETCDF4;
    else if(strcmp(optarg,"nc3")==0) format = NC_FORMAT_CLASSIC;
    else if(strcmp(optarg,"nc3_64")==0) format = NC_FORMAT_64BIT_OFFSET;
    else if(strcmp(optarg,"nc4c")==0) format = NC_FORMAT_NETCDF4_CLASSIC;
    else if(strcmp(optarg,"netcdf-4")==0) format = NC_FORMAT_NETCDF4;
    else if(strcmp(optarg,"netcdf-3")==0) format = NC_FORMAT_CLASSIC;
    else if(strcmp(optarg,"netcdf-64")==0) format = NC_FORMAT_64BIT_OFFSET;
    else if(strcmp(optarg,"netcdf-4-classic")==0) format = NC_FORMAT_NETCDF4_CLASSIC;
    else {fprintf(stderr,"unknown format: %s; ignored\n",optarg); format = 0;}
    return format;
}

static void
usage(void)
{
   fprintf(stderr,"usage: nc_test [-h] [-v] [-m <max messages>] [-T <nthreads>] [-F nc4|nc4|nc4_64|cdf5|nc4c]\n");
   exit(1);
}

int
main(int argc, char *argv[])
{
    int c, retcode;
    int  allfailsTotal = 0;        /* total number of failures across all threads */
#if defined(ENABLE_THREADSAFE)
    int i;
    Threadset* threadset = NULL;
#endif

#ifdef USE_PNETCDF
    MPI_Init(&argc, &argv);
#endif

    nc_initialize();

    verbose = 1;
    max_nmpt = 8;
    nthreads = 0;
    format = 0;
    
    while ((c = getopt(argc, argv, "hvF:M:T:")) != EOF) {
	switch(c) {
	case 'h': usage(); break;
	case 'v': verbose = 1; break;
	case 'F':
	  format = parseformat(optarg);
	  break;
	case 'M': sscanf(optarg,"%d",&max_nmpt); break;
	case 'T': sscanf(optarg,"%d",&nthreads); break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   goto done;
	}
    }

#if defined(ENABLE_THREADSAFE)
    if(nthreads > 0) {
        threaddata = (ThreadData*)calloc(nthreads,sizeof(ThreadData));
        threadset = (Threadset*)calloc(1,sizeof(Threadset));
        threadset->nthreads = nthreads;
        threadset->threadset = (pthread_t*)calloc(threadset->nthreads,sizeof(pthread_t));
        pthread_barrier_init(&barrier, NULL, nthreads);
        for(i=0; i<nthreads; i++) {
	    threaddata[i].id = i;
    	    threaddata[i].format = format;
	    threaddata[i].barrier = &barrier;
            pthread_create(&threadset->threadset[i], NULL, nc_test, (void*)&threaddata[i]);
	}
        /* Wait for all threads to finish */
        for(i=0; i<threadset->nthreads; i++)
            pthread_join(threadset->threadset[i], NULL);
        allfailsTotal = 0;
        for(i=0; i<nthreads; i++)
	    allfailsTotal += threaddata[i].nfailsTotal;
    } else {
        allfailsTotal = nfailsTotal;
    }	
#else
    allfailsTotal = nfailsTotal;
#endif

    fprintf(stderr, "\n*** Total number of failures: %d\n", allfailsTotal);

    if (allfailsTotal)
    {
       fprintf(stderr, "*** nc_test FAILURE!!!\n");
       return 2;
    }
    else
       fprintf(stderr, "*** nc_test SUCCESS!!!\n");

#if defined(ENABLE_THREADSAFE)
    if(nthreads > 0)
        pthread_barrier_destroy(&barrier);
    if(threaddata) free(threaddata);
    if(threadset) {if(threadset->threadset) free(threadset->threadset); free(threadset);}
#endif

done:
    nc_finalize();

#ifdef USE_PNETCDF
    MPI_Finalize();
#endif

    retcode = (allfailsTotal > 0 ? 1 : 0);
    exit(retcode);
    return retcode;
}
