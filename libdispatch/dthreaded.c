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
#include <errno.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <assert.h>

/* We always use some form of pthread, even on Windows. */
#include <pthread.h>

#include "netcdf.h"
#include "netcdf_threadsafe.h"
#include "ncthreaded.h"

/* Verbose assert */
#undef DEBUGASSERT

/* Print lock/unlock */
#undef DEBUGPRINT

#ifdef DEBUGPRINT
#define DEBUGASSERT
#endif

#define MAXDEPTH 32

#ifdef ENABLE_THREADSAFE

typedef struct NCmutex {
    pthread_mutex_t mutex;
    int refcount; /* # times called by same thread */
#ifdef THREADSAFE_TRACK
    struct {
        int depth;
        const char* stack[MAXDEPTH]; /* match lock/unlock */
     } fcns;
#endif
} NCmutex;

/**************************************************/
/* Global data */

static NCmutex NC_globalmutex;

static volatile int global_mutex_initialized = 0; /* initialize once */

/**************************************************/
#ifdef THREADSAFE_TRACK

#ifdef DEBUGASSERT
static void
assertprint(int cond, const char* fcn, int lineno, const char* scond)
{
    if(!cond) {
        int i;
        fprintf(stderr,"assertion failed: %s\n",scond);
        fprintf(stderr,"\tmutex: fcn=%s line=%d (%d)", fcn,lineno,NC_globalmutex.fcns.depth);
        for(i=0;i<NC_globalmutex.fcns.depth;i++) {
            fprintf(stderr," %s",NC_globalmutex.fcns.stack[i]);
        }
        fprintf(stderr,"\n");
    }
    assert(cond);
}
#define ASSERT(x) assertprint((x),__func__,__LINE__,(#x))
#else
#define ASSERT(x) assert(x)
#endif /* DEBUGASSERT */

#ifdef DEBUGPRINT
static const char*
fcntop(void)
{
    int depth = NC_globalmutex.fcns.depth;
//    assert(depth > 0);
    if(depth == 0) return "null";
    return NC_globalmutex.fcns.stack[depth-1];
}
#endif /* DEBUGPRINT */

static void
pushfcn(const char* fcn)
{
    int depth = NC_globalmutex.fcns.depth;
    ASSERT(depth < (MAXDEPTH-1));
    NC_globalmutex.fcns.stack[depth] = fcn;
    NC_globalmutex.fcns.depth++;
}

static void
popfcn(void)
{
    ASSERT(NC_globalmutex.fcns.depth > 0);
    NC_globalmutex.fcns.depth--;
//    NC_globalmutex.fcns.stack[NC_globalmutex.fcns.depth] = NULL;
}

#endif /* THREADSAFE_TRACK */

/**************************************************/

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
    assert(sizeof(pthread_t) <= PTHREAD_T_MAX_SIZE);
    ncmutexinit(&NC_globalmutex);
}

/* Thread module init */
void
NC_global_mutex_initialize(void)
{
    if(global_mutex_initialized) return;
    memset(&NC_globalmutex,0,sizeof(NC_globalmutex));

    /* We use pthread_once to guarantee that the mutex is initialized */
    pthread_once(&once_control,globalncmutexinit);
    global_mutex_initialized = 1;
}    

/* Thread module finalize*/
void
NC_global_mutex_finalize(void)
{
    if(!global_mutex_initialized) return;
    global_mutex_initialized = 0;
    pthread_mutex_destroy(&NC_globalmutex.mutex);
}    

#ifdef THREADSAFE_TRACK
void NC_lock(const char* fcn)
#else
void NC_lock(void)
#endif
{
    NCmutex* mutex = &NC_globalmutex;
    pthread_mutex_lock(&mutex->mutex);
    mutex->refcount++;
#ifdef THREADSAFE_TRACK
    pushfcn(fcn);
#ifdef DEBUGPRINT
    fprintf(stderr,"@%s lock count=%d depth=%d\n",fcn,mutex->refcount,mutex->fcns.depth); fflush(stderr);
#endif
#endif
}

#ifdef THREADSAFE_TRACK
void NC_unlock(const char* fcn)
#else
void NC_unlock(void)
#endif
{
    NCmutex* mutex = &NC_globalmutex;
#ifdef THREADSAFE_TRACK
#ifdef DEBUGPRINT
    fprintf(stderr,"@%s unlock count=%d depth=%d\n",fcntop(),mutex->refcount,mutex->fcns.depth); fflush(stderr);
#endif
    popfcn();
#endif
    assert(mutex->refcount > 0);
    mutex->refcount--;
    pthread_mutex_unlock(&mutex->mutex);
}

#ifdef __APPLE__

/* Apparently OS/X pthreads does not implement
   pthread_barrier_t. So we have to fake it.
*/

#define PTHREAD_BARRIER_SERIAL_THREAD   1

EXTERNL int
pthread_barrier_init(pthread_barrier_t* barrier, const pthread_barrierattr_t* attr, unsigned int count)
{
    int ret = 0;

    if(count == 0) {errno = EINVAL; ret = -1; goto done;}
    if((ret=pthread_mutex_init(&barrier->mutex, 0)))
        {ret = -1; goto done;}
    if((ret=pthread_cond_init(&barrier->cond, 0))) {
        if((ret=pthread_mutex_destroy(&barrier->mutex))) goto done;
        return -1;
    }
    barrier->tripCount = count;
    barrier->count = 0;
done:
    return ret;
}

EXTERNL int
pthread_barrier_destroy(pthread_barrier_t* barrier)
{
    int ret = 0;
    if((ret=pthread_cond_destroy(&barrier->cond))) goto done;
    if((ret=pthread_mutex_destroy(&barrier->mutex))) goto done;
done:
    return ret;
}

EXTERNL int
pthread_barrier_wait(pthread_barrier_t* barrier)
{
    int ret = 0;
    if((ret=pthread_mutex_lock(&barrier->mutex))) goto done;
    barrier->count++;
    if(barrier->count >= barrier->tripCount) {
        barrier->count = 0;
        if((ret=pthread_cond_broadcast(&barrier->cond))) goto done;
        if((ret=pthread_mutex_unlock(&barrier->mutex))) goto done;
        ret = PTHREAD_BARRIER_SERIAL_THREAD;
    } else {
        if((ret=pthread_cond_wait(&barrier->cond, &(barrier->mutex))))
            goto done;
        if((ret=pthread_mutex_unlock(&barrier->mutex))) goto done;
        ret = 0;
    }
done:
    return ret;
}

#endif /*__APPLE__*/

#endif /*ENABLE_THREADSAFE*/
