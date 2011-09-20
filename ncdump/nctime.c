/*********************************************************************
 *   Copyright 2008, University Corporation for Atmospheric Research
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id: nctime.c,v 1.9 2010/05/05 22:15:39 dmh Exp $
 *********************************************************************/

/* 
 * This code was extracted with permission from the CDMS time
 * conversion and arithmetic routines developed by Bob Drach, Lawrence
 * Livermore National Laboratory as part of the cdtime library.  Russ
 * Rew of the UCAR Unidata Program made changes and additions to
 * support the "-t" option of the netCDF ncdump utility, including a
 * 366-day climate calendar.
 *
 * For the complete time conversion and climate calendar facilities of
 * the CDMS library, get the original sources from LLNL.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <netcdf.h>
#include "utils.h"
#include "nccomps.h"
#include "dumplib.h"
#include "ncdump.h"		/* for fspec_t def */
#include "nctime.h"

extern fspec_t formatting_specs; /* set from command-line options */

static int cuErrOpts;			     /* Error options */
static int cuErrorOccurred = 0;		     /* True iff cdError was called */

#define CU_FATAL 1			     /* Exit immediately on fatal error */
#define CU_VERBOSE 2			     /* Report errors */

#define CD_DEFAULT_BASEYEAR "1979"	     /* Default base year for relative time (no 'since' clause) */
#define VALCMP(a,b) ((a)<(b)?-1:(b)<(a)?1:0)

/* forward declarations */
static void Cdh2e(CdTime *htime, double *etime);
static void cdChar2Comp(cdCalenType timetype, char* chartime, cdCompTime* comptime);
static void cdComp2Rel(cdCalenType timetype, cdCompTime comptime, char* relunits, double* reltime);
static void cdRel2CompMixed(double reltime, cdUnitTime unit, cdCompTime basetime, cdCompTime *comptime);
static void cdRel2Comp(cdCalenType timetype, char* relunits, double reltime, cdCompTime* comptime);

/* Trim trailing whitespace, up to n characters. */
/* If no whitespace up to the last character, set */
/* the last character to null, else set the first */
/* whitespace character to null. */
static void
cdTrim(char* s, int n)
{
	char* c;

	if(s==NULL)
		return;
	for(c=s; *c && c<s+n-1 && !isspace(*c); c++);
	*c='\0';
	return;
}

static
void cdError(char *fmt, ...){
	va_list args;
	
	cuErrorOccurred = 1;
	if(cuErrOpts & CU_VERBOSE){
		va_start(args,fmt);
		fprintf(stderr, "CDMS error: ");
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	}
	if(cuErrOpts & CU_FATAL)
		exit(1);
	return;
}

