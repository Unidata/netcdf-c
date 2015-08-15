/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test fix of bug involving creation of a file with pnetcdf APIs,
   then opening and modifying the file with netcdf.

   Author: Wei-keng Liao.
*/

/*
Goal is to verify that cdf-5 code is writing data that can
be read by pnetcdf and vice-versa.
*/

/*
    Compile:
        mpicc -g -o tst_cdf5format tst_cdf5format.c -lnetcdf -lcurl -lhdf5_hl -lhdf5 -lpnetcdf -lz -lm
    Run:
        nc_pnc
*/

#include <nc_tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <netcdf.h>
#include <netcdf_par.h>
#include <assert.h>

#define NVARS 6
#define NX    5
#define FILENAME "tst_pnetcdf.nc"

static void 
report(int stat, char* file, int line)
{
    fflush(stdout); /* Make sure our stdout is synced with stderr.*/
    if(stat != 0) {
        fprintf(stderr, "Sorry! Unexpected result, %s, line: %d: status=%d\n", \
	        __FILE__, __LINE__,stat);
    }
}

#define Error(stat) report(stat,__FILE__,__LINE__)

/*
Given ncid,
write meta-data and data
*/

int
write(int ncid, int parallel)
{
    int i, j, rank, nprocs, cmode, varid[NVARS], dimid[2], *buf;
    int err = 0;
    char str[32];
    size_t start[2], count[2];
    int stat = NC_NOERR;

    /* define dimension */
    if (stat=nc_def_dim(ncid, "Y", NC_UNLIMITED, &dimid[0])) Error(stat);;
    if (stat=nc_def_dim(ncid, "X", NX,           &dimid[1])) Error(stat);;

    /* Odd numbers are fixed variables, even numbers are record variables */
    for (i=0; i<NVARS; i++) {
        if (i%2) {
            sprintf(str,"fixed_var_%d",i);
            if (nc_def_var(ncid, str, NC_INT, 1, dimid+1, &varid[i])) Error(stat);;
        }
        else {
            sprintf(str,"record_var_%d",i);
            if (stat=nc_def_var(ncid, str, NC_INT, 2, dimid, &varid[i])) Error(stat);;
        }
    }
    if (stat=nc_enddef(ncid)) Error(stat);;
<<<<<<< HEAD
=======

>>>>>>> 896fe992ff193edcd1a7f2ad592d144a91de3de5
    for (i=0; i<NVARS; i++) {
	if(parallel) {
            /* Note NC_INDEPENDENT is the default */
            if (stat=nc_var_par_access(ncid, varid[i], NC_INDEPENDENT)) Error(stat);;
	}
    }

    /* write all variables */
    buf = (int*) malloc(NX * sizeof(int));
    for (i=0; i<NVARS; i++) {
        for (j=0; j<NX; j++) buf[j] = i*10 + j;
        if (i%2) {
            start[0] = 0; count[0] = NX;
            if (stat=nc_put_vara_int(ncid, varid[i], start, count, buf)) Error(stat);;
        }
        else {
            start[0] = 0; start[1] = 0;
            count[0] = 1; count[1] = NX;
            if (stat=nc_put_vara_int(ncid, varid[i], start, count, buf)) Error(stat);;
        }
    }
    return NC_NOERR;
}

int
extend(int ncid)
{
    int i, j, rank, nprocs, cmode, varid[NVARS], dimid[2], *buf;
    int err = 0;
    char str[32];
    size_t start[2], count[2];
    int stat = NC_NOERR;

    if (stat=nc_redef(ncid)) Error(stat);;
    /* add attributes to make header grow */
    for (i=0; i<NVARS; i++) {
        sprintf(str, "annotation_for_var_%d",i);
        if (stat=nc_put_att_text(ncid, varid[i], "text_attr", strlen(str), str)) Error(stat);;
    }
    if (stat=nc_enddef(ncid)) Error(stat);;
    return NC_NOERR;
}

