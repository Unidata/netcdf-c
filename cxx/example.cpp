#include <string.h>
#include "netcdfcpp.h"

#define NUM(array) (sizeof(array)/sizeof(array[0]))

int
main( void )
{
    const char* path = "Example.nc";
    NcFile nc (path, NcFile::Replace); // Create and leave in define mode

    // Check if the file was opened
    if (! nc.is_valid()) {
	std::cerr << "can't create netCDF file " << path << "\n";
	return 0;
    }

    // Create dimensions
    NcDim* latd = nc.add_dim("lat", 4);
    NcDim* lond = nc.add_dim("lon", 3);
    NcDim* frtimed = nc.add_dim("frtime");
    NcDim* timelend = nc.add_dim("timelen",20); 

    // Create variables and their attributes
    NcVar* P = nc.add_var("P", ncFloat, frtimed, latd, lond);
    P->add_att("long_name", "pressure at maximum wind");
    P->add_att("units", "hectopascals");
    static float range[] = {0., 1500.};
    P->add_att("valid_range", NUM(range), range);
    P->add_att("_FillValue", -9999.0f);

    NcVar* lat = nc.add_var("lat", ncFloat, latd);
    lat->add_att("long_name", "latitude");
    lat->add_att("units", "degrees_north");

    NcVar* lon = nc.add_var("lon", ncFloat, lond);
    lon->add_att("long_name", "longitude");
    lon->add_att("units", "degrees_east");

    NcVar* frtime = nc.add_var("frtime", ncLong, frtimed);
    frtime->add_att("long_name", "forecast time");
    frtime->add_att("units", "hours");

    NcVar* reftime = nc.add_var("reftime",ncChar,timelend);
    reftime->add_att("long_name", "reference time");
    reftime->add_att("units", "text_time");

    NcVar* scalar = nc.add_var("scalarv",ncLong);
    scalar->add_att("scalar_att", 1.0);

    // Global attributes
    nc.add_att("history", "created by Unidata LDM from NPS broadcast");
    nc.add_att("title", "NMC Global Product Set: Pressure at Maximum Wind");

    // Start writing data, implictly leaves define mode

    float* lats = new float[latd->size()];
    int i;
    for(i = 0; i < latd->size(); i++)
      lats[i] = -90. + 2.5*i;
    lat->put(lats, latd->size());

    float* lons = new float[lond->size()];
    for(i = 0; i < lond->size(); i++)
      lons[i] = -180. + 5.*i;
    lon->put(lons, lond->size());

    static int frtimes[] = {12, 18};
    frtime->put(frtimes, NUM(frtimes));

    static const char* s = "1992 03 04 12:00" ;
    reftime->put(s, strlen(s));

    static float P_data[2][4][3] = {
	{{950, 951, 952}, {953, 954, 955}, {956, 957, 958}, {959, 960, 961}},
	{{962, 963, 964}, {965, 966, 967}, {968, 969, 970}, {971, 972, 973}}
      };
    P->put(&P_data[0][0][0], P->edges());
    
    // close of nc takes place in destructor
}
