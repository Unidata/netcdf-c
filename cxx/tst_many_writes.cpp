#include <iostream>
#include <sstream>

extern "C" {
#include <netcdf.h>
}


using namespace std;

//Exception class
class NcErrorException : public exception
{
public:
   NcErrorException(const string& descr) throw(): exception(), _descr(descr)  {};
   ~NcErrorException() throw() {};
   
   const char* what() const throw() { ostringstream err; err << "NcErrorException: " << _descr;  return err.str().c_str();   };
   
   
private:
   string _descr;
};

void handle_error(int status) {
   if (status != NC_NOERR) {
      throw NcErrorException(nc_strerror(status));
   }
};

/******MAIN********/
int main(int argc, char** argv)
{
   int NUMVARS = 5;
   size_t NUMREC=10000;
   int fileId, dimId, varId[NUMVARS];
   string filename("tst_many_writes.nc");
   
   cout << "\n*** Testing netCDF-4 file with user-provided test (thanks Marica!)\n";

   try{
      //create the netcdf-4 file
      handle_error ( nc_create(filename.c_str(), NC_NETCDF4, &fileId) );

      //define the unlimited dimension "rec"
      handle_error ( nc_def_dim(fileId, "rec", NC_UNLIMITED, &dimId) ); //--> Segmentation Fault
      //handle_error ( nc_def_dim(fileId, "rec", NUMREC, &dimId) );  //--> Good!!
      
      int dimids[1] = {dimId};
      
      //define NUMVARS variables named field_%i using a loop
      for (int v=0; v<NUMVARS; v++)
      {
      	 size_t chunkSize[1] = {100000};
      	 ostringstream varName; varName << "field_" << v;
      	 handle_error ( nc_def_var(fileId, varName.str().c_str(), NC_DOUBLE, 1, dimids , &varId[v]) );
      	 handle_error ( nc_def_var_chunking(fileId, varId[v], NC_CHUNKED, chunkSize ) );
      }
      handle_error ( nc_enddef(fileId) );
      
      //write data to the NUMVARS variables using nc_put_var1_double
      double data = 100;
      size_t index[1] ;
      char charName[NC_MAX_NAME+1];
      
      for (int v=0; v<NUMVARS; v++)
      {
      	 handle_error ( nc_inq_varname(fileId, varId[v], charName ) );
	 
      	 for (size_t i = 0; i< NUMREC  ; i++)
      	 {
      	    index[0] = i;
      	    handle_error ( nc_put_var1_double(fileId, varId[v], index, &data ) );
      	 }
      }

      //close file
      handle_error ( nc_close(fileId) );
      cout << "*** nctst SUCCESS!\n";      
   }
   catch(exception &ex) //exception handling
   {
      cerr << "Exception caught: " << ex.what() << endl;
      cout << "*** nctst FAILURE!\n";
      return -1;
   }
}
