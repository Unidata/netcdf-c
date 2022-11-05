/*! \file netcdf_threadsafe.h
 *
 * Main header file for thread safe functionality.
 *
 * Copyright 2018 University Corporation for Atmospheric
 * Research/Unidata. See COPYRIGHT file for more info.
 *
 * See \ref copyright file for more info.
 *
 */

#ifndef NETCDF_THREADSAFE_H
#define NETCDF_THREADSAFE_H 1

#include <pthhead.h>

#ifdef __APPLE__

/* Apparently OS/X pthreads does not implement
   pthread_barrier_t. So we have to fake it.
*/
typedef int pthread_barrierattr_t;
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int tripCount;
} pthread_barrier_t;

EXTERNL int pthread_barrier_init(pthread_barrier_t* barrier, const pthread_barrierattr_t* attr, unsigned int count);
EXTERNL int pthread_barrier_destroy(pthread_barrier_t* barrier);
EXTERNL int pthread_barrier_wait(pthread_barrier_t* barrier);

#endif /*__APPLE__*/

#if defined(__cplusplus)
}
#endif

#endif /* NETCDF_THREADSAFE_H */
