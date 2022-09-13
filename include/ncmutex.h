/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef _NCMUTEX_H_
#define _NCMUTEX_H_

#define DEBUGAPI

#ifdef ENABLE_THREADSAFE
extern void NC_global_mutex_initialize(void);
extern void NC_global_mutex_finalize(void);
#ifdef DEBUGAPI
extern void NC_lock(const char* fcn);
extern void NC_unlock(const char* fcn);
#else
extern void NC_lock(void);
extern void NC_unlock(void);
#endif
#ifdef DEBUGAPI
#define NCLOCK NC_lock(__func__)
#define NCUNLOCK NC_unlock(__func__)
#else
#define NCLOCK NC_lock()
#define NCUNLOCK NC_unlock()
#endif
#else
#define NCLOCK
#define NCUNLOCK
#endif

#ifdef HAVE_PTHREADS
#ifdef __APPLE__

#include <pthread.h>

/* Apparently OS/X pthreads does not implement
   pthread_barrier_t. So we have to fake it.
*/
#define PTHREAD_BARRIER_SERIAL_THREAD   1

typedef int pthread_barrierattr_t;

typedef struct pthread_barrier_t {
  pthread_mutex_t         mutex;
  pthread_cond_t          cond;
  volatile uint32_t       flag;
  size_t                  count;
  size_t                  num;
} pthread_barrier_t;

extern int pthread_barrier_init(pthread_barrier_t* barrier, const pthread_barrierattr_t* attr, unsigned int count);
extern int pthread_barrier_destroy(pthread_barrier_t* barrier);
extern int pthread_barrier_wait(pthread_barrier_t* barrier);

#endif /*__APPLE__*/
#endif /*HAVE_PTHREADS*/

#endif /*_NCMUTEX_H_*/


