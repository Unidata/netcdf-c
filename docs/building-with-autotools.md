Build Instructions for NetCDF-C using autoconf/automake/libtool {#netCDF-autotools}
===========================================

[TOC]

# Overview {#autotools_overview}

A CMake build of netCDF is also supported. Users should consider using
the CMake build, which builds netCDF on a wider range of platforms,
include Microsoft Windows with Visual Studio. The autotools based
build may eventually be retired; users may considering transitioning
to the CMake build. See \ref netCDF-CMake.

We also provide pre-built binary downloads for the shared versions of
netCDF for use with Visual Studio.
		
# Requirements {#autotools_requirements}
The following packages are required to build netCDF-C.

* netCDF-C Source Code
* Optional Requirements:
	* HDF5 Libraries for netCDF4/HDF5 support.
	* libcurl for DAP support.

<center>
<img src="deptree.jpg" height="250px" />
</center>

# The Autotools Build Process {#autotools_build}

There are four steps in the Build Process when using autotools:

1. Configuration: Before compiling, the software is configured based on the desired options.
2. Building: Once configuration is complete, the libraries are compiled.
3. Testing: Post-build, it is possible to run tests to ensure the functionality of the netCDF-C libraries.
4. Installation: If all tests pass, the libraries can be installed in the location specified during configuration.

For users who prefer pre-built binaries, installation packages are available at \ref winbin

## Configuration {#autotools_configuration}

The output of the configuration step are the makefiles which build the library, utilities, and tests.

### Common Configure Options {#autotools_common_options}

| **Option** | **Autotools** | **CMake** |
| :------- | :---- | :----- |
Specify Install Location | --prefix=PREFIX | -DCMAKE\_INSTALL\_PREFIX=PREFIX
Enable/Disable netCDF-4 | --enable-netcdf-4<br>--disable-netcdf-4 | -DENABLE\_NETCDF\_4=ON <br> -DENABLE\_NETCDF\_4=OFF
Enable/Disable DAP | --enable-dap <br> --disable-dap | -DENABLE\_DAP=ON <br> -DENABLE\_DAP=OFF
Enable/Disable Utilities | --enable-utilities <br> --disable-utilities | -DBUILD\_UTILITIES=ON <br> -DBUILD\_UTILITIES=OFF
Specify shared/Static Libraries | --enable-shared <br> --enable-static | -DBUILD\_SHARED\_LIBS=ON <br> -DBUILD\_SHARED\_LIBS=OFF
Enable/Disable Tests | --enable-testsets <br> --disable-testsets | -DENABLE\_TESTS=ON <br> -DENABLE\_TESTS=OFF
Specify a custom library location | Use *CFLAGS* and *LDFLAGS* | -DCMAKE\_PREFIX\_PATH=/usr/custom_libs/
Enable parallel I/O tests | --enable-parallel-tests | -DNETCDF\_ENABLE_PARALLEL\_TESTS=ON

A full list of options can be found by invoking `configure --help`. 

### Configuring your build from the command line. {#autotools_command_line}

The easiest configuration case would be one in which all of the
dependent libraries are installed on the system path (in either
Unix/Linux or Windows) and all the default options are desired. In the
source directory:

> $ ./configure

If you have libraries installed in a custom directory, you will need to
set the CPPFLAGS and LDFLAGS environment variables to tell the compiler where the
libraries are installed. For example:

> $ CPPFLAGS=-I/usr/local/hdf5-1.14.3/include LDFLAGS=-L/usr/local/hdf5-1.14.3/lib ./configure --prefix=/usr/local/netcdf-c-4.9.3

#### Building with Parallel I/O. {#autotools_parallel_io}

NetCDF will build with parallel I/O if the C compiler is an MPI
compiler, and HDF5 was built for parallel I/O. To build netcdf-c for
parallel I/O, first build HDF5 for parallel I/O, then build netcdf-c
like this:

> $ CC=mpicc CPPFLAGS=-I/usr/local/hdf5-1.14.3_mpich/include LDFLAGS=-L/usr/local/hdf5-1.14.3_mpicj/lib ./configure --prefix=/usr/local/netcdf-c-4.9.3_mpich --enable-parallel-tests

The parallel I/O tests will only run if the additional configure
option is used: --enable-parallel-tests. Those tests run (by default)
with mpiexec, on 4, 16, or 32 processors. If mpiexec cannot be used on
your login-nodes, a different command can be used to launch the
parallel I/O tests. Used the --with-mpiexec configure option to set a
different parallel I/O job launcher:

> $ CC=mpicc CPPFLAGS=-I/usr/local/hdf5-1.14.3_mpich/include LDFLAGS=-L/usr/local/hdf5-1.14.3_mpicj/lib ./configure --prefix=/usr/local/netcdf-c-4.9.3_mpich --enable-parallel-tests --with-mpiexec='srun -A acct_name -q queue_name'

## Setting the HDF5 Plugin Path

Use the --with-plugin-dir= option to set the HDF5 plugin path. This
path must contain the directory where HDF5 filters have been
installed. --with-plugin-dir accepts three values:

* yes - use the first directory in environment variable
HDF5_PLUGIN_PATH (if it is set), or /usr/local/hdf5/lib/plugin (Unix)
or ${ALLUSERSPROFILE}\\hdfd5\\lib\\plugin (Windows).

* a path - path to be used for plugins.

* no - do not install or use plugins.

## Checking the Configure Summary

At the end of the configure, a summary of the build will be
output. It's important to check this to ensure that the desired build
features are accurately set. For example:

<pre>
# NetCDF C Configuration Summary
==============================

# General
-------
NetCDF Version:		4.9.4-development
Dispatch Version:       5
Configured On:		Wed Aug  7 06:53:22 MDT 2024
Host System:		x86_64-pc-linux-gnu
Build Directory: 	/home/ed/netcdf-c
Install Prefix:         /usr/local/netcdf-c-4.9.3
Plugin Install Prefix:  N.A.

# Compiling Options
-----------------
C Compiler:		/usr/bin/gcc
CFLAGS:			 -fno-strict-aliasing
CPPFLAGS:		-I/usr/local/hdf5-1.14.3/include
LDFLAGS:		-L/usr/local/hdf5-1.14.3/lib
AM_CFLAGS:		
AM_CPPFLAGS:		
AM_LDFLAGS:		
Shared Library:		yes
Static Library:		yes
Extra libraries:	-lhdf5_hl -lhdf5 -lm -lz -lsz -lzstd -lxml2 -lcurl 
XML Parser:             libxml2

# Features
--------
Benchmarks:		no
NetCDF-2 API:		yes
HDF4 Support:		no
HDF5 Support:		yes
CDF5 Support:		yes
NC-4 Parallel Support:	no
PnetCDF Support:	no

DAP2 Support:		yes
DAP4 Support:		yes
Byte-Range Support:	yes

S3 Support:	        no
S3 SDK:  	        none

NCZarr Support:		yes
NCZarr Zip Support:     no

Diskless Support:	yes
MMap Support:		no
ERANGE Fill Support:	no
Relaxed Boundary Check:	yes

Quantization:		yes
Logging:     		no
SZIP Write Support:     yes
Standard Filters:       bz2 deflate szip zstd
ZSTD Support:           yes
Parallel Filters:       yes
</pre>

Important settings include:
* Install Prefix - this is where netCDF will be installed.
* HDF5 Support - must be 'yes' to use netcdf-4/HDF5 files.
* NC-4 Parallel Support - Must be 'yes' to use HDF5 parallel I/O.
* PnetCDF Support - Must be 'yes' to use pnetcdf library for parallel I/O with classic netCDF files.
* Standard Filters - Must include 'zstd' if zstandard compression is to be used.
* DAP2/DAP4 Support - Must be 'yes' if OPeNDAP is to be used.

## Building {#autotools_building}

The compiler can be executed directly with 'make'.

> $ make

## Testing {#autotools_testing}

Testing can be executed with:

> $ make check

## Installation {#autotools_installation}

Once netCDF has been built and tested, it may be installed using the following command:

> $ make install


