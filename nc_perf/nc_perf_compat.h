/* This is part of the netCDF package.
   Copyright 2026 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Portable access to the POSIX facilities the nc_perf benchmarks use.

   Every benchmark in this directory measures elapsed or CPU time, and until
   now each one reached for <sys/time.h>, <sys/resource.h> and <unistd.h>
   directly.  Those three headers do not exist in the Microsoft toolchain, so
   19 of the 23 sources here failed to compile with MSVC -- which is why
   ENABLE_BENCHMARKS could not be turned on for a Visual Studio build at all.

   What the benchmarks actually use from them is small:

     struct timeval, gettimeofday()      everywhere
     struct rusage, getrusage()          tst_chunks3.c
     getopt(), optarg, optind            bm_file.c, tst_ar4*.c  (via unistd.h)
     strtok_r()                          bm_file.c

   Windows can supply all of them, so supply them here in one place rather
   than repeating the same guards in 19 files.  getopt comes from the bundled
   XGetopt that ncdump, ncgen and nczarr_test already use on Windows; a target
   that needs it must add ${XGETOPTSRC} to its sources, as those do.

   Include this instead of <sys/time.h> / <sys/resource.h> / <unistd.h>.

   Ed Hartnett, H. Joe Lee
*/

#ifndef NC_PERF_COMPAT_H
#define NC_PERF_COMPAT_H

#include "config.h"

#include <errno.h>

#ifdef _WIN32
/* <winsock2.h> must be reached before anything pulls in <windows.h>, which
   would otherwise include the incompatible <winsock.h> and give several
   hundred redefinition errors. XGetopt.h below includes <windows.h> without
   WIN32_LEAN_AND_MEAN, so this block has to come first. struct timeval comes
   from here too. */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>
#endif /* _WIN32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

/* getopt() reached these sources through <unistd.h>.  On Windows it comes
   from the bundled XGetopt, the same way ncdump and ncgen get it. */
#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

/* MSVC spells strtok_r as strtok_s, with the same three arguments and the
   same semantics. */
#if defined(_WIN32) && !defined(__MINGW32__)
#define strtok_r(str, delim, saveptr) strtok_s((str), (delim), (saveptr))
#endif

#if defined(_WIN32) && !defined(HAVE_SYS_TIME_H)

/** Win32 stand-in for gettimeofday(2).

    GetSystemTimeAsFileTime counts 100 ns ticks since 1601-01-01 UTC; the
    constant below is the number of those ticks between that epoch and the
    Unix one.  The benchmarks only ever subtract two of these from each
    other, so what matters is the tick rate rather than the origin -- but
    getting the origin right costs one subtraction and keeps any absolute
    value printed by a benchmark meaningful.

    The tz argument is accepted and ignored, as it is on every modern POSIX
    system.  Returns 0; there is no failure mode. */
static int
nc_perf_gettimeofday(struct timeval *tv, void *tz)
{
   static const unsigned long long EPOCH_DELTA_100NS = 116444736000000000ULL;
   FILETIME ft;
   ULARGE_INTEGER t;

   (void)tz;
   if (!tv)
      return 0;
   GetSystemTimeAsFileTime(&ft);
   t.LowPart = ft.dwLowDateTime;
   t.HighPart = ft.dwHighDateTime;
   t.QuadPart -= EPOCH_DELTA_100NS;
   tv->tv_sec = (long)(t.QuadPart / 10000000ULL);
   tv->tv_usec = (long)((t.QuadPart % 10000000ULL) / 10ULL);
   return 0;
}
#define gettimeofday(tv, tz) nc_perf_gettimeofday((tv), (tz))

#endif /* _WIN32 && !HAVE_SYS_TIME_H */

#if defined(_WIN32) && !defined(HAVE_SYS_RESOURCE_H)

#define RUSAGE_SELF 0

/** The part of struct rusage that nc_perf reads.

    tst_chunks3.c sums ru_utime and ru_stime and divides by the repetition
    count, and reads ru_inblock/ru_oublock without printing them.  Nothing
    here touches the rest of the POSIX structure. */
struct rusage {
   struct timeval ru_utime; /**< User CPU time used. */
   struct timeval ru_stime; /**< System CPU time used. */
   long ru_inblock;         /**< Block input operations; always 0 on Win32. */
   long ru_oublock;         /**< Block output operations; always 0 on Win32. */
};

/** Win32 stand-in for getrusage(2).

    GetProcessTimes reports kernel and user CPU time for the process in
    100 ns ticks, which is exactly what ru_stime and ru_utime mean, so the
    measurement stays CPU time and remains comparable with the POSIX
    platforms.

    ru_inblock and ru_oublock have no Win32 equivalent and are reported as
    zero.  Only RUSAGE_SELF is supported; anything else fails the way
    getrusage(2) does, with -1 and EINVAL. */
static int
nc_perf_getrusage(int who, struct rusage *usage)
{
   FILETIME creation, exit, kernel, user;
   ULARGE_INTEGER k, u;

   if (who != RUSAGE_SELF || !usage) {
      errno = EINVAL;
      return -1;
   }
   if (!GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)) {
      errno = EINVAL;
      return -1;
   }
   k.LowPart = kernel.dwLowDateTime;
   k.HighPart = kernel.dwHighDateTime;
   u.LowPart = user.dwLowDateTime;
   u.HighPart = user.dwHighDateTime;

   usage->ru_stime.tv_sec = (long)(k.QuadPart / 10000000ULL);
   usage->ru_stime.tv_usec = (long)((k.QuadPart % 10000000ULL) / 10ULL);
   usage->ru_utime.tv_sec = (long)(u.QuadPart / 10000000ULL);
   usage->ru_utime.tv_usec = (long)((u.QuadPart % 10000000ULL) / 10ULL);
   usage->ru_inblock = 0;
   usage->ru_oublock = 0;
   return 0;
}
#define getrusage(who, usage) nc_perf_getrusage((who), (usage))

#endif /* _WIN32 && !HAVE_SYS_RESOURCE_H */

#endif /* NC_PERF_COMPAT_H */
