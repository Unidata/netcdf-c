/*********************************************************************
 * Copyright 2006, UCAR/Unidata See COPYRIGHT file for copying and
 * redistribution conditions.
 * 
 * This run a C++ test for netCDF involving very large files (> 2GB).
 * 
 * $Id: tst_large.cpp,v 1.1 2006/04/03 18:56:52 ed Exp $
 *********************************************************************/

#include "config.h"
#include <iostream>
using namespace std;

#include <string.h>
#include "netcdfcpp.h"

/* This is the magic number for classic format limits: 2 GiB - 4
   bytes. */
#define MAX_CLASSIC_BYTES 2147483644

/* This is the magic number for 64-bit offset format limits: 4 GiB - 4
   bytes. */
#define MAX_64OFFSET_BYTES 4294967292

/* Handy for constucting tests. */
#define QTR_CLASSIC_MAX (MAX_CLASSIC_BYTES/4)

#define ERR -2

// Generate a netCDF file
int
gen(const char* path, NcFile::FileFormat format)		
{

   // Create, leave in define mode
   NcFile nc(path, NcFile::Replace, NULL, 0, format); 

   // Check if the file was opened successfully
   if (! nc.is_valid()) {
      cerr << "can't create netCDF file " << path << "\n";
      return ERR;
   }

   // Turn off fill mode.
   nc.set_fill(NcFile::NoFill);

   // Create dimension
   NcDim* latd = nc.add_dim("lat", QTR_CLASSIC_MAX);

   // Create variable
   NcVar* P = nc.add_var("P", ncFloat, latd);

   // Start writing data, implictly leaves define mode
   static float *data;
   if (!(data = (float *)malloc(QTR_CLASSIC_MAX * sizeof(float))))
      return ERR;
   P->put(data, QTR_CLASSIC_MAX);

   free(data);
   return 0;
}

int
main( void )	// test new netCDF interface
{
   cout << "Creating large netCDF file with C++ API...";
   if (gen("tst_large.nc", NcFile::Offset64Bits))
      return ERR;
   cout << "ok\n";

#ifdef USE_NETCDF4
// Uncomment the following line to get lots of feedback from the library.
//    nc_set_log_level(5);

//     cout << "Creating netCDF4 file with C++ API...";
//     gen("nctst_netcdf4.nc", NcFile::Netcdf4);	// create a netCDF file
//     cout << "ok\n";
//     nc_set_log_level(0);

//     cout << "Creating netCDF4 classic file with C++ API...";
//     gen("nctst_netcdf4_classic.nc", NcFile::Netcdf4Classic);	// create a netCDF file
//     cout << "ok\n";
#endif

   return 0;
}
