dnl UD macros for netcdf configure

dnl 
dnl UD_CHECK_IEEE
dnl If the 'double' is not an IEEE double
dnl or the 'float' is not and IEEE single,
dnl define NO_IEEE_FLOAT
dnl
AC_DEFUN([UD_CHECK_IEEE],
[
AC_MSG_CHECKING(for IEEE floating point format)
AC_TRY_RUN([#ifndef NO_FLOAT_H
#include <float.h>
#endif

#define EXIT_NOTIEEE	1
#define EXIT_MAYBEIEEE	0

int
main()
{
#if	defined(FLT_RADIX)	&& FLT_RADIX != 2
		return EXIT_NOTIEEE;
#elif	defined(DBL_MAX_EXP)	&& DBL_MAX_EXP != 1024
		return EXIT_NOTIEEE;
#elif	defined(DBL_MANT_DIG)	&& DBL_MANT_DIG != 53
		return EXIT_NOTIEEE;
#elif 	defined(FLT_MAX_EXP)	&& !(FLT_MAX_EXP == 1024 || FLT_MAX_EXP == 128)
		return EXIT_NOTIEEE;
#elif	defined(FLT_MANT_DIG)	&& !(FLT_MANT_DIG == 53 || FLT_MANT_DIG == 24)
		return EXIT_NOTIEEE;
#else
	/* (assuming eight bit char) */
	if(sizeof(double) != 8)
		return EXIT_NOTIEEE;
	if(!(sizeof(float) == 4 || sizeof(float) == 8))
		return EXIT_NOTIEEE;

	return EXIT_MAYBEIEEE;
#endif
}],ac_cv_c_ieeefloat=yes, ac_cv_c_ieeefloat=no, :)
AC_MSG_RESULT($ac_cv_c_ieeefloat)
if test "$ac_cv_c_ieeefloat" = no; then
  AC_DEFINE([NO_IEEE_FLOAT], [], [no IEEE float on this platform])
fi
])


dnl
dnl Print which compilers are going to be used, the flags, and their
dnl locations. This is all to assist in debugging, and help with
dnl support questions.
dnl
AC_DEFUN([UD_DISPLAY_RESULTS],
[
AC_MSG_CHECKING(CPPFLAGS)
AC_MSG_RESULT($CPPFLAGS)
AC_MSG_CHECKING(CC CFLAGS)
AC_MSG_RESULT($CC $CFLAGS)
ud_type_cc=`type $CC`
AC_MSG_CHECKING(type $CC)
AC_MSG_RESULT($ud_type_cc)

AC_MSG_CHECKING(CXX)
if test -n "$CXX"; then
	AC_MSG_RESULT($CXX)
	AC_MSG_CHECKING(CXXFLAGS)
	AC_MSG_RESULT($CXXFLAGS)
	ud_type_CXX=`type $CXX`
	AC_MSG_CHECKING(type $CXX)
	AC_MSG_RESULT($ud_type_CXX)
else
	AC_MSG_RESULT(unset)
fi

AC_MSG_CHECKING(FC)
if test -n "$FC"; then
	AC_MSG_RESULT($FC)
	AC_MSG_CHECKING(FFLAGS)
	AC_MSG_RESULT($FFLAGS)
	ud_type_fc=`type $FC`
	AC_MSG_CHECKING(type $FC)
	AC_MSG_RESULT($ud_type_fc)
else
	AC_MSG_RESULT(unset)
fi

AC_MSG_CHECKING(F90)
if test -n "$F90"; then
	AC_MSG_RESULT($F90)
	AC_MSG_CHECKING(FCFLAGS)
	AC_MSG_RESULT($FCFLAGS)
	ud_type_F90=`type $F90`
	AC_MSG_CHECKING(type $F90)
	AC_MSG_RESULT($ud_type_F90)
else
	AC_MSG_RESULT(unset)
fi

AC_MSG_CHECKING(AR)
if test -n "$AR"; then
	AC_MSG_RESULT($AR)
	AC_MSG_CHECKING(AR_FLAGS)
	AC_MSG_RESULT($AR_FLAGS)
	ud_type_AR=`type $AR`
	AC_MSG_CHECKING(type $AR)
	AC_MSG_RESULT($ud_type_AR)
else
	AC_MSG_RESULT(unset)
fi

AC_MSG_CHECKING(NM)
if test -n "$NM"; then
	AC_MSG_RESULT($NM)
	AC_MSG_CHECKING(NMFLAGS)
	AC_MSG_RESULT($NMFLAGS)
#	ud_type_NM=`type $NM`
#	AC_MSG_CHECKING(type $NM)
#	AC_MSG_RESULT($ud_type_NM)
else
	AC_MSG_RESULT(unset)
fi

])

dnl Find the full path of a header file
dnl
dnl UD_CHECK_HEADER_PATH(file, [action-if-found], [action-if-not-found])
dnl Example:
dnl UD_CHECK_HEADER_PATH([math.h])
dnl AC_MSG_NOTICE([ac_cv_header_path_math_h=$ac_cv_header_path_math_h])
dnl
dnl
AC_DEFUN([UD_CHECK_HEADER_PATH],
[
    AS_VAR_PUSHDEF([ac_Path], [ac_cv_header_path_$1])dnl
    AC_CACHE_CHECK(
       [for full path of header file $1], [ac_Path],
       [AC_PREPROC_IFELSE(
           [AC_LANG_PROGRAM([[#include <$1>]])],
           [AS_VAR_SET([ac_Path], [`sed -n '/\.h"/s/.*"\(.*\)".*/\1/p' conftest.i | grep -m 1 $1`])],
           [AC_MSG_RESULT([not found])]
       )])
    AS_VAR_SET_IF([ac_Path], [$2], [$3])
    AS_VAR_POPDEF([ac_Path])dnl
])

