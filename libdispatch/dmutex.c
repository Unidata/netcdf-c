/** \file
Global data

This file wraps the handling of mutual exclusion structures (mutexes).

There exists a single, global mutex to control access to data
that is shared across more than one dispatch table.
There also is support for a per-dispatch table mutex.
The later allows for individual dispatchers to implement
finer-grain locking as opposed to locking the whole API.
*/

/*********************************************************************
*    Copyright 2019, UCAR/Unidata
*    See netcdf/COPYRIGHT file for copying and redistribution conditions.
* ********************************************************************/

#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#include <stdio.h>
#endif

#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif
#include <ncmutex.h>

#if !defined(HAVE_LIBPTHREAD) && !defined(_WIN32)
#error "Cannot find thread support library"
#endif

typedef struct NCmutex {
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_t mutex;
#endif
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#endif
} NCmutex;

static volatile NCmutex NC_globalmutex; /* globally accessible */

static volatile int global_mutex_initialized = 0; /* initialize once */

#ifdef HAVE_LIBPTHREAD
static pthread_once_t once_control = PTHREAD_ONCE_INIT;  /* for pthread_once */
#endif

#ifdef HAVE_LIBPTHREAD
static void
ncmutexinit(NCmutex* mutex)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
#if 0
    if(pthread_mutex_init(&mutex->mutex, &attr))
	mutex_initialized = 0;
    else
	mutex_initialized = 1;
#endif
    pthread_mutexattr_destroy(&attr);
}

/* Global mutex init; for use with once */
static void
globalncmutexinit(void)
{
    ncmutextinit(&NC_globalmutex);
}

#endif /*HAVE_LIBPTHREAD*/

/* Thread module init */
int
NC_global_mutex_initialize(void)
{
    if(global_mutex_initialized) return 1;

#ifdef HAVE_LIBPTHREAD
    /* We use pthread_once to guarantee that the mutex is initialized */
    pthread_once(&once_control,globalncmutexinit);
#elif defined(_MSC_VER)
    InitializeCriticalSectionAndSpinCount(&NC_globalmutex, 4000); /* is this itself thread-safe?*/
#endif
    mutex_initialized = 1;
    return mutex_initialized;
}    

void
NC_lock(void* mutex0)
{
    NCmutext* mutex = mutex0;
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_lock(&mutex->mutex);
#endif
#ifdef _WIN32
    EnterCriticalSection(&mutex->mutex);
#endif
}

void
NC_unlock(void* mutex0)
{
    NCmutext* mutex = mutex0;
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&mutex->mutex);
#endif
#ifdef _WIN32
    LeaveCriticalSection(&mutex->mutex);
#endif
}
