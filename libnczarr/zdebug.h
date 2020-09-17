/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#ifndef ZDEBUG_H
#define ZDEBUG_H

#undef ZDEBUG /* general debug */
#undef ZDEBUG1 /* detailed debug */

#undef ZCATCH /* Warning: significant performance impact */
#undef ZTRACING /* Warning: significant performance impact */

#include "ncexternl.h"
#include "nclog.h"

#ifdef ZCATCH
/* Place breakpoint on zbreakpoint to catch errors close to where they occur*/
#define THROW(e) zthrow((e),__FILE__,__LINE__)
#define ZCHECK(e) if((e)) {THROW(stat); goto done;} else {}
EXTERNL int zbreakpoint(int err);
EXTERNL int zthrow(int err, const char* fname, int line);
#else
#define ZCHECK(e) {if((e)) {goto done;}}
#define THROW(e) (e)
#endif

#ifdef ZTRACING
#define ZLOG(level,msg,...) nclog(level,msg,__VA_ARGS__)
#define ZTRACE(fmt,...) nclog(NCLOGDBG,"trace: %s:%s " fmt,__FILE__,__func__,__VA_ARGS__)
#else
#define ZLOG(level,msg,...)
#define ZTRACE(fmt,...)
#endif

/* printers */
EXTERNL void nczprint_reclaim(void);
EXTERNL char* nczprint_slice(NCZSlice);
EXTERNL char* nczprint_slices(int rank, NCZSlice*);
EXTERNL char* nczprint_slab(int rank, NCZSlice*);
EXTERNL char* nczprint_odom(NCZOdometer*);
EXTERNL char* nczprint_chunkrange(NCZChunkRange);
EXTERNL char* nczprint_projection(NCZProjection);
EXTERNL char* nczprint_sliceprojections(NCZSliceProjections);
EXTERNL char* nczprint_allsliceprojections(int r, NCZSliceProjections* slp);
EXTERNL char* nczprint_vector(size_t,size64_t*);
EXTERNL char* nczprint_slicex(NCZSlice slice, int raw);
EXTERNL char* nczprint_slicesx(int rank, NCZSlice* slices, int raw);
EXTERNL char* nczprint_projectionx(NCZProjection proj, int raw);
EXTERNL char* nczprint_sliceprojectionsx(NCZSliceProjections slp, int raw);

#ifdef ZDEBUG
EXTERNL void zdumpcommon(struct Common*);
#endif

#ifdef HAVE_EXECINFO_H
EXTERNL void NCZbacktrace(void);
#endif

/* Define the possible unit tests (powers of 2) */
#define UTEST_RANGE	1
#define UTEST_WALK	2
#define UTEST_TRANSFER	4

struct ZUTEST {
    int tests;
    void (*print)(int sort,...);
};
EXTERNL struct ZUTEST* zutest;

#endif /*ZDEBUG_H*/

