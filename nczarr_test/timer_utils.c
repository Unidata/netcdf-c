/*********************************************************************
 *   Copyright 2020, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#include "timer_utils.h"

#undef DEBUG

static int NCT_initialized = 0;

#ifdef _WIN32
LARGE_INTEGER frequency;
LARGE_INTEGER starttime;

void
NCT_inittimer(void)
{
    if(NCT_initialized) return;
#ifdef DEBUG
    fprintf(stderr,"timer mechanism: QueryPerformanceCounter\n");
#endif
    LARGE_INTEGER li;
    (void)QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&starttime);
#ifdef DEBUG
fprintf(stderr,"frequency=%lld starttime=%lld\n",frequency.QuadPart,starttime.QuadPart);
#endif
    NCT_initialized = 1;
}
#else
void
NCT_inittimer(void)
{
    if(NCT_initialized) return;
#ifdef DEBUG
#if defined HAVE_CLOCK_GETTIME
    fprintf(stderr,"timer mechanism: clock_gettime\n");
#elif defined HAVE_GETTIMEOFDAY 
    fprintf(stderr,"timer mechanism: gettimeofday\n");
#elif defined HAVE_GETRUSAGE
    fprintf(stderr,"timer mechanism: getrusage\n");
#else
    fprintf(stderr,"timer mechanism: Unknown\n");
#endif
#endif /*DEBUG*/
    NCT_initialized = 1;
}
#endif

void
NCT_marktime(Nanotime* nt)
{
#ifdef _WIN32
    LARGE_INTEGER endtime;
    QueryPerformanceCounter(&endtime);
    nt->tv_sec = endtime.QuadPart / 1000000000;
    nt->tv_nsec = endtime.QuadPart % 1000000000;
#ifdef DEBUG
fprintf(stderr,"endtime=%lld\n",endtime.QuadPart);
#endif
#endif

#ifndef _WIN32
/* Pick one */
#ifdef HAVE_CLOCK_GETTIME
    clockid_t clk_id = CLOCK_MONOTONIC;
    struct timespec t;
    clock_gettime(clk_id,&t);
    nt->tv_sec = (long long)t.tv_sec;
    nt->tv_nsec = (long long)t.tv_nsec;
#elif defined HAVE_GETTIMEOFDAY
    struct timeval tp;
    gettimeofday(&tp, NULL);
    nt->tv_sec = (long long)tp.tv_sec;
    nt->tv_nsec = 1000*(long long)tp.tv_usec;
# elif defined HAVE_GETRUSAGE
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    nt->tv_sec = (long long)(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec);
    nt->tv_nsec = (long long)(1000*(ru.ru_utime.tv_usec + ru.ru_stime.tv_usec));
#endif
#endif
}

void
NCT_elapsedtime(Nanotime* nt0, Nanotime* nt1, Nanotime* delta)
{
    long long nsec[2];
    long long deltansec;
    
    nsec[0] = nt0->tv_nsec+(1000000000 * nt0->tv_sec);
    nsec[1] = nt1->tv_nsec+(1000000000 * nt1->tv_sec);

    deltansec = nsec[1] - nsec[0];
    delta->tv_nsec = deltansec % 1000000000;
    delta->tv_sec = deltansec / 1000000000;
#ifdef DEBUG
fprintf(stderr,"delta=(%lld,%lld)\n",delta->tv_sec,delta->tv_nsec);
#endif
}

int
NCT_reporttime(unsigned nelems, Nanotime* times, struct TimeRange range, const char* tag)
{
    Nanotime delta;
    long long nsec,avg;
    double dnsec,dsec,davg;
    
    NCT_elapsedtime(&times[0],&times[1],&delta);
    nsec = NCT_nanoseconds(delta);
    avg = nsec / nelems;
#ifdef DEBUG
fprintf(stderr,"nsec=%lld avg=%lld\n",nsec,avg);
#endif
    dnsec = (double)nsec;
    dsec = dnsec / 1000000000.0;
#ifdef DEBUG
fprintf(stderr,"dsec=%g dnsec=%g\n",dsec,dnsec);
#endif
    davg = (dnsec/nelems);
    fprintf(stderr,"\t%s:\t%8.6lf sec",tag,dsec);
    fprintf(stderr," avg=%5.2lf nsec\n",davg);
#ifdef DEBUG
     fprintf(stderr,"range: min=%lld max=%lld\n",range.min,range.max);
#endif
    if(!NCT_rangetest(avg,range)) {
        fprintf(stderr,"*** WARNING: unexpectedly large timing values%s\n",tag);
    }
    return 1;
}

long long
NCT_nanoseconds(Nanotime time)
{
    return (time.tv_sec * 1000000000) + time.tv_nsec;
}

/* Provide a time range tester */
int
NCT_rangetest(long long nsec, struct TimeRange range)
{
    if(nsec < range.min) {
	fprintf(stderr,"range: time=%lld  < min=%lld\n",nsec,range.min);
	return 0;
    }
    if(nsec > range.max) {
	fprintf(stderr,"range: time=%lld  > max=%lld\n",nsec,range.max);
	return 0;
    }
    return 1;
}
