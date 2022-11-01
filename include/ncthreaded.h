/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef _NCTHREADED_H_
#define _NCTHREADED_H_

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

/* Define a single check for PTHREADS vs WIN32 */
#ifdef _WIN32
/* Win32 synchronization has priority */
#undef USEPTHREADS
#else
#ifdef HAVE_PTHREADS
#define USEPTHREADS
#endif
#endif

#ifdef USEPTHREADS
#include <pthread.h>
#else /*_WIN32*/
#include <windows.h>
#include <synchapi.h>
#endif

#endif /*_NCTHREADED_H_*/
