/*********************************************************************
 *   Copyright 2020, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef TIMER_UTILS_H
#define TIMER_UTILS_H 1

#include "netcdf.h"

/* Wrap time interval computations */
typedef struct Nanotime {
    long long tv_sec; /* seconds */
    long long tv_nsec; /* nanoseconds */
} Nanotime;


struct TimeRange {
    long long min;
    long long max;
};

extern void NCT_inittimer(void);
extern void NCT_marktime(Nanotime* nt);
extern void NCT_elapsedtime(Nanotime* nt0, Nanotime* nt1, Nanotime* delta);
extern int NCT_reporttime(unsigned count, Nanotime* times, struct TimeRange, const char* tag);
extern long long NCT_nanoseconds(Nanotime time);
extern int NCT_rangetest(long long nsecs, struct TimeRange range);
#endif /*TIMER_UTILS_H*/

