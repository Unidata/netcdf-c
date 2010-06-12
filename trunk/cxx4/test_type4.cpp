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

    const int mem2Size(3);
    const int mem3Size(10);
    struct struct3{
      short mem1;
      double mem3[mem3Size];
      int mem2[mem2Size];
    };
    
    NcCompoundType compoundType3(ncFile.addCompoundType("compoundType_3",sizeof(struct3)));
    vector<int> mem2Shape(1);mem2Shape[0]=mem2Size;
    vector<int> mem3Shape(1);mem3Shape[0]=mem3Size;
    compoundType3.addMember("member1",ncShort,offsetof(struct3,mem1));
    compoundType3.addMember("member3",ncDouble,offsetof(struct3,mem3),mem3Shape);
    compoundType3.addMember("member2",ncInt,offsetof(struct3,mem2),mem2Shape);
    NcDim dim3 = ncFile.addDim("dim3");
    NcVar var_3   = ncFile.addVar("var_3", compoundType3,dim3);
    // set the fill value
    
    struct3 fillValue;
    for(int j=0;j<mem3Size;j++)fillValue.mem3[j]=0;
    
#if 1
    //    var_3.setFill(true,&fillValue);
    // set the chunking;
    vector<size_t> chunksize(1); chunksize[0]=1000;
    var_3.setChunking(NcVar::nc_CHUNKED,chunksize);
    // set the compression
    bool enableShuffleFilter(true),enableDeflateFilter(true);
    int deflateLevel(1);
    var_3.setCompression(enableShuffleFilter,enableDeflateFilter,deflateLevel);
    // set checksum
    var_3.setChecksum(NcVar::nc_FLETCHER32);
#endif
    struct3 dummyStruct;
    dummyStruct.mem1=-1237;
    for(int j=23;j<mem2Size-23;j++)
      dummyStruct.mem2[j]=-(j*j*j-134597);
    for(int j=0;j<mem3Size;j++)
      dummyStruct.mem3[j]=j*j+1;
    
    vector<size_t> index(1);
    
    for (int i=0; i< 1000;i++) {
      index[0]=i;
      var_3.putVar(index,&dummyStruct);  
    }      
    
    
  }
 catch (NcException& e)
   {
     cout << "unknown error"<<endl;
     e.what();
   }
}
