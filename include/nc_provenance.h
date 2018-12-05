/* Copyright 2005-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file
 * @internal Contains information for creating provenance
 * info and/or displaying provenance info.
 *
 * @author Dennis Heimbigner, Ward Fisher
*/
/**************************************************/
/**

It has come to pass that we can't guarantee that this information is
contained only within netcdf4 files.  As a result, we need
to make printing these hidden attributes available to
netcdf3 as well.

For netcdf4 files, capture state information about the following:
1. Global: netcdf library version
2. Global: hdf5 library version
3. Per file: superblock version
4. Per File: was it created by netcdf-4?
5. Per file: _NCProperties attribute
*/

#ifndef _NCPROVENANCE_
#define _NCPROVENANCE_

#define NCPROPS "_NCProperties"
#define NCPVERSION "version" /* Of the properties format */
#define NCPHDF5LIB1 "hdf5libversion"
#define NCPNCLIB1 "netcdflibversion"
#define NCPHDF5LIB2 "hdf5"
#define NCPNCLIB2 "netcdf"
#define NCPROPS_VERSION (2)
/* Version 2 changes this because '|' was causing bash problems */
#define NCPROPSSEP1  '|'
#define NCPROPSSEP2  ','


/* Other hidden attributes */
#define ISNETCDF4ATT "_IsNetcdf4"
#define SUPERBLOCKATT "_SuperblockVersion"


#endif /* _NCPROVENANCE_ */
