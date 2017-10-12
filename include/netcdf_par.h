/*! \file
 *
 * Main header file for the Parallel C API.
 *
 * Copyright 2010 University Corporation for Atmospheric
 * Research/Unidata. See COPYRIGHT file for more info.
 *
 * This header file is for the parallel I/O functions of netCDF.
 *
 */
/* "$Id: netcdf_par.h,v 1.1 2010/06/01 15:46:49 ed Exp $" */

#ifndef NETCDF_PAR_H
#define NETCDF_PAR_H 1

#include <mpi.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* Use these with nc_var_par_access(). */
#define NC_INDEPENDENT 0
#define NC_COLLECTIVE 1

/* Create a file and enable parallel I/O. */
extern int
nc_create_par(const char *path, int cmode, MPI_Comm comm, MPI_Info info,
	      int *ncidp);

/* Open a file and enable parallel I/O. */
extern int
nc_open_par(const char *path, int mode, MPI_Comm comm, MPI_Info info,
	    int *ncidp);

/* Change a variable from independent (the default) to collective
 * access. */
extern int
nc_var_par_access(int ncid, int varid, int par_access);

extern int
nc_create_par_fortran(const char *path, int cmode, int comm,
		      int info, int *ncidp);
extern int
nc_open_par_fortran(const char *path, int mode, int comm,
		    int info, int *ncidp);

/* PIO functions. */
#ifndef PIO_Offset
typedef long long int PIO_Offset;
#endif

/* Init decomposition with 0-based compmap array. */
extern int
nc_init_decomp(int iosysid, int pio_type, int ndims, const int *gdimlen, int maplen,
	       const PIO_Offset *compmap, int *ioidp, int rearranger,
	       const PIO_Offset *iostart, const PIO_Offset *iocount);

/* Read a decomposition file. */
int nc_read_decomp(int iosysid, const char *filename, int *ioidp, MPI_Comm comm,
		   int pio_type, char *title, char *history, int *fortran_order);

/* Write a decomposition file. */
int nc_write_decomp(int iosysid, const char *filename, int cmode, int ioid,
		    char *title, char *history, int fortran_order);

/* Creates an MPI intracommunicator between a set of IO tasks and one
 * or more sets of computational tasks. */
extern int
nc_init_intracomm(MPI_Comm comp_comm, int num_iotasks, int stride, int base,
		  int rearr, int *iosysidp);

/* Free a decomposition map. */
int nc_free_decomp(int iosysid, int ioid);    

/* Finalize an IO system for PIO. */
int nc_finalize2(int iosysid);

#if defined(__cplusplus)
}
#endif

#endif /* NETCDF_PAR_H */
