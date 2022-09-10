/*********************************************************************
*    Copyright 2022, UCAR/Unidata
*    See netcdf/COPYRIGHT file for copying and redistribution conditions.
* ********************************************************************/

/** \file
Global data

This file wraps the handling of mutual exclusion structures (mutexes).

There exists a single, global mutex to wrap all externally visible API
calls.  An important requirement is that once you call the API, you
must not recursively call another API call because that leads to
deadlock.  Rather, you must only call dispatch table functions.  Note
that some API calls need not be protected by a mutex because they just
call other API call.
*/

#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <assert.h>

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif
#include <ncmutex.h>

/* Print lock/unlock */
#define DEBUGPRINT

typedef struct NCmutex {
#ifdef DEBUGAPI
    const char* fcn; /* match lock/unlock */
#endif
    int refcount; /* # times called by same thread */
#ifdef HAVE_PTHREADS
    pthread_mutex_t mutex;
#endif
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#endif
} NCmutex;

static NCmutex NC_globalmutex;

static volatile int global_mutex_initialized = 0; /* initialize once */

#ifdef HAVE_PTHREADS

static pthread_once_t once_control = PTHREAD_ONCE_INIT;  /* for pthread_once */

static void
ncmutexinit(NCmutex* mutex)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

/* Global mutex init; for use with once */
static void
globalncmutexinit(void)
{
    ncmutexinit(&NC_globalmutex);
}
#endif /*HAVE_PTHREADS*/

/* Thread module init */
void
NC_global_mutex_initialize(void)
{
    if(global_mutex_initialized) return;
    memset(&NC_globalmutex,0,sizeof(NC_globalmutex));

#ifdef WIN32
    InitializeCriticalSectionAndSpinCount(&NC_globalmutex.mutex, 4000); /* is this itself thread-safe?*/
#endif
#ifdef HAVE_PTHREADS
    /* We use pthread_once to guarantee that the mutex is initialized */
    pthread_once(&once_control,globalncmutexinit);
#endif
    global_mutex_initialized = 1;
}    

/* Thread module finalize*/
void
NC_global_mutex_finalize(void)
{
    if(!global_mutex_initialized) return;
    global_mutex_initialized = 0;
}    

#ifdef DEBUGAPI
void NC_lock(const char* fcn)
#else
void NC_lock(void)
#endif
{
    NCmutex* mutex = &NC_globalmutex;
#ifdef _WIN32
    EnterCriticalSection(&mutex->mutex);
#endif
#ifdef HAVE_PTHREADS
    pthread_mutex_lock(&mutex->mutex);
#endif
    mutex->refcount++;
#ifdef DEBUGAPI
    mutex->fcn = fcn;
#ifdef DEBUGPRINT
    fprintf(stderr,"@%s lock count=%d\n",mutex->fcn,mutex->refcount); fflush(stderr);
#endif
#endif
}

#ifdef DEBUGAPI
void NC_unlock(const char* fcn)
#else
void NC_unlock(void)
#endif
{
    NCmutex* mutex = &NC_globalmutex;
#ifdef DEBUGAPI
#ifdef DEBUGPRINT
    fprintf(stderr,"@%s unlock count=%d\n",mutex->fcn,mutex->refcount); fflush(stderr);
#endif
    mutex->fcn = NULL;
#endif
    assert(mutex->refcount > 0);
    mutex->refcount--;
#ifdef _WIN32
    LeaveCriticalSection(&mutex->mutex);
#endif
#ifdef HAVE_PTHREADS
    pthread_mutex_unlock(&mutex->mutex);
#endif
}
