/** \file
Global data

This file stores all known global data for the netcdf-c library.
It provides two structure types and instances.
1. Mutable data
2. Constant data 

Both must be locked for initialization, but after that the constant data
can be accessed without locking.  Access to the mutable data must always
occur in a critical section.
*/

/*
Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.
*/

#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#include <stdio.h>
#endif
#ifdef ENABLE_THREADSAFE
#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif
#endif

#ifdef ENABLE_THREADSAFE

#ifdef HAVE_LIBPTHREAD

static pthread_mutex_t NC_globalmutex; /* globally accessible */
static pthread_once_t once_control = PTHREAD_ONCE_INIT;  /* for pthread_once */

#elif defined(_MSC_VER)

static CRITICAL_SECTION NC_globalmutex;

#endif

static int mutex_initialized = 1; /* set to zero if we failed to initialize */

#ifdef HAVE_LIBPTHREAD
static void ncmutexinit(void)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    if(pthread_mutex_init(&NC_globalmutex, &attr))
	mutex_initialized = 0;
    else
	mutex_initialized = 1;
    pthread_mutexattr_destroy(&attr);
}

#endif

int
NC_mutex_initialize(void)
{
    if(mutex_initialized) return 1;

#ifdef HAVE_LIBPTHREAD
    /* We use pthread_once to guarantee that the mutex is initialized */
    pthread_once(&once_control,ncmutexinit);
#elif defined(_MSC_VER)
    InitializeCriticalSectionAndSpinCount(&NC_globalmutex, 4000); /* is this itself thread-safe?*/
    mutex_initialized = 1;
#endif
    if(!mutex_initialized) {
	fprintf(stderr,"nc_global_init: NC_globalmutex not initialized\n");
	return 0;
    }
    return 1;
}    

void
NC_lock(void)
{
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_lock(&NC_globalmutex);
#elif defined(_MSC_VER)
    EnterCriticalSection(&NC_globalmutex);
#else
#error "no mutex available"
#endif
}

void
NC_unlock(void)
{
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&NC_globalmutex);
#elif defined(_MSC_VER)
    LeaveCriticalSection(&NC_globalmutex);
#else
#error "no mutex available"
#endif
}

#else /*!ENABLE_THREADSAFE*/

int
NC_mutex_initialize(void)
{
    return 1;
}

void
NC_lock(void)
{
}

void
NC_unlock(void)
{
}

#endif /*ENABLE_THREADSAFE*/

