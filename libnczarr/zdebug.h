/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#ifndef ZDEBUG_H
#define ZDEBUG_H

#undef ZDEBUG /* general debug */

#define ZCATCH /* Warning: significant performance impact */
#define ZTRACING /* Warning: significant performance impact */

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
#define ZTRACE() nclog(NCLOGDBG,"trace: %s.%s",__FILE__,__func__)
#else
#define ZLOG(level,msg,...)
#define ZTRACE()
#endif

/* printers */
extern char* nczprint_slice(NCZSlice slice);
extern char* nczprint_odom(NCZOdometer odom);
extern char* nczprint_projection(NCZProjection proj);
extern char* nczprint_sliceindex(NCZSliceIndex si);

#endif /*ZDEBUG_H*/

