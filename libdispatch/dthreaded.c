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

/* Define a single check for PTHREADS vs WIN32 */
#ifdef _WIN32
/* Win32 syncronization has priority */
#undef USEPTHREADS
#else
#ifdef HAVE_PTHREADS
#define USEPTHREADS
#endif
#endif

#include <netcdf.h>
#include <netcdf_threadsafe.h>
#include <ncthreaded.h>

/* Unclear if we should CreateThread or _beginthreadex */
#define BEGINTHREADEX

/* Verbose assert */
#undef DEBUGASSERT

/* Print lock/unlock */
#undef DEBUGPRINT

#ifdef DEBUGPRINT
#define DEBUGASSERT
#endif

#define MAXDEPTH 32

#ifdef USEPTHREADS
#define RETURNTYPE void*
#define WINAPI
#else
#define RETURNTYPE DWORD
#endif

/* Types */

typedef struct NCmutex {
#ifdef USEPTHREADS
    pthread_mutex_t mutex;
#else
    CRITICAL_SECTION mutex;
#endif
    int refcount; /* # times called by same thread */
#ifdef DEBUGAPI
    struct {
        int depth;
        const char* stack[MAXDEPTH]; /* match lock/unlock */
     } fcns;
#endif
} NCmutex;

typedef struct NCthreadset {
    unsigned nthreads;
#ifdef USEPTHREADS
    pthread_t* threadset;
#else
#ifdef BEGINTHREADEX
    unintptr_t* threadset;
#else
    HANDLE* threads;
#endif
#endif
} NCthreadset;

typedef struct NCbarrier {
#ifdef USEPTHREADS
    pthread_barrier_t barrier;
#else
    SYNCHRONIZATION_BARRIER barrier;
#endif
    unsigned int count;
} NCbarrier;

#ifdef USEPTHREADS
#ifdef __APPLE__
/* Apparently OS/X pthreads does not implement
   pthread_barrier_t. So we have to fake it.
*/
#define PTHREAD_BARRIER_SERIAL_THREAD   1
typedef int pthread_barrierattr_t;
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int tripCount;
} pthread_barrier_t;

extern int pthread_barrier_init(pthread_barrier_t* barrier, const pthread_barrierattr_t* attr, unsigned int count);
extern int pthread_barrier_destroy(pthread_barrier_t* barrier);
extern int pthread_barrier_wait(pthread_barrier_t* barrier);

#endif /*__APPLE__*/
#endif /*USEPTHREADS*/

/**************************************************/
/* Global data */

static NCmutex NC_globalmutex;

static volatile int global_mutex_initialized = 0; /* initialize once */

/**************************************************/
#ifdef DEBUGAPI

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

#endif /* DEBUGAPI */

/**************************************************/
#ifdef USEPTHREADS

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
#endif /*USEPTHREADS*/

/* Thread module init */
void
NC_global_mutex_initialize(void)
{
    if(global_mutex_initialized) return;
    memset(&NC_globalmutex,0,sizeof(NC_globalmutex));

#ifdef USEPTHREADS
    /* We use pthread_once to guarantee that the mutex is initialized */
    pthread_once(&once_control,globalncmutexinit);
#else
    InitializeCriticalSection(&NC_globalmutex.mutex); /* is this itself thread-safe?*/
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
#ifdef USEPTHREADS
    pthread_mutex_lock(&mutex->mutex);
#else
    EnterCriticalSection(&mutex->mutex);
#endif
    mutex->refcount++;
#ifdef DEBUGAPI
    pushfcn(fcn);
#ifdef DEBUGPRINT
    fprintf(stderr,"@%s lock count=%d depth=%d\n",fcn,mutex->refcount,mutex->fcns.depth); fflush(stderr);
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
    fprintf(stderr,"@%s unlock count=%d depth=%d\n",fcntop(),mutex->refcount,mutex->fcns.depth); fflush(stderr);
#endif
    popfcn();
#endif
    ASSERT(mutex->refcount > 0);
    mutex->refcount--;
#ifdef USEPTHREADS
    pthread_mutex_unlock(&mutex->mutex);
#else
    LeaveCriticalSection(&mutex->mutex);
#endif
}

/**************************************************/
/* Provide Barrier and Thread support primarily for testing. */

#ifdef USEPTHREADS /* Pthreads Version */

/**************************************************/
/* Struct reclaim functions */

static void
freethreadset(NCthreadset* ncts)
{
    if(ncts != NULL) {
	if(ncts->threadset != NULL) free(ncts->threadset);
	free(ncts);
    }
}

static void
freebarrier(NCbarrier* ncb)
{
    if(ncb != NULL) {
	free(ncb);
    }
}

/**************************************************/
/* Create/Destroy a threadset */

int
NC_threadset_create(unsigned nthreads, NC_threadset_t** threadsetp)
{
    int stat = NC_NOERR;
    NCthreadset* ncts = NULL;
    if(threadsetp == NULL) {stat = NC_EINVAL; goto done;}
    if((ncts = (NCthreadset*)calloc(1,sizeof(NCthreadset)))==NULL)
        {stat = NC_ENOMEM; goto done;}
    ncts->nthreads = nthreads;
    if(nthreads > 0) {
#ifdef USEPTHREADS
	ncts->threadset = (pthread_t*)calloc(nthreads,sizeof(pthread_t));
#else
#ifdef BEGINTHREADEX
	ncts->threadset = (uintptr_t*)calloc(nthreads,sizeof(uintptr_t));
#else
	ncts->threadset = (HANDLE*)calloc(nthreads,sizeof(HANDLE));
#endif
#endif
	if(ncts->threadset == NULL)
	    {stat = NC_ENOMEM; goto done;}
    }
    *threadsetp = (void*)ncts; ncts = NULL;
done:
    if(ncts) freethreadset(ncts);
    return stat;
}

