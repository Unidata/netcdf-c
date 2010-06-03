#ifndef _NC3TOLNC3_CONV_
#define _NC3TOLNC3_CONV_

#include <stddef.h> /* size_t, ptrdiff_t */
#include <errno.h>  /* netcdf functions sometimes return system errors */

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * The Interface
 */
#define nc3__create lnc3__create
#define nc3_create lnc3_create
#define nc3__open lnc3__open
#define nc3_open lnc3_open
#define nc3_redef lnc3_redef
#define nc3__enddef lnc3__enddef
#define nc3_enddef lnc3_enddef
#define nc3_sync lnc3_sync
#define nc3_close lnc3_close
#ifndef PROTECT
#define nc3_abort lnc3_abort
#endif

/* Begin {get}_var1 */
#define nc3_get_var1 lnc3_get_var1
#define nc3_get_var1_text lnc3_get_var1_text
#define nc3_get_var1_uchar lnc3_get_var1_uchar
#define nc3_get_var1_schar lnc3_get_var1_schar
#define nc3_get_var1_short lnc3_get_var1_short
#define nc3_get_var1_int lnc3_get_var1_int
#define nc3_get_var1_long lnc3_get_var1_long
#define nc3_get_var1_float lnc3_get_var1_float
#define nc3_get_var1_double lnc3_get_var1_double
/* End {get}_var1 */
/* Begin {get}_vara */
#define nc3_get_vara lnc3_get_vara
#define nc3_get_vara_text lnc3_get_vara_text
#define nc3_get_vara_uchar lnc3_get_vara_uchar
#define nc3_get_vara_schar lnc3_get_vara_schar
#define nc3_get_vara_short lnc3_get_vara_short
#define nc3_get_vara_int lnc3_get_vara_int
#define nc3_get_vara_long lnc3_get_vara_long
#define nc3_get_vara_float lnc3_get_vara_float
#define nc3_get_vara_double lnc3_get_vara_double

/* End {get}_vara */
/* Begin {get}_vars */
/* 5/17/09: no longer need to capture the get_vars functions */
#ifdef IGNORE
#define nc3_get_vars lnc3_get_vars
#define nc3_get_vars_text lnc3_get_vars_text
#define nc3_get_vars_uchar lnc3_get_vars_uchar
#define nc3_get_vars_schar lnc3_get_vars_schar
#define nc3_get_vars_short lnc3_get_vars_short
#define nc3_get_vars_int lnc3_get_vars_int
#define nc3_get_vars_long lnc3_get_vars_long
#define nc3_get_vars_float lnc3_get_vars_float
#define nc3_get_vars_double lnc3_get_vars_double
#endif

/* End {get}_vars */
/* Begin {get}_varm */
#ifndef PROTECT
#define nc3_get_varm lnc3_get_varm
#define nc3_get_varm_text lnc3_get_varm_text
#define nc3_get_varm_uchar lnc3_get_varm_uchar
#define nc3_get_varm_schar lnc3_get_varm_schar
#define nc3_get_varm_short lnc3_get_varm_short
#define nc3_get_varm_int lnc3_get_varm_int
#define nc3_get_varm_long lnc3_get_varm_long
#define nc3_get_varm_float lnc3_get_varm_float
#define nc3_get_varm_double lnc3_get_varm_double
#endif

/* End {get}_varm */
/* Begin {get}_var */

#define nc3_get_var_text lnc3_get_var_text
#define nc3_get_var_uchar lnc3_get_var_uchar
#define nc3_get_var_schar lnc3_get_var_schar
#define nc3_get_var_short lnc3_get_var_short
#define nc3_get_var_int lnc3_get_var_int
#define nc3_get_var_long lnc3_get_var_long
#define nc3_get_var_float lnc3_get_var_float
#define nc3_get_var_double lnc3_get_var_double
/* End {get}_var */

/* #ifdef _CRAYMPP */
/*
 * Public interfaces to better support
 * CRAY multi-processor systems like T3E.
 * A tip of the hat to NERSC.
 */
/*
 * It turns out we need to declare and define
 * these public interfaces on all platforms
 * or things get ugly working out the
 * FORTRAN interface. On !_CRAYMPP platforms,
 * these functions work as advertised, but you
 * can only use "processor element" 0.
 */

#ifndef PROTECT
#define nc3__create_mp lnc3__create_mp
#define nc3__open_mp lnc3__open_mp
/*#define nc3_delete_mp lnc3_delete_mp*/
#endif

/* #endif _CRAYMPP */

#if defined(__cplusplus)
}
#endif

#endif /* _NETCDF_ */
