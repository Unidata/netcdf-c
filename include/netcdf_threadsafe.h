/*! \file netcdf_mem.h
 *
 * Main header file for thread-safe functionality.
 * This is mainly for testing, but users are welcome
 * to use it for their programs if they find it useful.
 *
 * Copyright 2018 University Corporation for Atmospheric
 * Research/Unidata. See COPYRIGHT file for more info.
 *
 * See \ref copyright file for more info.
 *
 */

/*
 * In order to use any of the netcdf_XXX.h files, it is necessary
 * to include netcdf.h followed by any netcdf_XXX.h files.
 * Various things (like EXTERNL) are defined in netcdf.h
 * to make them available for use by the netcdf_XXX.h files.
*/

#ifndef NETCDF_THREADSAFE_H
#define NETCDF_THREADSAFE_H 1

/* Types */
typedef void NC_barrier_t; /* opaque */
typedef void NC_threadset_t; /* opaque */
typedef uintptr_t (*NC_threadprog)(void* arg);

#if defined(__cplusplus)
extern "C" {
#endif

/* Create a threadset */
EXTERNL int NC_threadset_create(unsigned nthreads, NC_threadset_t** threadsetp);
EXTERNL int NC_threadset_destroy(NC_threadset_t* threadset);
EXTERNL int NC_threadset_join(NC_threadset_t* threadset);

/* Create a thread */
EXTERNL int NC_thread_create(NC_threadprog fcn, void* arg, NC_threadset_t* threadset, unsigned threadpos);
EXTERNL int NC_thread_exit(void);

/* Create/destroy a barrier */
EXTERNL int NC_barrier_create(unsigned count, NC_barrier_t** barrierp);
EXTERNL int NC_barrier_destroy(NC_barrier_t* barrier);
EXTERNL int NC_barrier_wait(NC_barrier_t* barrier);

/* Debug */
EXTERNL void NC_threadsafe_trace(void);

#if defined(__cplusplus)
}
#endif

#endif /* NETCDF_THREADSAFE_H */
