Documentation for getting and building netCDF with CMake
********************************************************

This document is for getting and building the netCDF C library and
utilities, version 4.3.  Other libraries that depend on the netCDF C
library, such as the Fortran and C++ libraries, are available as
separate distributions that can be built and installed after the C
library is successfully installed.  The netCDF-Java library is also a
separate distribution that is currently independent of the netCDF C
library.

Getting NetCDF
**************

The easiest way to get netCDF is through a package management program,
such as rpm, yum, adept, and others. NetCDF is available from many
different repositories, including the default Red Hat and Ubuntu
repositories.

When getting netCDF from a software repository, you will wish to get
the development version of the package ("netcdf-devel"). This includes
the netcdf.h header file.

Unfortunately, you may not be able to get a recent version of netCDF
from a package management system, in which case you must build from
source code. Get the

  ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf.tar.gz

source distribution for the latest, fully-tested release.

Alternatively, you may wish to try the 

  ftp://ftp.unidata.ucar.edu/pub/netcdf/snapshot/netcdf-4-daily.tar.gz

daily snapshot. It is generated nightly at the Unidata Program
Center. It has passed all tests on our (Linux) test machine, but not
necessarily all platform compatibility tests.

Warning: the daily snapshot release contains bug-fixes and new
features added since the last full release. It may also contain
portability bugs.

Once you have downloaded and unpacked the distribution, see the
following section on building.

For additional instructions on getting and building the netCDF 
dependencies, please see the 'INSTALL' text file.

Getting CMake
*************

CMake, a software configuration and testing tool, is maintained by
Kitware.  CMake is available in many linux package management
systems, as well as the 'macports' package management system for 
OSX. 

CMake may also be downloaded for these platforms, as well as Windows,
from the CMake website at http://www.cmake.org.

Building NetCDF with CMake
**************************

The netCDF-C library and utilities requires 3rd party libraries for
full functionality.

  *  Building with NetCDF-4 and the Remote Data Client
  *  Building NetCDF with Classic Library Only
  *  Building with HDF4 Support
  *  Building with Parallel I/O Support

Note that CMake encourages 'out-of-source-tree' builds, i.e. the
directory used to build netCDF is not the root of the netCDF
file structure.  For example, it is fairly common practice to
create a 'build' directory inside the source directory. The examples
in this file will use syntax which assumes the user is currently
located in c:\netcdf-src-dir\build\.

Building NetCDF with Classic Library Only
*****************************************

It is possible to build the netCDF C libraries and utilities so that
only the netCDF classic and 64-bit offset formats are supported, or
the remote data access client is not built.  (See

  http://www.unidata.ucar.edu/netcdf/docs/netcdf_format.html

for more information about the netCDF format variants.  See the
netCDF-DAP site

  http://opendap.org/netCDF-DAP 

for more information about remote client access to data on OPeNDAP
servers.)

To build without support for the netCDF-4 formats or the additional
netCDF-4 functions, but with remote access, use:

Windows:
	> cmake -DCMAKE_INSTALL_PREFIX=C:\Home\Ed\Local -DENABLE_NETCDF_4=OFF
	> cmake --build .
	> cmake --build . --target RUN_TESTS
	> cmake --build . --target INSTALL

Linux/Unix:	
  	> ./cmake -DCMAKE_INSTALL_PREFIX=/home/ed/local -DENABLE_NETCDF_4=OFF
   	> make test install

(Replace ``/home/ed/local'' with the name of the directory where
netCDF is to be installed.)

Starting with version 4.1.1 the netCDF C libraries and utilities have
supported remote data access, using the OPeNDAP protocols.  To build 
with full support for netCDF-4 APIs and format but without remote
client access, use:

Windows:
	> cmake -DCMAKE_INSTALL_PREFIX=C:\Home\Ed\Local -DENABLE_DAP=OFF
	> cmake --build .
	> cmake --build . --target RUN_TESTS
	> cmake --build . --target INSTALL

Linux/Unix
	> ./cmake -DCMAKE_INSTALL_PREFIX=/home/ed/local -DENABLE_DAP=OFF
	> make test install
	

If you get the message that netCDF installed correctly, then you are
done!
>>>> Pick Up Here <<<<



Building with HDF4 Support
**************************

The netCDF-4 library can (since version 4.1) read HDF4 data files, if
they were created with the SD (Scientific Data) API. To enable this
feature, use the --enable-hdf4 option. The location for the HDF4
header files and library must be set in the CPPFLAGS and LDFLAGS
options.

For HDF4 access to work, the library must be build with netCDF-4
features.

Building with Parallel I/O Support
**********************************

For parallel I/O to work, HDF5 must be installed with
–enable-parallel, and an MPI library (and related libraries) must be
made available to the HDF5 configure. This can be accomplished with
the mpicc wrapper script, in the case of MPICH2.

The following works to build HDF5 with parallel I/O on our netCDF
testing system:

  CC=mpicc ./configure --enable-parallel --prefix=/shecky/local_par --with-zlib=/shecky/local_par
  make check install

If the HDF5 used by netCDF has been built with parallel I/O, then
netCDF will also be built with support for parallel I/O. This allows
parallel I/O access to netCDF-4/HDF5 files.  (See

  http://www.unidata.ucar.edu/netcdf/docs/netcdf_format.html

for more information about the netCDF format variants.)

If parallel I/O access to netCDF classic and 64-bit offset files is
also needed, the parallel-netcdf library should also be installed,
(and the replacement pnetcdf.h at

  ftp://ftp.unidata.ucar.edu/pub/netcdf/contrib/pnetcdf.h

must be used). Then configure netCDF with the --enable-pnetcdf flag.

Linking to NetCDF
*****************

For static build, to use netCDF-4 you must link to all the libraries,
netCDF, HDF5, zlib, szip (if used with HDF5 build), and curl (if the
remote access client has not been disabled). This will mean -L options
to your build for the locations of the libraries, and -l (lower-case
L) for the names of the libraries.

For example, one user reports that she can build other applications
with netCDF-4 by setting the LIBS environment variable:

  LIBS='-L/X/netcdf-4.0/lib -lnetcdf -L/X/hdf5-1.8.6/lib -lhdf5_hl -lhdf5 -lz -lm -L/X/szip-2.1/lib -lsz'

For shared builds, only -lnetcdf is needed. All other libraries will
be found automatically.

The ``nc-config --all'' command can be used to learn what options are
needed for the local netCDF installation.

For example, this works for linking an application named myapp.c with
netCDF-4 libraries:

    cc -o myapp myapp.c `nc-config --cflags --libs`