int
read(int ncid)
{
    int i, j, rank, nprocs, cmode, varid[NVARS], dimid[2], *buf;
    int err = 0;
    char str[32];
    size_t start[2], count[2];
    int stat = NC_NOERR;
<<<<<<< HEAD
=======

>>>>>>> 896fe992ff193edcd1a7f2ad592d144a91de3de5
    /* read variables and check their contents */
    for (i=0; i<NVARS; i++) {
        for (j=0; j<NX; j++) buf[j] = -1;
        if (i%2) {
            start[0] = 0; count[0] = NX;
            if (stat=nc_get_var_int(ncid, varid[i], buf)) Error(stat);
            for (j=0; j<NX; j++)
                if (buf[j] != i*10 + j)
                    printf("unexpected read value var i=%d buf[j=%d]=%d should be %d\n",i,j,buf[j],i*10+j);
        }
        else {
            start[0] = 0; start[1] = 0;
            count[0] = 1; count[1] = NX;
            if (stat=nc_get_vara_int(ncid, varid[i], start, count, buf)) Error(stat);
            for (j=0; j<NX; j++)
                if (buf[j] != i*10+j)
                    printf("unexpected read value var i=%d buf[j=%d]=%d should be %d\n",i,j,buf[j],i*10+j);
        }
    }
    return NC_NOERR;
}


int main(int argc, char* argv[])
{
    int rank, nprocs, ncid, cmode, stat;
/*
    int i, j, rank, nprocs, ncid, cmode, varid[NVARS], dimid[2], *buf;
    int err = 0;
    char str[32];
    size_t start[2], count[2];
*/
    MPI_Comm comm=MPI_COMM_SELF;
    MPI_Info info=MPI_INFO_NULL;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (nprocs > 1 && rank == 0)
        printf("This test program is intended to run on ONE process\n");
    if (rank > 0) goto fn_exit;


#ifdef DISABLE_PNETCDF_ALIGNMENT
    MPI_Info_create(&info);
    MPI_Info_set(info, "nc_header_align_size", "1");
    MPI_Info_set(info, "nc_var_align_size",    "1");
#endif

    /* pnetcdf->cdf5 */
    printf("\nWrite using PNETCDF; Read using CDF5\n");
    
    cmode = NC_PNETCDF | NC_CLOBBER;
    if (stat=nc_create_par(FILENAME, cmode, comm, info, &ncid)) Error(stat);
    if (stat=write(ncid,1)) Error(stat);
    if (stat=nc_close(ncid)) Error(stat);
    /* re-open the file with netCDF (parallel) and enter define mode */
    if (stat=nc_open_par(FILENAME, NC_WRITE|NC_PNETCDF, comm, info, &ncid)) Error(stat);
    if(stat=extend(ncid)) Error(stat);
    if (stat=nc_close(ncid)) Error(stat);

    cmode = NC_CDF5 | NC_NOCLOBBER;
    if (stat=nc_open(FILENAME, cmode, &ncid)) ERR_RET;
    if (stat=read(ncid)) Error(stat);
    if (stat=nc_close(ncid)) Error(stat);

    unlink(FILENAME);

    /* cdf5->pnetcdf */
    printf("\nWrite using CDF-5; Read using PNETCDF\n");
    cmode = NC_CDF5 | NC_CLOBBER;
    if (stat=nc_create(FILENAME, cmode, &ncid)) ERR_RET;
    if (stat=write(ncid,0)) Error(stat);
    if (stat=nc_close(ncid)) Error(stat);
    /* re-open the file with netCDF (parallel) and enter define mode */
    if (stat=nc_open(FILENAME, NC_WRITE|NC_CDF5, &ncid)) ERR_RET;
    if (stat=extend(ncid)) Error(stat);
    if (stat=nc_close(ncid)) Error(stat);

    cmode = NC_PNETCDF | NC_NOCLOBBER;
    if (stat=nc_open_par(FILENAME, cmode, comm, info, &ncid)) ERR_RET;
    if (stat=read(ncid)) Error(stat);
    if (stat=nc_close(ncid)) Error(stat);

    if (info != MPI_INFO_NULL) MPI_Info_free(&info);

fn_exit:
    MPI_Finalize();
    SUMMARIZE_ERR;
    FINAL_RESULTS;
    return 0;
}
