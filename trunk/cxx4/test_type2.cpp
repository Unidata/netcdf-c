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

      struct struct3{
	int mem1;
	double mem2;
	short mem3[3];
      };
      vector<int> vecSize(2); vecSize[0]=6,vecSize[1]=3;
      
      NcCompoundType compoundType3(ncFile.addCompoundType("compoundType_3",sizeof(struct3)));
      compoundType3.addMember("member1",ncInt,offsetof(struct3,mem1));
      compoundType3.addMember("member2",ncDouble,offsetof(struct3,mem2));
      vector<int> mem3Shape(1);mem3Shape[0]=3;
      compoundType3.addMember("member3",ncShort,offsetof(struct3,mem3),mem3Shape);

      NcDim dim3 = ncFile.addDim("dim3",1);
      NcVar var_3   = ncFile.addVar("var_3", compoundType3,dim3);
      
      struct3 dummyStruct;
      dummyStruct.mem1=1;
      dummyStruct.mem2=-1.23456;
      dummyStruct.mem3[0]=1;
      dummyStruct.mem3[1]=-6;
      dummyStruct.mem3[2]=20;
      
      vector<size_t> index(1);index[0]=0;
      var_3.putVar(index,&dummyStruct);  
      
      //NcVar var_4(ncFile.getVar("var_3"));

      exit(1);
}
catch (NcException& e)
  {
    cout << "unknown error"<<endl;
    e.what();
  }
}
