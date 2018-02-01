/*! \file
 *
 * Main header file for the Parallel C API.
 *
 * Copyright 2010 University Corporation for Atmospheric
 * Research/Unidata. See COPYRIGHT file for more info.
 *
 * This header file is for the parallel I/O functions of netCDF.
 *
 * \author Ed Hartnett
 */

#ifndef NETCDF_PAR_H
#define NETCDF_PAR_H 1

#include <mpi.h>


/** Use with nc_var_par_access() to set parallel access to independent. */
#define NC_INDEPENDENT 0

/** Use with nc_var_par_access() to set parallel access to collective. */
#define NC_COLLECTIVE 1

/** Pass this to PIOc_set_iosystem_error_handling() as the iosysid in
 * order to set default error handling. */
#define NC_PIO_DEFAULT (-1)

/**
 * These are the supported error handlers for PIO.
 */
enum NC_PIO_ERROR_HANDLERS
{
   /** Errors cause abort. */
   NC_PIO_INTERNAL_ERROR = (-51),

   /** Errors are returned to caller with no internal action. */
   NC_PIO_RETURN_ERROR = (-53)
};

/**
 * These are the supported output data rearrangement methods.
 */
enum NC_PIO_REARRANGERS
{
   /** Box rearranger. */
   NC_PIO_REARR_BOX = 1,

   /** Subset rearranger. */
   NC_PIO_REARR_SUBSET = 2
};

#if defined(__cplusplus)
extern "C" {
#endif
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

   /* Creates an MPI intracommunicator between a set of IO tasks and
    * one or more sets of computational tasks. */
   extern int
   nc_init_intracomm(MPI_Comm comp_comm, int num_iotasks, int stride, int base,
                     int rearr, int *iosysidp);

   extern int
   nc_init_async(MPI_Comm world, int num_io_procs, int *io_proc_list,
                 int component_count, int *num_procs_per_comp, int **proc_list,
                 MPI_Comm *user_io_comm, MPI_Comm *user_comp_comm, int rearranger,
                 int *iosysidp);
   
   /* These are functions for PIO. They only function if library was
    * built with --enable-pio. */
   extern int
   nc_get_iosysid(int *iosysidp);
   
   extern int
   nc_set_iosysid(int iosysid);
   
   extern int
   nc_set_iosystem_error_handling(int iosysid, int method, int *old_method);
   
   extern int
   nc_advanceframe(int ncid, int varid);

   extern int
   nc_setframe(int ncid, int varid, int frame);

   extern int
   nc_write_darray(int ncid, int varid, int ioid, PIO_Offset arraylen, void *array,
                   void *fillvalue);

   extern int
   nc_read_darray(int ncid, int varid, int ioid, PIO_Offset arraylen, void *array);

   /* Free a decomposition map. */
   int nc_free_decomp(int iosysid, int ioid);

   /* Finalize an IO system for PIO. */
   int nc_free_iosystem(int iosysid);

#if defined(__cplusplus)
}
#endif

#endif /* NETCDF_PAR_H */
