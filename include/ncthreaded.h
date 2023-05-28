/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef _NCTHREADED_H_
#define _NCTHREADED_H_

/* Debug flags */
#undef THREADSAFE_TRACK
#define THREADSAFE_IDUNIQUE
#undef THREADSAFE_SINGLE
#define THREADSAFE_SINGLE_FAIL

#ifdef ENABLE_THREADSAFE
extern void NC_global_mutex_initialize(void);
extern void NC_global_mutex_finalize(void);
#ifdef THREADSAFE_TRACK
extern void NC_lock(const char* fcn);
extern void NC_unlock(const char* fcn);
#define NCLOCK NC_lock(__func__)
#define NCUNLOCK NC_unlock(__func__)
#else
extern void NC_lock(void);
extern void NC_unlock(void);
#define NCLOCK NC_lock()
#define NCUNLOCK NC_unlock()
#endif
#else /*!ENABLE_THREADSAFE*/
#define NCLOCK
#define NCUNLOCK
#endif

#endif /*_NCTHREADED_H_*/
