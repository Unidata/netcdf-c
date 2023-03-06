/* This is part of the netCDF package. Copyright 2005-2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.
*/

#ifndef BM_TIMER_H
#define BM_TIMER_H 1

#undef NOREPEAT
#undef NOCONTIG

/*
 * The following timing macros can be used by including the necessary
 * declarations with
 *
 *     TIMING_DECLS(seconds)
 *
 * and surrounding sections of code to be timed with the "statements"
 *
 *     TIMING_START
 *     [code to be timed goes here]
 *     TIMING_END(seconds)
 *
 * The macros assume the user has stored a description of what is
 * being timed in a 100-char string time_mess, and has included
 * <sys/times.h> and <sys/resource.h>.  The timing message printed by
 * TIMING_END is not terminated by a new-line, to permit appending
 * additional text to that line, so user must at least printf("\n")
 * after that.
 */

#define TIMING_DECLS(seconds)						       \
	long TMreps;		/* counts repetitions of timed code */ \
	long TMrepeats;		/* repetitions needed to exceed 0.1 second */ \
	Nanotime bnano,enano,delta;	/* start/stop times in nanoseconds */ \
        double seconds; \
	NCT_inittimer();

#ifndef NOREPEAT
#define TIMING_START \
	TMrepeats = 1; \
	do { \
	    NCT_marktime(&bnano); \
	    for(TMreps=0; TMreps < TMrepeats; TMreps++) {

#define TIMING_END(seconds)				\
            } \
	    NCT_marktime(&enano); \
	    NCT_elapsedtime(&bnano,&enano,&delta); \
	    TMrepeats *= 2; \
	} while (NCT_nanoseconds(delta) < 100000000 ); \
	seconds = ((double)NCT_nanoseconds(delta)) / (1000000000.0 * TMreps); \

#else /*NOREPEAT*/

#define TIMING_START \
	do { \
fprintf(stderr,"TIME_START\n"); \
	    NCT_marktime(&bnano); \
	    {

#define TIMING_END(time_mess,seconds) \
	    } \
	    NCT_marktime(&enano); \
	    NCT_elapsedtime(&bnano,&enano,&delta); \
        } while (0); \
fprintf(stderr,"TIME_END\n"); \
	seconds = ((double)NCT_nanoseconds(delta)) / (1000000000.0 * TMreps); \
	printf("%-45.45s %#08.6F sec", time_mess, seconds);

#endif /*NOREPEAT*/

#endif /*BM_TIMER_H*/
