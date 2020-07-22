/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#ifndef ZDEBUG_H
#define ZDEBUG_H

/* Enable unit testing; on by default */
#define ZUT

#undef ZDEBUG /* general debug */
#undef ZDEBUG1 /* detailed debug */

#undef ZCATCH /* Warning: significant performance impact */
#undef ZTRACING /* Warning: significant performance impact */

#ifdef ZCATCH
/* Place breakpoint on zbreakpoint to catch errors close to where they occur*/
#define THROW(e) zthrow((e),__FILE__,__LINE__)
#define ZCHECK(e) if((e)) {THROW(stat); goto done;} else {}
extern int zbreakpoint(int err);
extern int zthrow(int err, const char* fname, int line);
#else
#define ZCHECK(e) {if((e)) {goto done;}}
#define THROW(e) (e)
#endif

#include "nclog.h"
#ifdef ZTRACING
#define ZLOG(level,msg,...) nclog(level,msg,__VA_ARGS__)
#define ZTRACE(fmt,...) nclog(NCLOGDBG,"trace: %s:%s " fmt,__FILE__,__func__,__VA_ARGS__)
#else
#define ZLOG(level,msg,...)
#define ZTRACE(fmt,...)
#endif

/* printers */
extern void nczprint_reclaim(void);
extern char* nczprint_slice(NCZSlice);
extern char* nczprint_slices(int rank, NCZSlice*);
extern char* nczprint_slab(int rank, NCZSlice*);
extern char* nczprint_odom(NCZOdometer*);
extern char* nczprint_chunkrange(NCZChunkRange);
extern char* nczprint_projection(NCZProjection);
extern char* nczprint_sliceprojections(NCZSliceProjections);
extern char* nczprint_allsliceprojections(int r, NCZSliceProjections* slp);
extern char* nczprint_vector(size_t,size64_t*);
extern char* nczprint_slicex(NCZSlice slice, int raw);
extern char* nczprint_slicesx(int rank, NCZSlice* slices, int raw);
extern char* nczprint_projectionx(NCZProjection proj, int raw);
extern char* nczprint_sliceprojectionsx(NCZSliceProjections slp, int raw);

#ifdef ZDEBUG
extern void zdumpcommon(struct Common*);
#endif

#ifdef HAVE_EXECINFO_H
extern void NCZbacktrace(void);
#endif

/* Define the possible unit tests (powers of 2) */
#define UTEST_RANGE	1
#define UTEST_WALK	2
#define UTEST_TRANSFER	4

#ifdef ZUT
extern struct ZUTEST {
    int tests;
    void (*print)(int sort,...);
} zutest;
#endif

#endif /*ZDEBUG_H*/

