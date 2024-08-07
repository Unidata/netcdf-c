Build Instructions for NetCDF-C using autoconf/automake/libtool {#netCDF-autotools}
===========================================

[TOC]

# Overview {#cmake_overview}

Starting with netCDF-C 4.3.0, we are happy to announce the inclusion of CMake support.  CMake will allow for building netCDF on a wider range of platforms, include Microsoft Windows with Visual Studio.  CMake support also provides robust unit and regression testing tools.  We will also maintain the standard autotools-based build system in parallel.

In addition to providing new build options for netCDF-C, we will also provide pre-built binary downloads for the shared versions of netCDF for use with Visual Studio.  

		
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
Specify Install Location | --prefix=PREFIX | -D"CMAKE\_INSTALL\_PREFIX=PREFIX"
Enable/Disable netCDF-4 | --enable-netcdf-4<br>--disable-netcdf-4 | -D"ENABLE\_NETCDF\_4=ON" <br> -D"ENABLE\_NETCDF\_4=OFF"
Enable/Disable DAP | --enable-dap <br> --disable-dap | -D"ENABLE\_DAP=ON" <br> -D"ENABLE\_DAP=OFF"
Enable/Disable Utilities | --enable-utilities <br> --disable-utilities | -D"BUILD\_UTILITIES=ON" <br> -D"BUILD\_UTILITIES=OFF"
Specify shared/Static Libraries | --enable-shared <br> --enable-static | -D"BUILD\_SHARED\_LIBS=ON" <br> -D"BUILD\_SHARED\_LIBS=OFF"
Enable/Disable Tests | --enable-testsets <br> --disable-testsets | -D"ENABLE\_TESTS=ON" <br> -D"ENABLE\_TESTS=OFF"
Specify a custom library location | Use *CFLAGS* and *LDFLAGS* | -D"CMAKE\_PREFIX\_PATH=/usr/custom_libs/"

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

## Building {#autotools_building}

The compiler can be executed directly with 'make'.

> $ make

## Testing {#autotools_testing}

Testing can be executed with:

> $ make check

## Installation {#autotools_installation}

Once netCDF has been built and tested, it may be installed using the following command:

> $ make install


