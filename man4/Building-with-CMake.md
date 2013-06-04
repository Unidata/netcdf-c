\page netCDF-CMake Build Instructions for netCDF-C using CMake

# Overview

Starting with netCDF-C 4.3, we are happy to announce the inclusion of CMake support.  CMake will allow for building netCDF on a wider range of platforms, include Microsoft Windows with Visual Studio.  CMake support also provides robust unit and regression testing tools.  We will also maintain the standard autotools-based build system in parallel.

In addition to providing new build options for netCDF-C, we will also provide pre-built binary downloads for the shared versions of netCDF for use with Visual Studio.  

# Errata

## netCDF 4.3.0

* When netCDF 4.3.0 was originally released, the file `nc-config.in.cmake` was accidentally excluded.  This file can be downloaded [here]. The file should be placed in the root of the netcdf source directory.

[here]: https://www.unidata.ucar.edu/software/netcdf/win_netcdf/nc-config.in.cmake "nc-config.in.cmake"

* When building shared libraries via CMake on Unix/Linux, the resulting files have the soversion 4.3.0.  This is a mistake, the soversion should be 7.2.0.  This can be rectified post-install by creating a symbolic link from libnetcdf.4.3.0.so to libnetcdf.7.2.0.so.  This would be accomplished thusly:
		
		> sudo ln -s /usr/local/lib/libnetcdf.4.3.0.so /usr/local/lib/libnetcdf.7.2.0.so
		
# Requirements
The following packages are required to build netCDF-C using CMake.

* netCDF-C Source Code
* CMake version 2.8.9 or greater.
* Optional Requirements:
	* HDF5 Libraries for netCDF4/HDF5 support.
	* libcurl for DAP support.

<center>
<img src="deptree.jpg" height="250px" />
</center>

# The CMake Build Process

There are four steps in the Build Process when using CMake

1. Configuration: Before compiling, the software is configured based on the desired options.
2. Building: Once configuration is complete, the libraries are compiled.
3. Testing: Post-build, it is possible to run tests to ensure the functionality of the netCDF-C libraries.
4. Installation: If all tests pass, the libraries can be installed in the location specified during configuration.

For users who prefer pre-built binaries, installation packages are available at \ref winbin

## Configuration

The output of the configuration step is a project file based on the appropriate configurator specified.  Common configurators include:

* Unix Makefiles
* Visual Studio
* CodeBlocks
* ... and others

### Common CMake Options

| **Option** | **Autotools** | **CMake** |
| :------- | :---- | :----- |
Specify Install Location | --prefix=PREFIX | -D"CMAKE\_INSTALL\_PREFIX=PREFIX"
Enable/Disable netCDF-4 | --enable-netcdf-4<br>--disable-netcdf-4 | -D"ENABLE\_NETCDF\_4=ON" <br> -D"ENABLE\_NETCDF\_4=OFF"
Enable/Disable DAP | --enable-dap <br> --disable-dap | -D"ENABLE\_DAP=ON" <br> -D"ENABLE\_DAP=OFF"
Enable/Disable Utilities | --enable-utilities <br> --disable-utilities | -D"BUILD\_UTILITIES=ON" <br> -D"BUILD\_UTILITIES=OFF"
Specify shared/Static Libraries | --enable-shared <br> --enable-static | -D"BUILD\_SHARED\_LIBS=ON" <br> -D"BUILD\_SHARED\_LIBS=OFF"
Enable/Disable Tests | --enable-testsets <br> --disable-testsets | -D"ENABLE\_TESTS=ON" <br> -D"ENABLE\_TESTS=OFF"
Specify a custom library location | Use *CFLAGS* and *LDFLAGS* | -D"CMAKE\_PREFIX\_PATH=/usr/custom_libs/"

A full list of *basic* options can be found by invoking `cmake [Source Directory] -L`. To enable a list of *basic* and *advanced* options, one would invoke `cmake [Source Directory] -LA`.

### Configuring your build from the command line.

The easiest configuration case would be one in which all of the dependent libraries are installed on the system path (in either Unix/Linux or Windows) and all the default options are desired. From the build directory (often, but not required to be located within the source directory):

> \> cmake [Source Directory]

If you have libraries installed in a custom directory, you may need to specify the **CMAKE\_PREFIX_PATH** variable to tell cmake where the libraries are installed. For example:

> \> cmake [Source Directory] -DCMAKE\_PREFIX\_PATH=/usr/custom_libraries/



## Building

The compiler can be executed directly with 'make' or the appropriate command for the configurator which was used.  

> \> make

Building can also be executed indirectly via cmake:

> \> cmake --build [Build Directory]



## Testing

Testing can be executed several different ways:

> \> make test

or

> \> ctest

or

> \> cmake --build [Build Directory] --target test

## Installation

Once netCDF has been built and tested, it may be installed using the following commands:

> \> make install

or

> \> cmake --build [Build Directory] --target install

# CMake Frequently Asked Questions (FAQ)

* **How do I specify how to build a shared or static library?**

		-DBUILD_SHARED_LIBS=[ON/OFF]
		
* **Can I build both shared and static libraries at the same time?**

		Not at this time; it is required to instead build first one version, and then the other, if you need both.
		

* **How can I specify linking against a particular library?**

		It depends on the library.  To specify a custom ZLib, for example, you would do the following:
			-DZLIB_LIBRARY=/path/to/my/zlib.lib
			
		HDF5 is more complex, since it requires both the hdf5 and hdf5_hl libraries. You would specify custom HDF5 libraries as follows:
		
			* -DHDF5_LIB=/path/to/hdf5.lib
			* -DHDF5_HL_LIB=/path/to/hdf5_hl.lib
			* -DHDF5_INCLUDE_DIR=/path/to/hdf5/include

		Alternatively, you may specify 
			* -DHDF5_LIBRARIES="/path/to/hdf5.lib;/path/to/hdf5_hl.lib" -DHDF5_INCLUDE_DIRS=/path/to/hdf5/include/
			
* **What if I want to link against multiple libraries in a non-standard location?**
	
		You can specify the path to search when looking for dependencies and header files using the CMAKE_PREFIX_PATH variable:
	
	
		> cmake [Source Directory] -DCMAKE_PREFIX_PATH=c:\shared\libs\
		or
		> cmake [Source Directory] -DCMAKE_PREFIX_PATH=/usr/custom_library_locations/		

			
* **How can I see the options available to CMake?**

		> cmake [path to source tree] -L	- This will show the basic options.
		> cmake [path to source tree] -LA	- This will show the basic and advanced options.
