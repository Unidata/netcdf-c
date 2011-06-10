#include <iostream>
#include <iomanip>
#include <netcdf>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;


int main()
{
try
  {
    cout<<"Opening file \"firstFile.cdf\" with NcFile::replace"<<endl;
    NcFile ncFile("firstFile.cdf",NcFile::replace);
    
    cout<<left<<setw(50)<<"Testing addGroup(\"groupName\")";
    NcGroup groupA(ncFile.addGroup("groupA"));
    NcGroup groupA0(ncFile.addGroup("groupA0"));
    NcGroup groupB(groupA.addGroup("groupB"));
    NcGroup groupC(groupA.addGroup("groupC"));
    
    cout <<left<<setw(50)<<"Number of groups in the top-level group is"<ncFile.getGroupCount()<<endl;

    // create two dimensions
    ncFile.addDim("dim1",11);
    ncFile.addDim("dim2");
    
    // create a variable 
    NcVar var1  = ncFile.addVar("varA",ncByte,dim1);

    // create another variable 
    vector<string> stringArray(2);
    stringArray[0] = "dim1";
    stringArray[1] = "dim2";
    NcVar var2  = ncFile.addVar("varB",ncByte,stringArray);
  }
 catch (NcException& e)
   {
     cout << "unknown error"<<endl;
     e.what();
   }
}
