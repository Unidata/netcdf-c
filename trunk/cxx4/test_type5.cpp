// Purpose: Converts ida3 format xma data to netcdf4
// Usage:   xma2netcdf <shot number>


#include <iostream>
#include <iomanip>
#include <string>
#include <cstddef>
#include <netcdf>

#include <stdio.h>
#include <stddef.h>
#include "test_utilities.h"
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

int main()
{
try
  {
    NcFile ncFile("firstFile.cdf",NcFile::replace);

    const int mem3Size(1000);
    struct struct3{
      double mem3[mem3Size];
    };
    
    NcCompoundType compoundType3(ncFile.addCompoundType("compoundType_3",sizeof(struct3)));
    vector<int> mem3Shape(1);mem3Shape[0]=mem3Size;
    compoundType3.addMember("member3",ncDouble,offsetof(struct3,mem3),mem3Shape);
    NcDim timeDim = ncFile.addDim("time");
    NcDim pfCoilDim = ncFile.addDim("pfCoil");
    vector<NcDim> dimVector(2);
    dimVector[1]=timeDim;
    dimVector[0]=pfCoilDim;
    NcVar var_3   = ncFile.addVar("var_3", compoundType3,dimVector);

    // set the fill value
    struct3 fillValue;
    for(int j=0;j<mem3Size;j++)fillValue.mem3[j]=0;
    
#if 1
    //var_3.setFill(true,&fillValue);
    // set the chunking;
    vector<size_t> chunksize(2);
    chunksize[0]=2;
    chunksize[1]=100;
    var_3.setChunking(NcVar::nc_CHUNKED,chunksize);
    // set the compression
    bool enableShuffleFilter(true),enableDeflateFilter(true);
    int deflateLevel(1);
    var_3.setCompression(enableShuffleFilter,enableDeflateFilter,deflateLevel);
    // set checksum
    var_3.setChecksum(NcVar::nc_FLETCHER32);
#endif
    struct3 dummyStruct;
    for(int j=0;j<mem3Size;j++)
      dummyStruct.mem3[j]=j*j+1;
    for(int j=mem3Size/2;j<mem3Size;j++)
      dummyStruct.mem3[j]=0.;

    struct3 dummyStruct2;
    for(int j=0;j<mem3Size;j++)
      dummyStruct2.mem3[j]=-1./(j*j*j+1);
    
    vector<size_t> index(2);
    
    for (int i=0; i< 100;i++) {
      index[1]=i;
      index[0]=0;
      var_3.putVar(index,&dummyStruct);  
      index[0]=1;
      var_3.putVar(index,&dummyStruct2);  
    }      
    
    
  }
 catch (NcException& e)
   {
     cout << "unknown error"<<endl;
     e.what();
   }
}
