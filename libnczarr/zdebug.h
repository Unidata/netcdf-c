/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#ifndef ZDEBUG_H
#define ZDEBUG_H

#undef ZDEBUG /* general debug */
#undef ZDEBUG1 /* detailed debug */

#define ZCATCH /* Warning: significant performance impact */
#define ZTRACING /* Warning: significant performance impact */

#include "ncexternl.h"
#include "nclog.h"

#ifdef ZCATCH
/* Place breakpoint on zbreakpoint to catch errors close to where they occur*/
#define THROW(e) zthrow((e),__FILE__, __func__, __LINE__)
#define ZCHECK(e) if((e)) {THROW(stat); goto done;} else {}
EXTERNL int zbreakpoint(int err);
EXTERNL int zthrow(int err, const char* fname, const char* fcn, int line);
#else
#define ZCHECK(e) {if((e)) {goto done;}}
#define THROW(e) (e)
#endif

#ifdef ZTRACING
#define ZTRACE(level,fmt,...) nctrace((level),__func__,fmt,##__VA_ARGS__)
#define ZTRACEMORE(level,fmt,...) nctracemore((level),fmt,##__VA_ARGS__)
#define ZUNTRACE(e) ncuntrace(__func__,(e),NULL)
#define ZUNTRACEX(e,fmt,...) ncuntrace(__func__,(e),fmt,##__VA_ARGS__)
#else
#define ZTRACE(level,fmt,...)
#define ZUNTRACE(e)
#define ZUNTRACEX(e,fmt,...)
#endif

/* printers */
EXTERNL void nczprint_reclaim(void);
EXTERNL char* nczprint_slice(NCZSlice);
EXTERNL char* nczprint_slices(int rank, const NCZSlice*);
EXTERNL char* nczprint_slab(int rank, const NCZSlice*);
EXTERNL char* nczprint_odom(const NCZOdometer*);
EXTERNL char* nczprint_chunkrange(const NCZChunkRange);
EXTERNL char* nczprint_projection(const NCZProjection);
EXTERNL char* nczprint_sliceprojections(const NCZSliceProjections);
EXTERNL char* nczprint_allsliceprojections(int r, const NCZSliceProjections* slp);
EXTERNL char* nczprint_slicex(const NCZSlice slice, int raw);
EXTERNL char* nczprint_slicesx(int rank, const NCZSlice* slices, int raw);
EXTERNL char* nczprint_projectionx(const NCZProjection proj, int raw);
EXTERNL char* nczprint_sliceprojectionsx(const NCZSliceProjections slp, int raw);
EXTERNL char* nczprint_vector(size_t,const size64_t*);
EXTERNL char* nczprint_idvector(size_t,const int*);
EXTERNL char* nczprint_sizevector(size_t,const size_t*);
EXTERNL char* nczprint_envv(const char** envv);

EXTERNL void zdumpcommon(const struct Common*);

/* Define the possible unit tests (powers of 2) */
#define UTEST_RANGE	 1
#define UTEST_WALK	 2
#define UTEST_TRANSFER	 4
#define UTEST_WHOLECHUNK 8

struct ZUTEST {
    int tests;
    void (*print)(int sort,...);
};
EXTERNL struct ZUTEST* zutest;

#endif /*ZDEBUG_H*/