#define ISLEAP(year,timeType)	((timeType & Cd366) || (((timeType) & CdHasLeap) && (!((year) % 4) && (((timeType) & CdJulianType) || (((year) % 100) || !((year) % 400))))))
static int mon_day_cnt[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
static int days_sum[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

/* Compute month and day from year and day-of-year.
 *
 *	Input:
 *		doy	     (int)  (day-of-year)
 *		date->year   (long)  (year since 0 BC)
 *              date->timeType (CdTimetype) (time type)
 *              date->baseYear   base year for relative times
 *	Output: 
 *		date->month  (short)  (month in year) 
 *		date->day    (short)  (day in month)
 *
 * 
 * Derived from NRL NEONS V3.6.
 */

static void
CdMonthDay(int *doy, CdTime *date)
{
	int i;				/* month counter */
	int idoy;			/* day of year counter */
	long year;

	if ((idoy = *doy) < 1) {
		date->month = 0;
		date->day   = 0;
		return;
	}

	if(!(date->timeType & CdChronCal))   /* Ignore year for Clim calendar */
		year = 0;
	else if(!(date->timeType & CdBase1970))	/* year is offset from base for relative time */
		year = date->baseYear + date->year;
	else
		year = date->year;

	if (ISLEAP(year,date->timeType)) {
		mon_day_cnt[1] = 29;
	} else {
		mon_day_cnt[1] = 28;
	}
	date->month	= 0;
	for (i = 0; i < 12; i++) {
		(date->month)++;
		date->day	= idoy;
		if ((idoy -= ((date->timeType & Cd365) ? (mon_day_cnt[date->month-1]) : 30)) <= 0) {
			return;
		}
	}
	return;
}

/* Compute day-of-year from year, month and day
 * 
 *	Input:
 *		date->year  (long)  (year since 0 BC)
 *		date->month (short)  (month in year)
 *		date->day   (short)  (day in month)
 *              date->baseYear   base year for relative times
 *	Output: doy         (int)  (day-of-year)
 * 
 * Derived from NRL NEONS V3.6
 */

static void
CdDayOfYear(CdTime *date, int *doy)
{
	int leap_add = 0;		/* add 1 day if leap year */
	int month;			/* month */
	long year;

   	month	= date->month;
	if (month < 1 || month > 12) {
		cdError( "Day-of-year error; month: %d\n", month);
		month = 1;	
	}

	if(!(date->timeType & CdChronCal))   /* Ignore year for Clim calendar */
		year = 0;
	else if(!(date->timeType & CdBase1970))	/* year is offset from base for relative time */
		year = date->baseYear + date->year;
	else
		year = date->year;

	if (ISLEAP(year,date->timeType) && month > 2) leap_add = 1;
	if( ((date->timeType) & Cd365) || ((date->timeType) & Cd366) ) {
	    *doy = days_sum[month-1] + date->day + leap_add ;
	} else {		/* date->timeType & Cd360 */
	    *doy = 30*(month-1) + date->day + leap_add ;
	}
	return;
}

/* Convert epochal time (hours since 00 jan 1, 1970)
 *   to human time (structured)
 * 
 * Input: 
 *   etime = epochal time representation
 *   timeType = time type (e.g., CdChron, CdClim, etc.) as defined in cdms.h
 *   baseYear = base real, used for relative time types only
 * 
 * Output: htime = human (structured) time representation
 * 
 * Derived from NRL Neons V3.6
 */
static void
Cde2h(double etime, CdTimeType timeType, long baseYear, CdTime *htime)
{
	long 	ytemp;			/* temporary year holder */
	int 	yr_day_cnt;		/* count of days in year */
	int 	doy;			/* day of year */
	int     daysInLeapYear;		     /* number of days in a leap year */
	int     daysInYear;		     /* days in non-leap year */
	extern void CdMonthDay(int *doy, CdTime *date);

	doy	= (long) floor(etime / 24.) + 1;
	htime->hour	= etime - (double) (doy - 1) * 24.;

					     /* Correct for goofy floor func on J90 */
	if(htime->hour >= 24.){
		doy += 1;
		htime->hour -= 24.;
	}

	htime->baseYear = (timeType & CdBase1970) ? 1970 : baseYear;
	if(!(timeType & CdChronCal)) htime->baseYear = 0; /* Set base year to 0 for Clim */
	if(timeType & Cd366) {
	    daysInLeapYear = 366;
	    daysInYear = 366;
	} else {
	    daysInLeapYear = (timeType & Cd365) ? 366 : 360;
	    daysInYear = (timeType & Cd365) ? 365 : 360;
	}

	if (doy > 0) {
		for (ytemp = htime->baseYear; ; ytemp++) {
			yr_day_cnt = ISLEAP(ytemp,timeType) ? daysInLeapYear : daysInYear;
			if (doy <= yr_day_cnt) break;
			doy -= yr_day_cnt;
		}
	} else {
		for (ytemp = htime->baseYear-1; ; ytemp--) {
			yr_day_cnt = ISLEAP(ytemp,timeType) ? daysInLeapYear : daysInYear;
			doy += yr_day_cnt;
			if (doy > 0) break;
		}
	}
        htime->year = (timeType & CdBase1970) ? ytemp : (ytemp - htime->baseYear);
	if(!(timeType & CdChronCal)) htime->year = 0; /* Set year to 0 for Clim */
	htime->timeType = timeType;
	CdMonthDay(&doy,htime);

        return;
}

/* Add 'nDel' times 'delTime' to epochal time 'begEtm',
 * return the result in epochal time 'endEtm'.
 */
static void
CdAddDelTime(double begEtm, long nDel, CdDeltaTime delTime, CdTimeType timeType,
	     long baseYear, double *endEtm)
{
	double delHours;
	long delMonths, delYears;
	CdTime bhtime, ehtime;

	extern void Cde2h(double etime, CdTimeType timeType, long baseYear, CdTime *htime);
	extern void Cdh2e(CdTime *htime, double *etime);

	switch(delTime.units){
	  case CdYear:
		delMonths = 12;
		break;
	  case CdSeason:
		delMonths = 3;
		break;
	  case CdMonth:
		delMonths = 1;
		break;
	  case CdWeek:
		delHours = 168.0;
		break;
	  case CdDay:
		delHours = 24.0;
		break;
	  case CdHour:
		delHours = 1.0;
		break;
	  case CdMinute:
		delHours = 1./60.;
		break;
	  case CdSecond:
		delHours = 1./3600.;
		break;
	  default:
		cdError("Invalid delta time units: %d\n",delTime.units);
		return;
	}

	switch(delTime.units){
	  case CdYear: case CdSeason: case CdMonth:
		Cde2h(begEtm,timeType,baseYear,&bhtime);
		delMonths = delMonths * nDel * delTime.count + bhtime.month - 1;
		delYears = (delMonths >= 0 ? (delMonths/12) : (delMonths+1)/12 - 1);
		ehtime.year = bhtime.year + delYears;
		ehtime.month = delMonths - (12 * delYears) + 1;
		ehtime.day = 1;
		ehtime.hour = 0.0;
		ehtime.timeType = timeType;
		ehtime.baseYear = !(timeType & CdChronCal) ? 0 :
			(timeType & CdBase1970) ? 1970 : baseYear; /* base year is 0 for Clim, */
								   /* 1970 for Chron, */
								   /* or input base year for Rel */
		Cdh2e(&ehtime,endEtm);
		break;
	  case CdWeek: case CdDay: case CdHour: case CdMinute: case CdSecond:
		delHours *= (nDel * delTime.count);
		*endEtm = begEtm + delHours;
		break;
	  default: break;
	}
	return;
}

/* Parse relative units, returning the unit and base component time. */
/* Function returns 1 if error, 0 on success */
static int
cdParseRelunits(cdCalenType timetype, char* relunits, cdUnitTime* unit, cdCompTime* base_comptime)
{
	char charunits[CD_MAX_RELUNITS];
	char basetime_1[CD_MAX_CHARTIME];
	char basetime_2[CD_MAX_CHARTIME];
	char basetime[CD_MAX_CHARTIME];
	int nconv1, nconv2, nconv;

					     /* Parse the relunits */
	/* Allow ISO-8601 "T" date-time separator as well as blank separator */
	nconv1 = sscanf(relunits,"%s since %[^T]T%s",charunits,basetime_1,basetime_2);
	if(nconv1==EOF || nconv1==0){
		cdError("Error on relative units conversion, string = %s\n",relunits);
		return 1;
	}
	nconv2 = sscanf(relunits,"%s since %s %s",charunits,basetime_1,basetime_2);
	if(nconv2==EOF || nconv2==0){
		cdError("Error on relative units conversion, string = %s\n",relunits);
		return 1;
	}
	if(nconv1 < nconv2) {
	    nconv = nconv2;
	} else {
	    nconv = sscanf(relunits,"%s since %[^T]T%s",charunits,basetime_1,basetime_2);
	}

					     /* Get the units */
	cdTrim(charunits,CD_MAX_RELUNITS);
	if(!strncmp(charunits,"sec",3) || !strcmp(charunits,"s")){
		*unit = cdSecond;
	}
	else if(!strncmp(charunits,"min",3) || !strcmp(charunits,"mn")){
		*unit = cdMinute;
	}
	else if(!strncmp(charunits,"hour",4) || !strcmp(charunits,"hr")){
		*unit = cdHour;
	}
	else if(!strncmp(charunits,"day",3) || !strcmp(charunits,"dy")){
		*unit = cdDay;
	}
	else if(!strncmp(charunits,"week",4) || !strcmp(charunits,"wk")){
		*unit = cdWeek;
	}
	else if(!strncmp(charunits,"month",5) || !strcmp(charunits,"mo")){
		*unit = cdMonth;
	}
	else if(!strncmp(charunits,"season",6)){
		*unit = cdSeason;
	}
	else if(!strncmp(charunits,"year",4) || !strcmp(charunits,"yr")){
		if(!(timetype & cdStandardCal)){
			cdError("Error on relative units conversion: climatological units cannot be 'years'.\n");
			return 1;
		}
		*unit = cdYear;
	}
	else {
		cdError("Error on relative units conversion: invalid units = %s\n",charunits);
		return 1;
	}

					     /* Build the basetime, if any (default is 1979), */
					     /* or month 1 for climatological time. */
	if(nconv == 1){
		if(timetype & cdStandardCal)
			strcpy(basetime,CD_DEFAULT_BASEYEAR);
		else
			strcpy(basetime,"1");
	}
					     /* Convert the basetime to component, then epochal (hours since 1970) */
	else{
		if(nconv == 2){
			cdTrim(basetime_1,CD_MAX_CHARTIME);
			strcpy(basetime,basetime_1);
		}
		else{
			cdTrim(basetime_1,CD_MAX_CHARTIME);
			cdTrim(basetime_2,CD_MAX_CHARTIME);
			sprintf(basetime,"%s %s",basetime_1,basetime_2);
		}
	}

	cdChar2Comp(timetype, basetime, base_comptime);

	return 0;
}

/* ca - cb in Gregorian calendar */
/* Result is in hours. */
static double
cdDiffGregorian(cdCompTime ca, cdCompTime cb){

	double rela, relb;

	cdComp2Rel(cdStandard, ca, "hours", &rela);
	cdComp2Rel(cdStandard, cb, "hours", &relb);
	return (rela - relb);
}

/* Return -1, 0, 1 as ca is less than, equal to, */
/* or greater than cb, respectively. */
static int 
cdCompCompare(cdCompTime ca, cdCompTime cb){

	int test;

	if ((test = VALCMP(ca.year, cb.year)))
		return test;
	else if ((test = VALCMP(ca.month, cb.month)))
		return test;
	else if ((test = VALCMP(ca.day, cb.day)))
		return test;
	else
		return (VALCMP(ca.hour, cb.hour));
}

/* ca - cb in Julian calendar.  Result is in hours. */
static double
cdDiffJulian(cdCompTime ca, cdCompTime cb){

	double rela, relb;

	cdComp2Rel(cdJulian, ca, "hours", &rela);
	cdComp2Rel(cdJulian, cb, "hours", &relb);
	return (rela - relb);
}

/* ca - cb in mixed Julian/Gregorian calendar. */
/* Result is in hours. */
static double
cdDiffMixed(cdCompTime ca, cdCompTime cb){

	static cdCompTime ZA = {1582, 10, 5, 0.0};
	static cdCompTime ZB = {1582, 10, 15, 0.0};
	double result;

	if (cdCompCompare(cb, ZB) == -1){
		if (cdCompCompare(ca, ZB) == -1) {
			result = cdDiffJulian(ca, cb);
		}
		else {
			result = cdDiffGregorian(ca, ZB) + cdDiffJulian(ZA, cb);
		}
	}
	else {
		if (cdCompCompare(ca, ZB) == -1){
			result = cdDiffJulian(ca, ZA) + cdDiffGregorian(ZB, cb);
		}
		else {
			result = cdDiffGregorian(ca, cb);
		}
	}
	return result;
}

/* Divide ('endEtm' - 'begEtm') by 'delTime',
 * return the integer portion of the result in 'nDel'.
 */
static void
CdDivDelTime(double begEtm, double endEtm, CdDeltaTime delTime, CdTimeType timeType,
	     long baseYear, long *nDel)
{
	double delHours, frange;
	long delMonths, range;
	CdTime bhtime, ehtime;
	int hoursInYear;
	
	extern void Cde2h(double etime, CdTimeType timeType, long baseYear, CdTime *htime);

	switch(delTime.units){
	  case CdYear:
		delMonths = 12;
		break;
	  case CdSeason:
		delMonths = 3;
		break;
	  case CdMonth:
		delMonths = 1;
		break;
	  case CdWeek:
		delHours = 168.0;
		break;
	  case CdDay:
		delHours = 24.0;
		break;
	  case CdHour:
		delHours = 1.0;
		break;
	  case CdMinute:
		delHours = 1./60.;
		break;
	  case CdSecond:
		delHours = 1./3600.;
		break;
	  default:
		cdError("Invalid delta time units: %d\n",delTime.units);
		return;
	}

	switch(delTime.units){
	  case CdYear: case CdSeason: case CdMonth:
		delMonths *= delTime.count;
		Cde2h(begEtm,timeType,baseYear,&bhtime);
		Cde2h(endEtm,timeType,baseYear,&ehtime);
		if(timeType & CdChronCal){   /* Chron and Rel time */
			range = 12*(ehtime.year - bhtime.year)
				+ (ehtime.month - bhtime.month);
		}
		else{			     /* Clim time, ignore year */
			range = (ehtime.month - bhtime.month);
			if(range < 0) range += 12;
		}
		*nDel = abs(range)/delMonths;
		break;
	  case CdWeek: case CdDay: case CdHour: case CdMinute: case CdSecond:
		delHours *= (double)delTime.count;
		if(timeType & CdChronCal){   /* Chron and Rel time */
			frange = fabs(endEtm - begEtm);
		}
		else{			     /* Clim time, ignore year, but */
					     /* wraparound relative to hours-in-year*/
			frange = endEtm - begEtm;
			if(timeType & Cd366) {
			    hoursInYear = 8784;
			} else {
			    hoursInYear = (timeType & Cd365) ? 8760. : 8640.;
			}
					     /* Normalize frange to interval [0,hoursInYear) */
			if(frange < 0.0 || frange >= hoursInYear)
				frange -= hoursInYear * floor(frange/hoursInYear);
		}
		*nDel = (frange + 1.e-10*delHours)/delHours;
		break;
	    default: break;
	}
	return;
}

/* Value is in hours. Translate to units. */
static double
cdFromHours(double value, cdUnitTime unit){
	double result;

	switch(unit){
	case cdSecond:
		result = value * 3600.0;
		break;
	case cdMinute:
		result = value * 60.0;
		break;
	case cdHour:
		result = value;
		break;
	case cdDay:
		result = value/24.0;
		break;
	case cdWeek:
		result = value/168.0;
		break;
	case cdMonth:
	case cdSeason:
	case cdYear:
	case cdFraction:
	default:
    		cdError("Error on conversion from hours to vague unit");
		result = 0;
		break;
	}
	return result;
}
					     /* Map to old timetypes */
static int
cdToOldTimetype(cdCalenType newtype, CdTimeType* oldtype)
{
	switch(newtype){
	  case cdStandard:
		*oldtype = CdChron;
		break;
	  case cdJulian:
		*oldtype = CdJulianCal;
		break;
	  case cdNoLeap:
		*oldtype = CdChronNoLeap;
		break;
	  case cd360:
		*oldtype = CdChron360;
		break;
	  case cd366:
		*oldtype = CdChron366;
		break;
	  case cdClim:
		*oldtype = CdClim;
		break;
	  case cdClimLeap:
		*oldtype = CdClimLeap;
		break;
	  case cdClim360:
		*oldtype = CdClim360;
		break;
	  default:
		cdError("Error on relative units conversion, invalid timetype = %d",newtype);
		return 1;
	}
	return 0;
}

/* Convert human time to epochal time (hours since 00 jan 1, 1970)
 * 
 * Input: htime = human time representation
 * 
 * Output: etime = epochal time representation
 * 
 * Derived from NRL Neons V3.6
 */
static void
Cdh2e(CdTime *htime, double *etime)
{
	long 	ytemp, year;			/* temporary year holder */
	int	day_cnt;		/* count of days */
	int 	doy;			/* day of year */
	long    baseYear;		     /* base year for epochal time */
	int     daysInLeapYear;		     /* number of days in a leap year */
	int     daysInYear;		     /* days in non-leap year */
	extern void CdDayOfYear(CdTime *date, int *doy);

	CdDayOfYear(htime,&doy);
	
	day_cnt	= 0;

	baseYear = ((htime->timeType) & CdBase1970) ? 1970 : htime->baseYear;
	year = ((htime->timeType) & CdBase1970) ? htime->year : (htime->year + htime->baseYear);
	if(!((htime->timeType) & CdChronCal)) baseYear = year = 0;	/* set year and baseYear to 0 for Clim */
	if((htime->timeType) & Cd366) {
	    daysInLeapYear = 366;
	    daysInYear = 366;
	} else {
	    daysInLeapYear = ((htime->timeType) & Cd365) ? 366 : 360;
	    daysInYear = ((htime->timeType) & Cd365) ? 365 : 360;
	}
	
	if (year > baseYear) {
		for (ytemp = year - 1; ytemp >= baseYear; ytemp--) {
			day_cnt += ISLEAP(ytemp,htime->timeType) ? daysInLeapYear : daysInYear;
		}
	} else if (year < baseYear) {
		for (ytemp = year; ytemp < baseYear; ytemp++) {
			day_cnt -= ISLEAP(ytemp,htime->timeType) ? daysInLeapYear : daysInYear;
		}
	}	
	*etime	= (double) (day_cnt + doy - 1) * 24. + htime->hour;
        return;
}

/* Validate the component time, return 0 if valid, 1 if not */
static int
cdValidateTime(cdCalenType timetype, cdCompTime comptime)
{
	if(comptime.month<1 || comptime.month>12){
		cdError("Error on time conversion: invalid month = %hd\n",comptime.month);
		return 1;
	}
	if(comptime.day<1 || comptime.day>31){
		cdError("Error on time conversion: invalid day = %hd\n",comptime.day);
		return 1;
	}
	if(comptime.hour<0.0 || comptime.hour>24.0){
		cdError("Error on time conversion: invalid hour = %lf\n",comptime.hour);
		return 1;
	}
	return 0;
}

static void
cdChar2Comp(cdCalenType timetype, char* chartime, cdCompTime* comptime)
{
	double sec;
	int ihr, imin, nconv;
	long year;
	short day;
	short month;

	comptime->year = CD_NULL_YEAR;
	comptime->month = CD_NULL_MONTH;
	comptime->day = CD_NULL_DAY;
	comptime->hour = CD_NULL_HOUR;
	
	if(timetype & cdStandardCal){
		nconv = sscanf(chartime,"%ld-%hd-%hd %d:%d:%lf",&year,&month,&day,&ihr,&imin,&sec);
		if(nconv==EOF || nconv==0){
			cdError("Error on character time conversion, string = %s\n",chartime);
			return;
		}
		if(nconv >= 1){
			comptime->year = year;
		}
		if(nconv >= 2){
			comptime->month = month;
		}
		if(nconv >= 3){
			comptime->day = day;
		}
		if(nconv >= 4){
			if(ihr<0 || ihr>23){
				cdError("Error on character time conversion: invalid hour = %d\n",ihr);
				return;
			}
			comptime->hour = (double)ihr;
		}
		if(nconv >= 5){
			if(imin<0 || imin>59){
				cdError("Error on character time conversion: invalid minute = %d\n",imin);
				return;
			}
			comptime->hour += (double)imin/60.;
		}
		if(nconv >= 6){
			if(sec<0.0 || sec>60.0){
				cdError("Error on character time conversion: invalid second = %lf\n",sec);
				return;
			}
			comptime->hour += sec/3600.;
		}
	}
	else{				     /* Climatological */
		nconv = sscanf(chartime,"%hd-%hd %d:%d:%lf",&month,&day,&ihr,&imin,&sec);
		if(nconv==EOF || nconv==0){
			cdError("Error on character time conversion, string = %s",chartime);
			return;
		}
		if(nconv >= 1){
			comptime->month = month;
		}
		if(nconv >= 2){
			comptime->day = day;
		}
		if(nconv >= 3){
			if(ihr<0 || ihr>23){
				cdError("Error on character time conversion: invalid hour = %d\n",ihr);
				return;
			}
			comptime->hour = (double)ihr;
		}
		if(nconv >= 4){
			if(imin<0 || imin>59){
				cdError("Error on character time conversion: invalid minute = %d\n",imin);
				return;
			}
			comptime->hour += (double)imin/60.;
		}
		if(nconv >= 5){
			if(sec<0.0 || sec>60.0){
				cdError("Error on character time conversion: invalid second = %lf\n",sec);
				return;
			}
			comptime->hour += sec/3600.;
		}
	}
	(void)cdValidateTime(timetype,*comptime);
	return;
}

/* Convert ct to relunits (unit, basetime) */
/* in the mixed Julian/Gregorian calendar. */
/* unit is anything but year, season, month. unit and basetime are */
/* from the parsed relunits. Return result in reltime. */
static void
cdComp2RelMixed(cdCompTime ct, cdUnitTime unit, cdCompTime basetime, double *reltime){

	double hourdiff;

	hourdiff = cdDiffMixed(ct, basetime);
	*reltime = cdFromHours(hourdiff, unit);
	return;
}

static void
cdComp2Rel(cdCalenType timetype, cdCompTime comptime, char* relunits, double* reltime)
{
	cdCompTime base_comptime;
	CdDeltaTime deltime;
	CdTime humantime;
	CdTimeType old_timetype;
	cdUnitTime unit;
	double base_etm, etm, delta;
	long ndel, hoursInYear;
	
					     /* Parse the relunits */
	if(cdParseRelunits(timetype, relunits, &unit, &base_comptime))
		return;

					     /* Handle mixed Julian/Gregorian calendar */
	if (timetype == cdMixed){
		switch(unit){
		case cdWeek: case cdDay: case cdHour: case cdMinute: case cdSecond:
			cdComp2RelMixed(comptime, unit, base_comptime, reltime);
			return;
		case cdYear: case cdSeason: case cdMonth:
			timetype = cdStandard;
			break;
		case cdFraction:
		        cdError("invalid unit in conversion");
		        break;
		default: break;
		}
	}
	
					     /* Convert basetime to epochal */
	humantime.year = base_comptime.year;
	humantime.month = base_comptime.month;
	humantime.day = base_comptime.day;
	humantime.hour = base_comptime.hour;
	humantime.baseYear = 1970;
					     /* Map to old-style timetype */
	if(cdToOldTimetype(timetype,&old_timetype))
		return;
	humantime.timeType = old_timetype;
	Cdh2e(&humantime,&base_etm);

					     /* Map end time to epochal */
	humantime.year = comptime.year;
	humantime.month = comptime.month;
	humantime.day = comptime.day;
	humantime.hour = comptime.hour;
	Cdh2e(&humantime,&etm);
					     /* Calculate relative time value for months or hours */
	deltime.count = 1;
	deltime.units = (CdTimeUnit)unit;
	switch(unit){
	  case cdWeek: case cdDay: case cdHour: case cdMinute: case cdSecond:
		delta = etm - base_etm;
		if(!(timetype & cdStandardCal)){	/* Climatological time */
			hoursInYear = (timetype & cd365Days) ? 8760. : (timetype & cdHasLeap) ? 8784. : 8640.;
					     /* Normalize delta to interval [0,hoursInYear) */
			if(delta < 0.0 || delta >= hoursInYear)
				delta -= hoursInYear * floor(delta/hoursInYear);
		}
		break;
	  case cdYear: case cdSeason: case cdMonth:
		CdDivDelTime(base_etm, etm, deltime, old_timetype, 1970, &ndel);
		break;
	  case cdFraction:
	        cdError("invalid unit in conversion");
		break;
	  default: break;
	}

					     /* Convert to output units */
	switch(unit){
	  case cdSecond:
		*reltime = 3600.0 * delta;
		break;
	  case cdMinute:
		*reltime = 60.0 * delta;
		break;
	  case cdHour:
		*reltime = delta;
		break;
	  case cdDay:
		*reltime = delta/24.0;
		break;
	  case cdWeek:
		*reltime = delta/168.0;
		break;
	  case cdMonth: case cdSeason: case cdYear: /* Already in correct units */
		if(timetype & cdStandardCal)
			*reltime = (base_etm <= etm) ? (double)ndel : (double)(-ndel);
		else			     /* Climatological time is already normalized*/
			*reltime = (double)ndel;
		break;
	  default:
		cdError("invalid unit in conversion");
		break;
	}

	return;
}

/* Add (value,unit) to comptime. */
/* value is in hours. */
/* calendar is anything but cdMixed. */
static void
cdCompAdd(cdCompTime comptime, double value, cdCalenType calendar, cdCompTime *result){

	double reltime;

	cdComp2Rel(calendar, comptime, "hours", &reltime);
	reltime += value;
	cdRel2Comp(calendar, "hours", reltime, result);
	return;
}

/* Add value in hours to ct, in the mixed Julian/Gregorian
 * calendar. */
static void
cdCompAddMixed(cdCompTime ct, double value, cdCompTime *result){

	static cdCompTime ZA = {1582, 10, 5, 0.0};
	static cdCompTime ZB = {1582, 10, 15, 0.0};
	double xj, xg;

	if (cdCompCompare(ct, ZB) == -1){
		xj = cdDiffJulian(ZA, ct);
		if (value <= xj){
			cdCompAdd(ct, value, cdJulian, result);
		}
		else {
			cdCompAdd(ZB, value-xj, cdStandard, result);
		}
	}
	else {
		xg = cdDiffGregorian(ZB, ct);
		if (value > xg){
			cdCompAdd(ct, value, cdStandard, result);
		}
		else {
			cdCompAdd(ZA, value-xg, cdJulian, result);
		}
	}
	return;
}

/* Return value expressed in hours. */
static double
cdToHours(double value, cdUnitTime unit){

	double result = 0;

	switch(unit){
	case cdSecond:
		result = value/3600.0;
		break;
	case cdMinute:
		result = value/60.0;
		break;
	case cdHour:
		result = value;
		break;
	case cdDay:
		result = 24.0 * value;
		break;
	case cdWeek:
		result = 168.0 * value;
		break;
	default:
	        cdError("invalid unit in conversion");
		break;

	}
	return result;
}

/* Convert relative time (reltime, unit, basetime) to comptime in the
 * mixed Julian/Gregorian calendar. unit is anything but year, season,
 * month. unit and basetime are from the parsed relunits. Return
 * result in comptime. */
static void
cdRel2CompMixed(double reltime, cdUnitTime unit, cdCompTime basetime, cdCompTime *comptime){

	reltime = cdToHours(reltime, unit);
	cdCompAddMixed(basetime, reltime, comptime);
	return;
}


static void
cdRel2Comp(cdCalenType timetype, char* relunits, double reltime, cdCompTime* comptime)
{
	CdDeltaTime deltime;
	CdTime humantime;
	CdTimeType old_timetype;
	cdCompTime base_comptime;
	cdUnitTime unit, baseunits;
	double base_etm, result_etm;
	double delta;
	long idelta;

					     /* Parse the relunits */
	if(cdParseRelunits(timetype, relunits, &unit, &base_comptime))
		return;

	if (timetype == cdMixed){
		switch(unit){
		case cdWeek: case cdDay: case cdHour: case cdMinute: case cdSecond:
			cdRel2CompMixed(reltime, unit, base_comptime, comptime);
			return;
		case cdYear: case cdSeason: case cdMonth:
			timetype = cdStandard;
			break;
		case cdFraction:
		        cdError("invalid unit in conversion");
		        break;
		default: break;
		}
	}

	baseunits =cdBadUnit;
	switch(unit){
	  case cdSecond:
		delta = reltime/3600.0;
		baseunits = cdHour;
		break;
	  case cdMinute:
		delta = reltime/60.0;
		baseunits = cdHour;
		break;
	  case cdHour:
		delta = reltime;
		baseunits = cdHour;
		break;
	  case cdDay:
		delta = 24.0 * reltime;
		baseunits = cdHour;
		break;
	  case cdWeek:
		delta = 168.0 * reltime;
		baseunits = cdHour;
		break;
	  case cdMonth:
		idelta = (long)(reltime + (reltime<0 ? -1.e-10 : 1.e-10));
		baseunits = cdMonth;
		break;
	  case cdSeason:
		idelta = (long)(3.0 * reltime + (reltime<0 ? -1.e-10 : 1.e-10));
		baseunits = cdMonth;
		break;
	  case cdYear:
		idelta = (long)(12 * reltime + (reltime<0 ? -1.e-10 : 1.e-10));
		baseunits = cdMonth;
		break;
	  default:
	        cdError("invalid unit in conversion");
		break;
	}

	deltime.count = 1;
	deltime.units = (CdTimeUnit)baseunits;

	humantime.year = base_comptime.year;
	humantime.month = base_comptime.month;
	humantime.day = base_comptime.day;
	humantime.hour = base_comptime.hour;
	humantime.baseYear = 1970;
					     /* Map to old-style timetype */
	if(cdToOldTimetype(timetype,&old_timetype))
		return;
	humantime.timeType = old_timetype;

	Cdh2e(&humantime,&base_etm);
					     /* If months, seasons, or years, */
	if(baseunits == cdMonth){

					     /* Calculate new epochal time from integer months. */
					     /* Convert back to human, then comptime. */
					     /* For zero reltime, just return the basetime*/
		if(reltime != 0.0){
			CdAddDelTime(base_etm,idelta,deltime,old_timetype,1970,&result_etm);
			Cde2h(result_etm, old_timetype, 1970, &humantime);
		}
	}
					     /* Calculate new epochal time. */
					     /* Convert back to human, then comptime. */
	else{
		Cde2h(base_etm+delta, old_timetype, 1970, &humantime);
		
	}
	comptime->year = humantime.year;
	comptime->month = humantime.month;
	comptime->day = humantime.day;
	comptime->hour = humantime.hour;

	return;
}

/* rkr: output as ISO 8601 strings */
static void
cdComp2Iso(cdCalenType timetype, int separator, cdCompTime comptime, char* time)
{
	double dtmp, sec;
	int ihr, imin, isec;
	int nskip;

	if(cdValidateTime(timetype,comptime))
		return;
	
	ihr = (int)comptime.hour;
	dtmp = 60.0 * (comptime.hour - (double)ihr);
	imin = (int)dtmp;
	sec = 60.0 * (dtmp - (double)imin);
	isec = sec;

	if(sec == isec)
	    if(isec == 0)
		if(imin == 0)
		    if(ihr == 0)
			nskip = 4;
		    else
			nskip = 3;
		else
		    nskip = 2;
	    else
		nskip = 1;
	else
	    nskip = 0;

	if(timetype & cdStandardCal){
	    switch (nskip) {
	    case 0:		/* sec != 0 && (int)sec != sec */
		sprintf(time,"%4.4ld-%2.2hd-%2.2hd%c%2.2d:%2.2d:%lf",
			comptime.year,comptime.month,comptime.day,separator,ihr,imin,sec);
		break;
	    case 1:
		sprintf(time,"%4.4ld-%2.2hd-%2.2hd%c%2.2d:%2.2d:%2.2d",
			comptime.year,comptime.month,comptime.day,separator,ihr,imin,isec);
		break;
	    case 2:
		sprintf(time,"%4.4ld-%2.2hd-%2.2hd%c%2.2d:%2.2d",
			comptime.year,comptime.month,comptime.day,separator,ihr,imin);
		break;
	    case 3:
		sprintf(time,"%4.4ld-%2.2hd-%2.2hd%c%2.2d",
			comptime.year,comptime.month,comptime.day,separator,ihr);
		break;
	    case 4:
		sprintf(time,"%4.4ld-%2.2hd-%2.2hd",
			comptime.year,comptime.month,comptime.day);
		break;
	    }
	}
	else {				     /* Climatological */
	    switch (nskip) {
	    case 0:		/* sec != 0 && (int)sec != sec */
		sprintf(time,"%2.2hd-%2.2hd%c%2.2d:%2.2d:%lf",
			comptime.month,comptime.day,separator,ihr,imin,sec);
		break;
	    case 1:
		sprintf(time,"%2.2hd-%2.2hd%c%2.2d:%2.2d:%2.2d",
			comptime.month,comptime.day,separator,ihr,imin,isec);
		break;
	    case 2:
		sprintf(time,"%2.2hd-%2.2hd%c%2.2d:%2.2d",
			comptime.month,comptime.day,separator,ihr,imin);
		break;
	    case 3:
		sprintf(time,"%2.2hd-%2.2hd%c%2.2d",
			comptime.month,comptime.day,separator,ihr);
		break;
	    case 4:
		sprintf(time,"%2.2hd-%2.2hd",
			comptime.month,comptime.day);
		break;
	    }
	}
	return;
}

/* rkr: added for output closer to ISO 8601 */
void
cdRel2Iso(cdCalenType timetype, char* relunits, int separator, double reltime, char* chartime)
{
	cdCompTime comptime;

	cdRel2Comp(timetype, relunits, reltime, &comptime);
	cdComp2Iso(timetype, separator, comptime, chartime);

	return;
}

/* rkr: added bounds functions to detect bounds variables of time variables */
static void
bounds_add(char *bounds_name, int ncid, int varid) {
    bounds_node_t *bnode = emalloc(sizeof(bounds_node_t) + 1);
    bounds_list.nbnds++;
    bnode->ncid = ncid;
    bnode->varid = varid;
    bnode->bounds_name = strdup(bounds_name);
    bnode->next = bounds_list.first;
    bounds_list.first = bnode;
}

boolean
is_bounds_var(char *varname, int *pargrpidp, int *parvaridp) {
    bounds_node_t *bp = bounds_list.first;
    for(; bp; bp = bp->next) {
	if(STREQ(bp->bounds_name, varname)) {
	    *pargrpidp = bp->ncid;
	    *parvaridp = bp->varid;
	    return true;
	}
    }
    return false;
} 

/* Return true only if this is a "bounds" attribute */
boolean
is_bounds_att(ncatt_t *attp) {
    if(attp->type == NC_CHAR && attp->valgp && STREQ((char *)attp->name, "bounds")) {
	return true;
    }
#ifdef USE_NETCDF4
    if(attp->type == NC_STRING && attp->valgp && STREQ((char *)attp->name, "bounds")) {
	return true;
    }
#endif /* USE_NETCDF4 */
    return false;
}

/* Insert info about a bounds attribute into bounds list, so we can
 * later determine which variables are bounds variables for which
 * other variables.  att must be a variable "bounds" attribute.  */
void
insert_bounds_info(int ncid, int varid, ncatt_t att) {
    static boolean uninitialized = true;
    if(uninitialized) {
	bounds_list.nbnds = 0;
	bounds_list.first = NULL;
    }
    assert(is_bounds_att(&att));
    bounds_add(att.valgp, ncid, varid);
}

/* Check for optional "calendar" attribute and return specified
 * calendar type, if present. */
static cdCalenType
calendar_type(int ncid, int varid) {
    int ctype;
    int stat;
    ncatt_t catt;
    static struct {
	char* attname;
	int type;
    } calmap[] = {
	{"gregorian", cdMixed},
	{"standard", cdMixed}, /* synonym */
	{"proleptic_gregorian", cdStandard},
	{"noleap", cdNoLeap},
	{"no_leap", cdNoLeap},
	{"365_day", cdNoLeap},	/* synonym */
	{"allleap", cd366},
	{"all_leap", cd366},	/* synonym */
	{"366_day", cd366},	/* synonym */
	{"360_day", cd360},
	{"julian", cdJulian},
	{"none", cdClim}	/* TODO: test this */
    };
#define CF_CAL_ATT_NAME "calendar"
    int ncals = (sizeof calmap)/(sizeof calmap[0]);
    ctype = cdMixed;  /* default mixed Gregorian/Julian ala udunits */
    stat = nc_inq_att(ncid, varid, CF_CAL_ATT_NAME, &catt.type, &catt.len);
    if(stat == NC_NOERR && catt.type == NC_CHAR && catt.len > 0) {
	char *calstr = (char *)emalloc(catt.len + 1);
	int itype;
	NC_CHECK(nc_get_att(ncid, varid, CF_CAL_ATT_NAME, calstr));	
	calstr[catt.len] = '\0';
	for(itype = 0; itype < ncals; itype++) {
	    if(strncmp(calstr, calmap[itype].attname, catt.len) == 0) {
		ctype = calmap[itype].type;
		break;
	    }
	}
	free(calstr);
    }
    return ctype;
}

void
get_timeinfo(int ncid1, int varid1, ncvar_t *vp) {
    ncatt_t uatt;		/* units attribute */
    int nc_status;		/* return from netcdf calls */
    char *units;
    int ncid = ncid1;
    int varid = varid1;

    vp->has_timeval = false; /* by default, turn on if criteria met */
    vp->timeinfo = 0;
    vp->is_bnds_var = false;
    /* for timeinfo, treat a bounds variable like its "parent" time variable */
    if(is_bounds_var(vp->name, &ncid, &varid)) {
	vp->is_bnds_var = true;
    }

    /* time variables must have appropriate units attribute or be a bounds variable */
    nc_status = nc_inq_att(ncid, varid, "units", &uatt.type, &uatt.len);
    if(nc_status == NC_NOERR && uatt.type == NC_CHAR) { /* TODO: NC_STRING? */
	units = emalloc(uatt.len + 1);
	NC_CHECK(nc_get_att(ncid, varid, "units", units));
	units[uatt.len] = '\0';
	/* check for calendar attribute (not required even for time vars) */
	vp->timeinfo = (timeinfo_t *)emalloc(sizeof(timeinfo_t));
	memset((void*)vp->timeinfo,0,sizeof(timeinfo_t));
	vp->timeinfo->calendar = calendar_type(ncid, varid);
	/* Parse relative units, returning the unit and base component time. */
 	if(cdParseRelunits(vp->timeinfo->calendar, units, 
			   &vp->timeinfo->unit, &vp->timeinfo->origin) != 0) {
	    /* error parsing units so just treat as not a time variable */
	    free(vp->timeinfo);
	    free(units);
	    vp->timeinfo = NULL;
	    return;
	}
	/* Currently this gets reparsed for every value, need function
	 * like cdRel2Comp that resuses parsed units? */
	vp->timeinfo->units = strdup(units);
	vp->has_timeval = true;
	free(units);
    }
    return;
}

/* print_att_times 
 * by Dave Allured, NOAA/PSD/CIRES.  
 * This version supports only primitive attribute types; do not call
 * for user defined types.  Print interpreted, human readable (ISO)
 * time strings for an attribute of a CF-like time variable.
 *
 * Print strings as CDL comments, following the normal non-decoded
 * numeric values, which were already printed by the calling function.
 * In the following example, this function prints only the right hand
 * side, starting at the two slashes:
 *
 *    time:actual_range = 51133., 76670. ; // "1940-01-01", "2009-12-01"
 *
 * This function may be called for ALL primitive attributes.
 * This function qualifies the attribute for numeric type and
 * inheriting valid time attributes (has_time).  If the attribute
 * does not qualify, this function prints nothing and safely
 * returns.
 *
 * This function interprets and formats time values with the SAME
 * methods already used in ncdump -t for data variables.
 *
 * This version has special line wrapping rules:
 *
 * (1) If the attribute has one or two values, the time strings are
 *     always printed on the same line.
 *
 * (2) If the attribute has three or more values, the time strings
 *     are always printed on successive lines, with line wrapping
 *     as needed.
 *
 * Assume: Preceeding call to pr_att_valgs has already screened
 * this attribute for valid primitive types for the current netcdf
 * model (netcdf 3 or 4).
 */

void
print_att_times(
    int ncid,
    int varid,			/* parent var ID */
    ncatt_t att			/* attribute structure */
    )
{
    nc_type type = att.type;	/* local copy */
    boolean wrap;
    boolean first_item;

    ncvar_t var;		/* fake var structure for the att values; */
				/* will add only the minimum necessary info */

/* For common disqualifications, print nothing and return immediately. */

    if (type == NC_CHAR || type == NC_STRING)	/* must be numeric */
	return;

    if (varid == NC_GLOBAL)	/* time units not defined for global atts */
	return;

    assert (att.len > 0);	/* should already be eliminated by caller */

#ifdef USE_NETCDF4
    assert ( type == NC_BYTE   || type == NC_SHORT  || type == NC_INT
          || type == NC_FLOAT  || type == NC_DOUBLE || type == NC_UBYTE
          || type == NC_USHORT || type == NC_UINT   || type == NC_INT64
          || type == NC_UINT64 );
#else   /* NETCDF3 */
    assert ( type == NC_BYTE   || type == NC_SHORT  || type == NC_INT
          || type == NC_FLOAT  || type == NC_DOUBLE );
#endif

/* Get time info from parent variable, and qualify. */

    memset((void*)&var,0,sizeof(var));	/* clear the fake var structure */
    get_timeinfo(ncid, varid, &var);	/* sets has_timeval, timeinfo members */
    
    if (var.has_timeval) {		/* no print unless time qualified */

/* Convert each value to ISO date/time string, and print. */
	
	size_t iel;				     /* attrib index */
	const char *valp = (const char *)att.valgp;  /* attrib value pointer */
	safebuf_t *sb = sbuf_new();		/* allocate new string buffer */
        int func;				/* line wrap control */
	int separator = ' ';			/* default between data and time */
	if(formatting_specs.iso_separator)
	    separator = 'T';
        
	var.type = att.type;		/* insert attrib type into fake var */

	for (iel = 0; iel < att.len; iel++) {
	    nctime_val_tostring(&var, sb, (void *)valp);  /* convert to str. */
	    valp += att.tinfo->size;	/* increment value pointer, by type */
	    if (iel < att.len - 1)	/* add comma, except for final value */
		sbuf_cat(sb, ",");

            first_item = (iel == 0);	/* identify start of list */
            
            wrap = (att.len > 2);	/* specify line wrap variations:     */
					/* 1 or 2 values: keep on same line, */
					/* more than 2: enable line wrap     */

            lput2 (sbuf_str(sb), first_item, wrap);
            				/* print string in CDL comment, */
            				/* with auto newline            */
	}

        sbuf_free(sb);			/* clean up */

	if(var.timeinfo->units)		/* clean up from get_timeinfo */
	    free(var.timeinfo->units);
	free(var.timeinfo);
    }
}