int
NC_threadset_destroy(NC_threadset_t* threadset)
{
    int i,stat = NC_NOERR;
    NCthreadset* ncts = (NCthreadset*)threadset;
    if(ncts == NULL) {stat = NC_EINVAL; goto done;}
    for(i=0;i<ncts->nthreads;i++) {
#ifdef USEPTHREADS
	/* There is no pthread_destroy */
#else
        if(!endthreadex(ncts->threads[i]))
            {stat = NC_EBADID; goto done;}        
#endif
    }
done:
    if(ncts) freethreadset(ncts);
    return stat;
}

int
NC_threadset_join(NC_threadset_t* threadset)
{
    int i,stat = NC_NOERR;
    NCthreadset* ncts = (NCthreadset*)threadset;
    if(ncts == NULL) {stat = NC_EINVAL; goto done;}
    for(i=0;i<ncts->nthreads;i++) {
#ifdef USEPTHREADS
        if(pthread_join(ncts->threadset[i],NULL) < 0)
            {stat = errno; goto done;}
#else
        if(!WaitForSingleObject(ncts->nthreads[i], INFINITE))
            {stat = NC_EBADID; goto done;}
#endif
    }
done:
    return stat;
}

/**************************************************/

int
NC_thread_create(NC_start_routine fcn, void* arg, NC_threadset_t* threadset, unsigned pos)
{
    int stat = NC_NOERR;
    NCthreadset* ncts = (NCthreadset*)threadset;
    uintptr_t id;
    if(threadset == NULL || fcn == NULL || pos < 0 || pos >= ncts->nthreads)
        {stat = NC_EINVAL; goto done;}
    id = (uintptr_t)ncts->threadset[pos];
    if(id != 0) {stat = NC_EBADID; goto done;}
#ifdef USEPTHREADS
    if(pthread_create(&ncts->threadset[pos], NULL, fcn, arg) < 0)
        {stat = errno; goto done;}
#else
#ifdef BEGINTHREADEX
    id = _beginthreadex( 
            NULL,	// default security attributes
            0,		// use default stack size  
            threadprog,	// thread function name
            arg,	// argument to thread function 
            0,		// use default creation flags 
            NULL	// returns the thread identifier 
	);
    if(id < 0) {stat = errno; goto done;}
    nct->threadset[pos] = id;
#else
    ncts->threadset[pos] = CreateThread( 
            NULL,	// default security attributes
            0,		// use default stack size  
            threadprog,	// thread function name
            arg,	// argument to thread function 
            0,		// use default creation flags 
            NULL	// returns the thread identifier 
	);
    if(nct->threadset[pos] == NULL) {stat = NC_EINVAL; goto done;}
#endif
#endif
done:
    return stat;
}

int
NC_barrier_create(unsigned count, NC_barrier_t** barrierp)
{
    int stat = NC_NOERR;
    NCbarrier* ncb = NULL;
    if(barrierp == NULL) {stat = NC_EINVAL; goto done;}
    if((ncb = (NCbarrier*)calloc(1,sizeof(NCbarrier)))==NULL)
        {stat = NC_ENOMEM; goto done;}
    ncb->count = count;
#ifdef USEPTHREADS
    if(pthread_barrier_init(&ncb->barrier,NULL,count) < 0)
        {stat = errno; goto done;}
#else
    if(!InitializeSynchronizationBarrier(&ncb->barrier,(LONG)count,(LONG)-1))
        {stat = NC_EBADID; goto done;}
#endif
    *barrierp = (void*)ncb; ncb = NULL;
done:
    if(ncb) free(ncb);    
    return stat;
}

int
NC_barrier_destroy(NC_barrier_t* barrier)
{
    int stat = NC_NOERR;
    NCbarrier* ncb = (NCbarrier*)barrier;
    if(ncb == NULL) {stat = NC_EINVAL; goto done;}
#ifdef USEPTHREADS
    if(pthread_barrier_destroy(&ncb->barrier) < 0)
	{stat = errno; goto done;}
#else
    if(!DeleteSynchronizationBarrier(ncts->barrier))
	{stat = NC_EBADID; goto done;}
#endif
done:
    if(ncb) freebarrier(ncb);
    return stat;
}

int
NC_barrier_wait(NC_barrier_t* barrier)
{
    int stat = NC_NOERR;
    NCbarrier* ncb = (NCbarrier*)barrier;
    if(ncb == NULL) {stat = NC_EINVAL; goto done;}
#ifdef USEPTHREADS
    if(pthread_barrier_wait(&ncb->barrier) < 0)
	{stat = errno; goto done;}
#else
    if(!EnterSynchronizationBarrier(ncb->barrier,SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY))
	{stat = NC_EBADID; goto done;}
#endif
done:
    return stat;
}

#endif

/**************************************************/
#ifdef USEPTHREADS
#ifdef __APPLE__

/* Apparently OS/X pthreads does not implement
   pthread_barrier_t. So we have to fake it.
*/

#include <errno.h>

int
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

int
pthread_barrier_destroy(pthread_barrier_t* barrier)
{
    int ret = 0;
    if((ret=pthread_cond_destroy(&barrier->cond))) goto done;
    if((ret=pthread_mutex_destroy(&barrier->mutex))) goto done;
done:
    return ret;
}

int
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
#endif /*USEPTHREADS*/
