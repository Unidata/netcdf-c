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

    const int mem3Size(100000);
    const int vlenSize(1);//100000;
    struct struct3{
      int mem1;
      double mem2;
      double mem3[mem3Size];
      nc_vlen_t mem4;
    };
    vector<int> vecSize(2); vecSize[0]=6,vecSize[1]=3;
    
    NcVlenType vlenType(ncFile.addVlenType("vlenType_1",ncDouble));
    
    
    NcCompoundType compoundType3(ncFile.addCompoundType("compoundType_3",sizeof(struct3)));
    compoundType3.addMember("member1",ncInt,offsetof(struct3,mem1));
    compoundType3.addMember("member2",ncDouble,offsetof(struct3,mem2));
    vector<int> mem3Shape(1);mem3Shape[0]=mem3Size;
    compoundType3.addMember("member3",ncDouble,offsetof(struct3,mem3),mem3Shape);
    compoundType3.addMember("member4",vlenType,offsetof(struct3,mem4));
    
    NcDim dim3 = ncFile.addDim("dim3");
    NcVar var_3   = ncFile.addVar("var_3", compoundType3,dim3);
    // set the fill value
    
    struct3 fillValue;
    fillValue.mem1=0;
    fillValue.mem2=0.0;
    for(int j=0;j<mem3Size;j++)fillValue.mem3[j]=0;
    fillValue.mem4.p=NULL;
    fillValue.mem4.len=vlenSize;
    
#if 1
    //var_3.setFill(true,&fillValue);
    // set the chunking;
    vector<size_t> chunksize(1); chunksize[0]=100;
    var_3.setChunking(NcVar::nc_CHUNKED,chunksize);
    // set the compression
    bool enableShuffleFilter(true),enableDeflateFilter(true);
    int deflateLevel(1);
    var_3.setCompression(enableShuffleFilter,enableDeflateFilter,deflateLevel);
    // set checksum
    var_3.setChecksum(NcVar::nc_FLETCHER32);
#endif
    struct3 dummyStruct;
    dummyStruct.mem1=1;
    dummyStruct.mem2=-1.23456;
    for(int j=0;j<mem3Size;j++)
      dummyStruct.mem3[j]=j*j+1;
    double* vlenPointer = (double*) malloc(vlenSize*sizeof(double));
    for(int j=0;j<vlenSize;j++)
      vlenPointer[j] = j*j+1.;//double(j)/1.21467;
    dummyStruct.mem4.p=vlenPointer;
    dummyStruct.mem4.len=vlenSize;
    
    vector<size_t> index(1);
    
    for (int i=0; i< 100;i++) {
      index[0]=i;
      var_3.putVar(index,&dummyStruct);  
    }      
    
    nc_free_vlen(&dummyStruct.mem4);
    exit(1);
    
  }
 catch (NcException& e)
   {
     cout << "unknown error"<<endl;
     e.what();
   }
}
