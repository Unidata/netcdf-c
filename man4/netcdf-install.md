\page netcdf-install-texi NetCDF Install (netCDF 4.1.3)

  ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------- ------------ ------------------------------------ ----------------------------------

NetCDF Installation and Porting Guide {.settitle}
=====================================

This document describes how to build and install the netCDF library,
version No value for VERSION on Unix and Windows systems. This document
was last updated on No value for UPDATED.

The current stable release of netCDF, version 4.1.3, can be obtained
from the netCDF web page at
[http://www.unidata.ucar.edu/netcdf](http://www.unidata.ucar.edu/netcdf).
Instructions for installing the current stable release version of netCDF
can be found at
[http://www.unidata.ucar.edu/netcdf/docs](http://www.unidata.ucar.edu/netcdf/docs).

If netCDF does not build and pass all tests, and you don’t find your
computing platform addressed in this document, then try
[http://www.unidata.ucar.edu/netcdf/other-builds.html](http://www.unidata.ucar.edu/netcdf/other-builds.html)
for reports of successful builds of this package in environments to
which we had no access.

For a brief introduction to the netCDF format and utilities see [The
NetCDF Tutorial: (netcdf-tutorial)Top](netcdf-tutorial.html#Top) section
‘Top’ in The NetCDF Tutorial.

For a complete description of the netCDF format and utilities see [The
NetCDF Users Guide: (netcdf)Top](netcdf.html#Top) section ‘Top’ in The
NetCDF Users Guide.

Programming guides are available for C (see [The NetCDF C Interface
Guide: (netcdf-c)Top](netcdf-c.html#Top) section ‘Top’ in The NetCDF C
Interface Guide), C++ (see [The NetCDF C++ Interface Guide:
(netcdf-cxx)Top](netcdf-cxx.html#Top) section ‘Top’ in The NetCDF C++
Interface Guide), Fortran 77 (see [The NetCDF Fortran 77 Interface
Guide: (netcdf-f77)Top](netcdf-f77.html#Top) section ‘Top’ in The NetCDF
Fortran 77 Interface Guide), and Fortran 90 (see [The NetCDF Fortran 90
Interface Guide: (netcdf-f90)Top](netcdf-f90.html#Top) section ‘Top’ in
The NetCDF Fortran 90 Interface Guide). All of these documents are
available from the netCDF-4 documentation page
[http://www.unidata.ucar.edu/netcdf/netcdf-4/newdocs](http://www.unidata.ucar.edu/netcdf/netcdf-4/newdocs).

Separate documentation for the netCDF Java library can be found at the
netCDF-Java website,
[http://www.unidata.ucar.edu/software/netcdf-java](http://www.unidata.ucar.edu/software/netcdf-java).

To learn more about netCDF, see the netCDF website
[http://www.unidata.ucar.edu/netcdf](http://www.unidata.ucar.edu/netcdf).

[1. Installing the NetCDF Binaries](#Binaries)

  

Getting NetCDF Binaries

[2. Quick Instructions for Installing NetCDF on
Unix](#Quick-Instructions)

  

How to Build, Quickly

[3. Building and Installing NetCDF on Unix Systems](#Building-on-Unix)

  

How to Build, with Details

[4. Using NetCDF on Unix Systems](#Using)

  

[5. Building and Installing NetCDF on Windows](#Building-on-Windows)

  

Building on Windows

[6. If Something Goes Wrong](#Build-Problems)

  

What if it Doesn’t Work?

[Index](#Combined-Index)

  

Index of Concepts

~~~~ {.menu-comment}
 — The Detailed Node Listing —

Building and Installing NetCDF on Unix Systems
~~~~

[3.1 Installation Requirements](#Requirements)

  

What’s Needed to Build NetCDF

[3.2 Specifying the Environment for Building](#Environment)

  

Setting the Build Environment

[3.3 Building on 64 Bit Platforms](#g_t64-Bit)

  

Building on 64-bit Platforms

[3.4 Building on Platforms with Parallel I/O](#parallel)

  

Building with Parallel I/O

[3.5 Running the configure Script](#Configure)

  

Running configure

[3.6 Running make](#Make)

  

[3.7 Testing the Build](#Testing)

  

[3.8 Installing NetCDF](#Installation)

  

Installing Everything

[3.9 Platform Specific Notes](#Platform-Notes)

  

Specific Platform Notes

[3.10 Additional Porting Notes](#Porting-Notes)

  

Porting Notes for New Platforms

[3.11 Contributing to NetCDF Source Code Development](#Source)

  

Working with the Source Code

~~~~ {.menu-comment}
Using NetCDF on Unix Systems
~~~~

[4.1 Using Linker Flags with NetCDF](#Linker-Flags)

  

[4.2 Using Compiler Flags with NetCDF](#Compiler-Flags)

  

[4.3 Using the nc-config Utility to Find Compiler and Linker
Flags](#nc_002dconfig)

  

~~~~ {.menu-comment}
Building and Installing NetCDF on Windows
~~~~

[5.1 Getting Prebuilt netcdf.dll](#Prebuilt-DLL)

  

Getting the Prebuilt DLLs

[5.2 Installing the DLL](#Installing-DLL)

  

Installing the DLLs

[5.3 Building netcdf.dll with VC++ 6.0](#Visual-Cplusplus)

  

Building with VC++ 6.0

[5.4 Using netcdf.dll with VC++ 6.0](#Using-DLL)

  

Using the DLLs with VC++ 6.0

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

  

Building with VC++ .NET

[5.6 Using netcdf.dll with VC++.NET](#Using-with-NET)

  

Using with VC++ .NET

~~~~ {.menu-comment}
If Something Goes Wrong
~~~~

[6.1 The Usual Build Problems](#Usual-Problems)

  

Problems which Occur Often

[6.2 Troubleshooting](#Troubleshooting)

  

Finding the Problem

[6.3 Finding Help On-line](#Finding-Help)

  

Getting Support

[6.4 Reporting Problems](#Reporting-Problems)

  

What to Send to Support

~~~~ {.menu-comment}
~~~~

* * * * *

  -------------------------------------------------- ------------------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Top "Previous section in reading order")]   [[\>](#Quick-Instructions "Next section in reading order")]       [[\<\<](#Top "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Quick-Instructions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------- ------------------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1. Installing the NetCDF Binaries {.chapter}
=================================

The easiest way to get netCDF is through a package management program,
such as rpm, yum, adept, and others. NetCDF is available from many
different repositories, including the default Red Hat and Ubuntu
repositories.

We no longer support pre-built binary distributions from Unidata.

After installing a binary distribution from one of the package
management systems, you will end up with files in 4 subdirectories, lib,
include, man, and bin.

The lib subdirectory holds the netCDF libraries (C, Fortran, and C++).
The include directory holds the necessary netcdf.h file (for C),
netcdf.inc (for Fortran), netcdfcpp.h (for C++), and the .mod files (for
Fortran 90). The bin directory holds the ncgen, ncdump, nccopy, and
nc-config utilities, and the man directory holds the netCDF
documentation.

When compiling a netCDF program, you will have to tell the linker where
to find the library (e.g. with the -L option of most C compilers), and
you will also have to tell the C pre-processor where to find the include
file (e.g. with the -I option). The nc-config utility can be used to
determine the right options to use.

If you are using shared libraries, you will also have to specify the
library location for run-time dynamic linking. See your compiler
documentation. For some general information see the netCDF FAQ “How do I
use shared libraries” at
[http://www.unidata.ucar.edu/netcdf/faq.html\#using\_shared](http://www.unidata.ucar.edu/netcdf/faq.html#using_shared).

* * * * *

  ------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Binaries "Previous section in reading order")]   [[\>](#Building-NetCDF-Without-HDF5 "Next section in reading order")]       [[\<\<](#Binaries "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Building-on-Unix "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2. Quick Instructions for Installing NetCDF on Unix {.chapter}
===================================================

Who has time to read long installation manuals these days?

When building netCDF-4, you must first decide whether to support the use
of HDF5 as a storage format.

* * * * *

  ----------------------------------------------------------------- -------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Quick-Instructions "Previous section in reading order")]   [[\>](#Building-NetCDF-With-HDF5 "Next section in reading order")]       [[\<\<](#Quick-Instructions "Beginning of this chapter or previous chapter")]   [[Up](#Quick-Instructions "Up section")]   [[\>\>](#Building-on-Unix "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- -------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.1 Building NetCDF Without HDF5 {.section}
--------------------------------

If you don’t want netCDF-4/HDF5, then build like this:

 

~~~~ {.example}
./configure --prefix=/home/ed/local --disable-netcdf-4
make check install
~~~~

(Replace “/home/ed/local” with the name of the directory where netCDF is
to be installed.)

If you get the message that netCDF installed correctly, then you are
done!

* * * * *

  --------------------------------------------------------------------------- --------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-NetCDF-Without-HDF5 "Previous section in reading order")]   [[\>](#Building-with-HDF4-Support "Next section in reading order")]       [[\<\<](#Quick-Instructions "Beginning of this chapter or previous chapter")]   [[Up](#Quick-Instructions "Up section")]   [[\>\>](#Building-on-Unix "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- --------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.2 Building NetCDF With HDF5 {.section}
-----------------------------

If you want to use the HDF5 storage format, you must have the HDF5 1.8.6
release. You must also have the zlib compression library, version 1.2.5.
Both of these packages are available from the netCDF-4 ftp site at
[ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4](ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4).

Make sure you run “make check” for the HDF5 and zlib distributions. They
are very well-behaved distributions, but sometimes the build doesn’t
work (perhaps because of something subtly misconfigured on the target
machine). If one of these libraries is not working, netCDF will have
serious problems.

Optionally, you can also build netCDF-4 with the szip 2.0 library
(a.k.a. szlib). NetCDF cannot create szipped data files, but can read
HDF5 data files that have used szip.

There are license restrictions on the use of szip, see the HDF5 web
page:
[http://hdf.ncsa.uiuc.edu/doc\_resource/SZIP/Commercial\_szip.html](http://hdf.ncsa.uiuc.edu/doc_resource/SZIP/Commercial_szip.html).
These license restrictions seem to apply to commercial users who are
writing data. (Data readers are not restricted.) But here at NetCDF
World Headquarters, in Sunny Boulder, Colorado, there are no lawyers,
only programmers, so please read the szip documents for the license
agreement to see how it applies to your situation.

If you wish to use szip, get it from the HDF5 download page:
[http://hdfgroup.org/HDF5//HDF5/release/beta/obtain518.html](http://hdfgroup.org/HDF5//HDF5/release/beta/obtain518.html).

If “make check” fails for either zlib or HDF5, the problem must be
resolved before the netCDF-4 installation can continue. For HDF5
problems, send email to the HDF5 help desk: help@hdfgroup.org.

Build zlib like this:

 

~~~~ {.example}
./configure --prefix=/home/ed/local
make check install
~~~~

Then you build HDF5, specifying the location of the zlib library:

 

~~~~ {.example}
./configure --with-zlib=/home/ed/local --prefix=/home/ed/local 
make check install
~~~~

Note that for shared libraries, you may need to add the install
directory to the LD\_LIBRARY\_PATH environment variable. See the FAQ for
more details on using shared libraries:
[http://www.unidata.ucar.edu/netcdf/faq.html](http://www.unidata.ucar.edu/netcdf/faq.html).

If you are building HDF5 with szip, then include the –with-szlib=
option, with the directory holding the szip library.

After HDF5 is done, build netcdf, specifying the location of the HDF5,
zlib, and (if built into HDF5) the szip header files and libraries in
the CPPFLAGS and LDFLAGS environment variables.

 

~~~~ {.example}
CPPFLAGS=-I/home/ed/local/include LDFLAGS=-L/home/ed/local/lib ./configure --prefix=/home/ed/local
make check install
~~~~

The configure script will try to find necessary tools in your path. When
you run configure you may optionally use the –prefix argument to change
the default installation directory. The above examples install the zlib,
HDF5, and netCDF-4 libraries in /home/ed/local/lib, the header file in
/home/ed/local/include, and the utilities in /home/ed/local/bin.

The default install root is /usr/local (so there’s no need to use the
prefix argument if you want the software installed there).

If HDF5 and zlib are found on your system, they will be used by netCDF
in the build. To prevent this use the –disable-netcdf-4 argument to
configure.

For static build, to use netCDF-4 you must link to all the libraries,
netCDF, HDF5, zlib, and (if used with HDF5 build) szip. This will mean
-L options to your build for the locations of the libraries, and -l
(lower-case L) for the names of the libraries.

For example, one user reports that she can build other applications with
netCDF-4 by setting the LIBS envoronment variable:

 

~~~~ {.example}
LIBS='-L/X/netcdf-4.0/lib -lnetcdf -L/X/hdf5-1.8.6/lib -lhdf5_hl -lhdf5 -lz -lm -L/X/szip-2.1/lib -lsz'
~~~~

For shared builds, only -lnetcdf is needed. All other libraries will be
found automatically.

The nc-config command can be used to learn what options are needed for
the local netCDF installation.

* * * * *

  ------------------------------------------------------------------------ ----------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-NetCDF-With-HDF5 "Previous section in reading order")]   [[\>](#Building-on-Unix "Next section in reading order")]       [[\<\<](#Quick-Instructions "Beginning of this chapter or previous chapter")]   [[Up](#Quick-Instructions "Up section")]   [[\>\>](#Building-on-Unix "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ ----------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.3 Building with HDF4 Support {.section}
------------------------------

The netCDF-4 library can (since version 4.1) read HDF4 data files, if
they were created with the SD (Scientific Data) API. To enable this
feature, use the –enable-hdf4 option. The location for the HDF4 header
files and library must be set in the CPPFLAGS and LDFLAGS options.

* * * * *

  ------------------------------------------------------------------------- ------------------------------------------------------- --- ------------------------------------------------------------------------------- --------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-with-HDF4-Support "Previous section in reading order")]   [[\>](#Requirements "Next section in reading order")]       [[\<\<](#Quick-Instructions "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- ------------------------------------------------------- --- ------------------------------------------------------------------------------- --------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3. Building and Installing NetCDF on Unix Systems {.chapter}
=================================================

The latest version of this document is available at
[http://www.unidata.ucar.edu/netcdf/docs/netcdf-install](http://www.unidata.ucar.edu/netcdf/docs/netcdf-install).

This document contains instructions for building and installing the
netCDF package from source on various platforms. Prebuilt binary
releases are (or soon will be) available for various platforms from
[http://www.unidata.ucar.edu/downloads/netcdf/index.jsp](http://www.unidata.ucar.edu/downloads/netcdf/index.jsp).

A good general tutorial on how software is built from source on Linux
platforms can me found at
[http://www.tuxfiles.org/linuxhelp/softinstall.html](http://www.tuxfiles.org/linuxhelp/softinstall.html).

  ---------------------------------------------------------------- ---- ---------------------------------
  [3.1 Installation Requirements](#Requirements)                        What’s Needed to Build NetCDF
  [3.2 Specifying the Environment for Building](#Environment)           Setting the Build Environment
  [3.3 Building on 64 Bit Platforms](#g_t64-Bit)                        Building on 64-bit Platforms
  [3.4 Building on Platforms with Parallel I/O](#parallel)              Building with Parallel I/O
  [3.5 Running the configure Script](#Configure)                        Running configure
  [3.6 Running make](#Make)                                             
  [3.7 Testing the Build](#Testing)                                     
  [3.8 Installing NetCDF](#Installation)                                Installing Everything
  [3.9 Platform Specific Notes](#Platform-Notes)                        Specific Platform Notes
  [3.10 Additional Porting Notes](#Porting-Notes)                       Porting Notes for New Platforms
  [3.11 Contributing to NetCDF Source Code Development](#Source)        Working with the Source Code
  ---------------------------------------------------------------- ---- ---------------------------------

* * * * *

  --------------------------------------------------------------- ------------------------------------------------------ --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-on-Unix "Previous section in reading order")]   [[\>](#Environment "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- ------------------------------------------------------ --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.1 Installation Requirements {.section}
-----------------------------

If you wish to build from source on a Windows (Win32) platform,
different instructions apply. See section [Building and Installing
NetCDF on Windows](#Building-on-Windows).

Depending on the platform, you may need up to 25 Mbytes of free space to
unpack, build, and run the tests. You will also need a Standard C
compiler. If you have compilers for FORTRAN 77, FORTRAN 90, or C++, the
corresponding netCDF language interfaces may also be built and tested.
Compilers and associated tools will only be found if they are in your
path, or if you specify the path and compiler in the appropriate
environment variable. (Example for csh: setenv CC /some/directory/cc).

If you want to run the large file tests, you will need about 13 GB of
free disk space, as some very large files are created. The created files
are immediately deleted after the tests complete. These large file tests
are not run unless the –enable-large-file-tests option is used with
configure. (The –with-temp-large option may also be used to specify a
directory to create the large files in).

Unlike the output from other netCDF test programs, each large test
program deletes its output before successfully exiting.

To use the netCDF-4 features you will also need to have a HDF5-1.8.6
release installed. HDF5, in turn, must have been built with zlib,
version 1.2.5.

A tested version of HDF5 and zlib can be found at the netCDF-4 ftp site
at
[ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4](ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4).

For more information about HDF5 see the HDF5 web site at
[http://hdfgroup.org/HDF5/](http://hdfgroup.org/HDF5/). For more
information about zlib see the zlib web site at
[http://www.zlib.net](http://www.zlib.net).

To use the DAP features you will also need to have a version of libcurl
(version 7.18.0 or later) installed. Depending on how this library was
built, you may also need zib (version 1.2.5 or later). Information about
libcurl may be obtained at [http://curl.haxx.se](http://curl.haxx.se).

* * * * *

  ----------------------------------------------------------- --------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Requirements "Previous section in reading order")]   [[\>](#Variable-Description-Notes "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- --------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.2 Specifying the Environment for Building {.section}
-------------------------------------------

The netCDF configure script searches your path to find the compilers and
tools it needed. To use compilers that can’t be found in your path, set
their environment variables.

The configure script will use gcc and associated GNU tools if they are
found. Many users, especially those with performance concerns, will wish
to use a vendor supplied compiler.

For example, on an AIX system, users may wish to use xlc (the AIX
compiler) in one of its many flavors. Set environment variables before
the build to achieve this.

For example, to change the C compiler, set CC to xlc (in sh: export
CC=xlc). (But don’t forget to also set CXX to xlC, or else configure
will try to use g++, the GNU C++ compiler to build the netCDF C++ API.
Similarly set FC to xlf90 so that the Fortran APIs are built properly.)

By default, the netCDF library is built with assertions turned on. If
you wish to turn off assertions, set CPPFLAGS to -DNDEBUG (csh ex:
setenv CPPFLAGS -DNDEBUG).

If GNU compilers are used, the configure script sets CPPFLAGS to “-g
-O2”. If this is not desired, set CPPFLAGS to nothing, or to whatever
other value you wish to use, before running configure.

For cross-compiles, the following environment variables can be used to
override the default fortran/C type settings like this (in sh):

 

~~~~ {.example}
export NCBYTE_T=''integer(selected_int_kind(2))''
export NCSHORT_T=''integer*2''
export NF_INT1_T=''integer(selected_int_kind(2))''
export NF_INT2_T=''integer*2''
export NF_INT1_IS_C_SHORT=1
export NF_INT2_IS_C_SHORT=1
export NF_INT_IS_C_INT=1
export NF_REAL_IS_C_FLOAT=1
export NF_DOUBLEPRECISION_IS_C_DOUBLE=1
~~~~

In this case you will need to run configure with
–disable-fortran-compiler-check and –disable-fortran-type-check.

* * * * *

  ---------------------------------------------------------- ---------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Environment "Previous section in reading order")]   [[\>](#g_t64-Bit "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Environment "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- ---------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.2.1 Variable Description Notes {.subsection}

  ------------------------------------------------------- ------------------------------ ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  CC                                                      C compiler                     If you don’t specify this, the configure script will try to find a suitable C compiler. The default choice is gcc. If you wish to use a vendor compiler you must set CC to that compiler, and set other environment variables (as described below) to appropriate settings.
  FC                                                      Fortran compiler (if any)      If you don’t specify this, the configure script will try to find a suitable Fortran and Fortran 77 compiler. Set FC to "" explicitly, or provide the –disable-f77 option to configure, if no Fortran interface (neither F90 nor F77) is desired. Use –disable-f90 to disable the netCDF Fortran 90 API, but build the netCDF Fortran 77 API.
  F77                                                     Fortran 77 compiler (if any)   Only specify this if your platform explicitly needs a different Fortran 77 compiler. Otherwise use FC to specify the Fortran compiler. If you don’t specify this, the configure script will try to find a suitable Fortran compiler. For vendor compilers, make sure you’re using the same vendor’s Fortran 90 compiler. Using Fortran compilers from different vendors, or mixing vendor compilers with g77, the GNU F77 compiler, is not supported and may not work.
  CXX                                                     C++ compiler                   If you don’t specify this, the configure script will try to find a suitable C++ compiler. Set CXX to "" explicitly, or use the –disable-cxx configure option, if no C++ interface is desired. If using a vendor C++ compiler, use that vendor’s C compiler to compile the C interface. Using different vendor compilers for C and C++ may not work.
  CFLAGS                                                  C compiler flags               "-O" or "-g", for example.
  CPPFLAGS                                                C preprocessor options         "-DNDEBUG" to omit assertion checks, for example.
  FCFLAGS                                                 Fortran 90 compiler flags      "-O" or "-g", for example. These flags will be used for FORTRAN 90. If setting these you may also need to set FFLAGS for the FORTRAN 77 test programs.
  FFLAGS                                                  Fortran 77 compiler flags      "-O" or "-g", for example. If you need to pass the same arguments to the FORTRAN 90 build, also set FCFLAGS.
  CXXFLAGS                                                C++ compiler flags             "-O" or "-g", for example.
  ARFLAGS, NMFLAGS, FPP, M4FLAGS, LIBS, FLIBS, FLDFLAGS   Miscellaneous                  One or more of these were needed for some platforms, as specified below. Unless specified, you should not set these environment variables, because that may interfere with the configure script.
  ------------------------------------------------------- ------------------------------ ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

The section marked Tested Systems below contains a list of systems on
which we have built this package, the environment variable settings we
used, and additional commentary.

* * * * *

  ------------------------------------------------------------------------- --------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Variable-Description-Notes "Previous section in reading order")]   [[\>](#parallel "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- --------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.3 Building on 64 Bit Platforms {.section}
--------------------------------

The compiler options for SunOS, Irix, and AIX are listed below. The zlib
and HDF5 libraries must also be built with 64-bit options.

 `AIX`
:   Set -q64 option in all compilers, and set NMFLAGS to -X64, and
    AR\_FLAGS to ’-X64 cru’. Alternatively, set environment variable
    OBJECT\_MODE to 64 before running configure.

 `IRIX` 
:   Set the -64 option in all compilers.

 `SunOS`
:   Use the -xarch=v9 or -m64 flag on all compilers for Sparc, or -m64
    on x86 platforms.

* * * * *

  -------------------------------------------------------- ----------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#g_t64-Bit "Previous section in reading order")]   [[\>](#Building-HDF5-for-Parallel-I_002fO "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------- ----------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.4 Building on Platforms with Parallel I/O {.section}
-------------------------------------------

NetCDF makes available the parallel I/O features of HDF5 and the
parallel-netcdf libraries, allowing parallel I/O from netCDF-4 linked
programs.

* * * * *

  ------------------------------------------------------- -------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#parallel "Previous section in reading order")]   [[\>](#The-parallel_002dnetcdf-Library "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#parallel "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- -------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.4.1 Building HDF5 for Parallel I/O {.subsection}

For parallel I/O to work, HDF5 must be installed with –enable-parallel,
and an MPI library (and related libraries) must be made available to the
HDF5 configure. This can be accomplished with the mpicc wrapper script,
in the case of MPICH2.

The following works to build HDF5 with parallel I/O on our netCDF
testing system:

 

~~~~ {.example}
CC=mpicc ./configure --enable-parallel --prefix=/shecky/local_par --with-zlib=/shecky/local_par --disable-shared && make check install
~~~~

* * * * *

  --------------------------------------------------------------------------------- ---------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-HDF5-for-Parallel-I_002fO "Previous section in reading order")]   [[\>](#Building-NetCDF "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#parallel "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ---------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.4.2 The parallel-netcdf Library {.subsection}

Optionally, the parallel-netcdf library should also be installed, and
the replacement for pnetcdf.h should be copied from
ftp://ftp.unidata.ucar.edu/pub/netcdf/contrib/pnetcdf.h.

* * * * *

  ------------------------------------------------------------------------------ ---------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#The-parallel_002dnetcdf-Library "Previous section in reading order")]   [[\>](#Configure "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#parallel "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------ ---------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.4.3 Building NetCDF {.subsection}

To build netCDF with parallel I/O, build as usual, but point the
configure at a version of HDF5 that has been built for parallel I/O.

 

~~~~ {.example}
CPPFLAGS=-I/shecky/local_par/include
CXXFLAGS=-I/shecky/local_par/include
FFFLAGS=-I/shecky/local_par/include
FCFLAGS=-I/shecky/local_par/include LDFLAGS=-L/shecky/local_par/lib 
FC=mpif90 CXX=mpicxx CC=mpicc ./configure 
make check install
~~~~

To enable the parallel tests, specify –enable-parallel-tests as an
option to configure. These tests will be run as mpiexec calls. This may
not be appropriate on all systems, especially those which use some queue
for jobs.

To use parallel-netcdf to perform parallel I/O on classic and 64-bit
offset files, use the –enable-pnetcdf option.

For parallel builds the netCDF examples are not built. This is to avoid
cluttering them with MPI\_Init/Finalize calls.

* * * * *

  -------------------------------------------------------------- ----------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-NetCDF "Previous section in reading order")]   [[\>](#Make "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- ----------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.5 Running the configure Script {.section}
--------------------------------

To create the Makefiles needed to build netCDF, you must run the
provided configure script. Go to the top-level netCDF directory.

Decide where you want to install this package. Use this for the
"–prefix=" argument to the configure script below. The default
installation prefix is “/usr/local,” which will install the package’s
files in usr/local/bin, usr/local/lib, and usr/local/man. The default
can be overridden with the –prefix argument to configure.

Here’s how to execute the configure script with a different installation
directory:

 

~~~~ {.example}
    ./configure --prefix=/whatever/you/decided
~~~~

The above would cause the netCDF libraries to be installed in
/whatever/you/decided/lib, the header files in
/whatever/you/decided/include, the utilities (ncdump/ncgen) in
/whatever/you/decided/bin, and the man pages in
/whatever/you/decided/man.

If the configure script finds HDF5 in the system directories, it will
(attempt to) build the netCDF-4 enhanced features. To turn this off use
the –disable-netcdf-4 option.

There are other options for the configure script. The most useful ones
are listed below. Use the –help option to get the full list.

 `--prefix`  
:   Specify the directory under which netCDF will be installed.
    Subdirectories lib, bin, include, and man will be created there, if
    they don’t already exist.

 `--disable-netcdf-4`  
:   Turn off netCDF-4 features, even if HDF5 library is found.

 `--disable-shared`  
:   Build static libraries only.

 `--enable-dap`  
:   Enable DAP support. This flag is set by default if the configure
    script can locate a usable instance of the curl-config program. The
    curl-config program can be specified explicitly using
    –with-curl-config=/some/path/curl-config, or configure will attempt
    some heuristics to locate the curl-config program; typically by
    checking the PATH environment variable. If the flag –enable-dap flag
    is not set to either –enable-dap or –disable-dap, and a usable curl
    library can be found, then DAP support will be enabled by default.
    Note that when DAP is enabled, this can be tested for in a configure
    script by looking for the function “nc\_\_opendap”.

 `--with-curl-config`  
:   This flag may be used to specify the curl-config program so that DAP
    support can be enabled. Note that it should specify the actual
    program using something like
    –with-curl-config=/some/path/curl-config.

 `--enable-dap-remote-tests`  
:   If DAP support is enabled, then remote tests are run that utilize
    the test server at opendap.org. This option is enabled by default.
    Since that server may be inaccessible for a variety of reasons,
    these tests may fail, in which case this flag should be disabled.

 `--enable-dap-long-tests`  
:   If –enable-dap-remote-tests is enabled, then this flag can also be
    enabled to add extra tests that may take signficant time to execute.
    This flag is off by default.

 `--enable-hdf4`  
:   Turns on the HDF4 read layer. This reads HDF4 files created with the
    SD (Scientific Data) API of HDF4.

 `--enable-hdf4-file-tests`  
:   Causes make check to use wget to fetch some HDF4 data files from the
    Unidata FTP server, and check that they are properly understood.
    This is done as part of automatic netCDF testing, and should not be
    done by users.

 `--enable-pnetcdf`  
:   Allows parallel I/O with classic and 64-bit offset format files,
    using the parallel-netcdf (formerly pnetcdf) library from
    Argonne/Northwestern. The parallel-netcdf library must be installed,
    and a specially modified pnetcdf.h must be used. (Get it at
    ftp://ftp.unidata.ucar.edu/pub/netcdf/user/contrib/pnetcdf.h)

 `--with-udunits`  
:   Builds UDUNITS2 as well as netCDF. The UDUNITS2 package supports
    units of physical quantities (e.g., meters, seconds). Specifically,
    it supports conversion between string and binary representations of
    units, arithmetic manipulation of units, and conversion of numeric
    values between compatible units. For more information about UDUNITS,
    see: http://www.unidata.ucar.edu/software/udunits/

 `--disable-largefile`  
:   This omits OS support for large files (i.e. files larger than 2 GB).

 `--disable-fortran`  
:   Turns off Fortran 77 and Fortran 90 API. (Same as –disable-f77.)

 `--disable-f77`  
:   This turns off building of the F77 and F90 APIs. (The F90 API cannot
    be built without the F77 API). This also disables some of the
    configure tests that relate to fortran, including the test of the
    F90 compiler. Setting the environment variables FC or F77 to NULL
    will have the same effect as –disable-f77.

 `--disable-f90`  
:   This turns off the building of the F90 API. Setting the environment
    variable F90 to null for configure will have the same effect.

 `--disable-cxx`  
:   This turns off the building of the C++ API. Setting the environment
    variable CXX to null for configure will have the same effect.

 `--disable-v2`  
:   This turns of the V2 API. The V2 API is completely replaced with the
    V3 API, but is usually built with netCDF for backwards
    compatibility, and also because the C++ API depends on the V2 API.
    Setting this has the effect of automatically turning off the CXX
    API, as if –disable-cxx had also been specified.

 `--enable-cxx4`  
:   Turns on the new C++ API, which is currently under development, and
    will expose the full expanded model in the C++ API. The cxx4 API is
    experiemental, unfinished, and untested. It is provided for
    experiemental purposes only.

 `--enable-large-file-tests`  
:   Turn on tests for large files. These tests create files over 2 GB in
    size, and need about 13 GB of free disk space to run. These files
    are deleted after the test successfully completes. They will be
    created in the netCDF nc\_test directory, unless the
    –with-temp-large option is used to specify another location (see
    below).

 `--with-temp-large`  
:   Normally large files are not created during the netCDF build, but
    they will be if –enable-large-file-tests is specified (see above).
    In that case, this configure parameter can be used to specify a
    location to create these large files, for example:
    –with-large-files=/tmp/ed.

 `--enable-benchmarks`  
:   Turn on tests of the speed of various netCDF operations. Some of
    these operations take a long time to run (minutes, on a reasonable
    workstation).

 `--enable-valgrind-tests`  
:   Causes some tests to be re-run under valgrind, the memory testing
    tool. Valgrind must be present for this to work. Also HDF5 must be
    built with –enable-using-memchecker, and netCDF must be compiled
    without optimization (at least on the Unidata test platform where
    this is tested). The valgrind tests are run by shell script
    libsrc4/run\_valgrind\_tests.sh. It simply reruns the test programs
    in that directory, using valgrind, and with settings such that any
    error reported by valgrind will cause the “make check” to fail.

 `--disable-fortran-type-check`  
:   The netCDF configure compiles and runs some programs to test fortran
    vs. C type sizes. Setting this option turns off those test, and uses
    a set of default values (which can be overridden by environment
    variables see section [Specifying the Environment for
    Building](#Environment)).

 `--disable-examples`  
:   Starting with version 3.6.2, netCDF comes with some examples in the
    “examples” directory. By default, the examples are all built during
    a “make check” unless the –disable-examples option is provided.

 `--enable-extra-tests`  
:   This option may turn on tests which are known to fail (i.e. bugs
    that we are currently working to fix).

 `--with-default-chunk-size`  
:   Change the size (in bytes) that will be used as a target size when
    computing default chunksizes for netCDF-4/HDF5 chunked variables.

 `--default-chunks-in-cache`  
:   Change the number of chunks that are accomodated in the per-variable
    chunk caches that are used by default.

 `--max-default-cache-size`  
:   Change the maximum size of the per-variable chunk caches that are
    used by default.

 `--with-chunk-cache-size`  
:   Change the size of the default file-level chunk cache size that will
    be used when opening netCDF-4/HDF5 files.

 `--with-chunk-cache-nelems`  
:   Change the size of the default file-level chunk cache number of
    elements that will be used when opening netCDF-4/HDF5 files. Should
    be a prime number.

 `--with-chunk-cache-preemption`  
:   Change the default preemption of the file-level chunk cache that
    will be used when opening netCDF-4/HDF5 files. Must be a number
    between 0 and 1 (inclusive).

The configure script will examine your computer system – checking for
attributes that are relevant to building the netCDF package. It will
print to standard output the checks that it makes and the results that
it finds.

The configure script will also create the file "config.log", which will
contain error messages from the utilities that the configure script uses
in examining the attributes of your system. Because such an examination
can result in errors, it is expected that "config.log" will contain
error messages. Therefore, such messages do not necessarily indicate a
problem (a better indicator would be failure of the subsequent "make").
One exception, however, is an error message in "config.log" that
indicates that a compiler could not be started. This indicates a severe
problem in your compilation environment – one that you must fix. If this
occurs, configure will not complete and will exit with an error message
telling you about the problem.

* * * * *

  -------------------------------------------------------- -------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Configure "Previous section in reading order")]   [[\>](#Testing "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------- -------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.6 Running make {.section}
----------------

Run "make". This will build one or more netCDF libraries. It will build
the basic netCDF library libnetcdf.a. If you have Fortran 77 or Fortran
90 compilers, then the Fortran library will also be built
(libnetcdff.a). If you have a C++ compiler, then the C++ interface will
be built (libnetcdf\_c++.a.)

A “make” will also build the netCDF utilities ncgen(1) and ncdump(1).

Run make like this:

 

~~~~ {.example}
make
~~~~

* * * * *

  --------------------------------------------------- ------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Make "Previous section in reading order")]   [[\>](#Installation "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------- ------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.7 Testing the Build {.section}
---------------------

Run “make check” to verify that the netCDF library and executables have
been built properly (you can instead run “make test” which does the same
thing).

A make check will build and run various test programs that test the C,
Fortran, and C++ interfaces as well as the "ncdump" and "ncgen" utility
programs.

Lines in the output beginning with "\*\*\*" report on success or failure
of the tests; any failures will be reported before halting the test.
Compiler and linker warnings during the testing may be ignored.

Run the tests like this:

 

~~~~ {.example}
make check
~~~~

If you plan to use the 64-bit offset format (introduced in version
3.6.0) or the netCDF-4/HDF5 format to create very large files (i.e. with
variables larger than 2 GB), you should probably specify the
–enable-large-file-tests to configure, which tests the large file
features. You must have 13 GB of free disk space for these tests to
successfully run.

If you are running the large file tests, you may wish to use the
–with-temp-large option to specify a temporary directory for the large
files. (You may also set the environment variable TEMP\_LARGE before
running configure).

The default is to create the large files in the nc\_test subdirectory of
the netCDF build.

Run the large file tests like this:

 

~~~~ {.example}
./configure --enable-large-file-tests --with-temp-large=/home/ed/tmp
make check
~~~~

All of the large files are removed on successful completion of tests. If
the test fails, you may wish to make sure that no large files have been
left around.

If any of the the large file tests test fail, check to ensure that your
file system can handle files larger than 2 GiB by running the following
command:

 

~~~~ {.example}
 dd if=/dev/zero bs=1000000 count=3000 of=$(TEMP_LARGE)/largefile
~~~~

If your system does not have a /dev/zero, this test will fail. Then you
need to find some other way to create a file larger than 2 GiB to ensure
that your system can handle them.

See section [If Something Goes Wrong](#Build-Problems).

* * * * *

  ------------------------------------------------------ --------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Testing "Previous section in reading order")]   [[\>](#Platform-Notes "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------ --------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.8 Installing NetCDF {.section}
---------------------

To install the libraries and executables, run "make install". This will
install to the directory specified in the configure step.

Run the installation like this:

 

~~~~ {.example}
make install
~~~~

The install will put files in the following subdirectories of the
directory you provided as a prefix, creating the subdirectories if
needed:

 `lib`
:   Libraries will be installed here. If static libraries are built,
    without separate fortran libraries, then libnetcdf.a and
    libnetcdf.la will be installed. If the C++ API is built,
    libnetcdf\_c++.a and libnetcdf\_c++.la will be added. If separate
    fortran libraries are built, libnetcdff.a and libnetcdff.la will
    also be added.

    Static library users should ignore the .la files, and link to the .a
    files.

    Shared library builds will add some .so files to this directory, as
    well.

 `include`
:   Header files will be installed here. The C library header file is
    netcdf.h. If the C++ library is built, ntcdfcpp.h, ncvalues.h and
    netcdf.hh will be installed here. If the F77 API is built,
    netcdf.inc will be copied here. If the F90 API is built, the
    netcdf.mod and typesizes.mod files will be copied here as well.

 `bin`
:   Utilities ncdump and ncgen will be installed here.

 `man`
:   The ncdump/ncgen man pages will be installed in subdirectory man1,
    and the three man pages netcdf.3, netcdf\_f77.3, and netcdf\_f90.3
    will be installed in the man3 subdirectory.

 `share`
:   If the configure is called with the –enable-docs option, the netCDF
    documentation set will be built, and will be installed under the
    share directory, under the netcdf subdirectory. This will include
    postscript, PDF, info and text versions of all netCDF manuals. These
    manuals are also available at the netCDF web site.

Try linking your applications. Let us know if you have problems (see
section [Reporting Problems](#Reporting-Problems)).

* * * * *

  ----------------------------------------------------------- ---------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Installation "Previous section in reading order")]   [[\>](#AIX "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- ---------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.9 Platform Specific Notes {.section}
---------------------------

The following platform-specific note may be helpful when building and
installing netCDF. Consult your vendor manuals for information about the
options listed here. Compilers can change from version to version; the
following information may not apply to your platform.

Full output from some of the platforms of the test platforms for netCDF
No value for VERSION can be found at
[http://www.unidata.ucar.edu/netcdf/builds](http://www.unidata.ucar.edu/netcdf/builds).

* * * * *

  ------------------------------------------------------------- ------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Platform-Notes "Previous section in reading order")]   [[\>](#Cygwin "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.1 AIX {.subsection}

We found the vendor compilers in /usr/vac/bin, and included this in our
PATH. Compilers were xlc, xlf, xlf90, xlC.

The F90 compiler requires the qsuffix option to believe that F90 code
files can end with .f90. This is automatically turned on by configure
when needed:

 

~~~~ {.example}
    FCFLAGS=-qsuffix=f=f90
~~~~

We had to use xlf for F77 code, and xlf90 for F90 code.

To compile 64-bit code, set the appropriate environment variables
(documented below).

The environment variable OBJECT\_MODE can be set to 64, or use the -q64
option on all AIX compilers by setting CFLAGS, FFLAGS, and CXXFLAGS to
-q64.

The following is also necessary on an IBM AIX SP system for 64-bit mode:

 

~~~~ {.example}
    AR_FLAGS='-X64 cru'
    NMFLAGS='-X64'
~~~~

There are thread-safe versions of the AIX compilers. For example, xlc\_r
is the thread-safe C compiler. To use thread-safe compilers, override
the configure script by setting CC to xlc\_r; similarly for FC and CXX.

For large file support, AIX requires that the macro \_LARGE\_FILES be
defined. The configure script does this using AC\_SYS\_LARGEFILES.
Unfortunately, this misfires when OBJECT\_MODE is 64, or the q64 option
is used. The netCDF tries to fix this by turning on \_LARGE\_FILES
anyway in these cases.

The GNU C compiler does not mix successfully with the AIX fortran
compilers.

* * * * *

  -------------------------------------------------- ----------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#AIX "Previous section in reading order")]   [[\>](#HPUX "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------- ----------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.2 Cygwin {.subsection}

NetCDF builds under Cygwin tools on Windows just as with Linux.

* * * * *

  ----------------------------------------------------- ----------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Cygwin "Previous section in reading order")]   [[\>](#Irix "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- ----------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.3 HPUX {.subsection}

The HP Fortran compiler (f77, a.k.a. fort77, also f90) requires FLIBS to
include -lU77 for the fortran tests to work. The configure script does
this automatically.

For the c89 compiler to work, CPPFLAGS must include -D\_HPUX\_SOURCE.
This isn’t required for the cc compiler. The configure script adds this
as necessary.

For large file support, HP-UX requires \_FILE\_OFFSET\_BITS=64. The
configure script sets this automatically.

The HPUX C++ compiler doesn’t work on netCDF code. It’s too old for
that. So either use GNU to compile netCDF, or skip the C++ code by
setting CXX to ” (in csh: setenv CXX ”).

Building a 64 bit version may be possible with the following settings:

 

~~~~ {.example}
    CC=/bin/cc
    CPPFLAGS='-D_HPUX_SOURCE -D_FILE_OFFSET_BITS=64'    # large file support
    CFLAGS='-g +DD64'                           # 64-bit mode
    FC=/opt/fortran90/bin/f90                   # Fortran-90 compiler
    FFLAGS='-w +noppu +DA2.0W'                  # 64-bit mode, no "_" suffixes
    FLIBS=-lU77
    CXX=''                                      # no 64-bit mode C++ compiler
~~~~

Sometimes quotas or configuration causes HPUX disks to be limited to 2
GiB files. In this cases, netCDF cannot create very large files. Rather
confusingly, HPUX returns a system error that indicates that a value is
too large to be stored in a type. This may cause scientists to earnestly
check for attempts to write floats or doubles that are too large. In
fact, the problem seems to be an internal integer problem, when the
netCDF library attempts to read beyond the 2 GiB boundary. To add to the
confusion, the boundary for netCDF is slightly less than 2 GiB, since
netCDF uses buffered I/O to improve performance.

* * * * *

  --------------------------------------------------- ------------------------------------------------ --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#HPUX "Previous section in reading order")]   [[\>](#Linux "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------- ------------------------------------------------ --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.4 Irix {.subsection}

A 64-bit version can be built by setting the appropriate environment
variables.

Build 64-bit by setting CFLAGS, FFLAGS, and CXXFLAGS to -64.

On our machine, there is a /bin/cc and a /usr/bin/cc, and the -64 option
only works with the former.

* * * * *

  --------------------------------------------------- ---------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Irix "Previous section in reading order")]   [[\>](#Macintosh "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------- ---------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.5 Linux {.subsection}

The gFortran flag is required with GNU fortran:

 

~~~~ {.example}
    CPPFLAGS=-DgFortran
~~~~

For Portland Group Fortran, set pgiFortran instead:

 

~~~~ {.example}
    CPPFLAGS=-DpgiFortran
~~~~

Portland Group F90/F95 does not mix with GNU g77.

The netCDF configure script should notice which fortran compiler is
being used, and set these automatically.

For large file support, \_FILE\_OFFSET\_BITS must be set to 64. The
netCDF configure script should set this automatically.

* * * * *

  ---------------------------------------------------- ----------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Linux "Previous section in reading order")]   [[\>](#OSF1 "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------- ----------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.6 Macintosh {.subsection}

The gFortran flag is required with GNU fortran (CPPFLAGS=-DgFortran).
The NetCDF configure script should and set this automatically.

For IBM compilers on the Mac, the following may work (we lack this test
environment):

 

~~~~ {.example}
    CC=/usr/bin/cc
    CPPFLAGS=-DIBMR2Fortran
    F77=xlf
    FC=xlf90
    FCFLAGS=-qsuffix=cpp=f90
~~~~

* * * * *

  -------------------------------------------------------- ------------------------------------------------ --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Macintosh "Previous section in reading order")]   [[\>](#SunOS "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------- ------------------------------------------------ --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.7 OSF1 {.subsection}

NetCDF builds out of the box on OSF1.

* * * * *

  --------------------------------------------------- --------------------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#OSF1 "Previous section in reading order")]   [[\>](#Handling-Fortran-Compilers "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------- --------------------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.8 SunOS {.subsection}

PATH should contain /usr/ccs/bin to find make, nm, ar, etc.

For large file support, \_FILE\_OFFSET\_BITS must be 64, also
\_LARGEFILE64\_SOURCE and \_LARGEFILE\_SOURCE must be set. Configure
will turn this on automatically for netCDF, but not for HDF5. So when
building HDF5, set these variables before running configure, or HDF5
will not be capable of producing large files.

To compile in 64-bit mode, set -m64 (formerly -xarch=v9, which works on
SPARC platforms only) on all compilers (i.e. in CFLAGS, FFLAGS, FCFLAGS,
and CXXFLAGS).

When compiling with GNU Fortran (g77), the -DgFortran flag is required
for the Fortran interface to work. The NetCDF configure script turns
this on automatically if needed.

* * * * *

  ---------------------------------------------------- -------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#SunOS "Previous section in reading order")]   [[\>](#Porting-Notes "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Platform-Notes "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------- -------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.9.9 Handling Fortran Compilers {.subsection}

Commercial fortran compilers will generally require at least one flag in
the CPPFLAGS variable. The netCDF configure script tries to set this for
you, but won’t try if you have used –disable-flag-setting, or if you
have already set CPPFLAGS, CFLAGS, CXXFLAGS, FCFLAGS, or FFLAGS
yourself.

The first thing to try is to set nothing and see if the netCDF configure
script finds your fortran compiler, and sets the correct flags
automatically.

If it doesn’t find the correct fortran compiler, you can next try
setting the FC environment variable to the compiler you wish to use, and
then see if the configure script can set the correct flags for that
compiler.

If all that fails, you must set the flags yourself.

The Intel compiler likes the pgiFortran flag, as does the Portland Group
compiler. (Automatically turned on if your fortran compiler is named
“ifort” or “pgf90”).

Alternatively, Intel has provided a web page on “Building netCDF with
the Intel compilers” at
[http://www.intel.com/support/performancetools/sb/CS-027812.htm](http://www.intel.com/support/performancetools/sb/CS-027812.htm),
providing instructions for building netCDF (without using the pgiFortran
flag).

The Portland Group also has a “PGI Guide to NetCDF” at
[http://www.pgroup.com/resources/netcdf/netcdf362\_pgi71.htm](http://www.pgroup.com/resources/netcdf/netcdf362_pgi71.htm).

* * * * *

  ------------------------------------------------------------------------- ------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Handling-Fortran-Compilers "Previous section in reading order")]   [[\>](#Source "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- ------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.10 Additional Porting Notes {.section}
-----------------------------

The configure and build system should work on any system which has a
modern "sh" shell, "make", and so on. The configure and build system is
less portable than the "C" code itself, however. You may run into
problems with the "include" syntax in the Makefiles. You can use GNU
make to overcome this, or simply manually include the specified files
after running configure.

Instruction for building netCDF on other platforms can be found at
[http://www.unidata.ucar.edu/netcdf/other-builds.html](http://www.unidata.ucar.edu/netcdf/other-builds.html).
If you build netCDF on a new platform, please send your environment
variables and any other important notes to
support-netcdf@unidata.ucar.edu and we will add the information to the
other builds page, with a credit to you.

If you can’t run the configure script, you will need to create config.h
and fortran/nfconfig.inc. Start with ncconfig.in and fortran/nfconfig.in
and set the defines as appropriate for your system.

Operating system dependency is isolated in the "ncio" module. We provide
two versions. posixio.c uses POSIX system calls like "open()", "read()"
and "write(). ffio.c uses a special library available on CRAY systems.
You could create other versions for different operating systems. The
program "t\_ncio.c" can be used as a simple test of this layer.

Note that we have not had a Cray to test on for some time. In
particular, large file support is not tested with ffio.c.

Numerical representation dependency is isolated in the "ncx" module. As
supplied, ncx.m4 (ncx.c) supports IEEE floating point representation,
VAX floating point, and CRAY floating point. BIG\_ENDIAN vs
LITTLE\_ENDIAN is handled, as well as various sizes of "int", "short",
and "long". We assume, however, that a "char" is eight bits.

There is a separate implementation of the ncx interface available as
ncx\_cray.c which contains optimizations for CRAY vector architectures.
Move the generic ncx.c out of the way and rename ncx\_cray.c to ncx.c to
use this module. By default, this module does not use the IEG2CRAY and
CRAY2IEG library calls. When compiled with aggressive in-lining and
optimization, it provides equivalent functionality with comparable speed
and clearer error semantics. If you wish to use the IEG library
functions, compile this module with -DUSE\_IEG.

* * * * *

  ------------------------------------------------------------ ------------------------------------------------ --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Porting-Notes "Previous section in reading order")]   [[\>](#Using "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Unix "Up section")]   [[\>\>](#Using "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ------------------------------------------------ --- ----------------------------------------------------------------------------- ---------------------------------------- --------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.11 Contributing to NetCDF Source Code Development {.section}
---------------------------------------------------

Most users will not be interested in working directly with the netCDF
source code. Rather, they will write programs which call netCDF
functions, and delve no further. However some intrepid users may wish to
dig into the netCDF code to study it, to try and spot bugs, or to make
modifications for their own purposes.

To work with the netCDF source code, several extra utilities are
required to fully build everything from source. If you are going to
modify the netCDF source code, you will need some or all of the
following Unix tools.

 `m4`  
:   Macro processing language used heavily in libsrc, nc\_test.
    Generates (in these cases) C code from m4 source. Version 1.4 works
    fine with release 3.5.1 through 3.6.2.

 `flex and yacc`  
:   Used in ncgen directory to parse CDL files. Generates C files from
    .y and .l files. You only need to use this to modify ncgen’s
    understanding of CDL grammar.

 `makeinfo`  
:   Generates all documentation formats (except man pages) from texinfo
    source. I’m using makeinfo version 4.8, as of release 3.6.2. If you
    have trouble with makeinfo, upgrade to this version and try again.
    You only need makeinfo if you want to modify the documentation.

 `tex`  
:   Knuth’s venerable typesetting system. The version I am running (for
    netCDF release 3.6.2) is TeX 3.141592 (Web2C 7.5.4). I have found
    that some recent installations of TeX will not build the netCDF
    documentation - it’s not clear to me why.

    The user generally will just want to download the latest version of
    netCDF documents at the netCDF website.
    [http://www.unidata.ucar.edu/netcdf/docs](http://www.unidata.ucar.edu/netcdf/docs).

 `autoconf`  
:   Generates the configure script. Version 2.59 or later is required.

 `automake`  
:   Since version 3.6.2 of netCDF, automake is used to generate the
    Makefile.in files needed by the configure script to build the
    Makefiles.

 `libtool`  
:   Since version 3.6.2 of netCDF, libtool is used to help generate
    shared libraries platforms which support them. Version 2.1a of
    libtool is required.

 `sed`  
:   This text processing tool is used to process some of the netCDF
    examples before they are included in the tutorial. This is only
    needed to build the documentation, which the user generally will not
    need to do.

NetCDF has a large and enterprising user community, and a long history
of accepting user modifications into the netCDF code base. Examples
include the 64-bit offset format, and the F90 API.

All suggested changes and additions to netCDF code can be sent to
support-netcdf@unidata.ucar.edu.

* * * * *

  ----------------------------------------------------- ------------------------------------------------------- --- ----------------------------------------------------------------------------- --------------------------- ----------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Source "Previous section in reading order")]   [[\>](#Linker-Flags "Next section in reading order")]       [[\<\<](#Building-on-Unix "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Building-on-Windows "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- ------------------------------------------------------- --- ----------------------------------------------------------------------------- --------------------------- ----------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4. Using NetCDF on Unix Systems {.chapter}
===============================

To use netCDF you must link to the netCDF library, and, if using the
netCDF-4/HDF5 features, also two HDF5, at least one compression library,
and (on some systems) the math library.

  ------------------------------------------------------------------------------------- ---- --
  [4.1 Using Linker Flags with NetCDF](#Linker-Flags)                                        
  [4.2 Using Compiler Flags with NetCDF](#Compiler-Flags)                                    
  [4.3 Using the nc-config Utility to Find Compiler and Linker Flags](#nc_002dconfig)        
  ------------------------------------------------------------------------------------- ---- --

* * * * *

  ---------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ----------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Using "Previous section in reading order")]   [[\>](#Compiler-Flags "Next section in reading order")]       [[\<\<](#Using "Beginning of this chapter or previous chapter")]   [[Up](#Using "Up section")]   [[\>\>](#Building-on-Windows "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ----------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.1 Using Linker Flags with NetCDF {.section}
----------------------------------

For this to work, you have to tell the linker which libraries to link to
(with the -l option), and where to find them (with the -L option).

Use the -L option to your linker to pass the directories in which
netCDF, HDF5, and zlib are installed.

Use the -l (lower-case L) option to list the libraries, which must be
listed in the correct order:

 

~~~~ {.example}
-lnetcdf -lhdf5_hl -lhdf5 -lz -lm
~~~~

If szip was used when building HDF5, you must also use -lsz.

On some systems you must also include -lm for the math library.

If HDF4 was used when building netCDF, you must also use -lmfhdf -ldf
-ljpeg.

Finally, if you use the parallel-netcdf library, you must use -lpnetcdf.

The worst case scenario is, using all of the above libraries:

 

~~~~ {.example}
-lnetcdf -lpnetcdf -lmfhdf -ldf -ljpeg -lhdf5_hl -lhdf5 -lz -lsz -lm
~~~~

In such a case one also needs to provide the locations of the libraries,
with the -L flag. If libraries are installed in the same directory, this
is easier.

Use the nc-config to learn the exact flags needed on your system (see
section [Using the nc-config Utility to Find Compiler and Linker
Flags](#nc_002dconfig)).

* * * * *

  ----------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ----------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Linker-Flags "Previous section in reading order")]   [[\>](#nc_002dconfig "Next section in reading order")]       [[\<\<](#Using "Beginning of this chapter or previous chapter")]   [[Up](#Using "Up section")]   [[\>\>](#Building-on-Windows "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ----------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.2 Using Compiler Flags with NetCDF {.section}
------------------------------------

Depending on how netCDF was built, you may need to use compiler flags
when building your code. For example, many systems build both 32-bit and
64-bit binaries. The GNU C compiler, for example, uses -m32 and -m64 as
compiler flags for this purpose.

If netCDF is built with the default compiler flags (i.e. no special
flags are used), then no flags need to be used by the user.

If netCDF is built using flags that control architecture or other
important aspects of the binary output, then those flags may need to be
set by all users as well.

* * * * *

  ------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ----------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Compiler-Flags "Previous section in reading order")]   [[\>](#Building-on-Windows "Next section in reading order")]       [[\<\<](#Using "Beginning of this chapter or previous chapter")]   [[Up](#Using "Up section")]   [[\>\>](#Building-on-Windows "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ----------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.3 Using the nc-config Utility to Find Compiler and Linker Flags {.section}
-----------------------------------------------------------------

To assist with the setting of compiler and linker flags, the nc-config
utility is provided with netCDF. The nc-config utility is a very simple
script which contains the settings of the C and Fortran flags used
during the netCDF build.

For example, the nc-config command can be used to get the command line
options needed to link a C program to netCDF:

 

~~~~ {.example}
nc-config --libs
-L/usr/local/lib -lnetcdf -L/shecky/local_post/lib -lhdf5_hl -lhdf5 -lz
~~~~

The nc-config utility can also be used to learn about the features of
the current netCDF installation. For example, it can show whether
netCDF-4 is available:

 

~~~~ {.example}
./nc-config --has-nc4
yes
~~~~

Use the –help option to nc-config for a full list of available
information.

* * * * *

  ------------------------------------------------------------ ------------------------------------------------------- --- ------------------------------------------------------------------ --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_002dconfig "Previous section in reading order")]   [[\>](#Prebuilt-DLL "Next section in reading order")]       [[\<\<](#Using "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Build-Problems "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ------------------------------------------------------- --- ------------------------------------------------------------------ --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5. Building and Installing NetCDF on Windows {.chapter}
============================================

NetCDF can be built and used from a variety of development environments
on Windows. The netCDF library is implemented as a Windows dynamic link
library (DLL). The simplest way to get started with netCDF under Windows
is to download the pre-built DLL from the Unidata web site.

Building under the Cygwin port of GNU tools is treated as a Unix
install. See section [Platform Specific Notes](#Platform-Notes).

Instructions are also given for building the netCDF DLL from the source
code.

VC++ documentation being so voluminous, finding the right information
can be a chore. There’s a good discussion of using DLLs called “About
Dynamic-Link Libraries” at (perhaps)
[http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic\_link\_libraries.asp](http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic_link_libraries.asp).

From the .NET point of view, the netCDF dll is unmanaged code. As a
starting point, see the help topic “Consuming Unmanaged DLL Functions”
which may be found at
[http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpguide/html/cpconConsumingUnmanagedDLLFunctions.asp](http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpguide/html/cpconConsumingUnmanagedDLLFunctions.asp),
unless the page has been moved.

  ------------------------------------------------------------- ---- ------------------------------
  [5.1 Getting Prebuilt netcdf.dll](#Prebuilt-DLL)                   Getting the Prebuilt DLLs
  [5.2 Installing the DLL](#Installing-DLL)                          Installing the DLLs
  [5.3 Building netcdf.dll with VC++ 6.0](#Visual-Cplusplus)         Building with VC++ 6.0
  [5.4 Using netcdf.dll with VC++ 6.0](#Using-DLL)                   Using the DLLs with VC++ 6.0
  [5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)        Building with VC++ .NET
  [5.6 Using netcdf.dll with VC++.NET](#Using-with-NET)              Using with VC++ .NET
  ------------------------------------------------------------- ---- ------------------------------

* * * * *

  ------------------------------------------------------------------ --------------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-on-Windows "Previous section in reading order")]   [[\>](#Installing-DLL "Next section in reading order")]       [[\<\<](#Building-on-Windows "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Windows "Up section")]   [[\>\>](#Build-Problems "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ --------------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.1 Getting Prebuilt netcdf.dll {.section}
-------------------------------

We have pre-built Win32 binary versions of the netcdf dll and static
library, as well as ncgen.exe and ncdump.exe (dll and static versions).
You can get them from
[ftp://ftp.unidata.ucar.edu/pub/netcdf/contrib/win32/netcdf-3.6.1-beta1-win32dll.zip](ftp://ftp.unidata.ucar.edu/pub/netcdf/contrib/win32/netcdf-3.6.1-beta1-win32dll.zip).
(Note: we don’t have a C++ interface here).

* * * * *

  ----------------------------------------------------------- ----------------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Prebuilt-DLL "Previous section in reading order")]   [[\>](#Visual-Cplusplus "Next section in reading order")]       [[\<\<](#Building-on-Windows "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Windows "Up section")]   [[\>\>](#Build-Problems "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- ----------------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.2 Installing the DLL {.section}
----------------------

Whether you get the pre-built DLL or build your own, you’ll then have to
install it somewhere so that your other programs can find it and use it.

To install a DLL, you just have to leave it in some directory, and
(possibly) tell your compiler in which directory to look for it.

A DLL is a library, and functions just like libraries under the Unix
operating system. As with any library, the point of the netCDF DLL is to
provide functions that you can call from your own code. When you compile
that code, the linker needs to be able to find the library, and then it
pulls out the functions that it needs. In the Unix world, the -L option
tells the compiler where to look for a library. In Windows, library
search directories can be added to the project’s property dialog.

Similarly, you will need to put the header file, netcdf.h, somewhere
that you compiler can find it. In the Unix world, the -I option tells
the compiler to look in a certain directory to find header files. In the
Windows world, you set this in the project properties dialog box of your
integrated development environment.

Therefore, installing the library means nothing more than copying the
DLL somewhere that your compiler can find it, and telling the compiler
where to look for them.

The standard place to put DLLs is Windows\\System32 folder (for
Windows2000/XP) or the Windows\\System folder (for Windows 98/ME). If
you put the DLL there, along with the ncgen and ncdump executables, you
will be able to use the DLL and utilities without further work, because
compilers already look there for DLLs and EXEs.

Instead of putting the DLL and EXEs into the system directory, you can
leave them wherever you want, and every development project that uses
the dll will have to be told to search the netCDF directory when it’s
linking, or, the chosen directory can be added to your path.

On the .NET platform, you can also try to use the global assembly cache.
(To learn how, see MSDN topic “Global Assembly Cache”, at
[www.msdn.microsoft.com](www.msdn.microsoft.com)).

Following Windows conventions, the netCDF files belong in the following
places:

  ----------------------- ------------------------------- --------------------------------------------------------
  File(s)                 Description                     Location
  netcdf.dll              C and Fortran function in DLL   Windows\\System (98/ME) or Windows\\System32 (2000/XP)
  netcdf.lib              Library file                    Windows\\System (98/ME) or Windows\\System32 (2000/XP)
  ncgen.exe, ncdump.exe   NetCDF utilities                Windows\\System (98/ME) or Windows\\System32 (2000/XP)
  netcdf-3                netCDF source code              Program Files\\Unidata
  ----------------------- ------------------------------- --------------------------------------------------------

* * * * *

  ------------------------------------------------------------- ---------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Installing-DLL "Previous section in reading order")]   [[\>](#Using-DLL "Next section in reading order")]       [[\<\<](#Building-on-Windows "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Windows "Up section")]   [[\>\>](#Build-Problems "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ---------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.3 Building netcdf.dll with VC++ 6.0 {.section}
-------------------------------------

The most recent releases of netCDF aren’t tested under VC++ 6.0. (They
are tested with VC++.NET). Older versions of the library, notably 3.5.0,
did compile with VC++ 6.0, and the instructions for doing so are
presented below.

Note that the introduction of better large file support (for files
larger than 2 GiB) in version 3.6.0 and greater requires an off\_t type
of 8 bytes, and it’s not clear how, or if, this can be found in VC++
6.0.

To build the library yourself, get the file
ftp://ftp.unidata.ucar.edu/pub/netcdf/contrib/win32/netcdf-3.5.0.win32make.VC6.zip

The makefiles there describe how to build netcdf-3.5 using the using
Microsoft Visual C++ 6.x and (optionally) Digital Visual Fortran 6.x.
Because of difficulties in getting Microsoft Visual Studio to fall in
line with our existing source directory scheme, we chose \_not\_ to
build the system "inside" Visual Studio. Instead, we provide a simple
group of "msoft.mak" files which can be used. If you wish to work in
Visual Studio, go ahead. Read the section called "Macros" at the end of
this discussion.

As of this writing, we have not tried compiling the C++ interface in
this environment.

nmake is a Microsoft version of make, which comes with VC 6.0 (and VC
7.0) in directory C:\\Program Files\\Microsoft Visual Studio\\VC98\\Bin
(or, for VC 7.0, C:\\Program Files\\Microsoft Visual Studio .NET
2003\\Vc7\\bin).

To build netcdf, proceed as follows:

 `unpack source distribution.`\
 `copy netcdf-3.5.0.win32make.VC6.zip` 
:   copy netcdf-3.5.0.win32make.VC6.zip into the netcdf-3.5.0/src
    directory, and unzip it from there.

 `cd src\libsrc; nmake /f msoft.mak`
:   Run this command in src\\libsrc. This will build netcdf.lib and
    netcdf.dll Note: This makefiles make DLLs. To make static libraries
    see section on static libraries.

 `nmake /f msoft.mak test`
:   Optionally, in src\\libsrc, make and run the simple test.

 `cd ..\fortran; nmake /f msoft.mak`
:   Optionally build the fortran interface and rebuild dll in ..\\libsrc
    to include the fortran interface. Note Bene: We don’t provide a .DEF
    file, so this step changes the "ordinals" by which entry points in
    the DLL found. Some sites may wish to modify the msoft.mak file(s)
    to produce a separate library for the fortran interface.

 `nmake /f msoft.mak test`
:   (necessary if you want to use fortran code) While you are in
    src\\fortran; nmake /f msoft.mak test This tests the netcdf-2
    fortran interface.

 `cd ..\nctest; nmake /f msoft.mak test`
:   (optional, but recommended) In src\\nctest; nmake /f msoft.mak test
    This tests the netcdf-2 C interface.

 `cd ..\nc_test; nmake /f msoft.mak test`
:   (optional, but highly recommended) In src\\nc\_test; nmake /f
    msoft.mak test This tortures the netcdf-3 C interface.

 `cd ..\nf_test; nmake /f msoft.mak test`
:   (optional, but highly recommended if you built the fortran
    interface) In src\\nf\_test; nmake /f msoft.mak test This tortures
    the netcdf-3 fortran interface.

 `..\ncdump; nmake /f msoft.mak`
:   In src\\ncdump; nmake /f msoft.mak This makes ncdump.exe.

 `..\ncgen; nmake /f msoft.mak`
:   In src\\ncgen; nmake /f msoft.mak This makes ncgen.exe.

 `..\ncdump; nmake /f msoft.mak test`
:   (optional) In src\\ncdump; nmake /f msoft.mak test This tests
    ncdump. Both ncgen and ncdump need to be built prior to this test.
    Note the makefile sets the path so that ..\\libsrc\\netcdf.dll can
    be located.

 `..\ncgen; nmake /f msoft.mak test`
:   (optional) In src\\ncgen; nmake /f msoft.mak test This tests ncgen.
    Both ncgen and ncdump need to be built prior to this test. Note the
    makefile sets the path so that ..\\libsrc\\netcdf.dll can be
    located.

 `To Install`
:   Copy libsrc\\netcdf.lib to a LIBRARY directory. Copy
    libsrc\\netcdf.h and fortran/netcdf.inc to an INCLUDE directory.
    Copy libsrc\\netcdf.dll, ncdump/ncdump.exe, and ncgen/ncgen.exe to a
    BIN directory (someplace in your PATH).

* * * * *

  --------------------------------------------------------------- ------------------------------------------------------------ --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Visual-Cplusplus "Previous section in reading order")]   [[\>](#Building-with-NET "Next section in reading order")]       [[\<\<](#Building-on-Windows "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Windows "Up section")]   [[\>\>](#Build-Problems "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- ------------------------------------------------------------ --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.4 Using netcdf.dll with VC++ 6.0 {.section}
----------------------------------

To use the netcdf.dll:

​1. Place these in your include directory: netcdf.h C include file
netcdf.inc Fortran include file

2a. To use the Dynamic Library (shared) version of the netcdf library:
Place these in a directory that’s in your PATH: netcdf.dll library dll
ncgen.exe uses the dll ncdump.exe uses the dll

Place this in a library directory to link against: netcdf.lib library

2b. Alternatively, to use a static version of the library

Place this in a library directory to link against: netcdfs.lib library

Place these in a directory that’s in your PATH: ncgens.exe statically
linked (no DLL needed) ncdumps.exe statically linked (no DLL needed)

* * * * *

  -------------------------------------------------------- --------------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Using-DLL "Previous section in reading order")]   [[\>](#Using-with-NET "Next section in reading order")]       [[\<\<](#Building-on-Windows "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Windows "Up section")]   [[\>\>](#Build-Problems "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------- --------------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.5 Building netcdf.dll with VC++.NET {.section}
-------------------------------------

To build the netCDF dll with VC++.NET open the win32/NET/netcdf.sln file
with Visual Studio. Both Debug and Release configurations are available
- select one and build.

The resulting netcdf.dll file will be in subdirectory Release or Debug.

The netCDF tests will be built and run as part of the build process. The
Fortran 77 interface will be built, but not the Fortran 90 or C++
interfaces.

Unfortunately, different fortran compilers require different flag
settings in the netCDF configuration files. (In UNIX builds, this is
handled by the configure script.)

The quick\_large\_files test program is provided as an extra project,
however it is not run during the build process, but can be run from the
command line or the IDE. Note that, despite its name, it is not quick.
On Unix systems, this program runs in a few seconds, because of some
features of the Unix file system apparently not present in Windows.
Nonetheless, the program does run, and creates (then deletes) some very
large files. (So make sure you have at least 15 GiB of space available).
It takes about 45 minutes to run this program on our Windows machines,
so please be patient.

* * * * *

  ---------------------------------------------------------------- --------------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-with-NET "Previous section in reading order")]   [[\>](#Build-Problems "Next section in reading order")]       [[\<\<](#Building-on-Windows "Beginning of this chapter or previous chapter")]   [[Up](#Building-on-Windows "Up section")]   [[\>\>](#Build-Problems "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- --------------------------------------------------------- --- -------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.6 Using netcdf.dll with VC++.NET {.section}
----------------------------------

Load-time linking to the DLL is the most straightforward from C++. This
means the netcdf.lib file has to end up on the compile command line.
This being Windows, that’s hidden by a GUI.

In Visual Studio 2003 this can be done by modifying three of the
project’s properties.

Open the project properties window from the project menu. Go to the
linker folder and look at the general properties. Modify the property
“Additional Library Directories” by adding the directory which contains
the netcdf.dll and netcdf.lib files. Now go to the linker input
properties and set the property “Additional Dependencies” to netcdf.lib.

Finally, still within the project properties window, go to the C/C++
folder, and look at the general properties. Modify “Additional Include
Directories” to add the directory with the netcdf.h file.

Now use the netCDF functions in your C++ code. Of course any C or C++
file that wants to use the functions will need:

 

~~~~ {.example}
#include <netcdf.h>
~~~~

* * * * *

  ------------------------------------------------------------- --------------------------------------------------------- --- -------------------------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Using-with-NET "Previous section in reading order")]   [[\>](#Usual-Problems "Next section in reading order")]       [[\<\<](#Building-on-Windows "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- --------------------------------------------------------- --- -------------------------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6. If Something Goes Wrong {.chapter}
==========================

The netCDF package is designed to build and install on a wide variety of
platforms, but doesn’t always. It’s a crazy old world out there, after
all.

  ------------------------------------------------- ---- ----------------------------
  [6.1 The Usual Build Problems](#Usual-Problems)        Problems which Occur Often
  [6.2 Troubleshooting](#Troubleshooting)                Finding the Problem
  [6.3 Finding Help On-line](#Finding-Help)              Getting Support
  [6.4 Reporting Problems](#Reporting-Problems)          What to Send to Support
  ------------------------------------------------- ---- ----------------------------

* * * * *

  ------------------------------------------------------------- ------------------------------------------------------------------ --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Build-Problems "Previous section in reading order")]   [[\>](#Taking-the-Easy-Way-Out "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Build-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ------------------------------------------------------------------ --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.1 The Usual Build Problems {.section}
----------------------------

* * * * *

  ------------------------------------------------------------- --------------------------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Usual-Problems "Previous section in reading order")]   [[\>](#How-to-Clean-Up-the-Mess-from-a-Failed-Build "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Usual-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- --------------------------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.1.1 Taking the Easy Way Out {.subsection}

Why not take the easy way out if you can?

Many Linux systems contain package management programs which allow
netCDF to be installed easily. This is the prefered installation method
for netCDF.

Precompiled binaries for some platforms can be found at
[http://www.unidata.ucar.edu/downloads/netcdf/index.jsp](http://www.unidata.ucar.edu/downloads/netcdf/index.jsp).
Click on your platform, and copy the files from the bin, include, lib,
and man directories into your own local equivalents (Perhaps
/usr/local/bin, /usr/local/include, etc.).

* * * * *

  ---------------------------------------------------------------------- ------------------------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Taking-the-Easy-Way-Out "Previous section in reading order")]   [[\>](#Platforms-On-Which-NetCDF-is-Known-to-Work "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Usual-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- ------------------------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.1.2 How to Clean Up the Mess from a Failed Build {.subsection}

If you are trying to get the configure or build to work, make sure you
start with a clean distribution for each attempt. If netCDF failed in
the “make” you must clean up the mess before trying again. To clean up
the distribution:

 

~~~~ {.example}
make distclean
~~~~

* * * * *

  ------------------------------------------------------------------------------------------- ---------------------------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#How-to-Clean-Up-the-Mess-from-a-Failed-Build "Previous section in reading order")]   [[\>](#Platforms-On-Which-NetCDF-is-Reported-to-Work "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Usual-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------------- ---------------------------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.1.3 Platforms On Which NetCDF is Known to Work {.subsection}

At NetCDF World Headquarters (in sunny Boulder, Colorado), as part of
the wonderful Unidata organization, we have a wide variety of computers,
operating systems, and compilers. At night, house elves test netCDF on
all these systems.

Output for the netCDF test platforms can be found at
[http://www.unidata.ucar.edu/netcdf/builds](http://www.unidata.ucar.edu/netcdf/builds).

Compare the output of your build attempt with ours. Are you using the
same compiler? The same flags? Look for the configure output that lists
the settings of CC, FC, CXX, CFLAGS, etc.

On some systems you have to set environment variables to get the
configure and build to work.

For example, for a 64-bit IRIX install of the netCDF-3.6.2 release, the
variables are set before netCDF is configured or built. In this case we
set CFLAGS, CXXFLAGS, FCFLAGS, and FFLAGS.

 

~~~~ {.example}
flip% uname -a
IRIX64 flip 6.5 07080050 IP30 mips
flip% setenv CFLAGS -64
flip% setenv CXXFLAGS -64
flip% setenv FFLAGS -64
flip% setenv FCFLAGS -64
flip% make distclean;./configure;make check
~~~~

* * * * *

  ----------------------------------------------------------------------------------------- ------------------------------------------------------------------------ --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Platforms-On-Which-NetCDF-is-Known-to-Work "Previous section in reading order")]   [[\>](#If-You-Have-a-Broken-Compiler "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Usual-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------------- ------------------------------------------------------------------------ --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.1.4 Platforms On Which NetCDF is Reported to Work {.subsection}

If your platform isn’t listed on the successful build page, see if
another friendly netCDF user has sent in values for environment
variables that are reported to work:
([http://www.unidata.ucar.edu/netcdf/other-builds.html](http://www.unidata.ucar.edu/netcdf/other-builds.html)).

If you build on a system that we don’t have at Unidata (particularly if
it’s something interesting and exotic), please send us the settings that
work (and the entire build output would be nice too). Send them to
support-netcdf@unidata.ucar.edu.

* * * * *

  -------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Platforms-On-Which-NetCDF-is-Reported-to-Work "Previous section in reading order")]   [[\>](#What-to-Do-If-NetCDF-Still-Won_0027t-Build "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Usual-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.1.5 If You Have a Broken Compiler {.subsection}

For netCDF to build correctly, you must be able to compile C from your
environment, and, optionally, Fortran 77, Fortran 90, and C++. If C
doesn’t work, netCDF can’t compile.

What breaks a C compiler? Installation or upgrade mistakes when the C
compiler was installed, or multiple versions or compilers installed on
top of each other. Commercial compilers frequently require some
environment variables to be set, and some directories to appear ahead of
others in your path. Finally, if you have an expired or broken license,
your C compiler won’t work.

If you have a broken C compiler and a working C compiler in your PATH,
netCDF might only find the broken one. You can fix this by explicitly
setting the CC environmental variable to a working C compiler, and then
trying to build netCDF again. (Don’t forget to do a “make distclean”
first!)

If you can’t build a C program, you can’t build netCDF. Sorry, but
that’s just the way it goes. (You can get the GNU C compiler - search
the web for “gcc”).

If netCDF finds a broken Fortran 90, Fortran 77, or C++ compiler, it
will report the problem during the configure, and then drop the
associated API. For example, if the C++ compiler can’t compile a very
simple test program, it will drop the C++ interface. If you really want
the C++ API, set the CXX environment variable to a working C++ compiler.

* * * * *

  ---------------------------------------------------------------------------- ---------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#If-You-Have-a-Broken-Compiler "Previous section in reading order")]   [[\>](#Troubleshooting "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Usual-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ---------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.1.6 What to Do If NetCDF Still Won’t Build {.subsection}

If none of the above help, try our troubleshooting section: See section
[Troubleshooting](#Troubleshooting).

Also check to see of your problem has already been solved by someone
else (see section [Finding Help On-line](#Finding-Help)).

If you still can’t get netCDF to build, report your problem to Unidata,
but please make sure you submit all the information we need to help (see
section [Reporting Problems](#Reporting-Problems)).

* * * * *

  ----------------------------------------------------------------------------------------- ------------------------------------------------------------------------ --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#What-to-Do-If-NetCDF-Still-Won_0027t-Build "Previous section in reading order")]   [[\>](#Problems-During-Configuration "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Build-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------------- ------------------------------------------------------------------------ --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.2 Troubleshooting {.section}
-------------------

* * * * *

  -------------------------------------------------------------- ---------------------------------------------------------------------- --- --------------------------------------------------------------------------- --------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Troubleshooting "Previous section in reading order")]   [[\>](#Problems-During-Compilation "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Troubleshooting "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- ---------------------------------------------------------------------- --- --------------------------------------------------------------------------- --------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.2.1 Problems During Configuration {.subsection}

If the ./configure; make check fails, it’s a good idea to turn off the
C++ and Fortran interfaces, and try to build the C interface alone. All
other interfaces depend on the C interface, so nothing else will work
until the C interface works. To turn off C++ and Fortran, set
environment variables CXX and FC to NULL before running the netCDF
configure script (with csh: setenv FC ”;setenv CXX ”).

Turning off the Fortran and C++ interfaces results in a much shorter
build and test cycle, which is useful for debugging problems.

If the netCDF configure fails, most likely the problem is with your
development environment. The configure script looks through your path to
find all the tools it needs to build netCDF, including C compiler and
linker, the ar, ranlib, and others. The configure script will tell you
what tools it found, and where they are on your system. Here’s part of
configure’s output on a Linux machine:

 

~~~~ {.example}
checking CPPFLAGS... 
checking CC CFLAGS... cc -g -O2
checking type cc... cc is /usr/bin/cc
checking CXX... c++
checking CXXFLAGS... -g -O2
checking type c++... c++ is /usr/bin/c++
checking FC... gfortran
checking FFLAGS... -g -O2
checking type gfortran... gfortran is /usr/bin/gfortran
checking F90... gfortran
checking FCFLAGS... -g -O2
checking type gfortran... gfortran is /usr/bin/gfortran
checking AR... ar
checking AR_FLAGS... cru
checking type ar... ar is /usr/bin/ar
checking NM... /usr/bin/nm -B
checking NMFLAGS... 
checking for /usr/bin/nm... /usr/bin/nm -B
checking nm flags... 
~~~~

Make sure that the tools, directories, and flags are set to reasonable
values, and compatible tools. For example the GNU tools may not
inter-operate well with vendor tools. If you’re using a vendor compiler,
you may need to use the ar, nm, and ranlib that the vendor supplied.

As configure runs, it creates a config.log file. If configure crashes,
do a text search of config.log for thing it was checking before
crashing. If you have a licensing or tool compatibility problem, it will
be obvious in config.log.

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------------------ --- --------------------------------------------------------------------------- --------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Problems-During-Configuration "Previous section in reading order")]   [[\>](#Problems-During-Testing "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Troubleshooting "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------ --- --------------------------------------------------------------------------- --------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.2.2 Problems During Compilation {.subsection}

If the configure script runs, but the compile step doesn’t work, or the
tests don’t complete successfully, the problem is probably in your
CFLAGS or CPPFLAGS.

Frequently shared libraries are a rich source of problems. If your build
is not working, and you are using the –enable-shared option to generate
shared libraries, then try to build without –enable-shared, and see if
the static library build works.

* * * * *

  -------------------------------------------------------------------------- ------------------------------------------------------- --- --------------------------------------------------------------------------- --------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Problems-During-Compilation "Previous section in reading order")]   [[\>](#Finding-Help "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Troubleshooting "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ------------------------------------------------------- --- --------------------------------------------------------------------------- --------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.2.3 Problems During Testing {.subsection}

If you are planning on using large files (i.e. \> 2 GiB), then you may
wish to rerun configure with –enable-large-file-tests to ensure that
large files work on your system.

Some DAP tests (in the directory ncdap\_test) attempt to access an
external server at opendap.org. It is possible that the DAP server may
not be running at test time, or the network access may be faulty or that
the output of the test server has changed. In this case, the DAP tests
may fail. Because of this, the use of these tests is controlled by the
–enable-dap-remote-tests option.

* * * * *

  ---------------------------------------------------------------------- ------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Problems-During-Testing "Previous section in reading order")]   [[\>](#Reporting-Problems "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Build-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- ------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.3 Finding Help On-line {.section}
------------------------

The latest netCDF documentation (including this manual) can be found at
[http://www.unidata.ucar.edu/netcdf/docs](http://www.unidata.ucar.edu/netcdf/docs).

The output of successful build and test runs for recent versions of
netCDF can be found at
[http://www.unidata.ucar.edu/netcdf/builds](http://www.unidata.ucar.edu/netcdf/builds).

A list of known problems with netCDF builds, and suggested fixes, can be
found at
[http://www.unidata.ucar.edu/netcdf/docs/known\_problems.html](http://www.unidata.ucar.edu/netcdf/docs/known_problems.html).

Reportedly successful settings for platforms unavailable for netCDF
testing can be found at
[http://www.unidata.ucar.edu/netcdf/other-builds.html](http://www.unidata.ucar.edu/netcdf/other-builds.html).
If you build netCDF on a system that is not listed, please send your
environment settings, and the full output of your configure, compile,
and testing, to support-netcdf@unidata.ucar.edu. We will add the
information to the other-builds page, with a credit to you.

The replies to all netCDF support emails are on-line and can be
searched. Before reporting a problem to Unidata, please search this
on-line database to see if your problem has already been addressed in a
support email. If you are having build problems it’s usually useful to
search on your system host name. On Unix systems, use the uname command
to find it.

The netCDF Frequently Asked Questions (FAQ) list can be found at
[http://www.unidata.ucar.edu/netcdf/faq.html](http://www.unidata.ucar.edu/netcdf/faq.html).

To search the support database, see
[/search.jsp?support&netcdf](/search.jsp?support&netcdf).

The netCDF mailing list also can be searched; see
[/search.jsp?netcdfgroup](/search.jsp?netcdfgroup).

* * * * *

  ----------------------------------------------------------- --------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Finding-Help "Previous section in reading order")]   [[\>](#Combined-Index "Next section in reading order")]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Build-Problems "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- --------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.4 Reporting Problems {.section}
----------------------

To help us solve your problem, please include the following information
in your email to
[support-netcdf@unidata.ucar.edu](support-netcdf@unidata.ucar.edu).

Unfortunately, we can’t solve build questions without this information;
if you ask for help without providing it, we’re just going to have to
ask for it.

So why not send it immediately, and save us both the extra trouble?

1.  the exact version of netCDF - see the VERSION file.
2.  the \*complete\* output of “./configure”, “make”, and “make check.
    Yes, it’s long, but it’s all important.
3.  if the configure failed, the contents of config.log.
4.  if you are having problems with very large files (larger than 2GiB),
    send the output of "make check" after first running "make distclean"
    and invoking the configure script with the –enable-large-file-tests
    option included.

Although responses to your email will be available in our support
database, your email address is not included, to provide spammers with
one less place to harvest it from.

* * * * *

  ----------------------------------------------------------------- -------- --- --------------------------------------------------------------------------- --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reporting-Problems "Previous section in reading order")]   [ \> ]       [[\<\<](#Build-Problems "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [ \>\> ]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- -------- --- --------------------------------------------------------------------------- --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Index {.unnumbered}
=====

Jump to:  

[**-**](#Index_cp_symbol-1)   [**6**](#Index_cp_symbol-2)  
[**\_**](#Index_cp_symbol-3)   \
 [**A**](#Index_cp_letter-A)   [**B**](#Index_cp_letter-B)  
[**C**](#Index_cp_letter-C)   [**D**](#Index_cp_letter-D)  
[**E**](#Index_cp_letter-E)   [**F**](#Index_cp_letter-F)  
[**G**](#Index_cp_letter-G)   [**H**](#Index_cp_letter-H)  
[**I**](#Index_cp_letter-I)   [**K**](#Index_cp_letter-K)  
[**L**](#Index_cp_letter-L)   [**M**](#Index_cp_letter-M)  
[**N**](#Index_cp_letter-N)   [**O**](#Index_cp_letter-O)  
[**P**](#Index_cp_letter-P)   [**Q**](#Index_cp_letter-Q)  
[**R**](#Index_cp_letter-R)   [**S**](#Index_cp_letter-S)  
[**T**](#Index_cp_letter-T)   [**V**](#Index_cp_letter-V)  
[**W**](#Index_cp_letter-W)  

Index Entry

Section

* * * * *

-

[–default-chunks-in-cache](#index-_002d_002ddefault_002dchunks_002din_002dcache)

[3.5 Running the configure Script](#Configure)

[–disable-cxx](#index-_002d_002ddisable_002dcxx)

[3.5 Running the configure Script](#Configure)

[–disable-examples](#index-_002d_002ddisable_002dexamples)

[3.5 Running the configure Script](#Configure)

[–disable-f77](#index-_002d_002ddisable_002df77)

[3.5 Running the configure Script](#Configure)

[–disable-f90](#index-_002d_002ddisable_002df90)

[3.5 Running the configure Script](#Configure)

[–disable-fortran](#index-_002d_002ddisable_002dfortran)

[3.5 Running the configure Script](#Configure)

[–disable-fortran-type-check](#index-_002d_002ddisable_002dfortran_002dtype_002dcheck)

[3.5 Running the configure Script](#Configure)

[–disable-largefile](#index-_002d_002ddisable_002dlargefile)

[3.5 Running the configure Script](#Configure)

[–disable-netcdf-4](#index-_002d_002ddisable_002dnetcdf_002d4)

[3.5 Running the configure Script](#Configure)

[–disable-shared](#index-_002d_002ddisable_002dshared)

[3.5 Running the configure Script](#Configure)

[–disable-v2](#index-_002d_002ddisable_002dv2)

[3.5 Running the configure Script](#Configure)

[–enable-benchmarks](#index-_002d_002denable_002dbenchmarks)

[3.5 Running the configure Script](#Configure)

[–enable-cxx4](#index-_002d_002denable_002dcxx4)

[3.5 Running the configure Script](#Configure)

[–enable-dap](#index-_002d_002denable_002ddap)

[3.5 Running the configure Script](#Configure)

[–enable-dap-long-tests](#index-_002d_002denable_002ddap_002dlong_002dtests)

[3.5 Running the configure Script](#Configure)

[–enable-dap-remote-tests](#index-_002d_002denable_002ddap_002dremote_002dtests)

[3.5 Running the configure Script](#Configure)

[–enable-extra-tests](#index-_002d_002denable_002dextra_002dtests)

[3.5 Running the configure Script](#Configure)

[–enable-hdf4](#index-_002d_002denable_002dhdf4)

[3.5 Running the configure Script](#Configure)

[–enable-hdf4-file-tests](#index-_002d_002denable_002dhdf4_002dfile_002dtests)

[3.5 Running the configure Script](#Configure)

[–enable-large-file-tests](#index-_002d_002denable_002dlarge_002dfile_002dtests)

[3.5 Running the configure Script](#Configure)

[–enable-parallel-tests](#index-_002d_002denable_002dparallel_002dtests)

[3.4 Building on Platforms with Parallel I/O](#parallel)

[–enable-pnetcdf](#index-_002d_002denable_002dpnetcdf)

[3.5 Running the configure Script](#Configure)

[–enable-valgrind-tests](#index-_002d_002denable_002dvalgrind_002dtests)

[3.5 Running the configure Script](#Configure)

[–max-default-cache-size](#index-_002d_002dmax_002ddefault_002dcache_002dsize)

[3.5 Running the configure Script](#Configure)

[–prefix](#index-_002d_002dprefix)

[3.5 Running the configure Script](#Configure)

[–with-chunk-cache-nelems](#index-_002d_002dwith_002dchunk_002dcache_002dnelems)

[3.5 Running the configure Script](#Configure)

[–with-chunk-cache-preemption](#index-_002d_002dwith_002dchunk_002dcache_002dpreemption)

[3.5 Running the configure Script](#Configure)

[–with-chunk-cache-size](#index-_002d_002dwith_002dchunk_002dcache_002dsize)

[3.5 Running the configure Script](#Configure)

[–with-curl-config](#index-_002d_002dwith_002dcurl_002dconfig)

[3.5 Running the configure Script](#Configure)

[–with-default-chunk-size](#index-_002d_002dwith_002ddefault_002dchunk_002dsize)

[3.5 Running the configure Script](#Configure)

[–with-temp-large](#index-_002d_002dwith_002dtemp_002dlarge)

[3.5 Running the configure Script](#Configure)

[–with-udunits](#index-_002d_002dwith_002dudunits)

[3.5 Running the configure Script](#Configure)

* * * * *

6

[64-bit platforms](#index-64_002dbit-platforms)

[3.3 Building on 64 Bit Platforms](#g_t64-Bit)

* * * * *

\_

[\_LARGE\_FILES, on AIX](#index-_005fLARGE_005fFILES_002c-on-AIX)

[3.8 Installing NetCDF](#Installation)

* * * * *

A

[AIX 64-bit build](#index-AIX-64_002dbit-build)

[3.3 Building on 64 Bit Platforms](#g_t64-Bit)

[AIX, building on](#index-AIX_002c-building-on)

[3.9 Platform Specific Notes](#Platform-Notes)

[autoconf](#index-autoconf)

[3.11 Contributing to NetCDF Source Code Development](#Source)

[automake](#index-automake)

[3.11 Contributing to NetCDF Source Code Development](#Source)

* * * * *

B

[big endian](#index-big-endian)

[3.10 Additional Porting Notes](#Porting-Notes)

[binaries, windows](#index-binaries_002c-windows)

[5.1 Getting Prebuilt netcdf.dll](#Prebuilt-DLL)

[binary install](#index-binary-install)

[1. Installing the NetCDF Binaries](#Binaries)

[binary releases](#index-binary-releases)

[3. Building and Installing NetCDF on Unix Systems](#Building-on-Unix)

[bugs, reporting](#index-bugs_002c-reporting)

[6.4 Reporting Problems](#Reporting-Problems)

* * * * *

C

[compiler flags](#index-compiler-flags)

[4. Using NetCDF on Unix Systems](#Using)

[config.log](#index-config_002elog)

[3.5 Running the configure Script](#Configure)

[configure, running](#index-configure_002c-running)

[3.5 Running the configure Script](#Configure)

[CRAY, porting to](#index-CRAY_002c-porting-to)

[3.10 Additional Porting Notes](#Porting-Notes)

[Cygwin, building with](#index-Cygwin_002c-building-with)

[3.9 Platform Specific Notes](#Platform-Notes)

* * * * *

D

[debug directory, windows](#index-debug-directory_002c-windows)

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

[DLL](#index-DLL)

[5. Building and Installing NetCDF on Windows](#Building-on-Windows)

[dll, getting](#index-dll_002c-getting)

[5.1 Getting Prebuilt netcdf.dll](#Prebuilt-DLL)

[documentation](#index-documentation)

[6.3 Finding Help On-line](#Finding-Help)

[documents, latest version](#index-documents_002c-latest-version)

[3. Building and Installing NetCDF on Unix Systems](#Building-on-Unix)

* * * * *

E

[earlier netCDF versions](#index-earlier-netCDF-versions)

[3. Building and Installing NetCDF on Unix Systems](#Building-on-Unix)

[enable-large-file-tests](#index-enable_002dlarge_002dfile_002dtests)

[3.1 Installation Requirements](#Requirements)

[enable-large-file-tests](#index-enable_002dlarge_002dfile_002dtests-1)

[3.7 Testing the Build](#Testing)

[extra\_check requirements](#index-extra_005fcheck-requirements)

[3.1 Installation Requirements](#Requirements)

[extra\_test requirements](#index-extra_005ftest-requirements)

[3.1 Installation Requirements](#Requirements)

* * * * *

F

[FAQ for netCDF](#index-FAQ-for-netCDF)

[6.3 Finding Help On-line](#Finding-Help)

[ffio.c](#index-ffio_002ec)

[3.10 Additional Porting Notes](#Porting-Notes)

[flex and yacc](#index-flex-and-yacc)

[3.11 Contributing to NetCDF Source Code Development](#Source)

[fortran, Intel](#index-fortran_002c-Intel)

[3.9 Platform Specific Notes](#Platform-Notes)

[fortran, Portland Group](#index-fortran_002c-Portland-Group)

[3.9 Platform Specific Notes](#Platform-Notes)

* * * * *

G

[GNU make](#index-GNU-make)

[3.10 Additional Porting Notes](#Porting-Notes)

* * * * *

H

[HPUX, building on](#index-HPUX_002c-building-on)

[3.9 Platform Specific Notes](#Platform-Notes)

* * * * *

I

[install directory](#index-install-directory)

[3.5 Running the configure Script](#Configure)

[installation requirements](#index-installation-requirements)

[3.1 Installation Requirements](#Requirements)

[installing binary distribution](#index-installing-binary-distribution)

[1. Installing the NetCDF Binaries](#Binaries)

[installing netCDF](#index-installing-netCDF)

[3.8 Installing NetCDF](#Installation)

[Intel fortran](#index-Intel-fortran)

[3.9 Platform Specific Notes](#Platform-Notes)

[Irix, building on](#index-Irix_002c-building-on)

[3.9 Platform Specific Notes](#Platform-Notes)

* * * * *

K

[known problems](#index-known-problems)

[6.3 Finding Help On-line](#Finding-Help)

* * * * *

L

[large file tests](#index-large-file-tests)

[3.7 Testing the Build](#Testing)

[large file tests requirements](#index-large-file-tests-requirements)

[3.1 Installation Requirements](#Requirements)

[large file tests, for
windows](#index-large-file-tests_002c-for-windows)

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

[libtool](#index-libtool)

[3.11 Contributing to NetCDF Source Code Development](#Source)

[link options](#index-link-options)

[4. Using NetCDF on Unix Systems](#Using)

[Linux, building on](#index-Linux_002c-building-on)

[3.9 Platform Specific Notes](#Platform-Notes)

[little endian](#index-little-endian)

[3.10 Additional Porting Notes](#Porting-Notes)

* * * * *

M

[m4](#index-m4)

[3.11 Contributing to NetCDF Source Code Development](#Source)

[Macintosh, building on](#index-Macintosh_002c-building-on)

[3.9 Platform Specific Notes](#Platform-Notes)

[mailing lists](#index-mailing-lists)

[6.3 Finding Help On-line](#Finding-Help)

[make all\_large\_tests](#index-make-all_005flarge_005ftests)

[3.7 Testing the Build](#Testing)

[make check](#index-make-check)

[3.7 Testing the Build](#Testing)

[make install](#index-make-install)

[3.8 Installing NetCDF](#Installation)

[make lfs\_test](#index-make-lfs_005ftest)

[3.7 Testing the Build](#Testing)

[make slow\_check](#index-make-slow_005fcheck)

[3.7 Testing the Build](#Testing)

[make test](#index-make-test)

[3.7 Testing the Build](#Testing)

[make, running](#index-make_002c-running)

[3.6 Running make](#Make)

[makeinfo](#index-makeinfo)

[3.11 Contributing to NetCDF Source Code Development](#Source)

[Microsoft](#index-Microsoft)

[5. Building and Installing NetCDF on Windows](#Building-on-Windows)

[MPICH2](#index-MPICH2)

[3.4 Building on Platforms with Parallel I/O](#parallel)

* * * * *

N

[nc-config](#index-nc_002dconfig)

[4. Using NetCDF on Unix Systems](#Using)

[ncconfig.h](#index-ncconfig_002eh)

[3.10 Additional Porting Notes](#Porting-Notes)

[ncconfig.in](#index-ncconfig_002ein)

[3.10 Additional Porting Notes](#Porting-Notes)

[ncconfig.inc](#index-ncconfig_002einc)

[3.10 Additional Porting Notes](#Porting-Notes)

[ncdump, windows location](#index-ncdump_002c-windows-location)

[5.2 Installing the DLL](#Installing-DLL)

[ncgen, windows location](#index-ncgen_002c-windows-location)

[5.2 Installing the DLL](#Installing-DLL)

[ncio](#index-ncio)

[3.10 Additional Porting Notes](#Porting-Notes)

[ncx.m4](#index-ncx_002em4)

[3.10 Additional Porting Notes](#Porting-Notes)

[NET](#index-NET)

[5. Building and Installing NetCDF on Windows](#Building-on-Windows)

[netcdf.dll, location](#index-netcdf_002edll_002c-location)

[5.2 Installing the DLL](#Installing-DLL)

[netcdf.lib](#index-netcdf_002elib)

[5.2 Installing the DLL](#Installing-DLL)

* * * * *

O

[OBJECT\_MODE, on AIX](#index-OBJECT_005fMODE_002c-on-AIX)

[3.8 Installing NetCDF](#Installation)

[OSF1, building on](#index-OSF1_002c-building-on)

[3.9 Platform Specific Notes](#Platform-Notes)

[other builds document](#index-other-builds-document)

[6.3 Finding Help On-line](#Finding-Help)

* * * * *

P

[parallel platforms](#index-parallel-platforms)

[3.4 Building on Platforms with Parallel I/O](#parallel)

[porting notes, additional](#index-porting-notes_002c-additional)

[3.10 Additional Porting Notes](#Porting-Notes)

[Portland Group fortran](#index-Portland-Group-fortran)

[3.9 Platform Specific Notes](#Platform-Notes)

[posixio.c](#index-posixio_002ec)

[3.10 Additional Porting Notes](#Porting-Notes)

[prefix argument of configure](#index-prefix-argument-of-configure)

[3.5 Running the configure Script](#Configure)

[problems, reporting](#index-problems_002c-reporting)

[6.4 Reporting Problems](#Reporting-Problems)

* * * * *

Q

[quick unix instructions](#index-quick-unix-instructions)

[2. Quick Instructions for Installing NetCDF on
Unix](#Quick-Instructions)

[quick\_large\_files, in
VC++.NET](#index-quick_005flarge_005ffiles_002c-in-VC_002b_002b_002eNET)

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

* * * * *

R

[release directory, windows](#index-release-directory_002c-windows)

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

[reporting problems](#index-reporting-problems)

[6.4 Reporting Problems](#Reporting-Problems)

[running configure](#index-running-configure)

[3.5 Running the configure Script](#Configure)

[running make](#index-running-make)

[3.6 Running make](#Make)

* * * * *

S

[sed](#index-sed)

[3.11 Contributing to NetCDF Source Code Development](#Source)

[shared libraries, building](#index-shared-libraries_002c-building)

[2. Quick Instructions for Installing NetCDF on
Unix](#Quick-Instructions)

[shared libraries, using](#index-shared-libraries_002c-using)

[1. Installing the NetCDF Binaries](#Binaries)

[successful build output, on
web](#index-successful-build-output_002c-on-web)

[6.3 Finding Help On-line](#Finding-Help)

[SunOS 64-bit build](#index-SunOS-64_002dbit-build)

[3.3 Building on 64 Bit Platforms](#g_t64-Bit)

[SunOS, building on](#index-SunOS_002c-building-on)

[3.9 Platform Specific Notes](#Platform-Notes)

[support email](#index-support-email)

[6.4 Reporting Problems](#Reporting-Problems)

* * * * *

T

[TEMP\_LARGE](#index-TEMP_005fLARGE)

[3.7 Testing the Build](#Testing)

[testing large file features](#index-testing-large-file-features)

[3.7 Testing the Build](#Testing)

[testing, for windows](#index-testing_002c-for-windows)

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

[tests, running](#index-tests_002c-running)

[3.7 Testing the Build](#Testing)

[tex](#index-tex)

[3.11 Contributing to NetCDF Source Code Development](#Source)

[troubleshooting](#index-troubleshooting)

[6.2 Troubleshooting](#Troubleshooting)

[turning off C++, Fortran
interface](#index-turning-off-C_002b_002b_002c-Fortran-interface)

[6.2 Troubleshooting](#Troubleshooting)

* * * * *

V

[VC++](#index-VC_002b_002b)

[5. Building and Installing NetCDF on Windows](#Building-on-Windows)

[VC++ 6.0, building
with](#index-VC_002b_002b-6_002e0_002c-building-with)

[5.3 Building netcdf.dll with VC++ 6.0](#Visual-Cplusplus)

[VC++ 6.0, using netcdf
with](#index-VC_002b_002b-6_002e0_002c-using-netcdf-with)

[5.4 Using netcdf.dll with VC++ 6.0](#Using-DLL)

[VC++.NET, building
with](#index-VC_002b_002b_002eNET_002c-building-with)

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

[VC++.NET, using netcdf
with](#index-VC_002b_002b_002eNET_002c-using-netcdf-with)

[5.6 Using netcdf.dll with VC++.NET](#Using-with-NET)

[visual studio 2003 properties](#index-visual-studio-2003-properties)

[5.6 Using netcdf.dll with VC++.NET](#Using-with-NET)

* * * * *

W

[windows large file tests](#index-windows-large-file-tests)

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

[windows testing](#index-windows-testing)

[5.5 Building netcdf.dll with VC++.NET](#Building-with-NET)

[windows, building on](#index-windows_002c-building-on)

[5. Building and Installing NetCDF on Windows](#Building-on-Windows)

* * * * *

Jump to:  

[**-**](#Index_cp_symbol-1)   [**6**](#Index_cp_symbol-2)  
[**\_**](#Index_cp_symbol-3)   \
 [**A**](#Index_cp_letter-A)   [**B**](#Index_cp_letter-B)  
[**C**](#Index_cp_letter-C)   [**D**](#Index_cp_letter-D)  
[**E**](#Index_cp_letter-E)   [**F**](#Index_cp_letter-F)  
[**G**](#Index_cp_letter-G)   [**H**](#Index_cp_letter-H)  
[**I**](#Index_cp_letter-I)   [**K**](#Index_cp_letter-K)  
[**L**](#Index_cp_letter-L)   [**M**](#Index_cp_letter-M)  
[**N**](#Index_cp_letter-N)   [**O**](#Index_cp_letter-O)  
[**P**](#Index_cp_letter-P)   [**Q**](#Index_cp_letter-Q)  
[**R**](#Index_cp_letter-R)   [**S**](#Index_cp_letter-S)  
[**T**](#Index_cp_letter-T)   [**V**](#Index_cp_letter-V)  
[**W**](#Index_cp_letter-W)  

* * * * *

  ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------- ------------ ------------------------------------ ----------------------------------

About This Document
===================

This document was generated by *Ward Fisher* on *January 22, 2014* using
[*texi2html 1.82*](http://www.nongnu.org/texi2html/).

The buttons in the navigation panels have the following meaning:

  Button       Name          Go to                                           From 1.2.3 go to
  ------------ ------------- ----------------------------------------------- ------------------
  [ \< ]       Back          Previous section in reading order               1.2.2
  [ \> ]       Forward       Next section in reading order                   1.2.4
  [ \<\< ]     FastBack      Beginning of this chapter or previous chapter   1
  [ Up ]       Up            Up section                                      1.2
  [ \>\> ]     FastForward   Next chapter                                    2
  [Top]        Top           Cover (top) of document                          
  [Contents]   Contents      Table of contents                                
  [Index]      Index         Index                                            
  [ ? ]        About         About (help)                                     

where the **Example** assumes that the current position is at
**Subsubsection One-Two-Three** of a document of the following
structure:

-   1. Section One
    -   1.1 Subsection One-One
        -   ...

    -   1.2 Subsection One-Two
        -   1.2.1 Subsubsection One-Two-One
        -   1.2.2 Subsubsection One-Two-Two
        -   1.2.3 Subsubsection One-Two-Three     **\<== Current
            Position**
        -   1.2.4 Subsubsection One-Two-Four

    -   1.3 Subsection One-Three
        -   ...

    -   1.4 Subsection One-Four

* * * * *

This document was generated by *Ward Fisher* on *January 22, 2014* using
[*texi2html 1.82*](http://www.nongnu.org/texi2html/). \

