/*********************************************************************
  *   Copyright 2018, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *********************************************************************/

/** @file d4read.h
 * @brief Legacy read-function prototypes (superseded by ncd4.h).
 *
 * The active declarations are in ncd4.h as NCD4_readDMR() and NCD4_readDAP().
 * @author Dennis Heimbigner
 */

#ifndef D4READ_H
#define D4READ_H

/** @internal Fetch the DMR for @p info. */
extern int NCD4_readDMR(NCD4INFO* info, int flags);
/** @internal Fetch a full DAP response for @p info. */
extern int NCD4_readDAP(NCD4INFO* info, int flags, NCURI* uri);

#endif /*READ_H*/
