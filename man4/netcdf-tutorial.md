\page netcdf-tutorial-texi NetCDF Tutorial

  ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------- ------------ ------------------------------------ ----------------------------------

NetCDF Tutorial 
===============

This tutorial aims to give a quick and painless introduction to netCDF.

For the basic concepts of netCDF see [What is NetCDF?](#Intro). Read
this to understand the netCDF data models and how to use netCDF.

For sets of examples of increasing complexity see [Example
Programs](#Examples). The example programs are provided for each of four
netCDF API languages, C, C++, F77, and F90. (No new netCDF-4 API
extensions are used in these examples.)

For a quick reference to the important functions in the netCDF-3 C, C++,
Fortran 77, and Fortran 90 APIs, with hyper-links to the full
documentation of each function, see [The Functions You Need in
NetCDF-3](#Useful-Functions).

To learn about the new features of netCDF-4, see [API Extensions
Introduced with NetCDF-4](#API_002dExtensions). For some examples see
[NetCDF-4 Examples](#NetCDF_002d4-Examples).

NetCDF interface guides are available for C (see [The NetCDF C Interface
Guide: (netcdf-c)Top](netcdf-c.html#Top) section ‘Top’ in The NetCDF C
Interface Guide), C++ (see [The NetCDF C++ Interface Guide:
(netcdf-cxx)Top](netcdf-cxx.html#Top) section ‘Top’ in The NetCDF C++
Interface Guide), Fortran 77 (see [The NetCDF Fortran 77 Interface
Guide: (netcdf-f77)Top](netcdf-f77.html#Top) section ‘Top’ in The NetCDF
Fortran 77 Interface Guide), and Fortran 90 (see [The NetCDF Fortran 90
Interface Guide: (netcdf-f90)Top](netcdf-f90.html#Top) section ‘Top’ in
The NetCDF Fortran 90 Interface Guide).

This document applies to netCDF version No value for VERSION; it was
last updated on No value for UPDATED.

[1. What is NetCDF?](#Intro)

  

[2. Example Programs](#Examples)

  

Examples of increasing complexity.

[3. The Functions You Need in NetCDF-3](#Useful-Functions)

  

Quick reference to useful netCDF functions.

[4. API Extensions Introduced with NetCDF-4](#API_002dExtensions)

  

New features added in netCDF-4.

[5. NetCDF-4 Examples](#NetCDF_002d4-Examples)

  

[Index](#Combined-Index)

  

~~~~ 
 — The Detailed Node Listing —

What is NetCDF?
~~~~

[1.1 The Classic NetCDF Data Model](#Data-Model)

  

How netCDF classic sees data.

[1.2 The Common Data Model and NetCDF-4](#Common-Data-Model)

  

The new, expanded data model.

[1.3 NetCDF Error Handling](#Errors)

  

When things go tragically awry.

[1.4 Unlimited Dimensions](#Unlimited-Dimensions)

  

Arbitrarily extending a dimension.

[1.5 Fill Values](#Fill-Values)

  

Handling missing data.

[1.6 Tools for Manipulating NetCDF Files](#Tools)

  

Useful tools for netCDF files.

[1.7 The NetCDF Programming APIs](#APIs)

  

Programming languages and netCDF.

[1.8 NetCDF Documentation](#Documentation)

  

Introducing the netCDF documentation!

[1.9 A Note on NetCDF Versions and Formats](#Versions)

  

Different versions of netCDF.

~~~~ 
Example Programs
~~~~

[2.1 The simple\_xy Example](#simple_005fxy)

  

A very simple netCDF file.

[2.2 The sfc\_pres\_temp Example](#sfc_005fpres_005ftemp)

  

A more complex file with more metadata.

[2.3 The pres\_temp\_4D Example](#pres_005ftemp_005f4D)

  

A 4D file with an unlimited dimension.

~~~~ 
The simple_xy Example
~~~~

[2.1.1 simple\_xy\_wr.c and simple\_xy\_rd.c](#simple_005fxy-in-C)

  

[2.1.2 simple\_xy\_wr.f and simple\_xy\_rd.f](#simple_005fxy-in-F77)

  

[2.1.3 simple\_xy\_wr.f90 and simple\_xy\_rd.f90](#simple_005fxy-in-F90)

  

[2.1.4 simple\_xy\_wr.cpp and
simple\_xy\_rd.cpp](#simple_005fxy-in-C_002b_002b)

  

~~~~ 
simple_xy_wr.c and simple_xy_rd.c
~~~~

[2.1.1.1 simple\_xy\_wr.c](#simple_005fxy_005fwr_002ec)

  

[2.1.1.2 simple\_xy\_rd.c](#simple_005fxy_005frd_002ec)

  

~~~~ 
simple_xy_wr.f and simple_xy_rd.f
~~~~

[2.1.2.1 simple\_xy\_wr.f](#simple_005fxy_005fwr_002ef)

  

[2.1.2.2 simple\_xy\_rd.f](#simple_005fxy_005frd_002ef)

  

~~~~ 
simple_xy_wr.f90 and simple_xy_rd.f90
~~~~

[2.1.3.1 simple\_xy\_wr.f90](#simple_005fxy_005fwr_002ef90)

  

[2.1.3.2 simple\_xy\_rd.f90](#simple_005fxy_005frd_002ef90)

  

~~~~ 
simple_xy_wr.cpp and simple_xy_rd.cpp
~~~~

[2.1.4.1 simple\_xy\_wr.cpp](#simple_005fxy_005fwr_002ecpp)

  

[2.1.4.2 simple\_xy\_rd.cpp](#simple_005fxy_005frd_002ecpp)

  

~~~~ 
The sfc_pres_temp Example
~~~~

[2.2.1 sfc\_pres\_temp\_wr.c and
sfc\_pres\_temp\_rd.c](#sfc_005fpres_005ftemp-in-C)

  

[2.2.2 sfc\_pres\_temp\_wr.f and
sfc\_pres\_temp\_rd.f](#sfc_005fpres_005ftemp-in-F77)

  

[2.2.3 sfc\_pres\_temp\_wr.f90 and
sfc\_pres\_temp\_rd.f90](#sfc_005fpres_005ftemp-in-F90)

  

[2.2.4 sfc\_pres\_temp\_wr.cpp and
sfc\_pres\_temp\_rd.cpp](#sfc_005fpres_005ftemp-in-C_002b_002b)

  

~~~~ 
sfc_pres_temp_wr.c and sfc_pres_temp_rd.c
~~~~

[2.2.1.1 sfc\_pres\_temp\_wr.c](#sfc_005fpres_005ftemp_005fwr_002ec)

  

[2.2.1.2 sfc\_pres\_temp\_rd.c](#sfc_005fpres_005ftemp_005frd_002ec)

  

~~~~ 
sfc_pres_temp_wr.f and sfc_pres_temp_rd.f
~~~~

[2.2.2.1 sfc\_pres\_temp\_wr.f](#sfc_005fpres_005ftemp_005fwr_002ef)

  

[2.2.2.2 sfc\_pres\_temp\_rd.f](#sfc_005fpres_005ftemp_005frd_002ef)

  

~~~~ 
sfc_pres_temp_wr.f90 and sfc_pres_temp_rd.f90
~~~~

[2.2.3.1 sfc\_pres\_temp\_wr.f90](#sfc_005fpres_005ftemp_005fwr_002ef90)

  

[2.2.3.2 sfc\_pres\_temp\_rd.f90](#sfc_005fpres_005ftemp_005frd_002ef90)

  

~~~~ 
sfc_pres_temp_wr.cpp and sfc_pres_temp_rd.cpp
~~~~

[2.2.4.1 sfc\_pres\_temp\_wr.cpp](#sfc_005fpres_005ftemp_005fwr_002ecpp)

  

[2.2.4.2 sfc\_pres\_temp\_rd.cpp](#sfc_005fpres_005ftemp_005frd_002ecpp)

  

~~~~ 
The pres_temp_4D Example
~~~~

[2.3.1 pres\_temp\_4D\_wr.c and
pres\_temp\_4D\_rd.c](#pres_005ftemp_005f4D-in-C)

  

[2.3.2 pres\_temp\_4D\_wr.f and
pres\_temp\_4D\_rd.f](#pres_005ftemp_005f4D-in-F77)

  

[2.3.3 pres\_temp\_4D\_wr.f90 and
pres\_temp\_4D\_rd.f90](#pres_005ftemp_005f4D-in-F90)

  

[2.3.4 pres\_temp\_4D\_wr.cpp and
pres\_temp\_4D\_rd.cpp](#pres_005ftemp_005f4D-in-C_002b_002b)

  

~~~~ 
pres_temp_4D_wr.c and pres_temp_4D_rd.c
~~~~

[2.3.1.1 pres\_temp\_4D\_wr.c](#pres_005ftemp_005f4D_005fwr_002ec)

  

[2.3.1.2 pres\_temp\_4D\_rd.c](#pres_005ftemp_005f4D_005frd_002ec)

  

~~~~ 
pres_temp_4D_wr.f and pres_temp_4D_rd.f
~~~~

[2.3.2.1 pres\_temp\_4D\_wr.f](#pres_005ftemp_005f4D_005fwr_002ef)

  

[2.3.2.2 pres\_temp\_4D\_rd.f](#pres_005ftemp_005f4D_005frd_002ef)

  

~~~~ 
pres_temp_4D_wr.f90 and pres_temp_4D_rd.f90
~~~~

[2.3.3.1 pres\_temp\_4D\_wr.f90](#pres_005ftemp_005f4D_005fwr_002ef90)

  

[2.3.3.2 pres\_temp\_4D\_rd.f90](#pres_005ftemp_005f4D_005frd_002ef90)

  

~~~~ 
pres_temp_4D_wr.cpp and pres_temp_4D_rd.cpp
~~~~

[2.3.4.1 pres\_temp\_4D\_wr.cpp](#pres_005ftemp_005f4D_005fwr_002ecpp)

  

[2.3.4.2 pres\_temp\_4D\_rd.cpp](#pres_005ftemp_005f4D_005frd_002ecpp)

  

~~~~ 
The Functions You Need in NetCDF-3
~~~~

[3.1 Creating New Files and Metadata, an Overview](#Creation)

  

Creating netCDF files, adding metadata.

[3.2 Reading NetCDF Files of Known Structure](#Reading)

  

Reading netCDF files of known structure.

[3.3 Reading NetCDF Files of Unknown Structure](#Inquiry-Functions)

  

Learning about an unknown netCDF file.

[3.4 Reading and Writing Subsets of Data](#Subsets)

  

Reading and writing Subsets of data.

~~~~ 
Creating New Files and Metadata, an Overview
~~~~

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

  

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

  

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

  

[3.1.4 Creating a NetCDF File in C++](#Creation-in-C_002b_002b)

  

~~~~ 
Numbering of NetCDF IDs
~~~~

[3.2.2 Reading a Known NetCDF File in C](#Reading-in-C)

  

[3.2.3 Reading a Known NetCDF File in Fortran 77](#Reading-in-F77)

  

[3.2.4 Reading a Known NetCDF File in Fortran 90](#Reading-in-F90)

  

[3.2.5 Reading a Known NetCDF File in C++](#Reading-in-C_002b_002b)

  

~~~~ 
Reading NetCDF Files of Unknown Structure
~~~~

[3.3.1 Inquiry in C](#Inquiry-in-C)

  

[3.3.2 Inquiry in Fortran 77](#Inquiry-in-F77)

  

[3.3.3 Inquiry in Fortran 90](#Inquiry-in-F90)

  

[3.3.4 Inquiry Functions in the C++ API](#Inquiry-in-C_002b_002b)

  

~~~~ 
Reading and Writing Subsets of Data
~~~~

[3.4.1 Reading and Writing Subsets of Data in C](#Subsetting-in-C)

  

[3.4.2 Reading and Writing Subsets of Data in Fortran
77](#Subsetting-in-F77)

  

[3.4.3 Reading and Writing Subsets of Data in Fortran
90](#Subsetting-in-F90)

  

[3.4.4 Reading and Writing Subsets of Data in
C++](#Subsetting-in-C_002b_002b)

  

~~~~ 
API Extensions Introduced with NetCDF-4
~~~~

[4.1 Interoperability with HDF5](#Interoperability)

  

Reading and writing HDF5 files.

[4.2 Multiple Unlimited
Dimensions](#Multiple_002dUnlimited_002dDimensions)

  

Use more than one unlimited dimension.

[4.3 Groups](#Groups)

  

Organizing data hierarchically.

[4.4 Compound Types](#Compound_002dTypes)

  

Creating data type like C structs.

[4.5 Opaque Types](#Opaque_002dTypes)

  

Creating a data type of known size.

[4.6 Variable Length Arrays (VLEN)](#VLEN_002dType)

  

Variable length arrays.

[4.7 Strings](#Strings)

  

Storing strings of data.

[4.8 New Inquiry Functions](#New_002dinq_002dFunctions)

  

Functions to help explore a file.

[4.9 Parallel I/O with NetCDF](#Parallel)

  

How to get parallel I/O.

[4.10 The Future of NetCDF](#Future)

  

What’s coming next!

~~~~ 
Collective/Independent Access
~~~~

[4.9.3 simple\_xy\_par\_wr.c and
simple\_xy\_par\_rd.c](#simple_005fxy_005fpar-in-C)

  

~~~~ 
simple_xy_par_wr.c and simple_xy_par_rd.c
~~~~

[4.9.3.1 simple\_xy\_par\_wr.f90](#simple_005fxy_005fpar_005fwr_002ef90)

  

[4.9.3.2 simple\_xy\_par\_rd.f90](#simple_005fxy_005fpar_005frd_002ef90)

  

~~~~ 
NetCDF-4 Examples
~~~~

[5.1 The simple\_nc4 Example](#simple_005fnc4)

  

[5.2 The simple\_xy\_nc4 Example](#simple_005fxy_005fnc4)

  

~~~~ 
The simple_nc4 Example
~~~~

[5.1.1 simple\_nc4\_wr.c and simple\_nc4\_rd.c](#simple_005fnc4-in-C)

  

~~~~ 
simple_nc4_wr.c and simple_nc4_rd.c
~~~~

[5.1.1.1 simple\_nc4\_wr.c](#simple_005fnc4_005fwr_002ec)

  

[5.1.1.2 simple\_nc4\_rd.c](#simple_005fnc4_005frd_002ec)

  

~~~~ 
The simple_xy_nc4 Example
~~~~

[5.2.1 simple\_xy\_nc4\_wr.c and
simple\_xy\_nc4\_rd.c](#simple_005fxy_005fnc4-in-C)

  

[5.2.2 simple\_xy\_nc4\_wr.f and
simple\_xy\_nc4\_rd.f](#simple_005fxy_005fnc4-in-F77)

  

[5.2.3 simple\_xy\_nc4\_wr.f90 and
simple\_xy\_nc4\_rd.f90](#simple_005fxy_005fnc4-in-F90)

  

~~~~ 
simple_xy_nc4_wr.c and simple_xy_nc4_rd.c
~~~~

[5.2.1.1 simple\_xy\_nc4\_wr.c](#simple_005fxy_005fnc4_005fwr_002ec)

  

[5.2.1.2 simple\_xy\_nc4\_rd.c](#simple_005fxy_005fnc4_005frd_002ec)

  

~~~~ 
simple_xy_nc4_wr.f and simple_xy_nc4_rd.f
~~~~

[5.2.2.1 simple\_xy\_nc4\_wr.f](#simple_005fxy_005fnc4_005fwr_002ef)

  

[5.2.2.2 simple\_xy\_nc4\_rd.f](#simple_005fxy_005fnc4_005frd_002ef)

  

~~~~ 
simple_xy_nc4_wr.f90 and simple_xy_nc4_rd.f90
~~~~

[5.2.3.1 simple\_xy\_nc4\_wr.f90](#simple_005fxy_005fnc4_005fwr_002ef90)

  

[5.2.3.2 simple\_xy\_nc4\_rd.f90](#simple_005fxy_005fnc4_005frd_002ef90)

  

~~~~ 
~~~~

* * * * *

  -------------------------------------------------- ----------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Top "Previous section in reading order")]   [[\>](#Data-Model "Next section in reading order")]       [[\<\<](#Top "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------- ----------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1. What is NetCDF? 
==================

NetCDF is a set of data formats, programming interfaces, and software
libraries that help read and write scientific data files.

NetCDF was developed and is maintained at Unidata, part of the
University Corporation for Atmospheric Research (UCAR) Office of
Programs (UOP). Unidata is funded primarily by the National Science
Foundation.

Unidata provides data and software tools for use in geoscience education
and research. For more information see the web sites of Unidata
([http://www.unidata.ucar.edu](http://www.unidata.ucar.edu)), UOP
([http://www.uop.ucar.edu](http://www.uop.ucar.edu)), and UCAR
([http://www.ucar.edu](http://www.ucar.edu)).

This tutorial may serve as an introduction to netCDF. Full netCDF
documentation is available on-line (see section [NetCDF
Documentation](#Documentation)).

  -------------------------------------------------------------- ---- ---------------------------------------
  [1.1 The Classic NetCDF Data Model](#Data-Model)                    How netCDF classic sees data.
  [1.2 The Common Data Model and NetCDF-4](#Common-Data-Model)        The new, expanded data model.
  [1.3 NetCDF Error Handling](#Errors)                                When things go tragically awry.
  [1.4 Unlimited Dimensions](#Unlimited-Dimensions)                   Arbitrarily extending a dimension.
  [1.5 Fill Values](#Fill-Values)                                     Handling missing data.
  [1.6 Tools for Manipulating NetCDF Files](#Tools)                   Useful tools for netCDF files.
  [1.7 The NetCDF Programming APIs](#APIs)                            Programming languages and netCDF.
  [1.8 NetCDF Documentation](#Documentation)                          Introducing the netCDF documentation!
  [1.9 A Note on NetCDF Versions and Formats](#Versions)              Different versions of netCDF.
  -------------------------------------------------------------- ---- ---------------------------------------

* * * * *

  ---------------------------------------------------- ----------------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Intro "Previous section in reading order")]   [[\>](#Meteorological-Example "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------- ----------------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.1 The Classic NetCDF Data Model 
---------------------------------

The classic netCDF data model consists of *variables*, *dimensions*, and
*attributes*. This way of thinking about data was introduced with the
very first netCDF release, and is still the core of all netCDF files.

(In version 4.0, the netCDF data model has been expanded. See section
[The Common Data Model and NetCDF-4](#Common-Data-Model).)

 `Variables` 
:   N-dimensional arrays of data. Variables in netCDF files can be one
    of six types (char, byte, short, int, float, double). For more
    information see [(netcdf)Variables](netcdf.html#Variables) section
    ‘Variables’ in The NetCDF Users Guide.

 `Dimensions` 
:   describe the axes of the data arrays. A dimension has a name and a
    length. An unlimited dimension has a length that can be expanded at
    any time, as more data are written to it. NetCDF files can contain
    at most one unlimited dimension. For more information see
    [(netcdf)Dimensions](netcdf.html#Dimensions) section ‘Dimensions’ in
    The NetCDF Users Guide.

 `Attributes` 
:   annotate variables or files with small notes or supplementary
    metadata. Attributes are always scalar values or 1D arrays, which
    can be associated with either a variable or the file as a whole.
    Although there is no enforced limit, the user is expected to keep
    attributes small. For more information see
    [(netcdf)Attributes](netcdf.html#Attributes) section ‘Attributes’ in
    The NetCDF Users Guide.

For more information on the netCDF data model see [(netcdf)The NetCDF
Data Model](netcdf.html#The-NetCDF-Data-Model) section ‘The NetCDF Data
Model’ in The NetCDF Users Guide.

* * * * *

  --------------------------------------------------------- ------------------------------------------------------------ --- ------------------------------------------------------------------ ---------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Data-Model "Previous section in reading order")]   [[\>](#Common-Data-Model "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Data-Model "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------- ------------------------------------------------------------ --- ------------------------------------------------------------------ ---------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 1.1.1 Meteorological Example 

NetCDF can be used to store many kinds of data, but it was originally
developed for the Earth science community.

NetCDF views the world of scientific data in the same way that an
atmospheric scientist might: as sets of related arrays. There are
various physical quantities (such as pressure and temperature) located
at points at a particular latitude, longitude, vertical level, and time.

A scientist might also like to store supporting information, such as the
units, or some information about how the data were produced.

The axis information (latitude, longitude, level, and time) would be
stored as netCDF dimensions. Dimensions have a length and a name.

The physical quantities (pressure, temperature) would be stored as
netCDF variables. Variables are N-dimensional arrays of data, with a
name and an associated set of netCDF dimensions.

It is also customary to add one variable for each dimension, to hold the
values along that axis. These variables are called “coordinate
variables.” The latitude coordinate variable would be a one-dimensional
variable (with latitude as its dimension), and it would hold the
latitude values at each point along the axis.

The additional bits of metadata would be stored as netCDF attributes.

Attributes are always single values or one-dimensional arrays. (This
works out well for a string, which is a one-dimensional array of ASCII
characters.)

The pres\_temp\_4D example in this tutorial shows how to write and read
a file containing some four-dimensional pressure and temperature data,
including all the metadata needed. See section [The pres\_temp\_4D
Example](#pres_005ftemp_005f4D).

* * * * *

  --------------------------------------------------------------------- ------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Meteorological-Example "Previous section in reading order")]   [[\>](#Errors "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.2 The Common Data Model and NetCDF-4 
--------------------------------------

With netCDF-4, the netCDF data model has been extended, in a backwards
compatible way.

The new data model, which is known as the “Common Data Model” is part of
an effort here at Unidata to find a common engineering language for the
development of scientific data solutions. It contains the variables,
dimensions, and attributes of the classic data model, but adds:

-   groups A way of hierarchically organizing data, similar to
    directories in a Unix file system.
-   user-defined types The user can now define compound types (like C
    structures), enumeration types, variable length arrays, and opaque
    types.

These features may only be used when working with a netCDF-4/HDF5 file.
Files created in classic or 64-bit offset format cannot support groups
or user-defined types.

With netCDF-4/HDF5 files, the user may define groups, which may contain
variables, dimensions, and attributes. In this way, a group acts as a
container for the classic netCDF dataset. But netCDF-4/HDF5 files can
have many groups, organized hierarchically.

Each file begins with at least one group, the root group. The user may
then add more groups, receiving a new ncid for each group created.

Since each group functions as a complete netCDF classic dataset, it is
possible to have variables with the same name in two or more different
groups, within the same netCDF-4/HDF5 data file.

Dimensions have a special scope: they may be seen by all variables in
their group, and all descendant groups. This allows the user to define
dimensions in a top-level group, and use them in many sub-groups.

Since it may be necessary to write code which works with all types of
netCDF data files, we also introduce the ability to create netCDF-4/HDF5
files which follow all the rules of the classic netCDF model. That is,
these files are in HDF5, but will not support multiple unlimited
dimensions, user-defined types, groups, etc. They act just like a
classic netCDF file.

* * * * *

  ---------------------------------------------------------------- --------------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Common-Data-Model "Previous section in reading order")]   [[\>](#Unlimited-Dimensions "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- --------------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.3 NetCDF Error Handling 
-------------------------

Each netCDF function in the C, Fortran 77, and Fortran 90 APIs returns 0
on success, in the tradition of C. (For C++, see below).

When programming with netCDF in these languages, always check return
values of every netCDF API call. The return code can be looked up in
netcdf.h (for C programmers) or netcdf.inc (for Fortran programmers), or
you can use the strerror function to print out an error message. (See
[(netcdf-c)nc\_strerror](netcdf-c.html#nc_005fstrerror) section
‘nc\_strerror’ in The NetCDF C Interface
Guide/[(netcdf-f77)NF\_STRERROR](netcdf-f77.html#NF_005fSTRERROR)
section ‘NF\_STRERROR’ in The NetCDF Fortran 77 Interface
Guide/[(netcdf-f90)NF90\_STRERROR](netcdf-f90.html#NF90_005fSTRERROR)
section ‘NF90\_STRERROR’ in The NetCDF Fortran 90 Interface Guide).

In general, if a function returns an error code, you can assume it
didn’t do what you hoped it would. The exception is the NC\_ERANGE
error, which is returned by any of the reading or writing functions when
one or more of the values read or written exceeded the range for the
type. (For example if you were to try to read 1000 into an unsigned
byte.)

In the case of NC\_ERANGE errors, the netCDF library completes the
read/write operation, and then returns the error. The type conversion is
handled like a C type conversion, whether or not it is within range.
This may yield bad data, but the netCDF library just returns NC\_ERANGE
and leaves it up to the user to handle. (For more information about type
conversion see [(netcdf-c)Type
Conversion](netcdf-c.html#Type-Conversion) section ‘Type Conversion’ in
The NetCDF C Interface Guide).

Error handling in C++ is different. For some objects, the is\_valid()
method should be called. Other error handling is controlled by the
NcError class. For more information see [(netcdf-cxx)Class
NcError](netcdf-cxx.html#Class-NcError) section ‘Class NcError’ in The
NetCDF C++ Interface Guide.

For a complete list of netCDF error codes see [(netcdf-c)Error
Codes](netcdf-c.html#Error-Codes) section ‘Error Codes’ in The NetCDF C
Interface Guide.

* * * * *

  ----------------------------------------------------- ------------------------------------------------------ --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Errors "Previous section in reading order")]   [[\>](#Fill-Values "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- ------------------------------------------------------ --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.4 Unlimited Dimensions 
------------------------

Sometimes you don’t know the size of all dimensions when you create a
file, or you would like to arbitrarily extend the file along one of the
dimensions.

For example, model output usually has a time dimension. Rather than
specifying that there will be forty-two output times when creating the
file, you might like to create it with one time, and then add data for
additional times, until you wanted to stop.

For this purpose netCDF provides the unlimited dimension. By specifying
a length of “unlimited” when defining a dimension, you indicate to
netCDF that the dimension may be extended, and its length may increase.

In netCDF classic files, there can only be one unlimited dimension, and
it must be declared first in the list of dimensions for a variable.

For programmers, the unlimited dimension will correspond with the
slowest-varying dimension. In C this is the first dimension of an array,
in Fortran, the last.

The third example in this tutorial, pres\_temp\_4D, demonstrates how to
write and read data one time step at a time along an unlimited dimension
in a classic netCDF file. See section [The pres\_temp\_4D
Example](#pres_005ftemp_005f4D).

In netCDF-4/HDF5 files, any number of unlimited dimensions may be used,
and there is no restriction as to where they appear in a variable’s list
of dimension IDs.

For more detailed information about dimensions see
[(netcdf)Dimensions](netcdf.html#Dimensions) section ‘Dimensions’ in The
NetCDF Users Guide.

* * * * *

  ------------------------------------------------------------------- ------------------------------------------------ --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Unlimited-Dimensions "Previous section in reading order")]   [[\>](#Tools "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------- ------------------------------------------------ --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.5 Fill Values 
---------------

Sometimes there are missing values in the data, and some value is needed
to represent them.

For example, what value do you put in a sea-surface temperature variable
for points over land?

In netCDF, you can create an attribute for the variable (and of the same
type as the variable) called “\_FillValue” that contains a value that
you have used for missing data. Applications that read the data file can
use this to know how to represent these values.

Using attributes it is possible to capture metadata that would otherwise
be separated from the data. Various conventions have been established.
By using a set of conventions, a data producer is more likely to produce
files that can be easily shared within the research community, and that
contain enough details to be useful as a long-term archive. Conventions
also make it easier to develop software that interprets information
represented in data, because a convention selects one conventional way
to represent information when multiple equivalent representations are
possible.

For more information on \_FillValue and other attribute conventions, see
[(netcdf)Attribute Conventions](netcdf.html#Attribute-Conventions)
section ‘Attribute Conventions’ in The NetCDF Users Guide.

Climate and meteorological users are urged to follow the Climate and
Forecast (CF) metadata conventions when producing data files. For more
information about the CF conventions, see
[http://cf-pcmdi.llnl.gov](http://cf-pcmdi.llnl.gov).

For information about creating attributes, see [Creating New Files and
Metadata, an Overview](#Creation).

* * * * *

  ---------------------------------------------------------- ----------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Fill-Values "Previous section in reading order")]   [[\>](#APIs "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- ----------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.6 Tools for Manipulating NetCDF Files 
---------------------------------------

Many existing software applications can read and manipulate netCDF
files. Before writing your own program, check to see if any existing
programs meet your needs.

Two utilities come with the netCDF distribution: ncdump and ncgen. The
ncdump command reads a netCDF file and outputs text in a format called
CDL. The ncgen command reads a text file in CDL format, and generates a
netCDF data file.

One common use for ncdump is to examine the metadata of a netCDF file,
to see what it contains. At the beginning of each example in this
tutorial, an ncdump of the resulting data file is shown. See section
[The simple\_xy Example](#simple_005fxy).

For more information about ncdump and ncgen see [(netcdf)NetCDF
Utilities](netcdf.html#NetCDF-Utilities) section ‘NetCDF Utilities’ in
The NetCDF Users Guide.

The following general-purpose tools have been found to be useful in many
situations. Some of the tools on this list are developed at Unidata. The
others are developed elsewhere, and we can make no guarantees about
their continued availability or success. All of these tools are
open-source.

  --------- ----------------------------------------------------------------------------------------- ----------------------------------------------------------------------------------------------
  UDUNITS   Unidata library to help with scientific units.                                            [http://www.unidata.ucar.edu/software/udunits](http://www.unidata.ucar.edu/software/udunits)
  IDV       Unidata’s Integrated Data Viewer, a 3D visualization and analysis package (Java based).   [http://www.unidata.ucar.edu/software/idv](http://www.unidata.ucar.edu/software/idv)
  NCL       NCAR Command Language, a graphics and data manipulation package.                          [http://www.ncl.ucar.edu](http://www.ncl.ucar.edu)
  GrADS     The Grid Analysis and Display System package.                                             [http://grads.iges.org/grads/grads.html](http://grads.iges.org/grads/grads.html)
  NCO       NetCDF Command line Operators, tools to manipulate netCDF files.                          [http://nco.sourceforge.net](http://nco.sourceforge.net)
  --------- ----------------------------------------------------------------------------------------- ----------------------------------------------------------------------------------------------

For a list of netCDF tools that we know about see
[http://www.unidata.ucar.edu/netcdf/software.html](http://www.unidata.ucar.edu/netcdf/software.html).
If you know of any that should be added to this list, send email to
support-netcdf@unidata.ucar.edu.

* * * * *

  ---------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Tools "Previous section in reading order")]   [[\>](#Documentation "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.7 The NetCDF Programming APIs 
-------------------------------

Unidata supports netCDF APIs in C, C++, Fortran 77, Fortran 90, and
Java.

The Java API is a complete implementation of netCDF in Java. It is
distributed independently of the other APIs. For more information see
the netCDF Java page:
[http://www.unidata.ucar.edu/software/netcdf-java](http://www.unidata.ucar.edu/software/netcdf-java).
If you are writing web server software, you should certainly be doing so
in Java.

The C, C++, Fortran 77 and Fortran 90 APIs are distributed and installed
when the netCDF C library is built, if compilers exist to build them,
and if they are not turned off when configuring the netCDF build.

The C++ and Fortran APIs depend on the C API. Due to the nature of C++
and Fortran 90, users of those languages can also use the C and Fortran
77 APIs (respectively) directly.

In the netCDF-4.0 beta release, only the C API is well-tested. The
Fortran APIs include support for netCDF-4 advanced features, but need
more testing, which will be added in a future release of netCDF.

The C++ API can handle netCDF-4.0/HDF5 files, but can not yet handle
advanced netCDF-4 features. The successor to the current C++ API is
under active development, and will include support for netCDF-4 advanced
features.

Full documentation exists for each API (see section [NetCDF
Documentation](#Documentation)).

In addition, many other language APIs exist, including Perl, Python, and
Ruby. Most of these APIs were written and supported by netCDF users.
Some of them are listed on the netCDF software page, see
[http://www.unidata.ucar.edu/netcdf/software.html](http://www.unidata.ucar.edu/netcdf/software.html).
Since these generally use the C API, they should work well with
netCDF-4/HDF5 files, but the maintainers of the APIs must add support
for netCDF-4 advanced features.

In addition to the main netCDF-3 C API, there is an additional (older) C
API, the netCDF-2 API. This API produces exactly the same files as the
netCDF-3 API - only the API is different. (That is, users can create
either classic format files, the default, or 64-bit offset files, or
netCDF-4/HDF5 files.)

The version 2 API was the API before netCDF-3.0 came out. It is still
fully supported, however. Programs written to the version 2 API will
continue to work.

Users writing new programs should use the netCDF-3 API, which contains
better type checking, better error handling, and better documentation.

The netCDF-2 API is provided for backward compatibility. Documentation
for the netCDF-2 API can be found on the netCDF website, see
[http://www.unidata.ucar.edu/netcdf/old\_docs/really\_old/guide\_toc.html](http://www.unidata.ucar.edu/netcdf/old_docs/really_old/guide_toc.html).

* * * * *

  --------------------------------------------------- --------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#APIs "Previous section in reading order")]   [[\>](#Versions "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------- --------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.8 NetCDF Documentation 
------------------------

This tutorial is brief. A much more complete description of netCDF can
be found in The NetCDF Users Guide. It fully describes the netCDF model
and format. For more information see [The NetCDF Users Guide:
(netcdf)Top](netcdf.html#Top) section ‘Top’ in The NetCDF Users Guide.

The netCDF distribution, in various forms, can be obtained from the
netCDF web site:
[http://www.unidata.ucar.edu/netcdf](http://www.unidata.ucar.edu/netcdf).

A porting and installation guide for the C, C++, Fortran 77, and Fortran
90 APIs describes how to build these APIs on a variety of platforms. See
[The NetCDF Installation and Porting Guide:
(netcdf-install)Top](netcdf-install.html#Top) section ‘Top’ in The
NetCDF Installation and Porting Guide.

Language specific programming guides are available for netCDF for the C,
C++, Fortran 77, Fortran 90, and Java APIs:

 `C` 
:   [The NetCDF C Interface Guide: (netcdf-c)Top](netcdf-c.html#Top)
    section ‘Top’ in The NetCDF C Interface Guide.

 `C++` 
:   [The NetCDF C++ Interface Guide:
    (netcdf-cxx)Top](netcdf-cxx.html#Top) section ‘Top’ in The NetCDF
    C++ Interface Guide.

 `Fortran 77`
:   [The NetCDF Fortran 77 Interface Guide:
    (netcdf-f77)Top](netcdf-f77.html#Top) section ‘Top’ in The NetCDF
    Fortran 77 Interface Guide.

 `Fortran 90` 
:   [The NetCDF Fortran 90 Interface Guide:
    (netcdf-f90)Top](netcdf-f90.html#Top) section ‘Top’ in The NetCDF
    Fortran 90 Interface Guide.

 `Java`
:   [http://www.unidata.ucar.edu/software/netcdf-java/v2.1/NetcdfJavaUserManual.htm](http://www.unidata.ucar.edu/software/netcdf-java/v2.1/NetcdfJavaUserManual.htm).

Man pages for the C, F77, and F90 interfaces, and ncgen and ncdump, are
available on the documentation page of the netCDF web site
([http://www.unidata.ucar.edu/netcdf/docs](http://www.unidata.ucar.edu/netcdf/docs)),
and are installed with the netCDF distribution.

The latest version of all netCDF documentation can always be found at
the documentation page of the netCDF web site:
[http://www.unidata.ucar.edu/netcdf/docs](http://www.unidata.ucar.edu/netcdf/docs)

* * * * *

  ------------------------------------------------------------ --------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Documentation "Previous section in reading order")]   [[\>](#Classic-Format "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Intro "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ --------------------------------------------------------- --- ------------------------------------------------------------------ ----------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.9 A Note on NetCDF Versions and Formats 
-----------------------------------------

NetCDF has changed (and improved) over its lifetime. That means the user
must have some understanding of netCDF versions.

To add to the confusion, there are versions for the APIs, and also for
the data files that they produce. The API version is the version number
that appears in the tarball file that is downloaded from the netCDF
website. For example this document applied to API version No value for
VERSION.

The good news is that all netCDF files ever written can always be read
by the latest netCDF release. That is, we guarantee backward data
compatibility.

* * * * *

  ------------------------------------------------------- ---------------------------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Versions "Previous section in reading order")]   [[\>](#g_t64_002dbit-Offset-Format "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Versions "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ---------------------------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 1.9.1 Classic Format 

The default format is classic format. This is the original netCDF binary
format - the format that the netCDF library has been using for almost 20
years, since its introduction with the first release of netCDF. No
special flag is needed to create a file in classic format; it is the
default.

Classic format has some strict limitations for files larger than two
gigabytes. (see [(netcdf)NetCDF Classic Format
Limitations](netcdf.html#NetCDF-Classic-Format-Limitations) section
‘NetCDF Classic Format Limitations’ in The NetCDF Users Guide).

* * * * *

  ------------------------------------------------------------- ----------------------------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Classic-Format "Previous section in reading order")]   [[\>](#NetCDF_002d4_002fHDF5-Format "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Versions "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ----------------------------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 1.9.2 64-bit Offset Format 

In December, 2004, version 3.6.0 of the netCDF library was released. It
allows users to use a new version of the netCDF file format which
greatly expands the sizes of variables and files which may be written.

The format which was introduced in 3.6.0 is called “64-bit Offset
Format.”

Create files in this format by passing the 64-bit offset format flag to
the create call (for example, in C, set the NC\_64BIT\_OFFSET flag when
calling the function nc\_create. (see
[(netcdf-c)nc\_create](netcdf-c.html#nc_005fcreate) section ‘nc\_create’
in The NetCDF C Interface Guide).

64-bit offset is very useful for very large data files (over two
gigabytes), however these files can only be shared with those who have
upgraded to version 3.6.0 (or better) of netCDF. Earlier versions of
netCDF will not be able to read these files.

* * * * *

  -------------------------------------------------------------------------- ------------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#g_t64_002dbit-Offset-Format "Previous section in reading order")]   [[\>](#Sharing-Data "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Versions "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ------------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 1.9.3 NetCDF-4/HDF5 Format 

With version 4.0 of netCDF, we introduce another new data format:
netCDF-4/HDF5 format. This format is HDF5, with full use of the new
dimension scales, creation ordering, and other features of HDF5 added in
its version 1.8.0 release.

As with 64-bit offset, this format is turned on when the file is
created. (For example, with the nf\_netcdf4 flag in the nf\_create
function. see [(netcdf-f77)nf\_create](netcdf-f77.html#nf_005fcreate)
section ‘nf\_create’ in The NetCDF Fortran 77 Interface Guide).

* * * * *

  --------------------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF_002d4_002fHDF5-Format "Previous section in reading order")]   [[\>](#Classic-Model "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Versions "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 1.9.4 Sharing Data 

The classic format is the most portable. Classic format files can be
read correctly by any version of netCDF. A netCDF-4 user can create a
classic file, and share it with a user who has not upgraded netCDF since
the version 2.3 in 1994.

64-bit offset format files can be read by any user who has at least
version 3.6.0 of the netCDF API (released in Dec., 2004).

Users must have netCDF 4.0 to read netCDF-4/HDF5 files. However,
netCDF-4 does produce backward compatible classic and 64-bit offset
format files. That is, a netCDF-4.0 user can create a classic format
file, and share it with researchers who are still using a old version of
netCDF. Similarly a netCDF-4.0 user can read any existing netCDF data
file, whatever version of netCDF was used to create it.

* * * * *

  ----------------------------------------------------------- --------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Sharing-Data "Previous section in reading order")]   [[\>](#Examples "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Versions "Up section")]   [[\>\>](#Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- --------------------------------------------------- --- ------------------------------------------------------------------ -------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 1.9.5 Classic Model 

The original netCDF API represents a data model as well as a programming
API. That is, the idea of variables, attributes, and the six data types
(char, byte, short, integer, float, and double), comprises a model of
how data may be stored.

The netCDF-4 release expands this model with groups, user-defined types,
and new base types. New functions have been added to the APIs to
accommodate these extensions, but once they are used, the file can no
longer be output as a classic format file.

That is, once you use groups in your code, you can only produce
netCDF-4/HDF5 files. If you try to change the format to classic, you
will get an error when you attempt to use any of the group functions.

Since it is convenient to be able to produce files of all formats from
the same code (restricting oneself to the classic data model), a flag
has been added which, when used in the creation of a netCDF-4/HDF5 file,
will instruct the library to disallow the use of any advanced features
in the file.

This is referred to as a “classic model” netCDF-4/HDF5 file.

To get a classic model file, use the classic model flag when creating
the file. For example, in Fortran 77, use the nf\_classic\_model flag
when calling nf\_create (see
[(netcdf-f77)nf\_create](netcdf-f77.html#nf_005fcreate) section
‘nf\_create’ in The NetCDF Fortran 77 Interface Guide).

For more information about format issues see [The NetCDF Users Guide:
(netcdf)Format](netcdf.html#Format) section ‘Format’ in The NetCDF Users
Guide.

* * * * *

  ------------------------------------------------------------ -------------------------------------------------------- --- ------------------------------------------------------------------ --------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Classic-Model "Previous section in reading order")]   [[\>](#simple_005fxy "Next section in reading order")]       [[\<\<](#Intro "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ -------------------------------------------------------- --- ------------------------------------------------------------------ --------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2. Example Programs 
===================

The netCDF example programs show how to use netCDF.

In the netCDF distribution, the “examples” directory contains examples
in C, Fortran 77, Fortran 90, C++, and CDL.

There are three sets of netCDF-3 example programs in each language. Each
language has its own subdirectory under the “examples” directory (for
example, the Fortran 77 examples are in “examples/F77”).

There is also one example for netCDF-4, which is only provided in the C
language. This example will only be run if the –enable-netcdf-4 option
was used with configure.

The examples are built and run with the “make check” command. (For more
information on building netCDF, see [The NetCDF Installation and Porting
Guide: (netcdf-install)Top](netcdf-install.html#Top) section ‘Top’ in
The NetCDF Installation and Porting Guide).

The examples create, and then read, example data files of increasing
complexity.

The corresponding examples in each language create identical netCDF data
files. For example, the C program sfc\_pres\_temp\_wr.c produces the
same data file as the Fortran 77 program sfc\_pres\_temp\_wr.f.

For convenience, the complete source code in each language can be found
in this tutorial, as well as in the netCDF distribution.

  ----------------------------------------------------------- ---- -----------------------------------------
  [2.1 The simple\_xy Example](#simple_005fxy)                     A very simple netCDF file.
  [2.2 The sfc\_pres\_temp Example](#sfc_005fpres_005ftemp)        A more complex file with more metadata.
  [2.3 The pres\_temp\_4D Example](#pres_005ftemp_005f4D)          A 4D file with an unlimited dimension.
  ----------------------------------------------------------- ---- -----------------------------------------

* * * * *

  ------------------------------------------------------- ------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Examples "Previous section in reading order")]   [[\>](#simple_005fxy-in-C "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#Examples "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.1 The simple\_xy Example 
--------------------------

This example is an unrealistically simple netCDF file, to demonstrate
the minimum operation of the netCDF APIs. Users should seek to make
their netCDF files more self-describing than this primitive example.

As in all the netCDF tutorial examples, this example file is created by
C, Fortran 77, Fortran 90, and C++ programs, and by ncgen, which creates
it from a CDL script. All examples create identical files,
“simple\_xy.nc.”

The programs that create this sample file all have the base name
“simple\_xy\_wr”, with different extensions depending on the language.

Therefore the example files that create simple\_xy.nc can be found in:
C/simple\_xy\_wr.c, F77/simple\_xy\_wr.f, F90/simple\_xy\_wr.f90,
CXX/simple\_xy\_wr.cpp, and CDL/simple\_xy\_wr.cdl.

Corresponding read programs (C/simple\_xy\_rd.c, etc.) read the
simple\_xy.nc data file, and ensure that it contains the correct values.

The simple\_xy.nc data file contains two dimensions, “x” and “y”, and
one netCDF variable, “data.”

The utility ncdump can be used to show the contents of netCDF files. By
default, ncdump shows the CDL description of the file. This CDL
description can be fed into ncgen to create the data file.

The CDL for this example is shown below. For more information on ncdump
and ncgen see [(netcdf)NetCDF Utilities](netcdf.html#NetCDF-Utilities)
section ‘NetCDF Utilities’ in The NetCDF Users Guide.

  ---------------------------------------------------------------------------------- ---- --
  [2.1.1 simple\_xy\_wr.c and simple\_xy\_rd.c](#simple_005fxy-in-C)                      
  [2.1.2 simple\_xy\_wr.f and simple\_xy\_rd.f](#simple_005fxy-in-F77)                    
  [2.1.3 simple\_xy\_wr.f90 and simple\_xy\_rd.f90](#simple_005fxy-in-F90)                
  [2.1.4 simple\_xy\_wr.cpp and simple\_xy\_rd.cpp](#simple_005fxy-in-C_002b_002b)        
  ---------------------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------ --------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy "Previous section in reading order")]   [[\>](#simple_005fxy_005fwr_002ec "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ --------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.1.1 simple\_xy\_wr.c and simple\_xy\_rd.c 

These example programs can be found in the netCDF distribution, under
examples/C.

The example program simple\_xy\_wr.c creates the example data file
simple\_xy.nc. The example program simple\_xy\_rd.c reads the data file.

  --------------------------------------------------------- ---- --
  [2.1.1.1 simple\_xy\_wr.c](#simple_005fxy_005fwr_002ec)        
  [2.1.1.2 simple\_xy\_rd.c](#simple_005fxy_005frd_002ec)        
  --------------------------------------------------------- ---- --

* * * * *

  ----------------------------------------------------------------- --------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy-in-C "Previous section in reading order")]   [[\>](#simple_005fxy_005frd_002ec "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy-in-C "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- --------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.1.1.1 simple\_xy\_wr.c 

* * * * *

  ------------------------------------------------------------------------- --------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fwr_002ec "Previous section in reading order")]   [[\>](#simple_005fxy-in-F77 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy-in-C "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- --------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.1.1.2 simple\_xy\_rd.c 

* * * * *

  ------------------------------------------------------------------------- --------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005frd_002ec "Previous section in reading order")]   [[\>](#simple_005fxy_005fwr_002ef "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- --------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.1.2 simple\_xy\_wr.f and simple\_xy\_rd.f 

These example programs can be found in the netCDF distribution, under
examples/F77.

The example program simple\_xy\_wr.f creates the example data file
simple\_xy.nc. The example program simple\_xy\_rd.f reads the data file.

  --------------------------------------------------------- ---- --
  [2.1.2.1 simple\_xy\_wr.f](#simple_005fxy_005fwr_002ef)        
  [2.1.2.2 simple\_xy\_rd.f](#simple_005fxy_005frd_002ef)        
  --------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------- --------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy-in-F77 "Previous section in reading order")]   [[\>](#simple_005fxy_005frd_002ef "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy-in-F77 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------- --------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.1.2.1 simple\_xy\_wr.f 

* * * * *

  ------------------------------------------------------------------------- --------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fwr_002ef "Previous section in reading order")]   [[\>](#simple_005fxy-in-F90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy-in-F77 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- --------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.1.2.2 simple\_xy\_rd.f 

* * * * *

  ------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005frd_002ef "Previous section in reading order")]   [[\>](#simple_005fxy_005fwr_002ef90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.1.3 simple\_xy\_wr.f90 and simple\_xy\_rd.f90 

These example programs can be found in the netCDF distribution, under
examples/F90.

The example program simple\_xy\_wr.f90 creates the example data file
simple\_xy.nc. The example program simple\_xy\_rd.f90 reads the data
file.

  ------------------------------------------------------------- ---- --
  [2.1.3.1 simple\_xy\_wr.f90](#simple_005fxy_005fwr_002ef90)        
  [2.1.3.2 simple\_xy\_rd.f90](#simple_005fxy_005frd_002ef90)        
  ------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy-in-F90 "Previous section in reading order")]   [[\>](#simple_005fxy_005frd_002ef90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy-in-F90 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.1.3.1 simple\_xy\_wr.f90 

* * * * *

  --------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fwr_002ef90 "Previous section in reading order")]   [[\>](#simple_005fxy-in-C_002b_002b "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy-in-F90 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.1.3.2 simple\_xy\_rd.f90 

* * * * *

  --------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005frd_002ef90 "Previous section in reading order")]   [[\>](#simple_005fxy_005fwr_002ecpp "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.1.4 simple\_xy\_wr.cpp and simple\_xy\_rd.cpp 

These example programs can be found in the netCDF distribution, under
examples/CXX.

The example program simple\_xy\_wr.cpp creates the example data file
simple\_xy.nc. The example program simple\_xy\_rd.cpp reads the data
file.

  ------------------------------------------------------------- ---- --
  [2.1.4.1 simple\_xy\_wr.cpp](#simple_005fxy_005fwr_002ecpp)        
  [2.1.4.2 simple\_xy\_rd.cpp](#simple_005fxy_005frd_002ecpp)        
  ------------------------------------------------------------- ---- --

* * * * *

  --------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy-in-C_002b_002b "Previous section in reading order")]   [[\>](#simple_005fxy_005frd_002ecpp "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy-in-C_002b_002b "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.1.4.1 simple\_xy\_wr.cpp 

* * * * *

  --------------------------------------------------------------------------- ---------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fwr_002ecpp "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy-in-C_002b_002b "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ---------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.1.4.2 simple\_xy\_rd.cpp 

* * * * *

  --------------------------------------------------------------------------- --------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005frd_002ecpp "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp-in-C "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#Examples "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- --------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.2 The sfc\_pres\_temp Example 
-------------------------------

This example has been constructed for the meteorological mind.

Suppose you have some data you want to write to a netCDF file. For
example, you have one time step of surface temperature and surface
pressure, on a 6 x 12 latitude longitude grid.

To store this in netCDF, create a file, add two dimensions (latitude and
longitude) and two variables (pressure and temperature).

In this example we add some netCDF attributes, as is typical in
scientific applications, to further describe the data. In this case we
add a units attribute to every netCDF variable.

In this example we also add additional netCDF variables to describe the
coordinate system. These “coordinate variables” allow us to specify the
latitudes and longitudes that describe the data grid.

The CDL version of the data file, generated by ncdump, is shown below.

For more information on ncdump and ncgen see [(netcdf)NetCDF
Utilities](netcdf.html#NetCDF-Utilities) section ‘NetCDF Utilities’ in
The NetCDF Users Guide.

  ---------------------------------------------------------------------------------------------------- ---- --
  [2.2.1 sfc\_pres\_temp\_wr.c and sfc\_pres\_temp\_rd.c](#sfc_005fpres_005ftemp-in-C)                      
  [2.2.2 sfc\_pres\_temp\_wr.f and sfc\_pres\_temp\_rd.f](#sfc_005fpres_005ftemp-in-F77)                    
  [2.2.3 sfc\_pres\_temp\_wr.f90 and sfc\_pres\_temp\_rd.f90](#sfc_005fpres_005ftemp-in-F90)                
  [2.2.4 sfc\_pres\_temp\_wr.cpp and sfc\_pres\_temp\_rd.cpp](#sfc_005fpres_005ftemp-in-C_002b_002b)        
  ---------------------------------------------------------------------------------------------------- ---- --

* * * * *

  -------------------------------------------------------------------- ----------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp_005fwr_002ec "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ----------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.2.1 sfc\_pres\_temp\_wr.c and sfc\_pres\_temp\_rd.c 

These example programs can be found in the netCDF distribution, under
examples/C.

The example program sfc\_pres\_temp\_wr.c creates the example data file
sfc\_pres\_temp.nc. The example program sfc\_pres\_temp\_rd.c reads the
data file.

  ---------------------------------------------------------------------- ---- --
  [2.2.1.1 sfc\_pres\_temp\_wr.c](#sfc_005fpres_005ftemp_005fwr_002ec)        
  [2.2.1.2 sfc\_pres\_temp\_rd.c](#sfc_005fpres_005ftemp_005frd_002ec)        
  ---------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp-in-C "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp_005frd_002ec "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp-in-C "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.2.1.1 sfc\_pres\_temp\_wr.c 

* * * * *

  --------------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp_005fwr_002ec "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp-in-F77 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp-in-C "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.2.1.2 sfc\_pres\_temp\_rd.c 

* * * * *

  --------------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp_005frd_002ec "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp_005fwr_002ef "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.2.2 sfc\_pres\_temp\_wr.f and sfc\_pres\_temp\_rd.f 

These example programs can be found in the netCDF distribution, under
examples/F77.

The example program sfc\_pres\_temp\_wr.f creates the example data file
sfc\_pres\_temp.nc. The example program sfc\_pres\_temp\_rd.f reads the
data file.

  ---------------------------------------------------------------------- ---- --
  [2.2.2.1 sfc\_pres\_temp\_wr.f](#sfc_005fpres_005ftemp_005fwr_002ef)        
  [2.2.2.2 sfc\_pres\_temp\_rd.f](#sfc_005fpres_005ftemp_005frd_002ef)        
  ---------------------------------------------------------------------- ---- --

* * * * *

  --------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp-in-F77 "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp_005frd_002ef "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp-in-F77 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.2.2.1 sfc\_pres\_temp\_wr.f 

* * * * *

  --------------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp_005fwr_002ef "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp-in-F90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp-in-F77 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ----------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.2.2.2 sfc\_pres\_temp\_rd.f 

* * * * *

  --------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp_005frd_002ef "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp_005fwr_002ef90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.2.3 sfc\_pres\_temp\_wr.f90 and sfc\_pres\_temp\_rd.f90 

These example programs can be found in the netCDF distribution, under
examples/F90.

The example program sfc\_pres\_temp\_wr.f90 creates the example data
file sfc\_pres\_temp.nc. The example program sfc\_pres\_temp\_rd.f90
reads the data file.

  -------------------------------------------------------------------------- ---- --
  [2.2.3.1 sfc\_pres\_temp\_wr.f90](#sfc_005fpres_005ftemp_005fwr_002ef90)        
  [2.2.3.2 sfc\_pres\_temp\_rd.f90](#sfc_005fpres_005ftemp_005frd_002ef90)        
  -------------------------------------------------------------------------- ---- --

* * * * *

  --------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp-in-F90 "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp_005frd_002ef90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp-in-F90 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.2.3.1 sfc\_pres\_temp\_wr.f90 

* * * * *

  ----------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp_005fwr_002ef90 "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp-in-C_002b_002b "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp-in-F90 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- ---------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.2.3.2 sfc\_pres\_temp\_rd.f90 

* * * * *

  ----------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp_005frd_002ef90 "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp_005fwr_002ecpp "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.2.4 sfc\_pres\_temp\_wr.cpp and sfc\_pres\_temp\_rd.cpp 

These example programs can be found in the netCDF distribution, under
examples/CXX.

The example program sfc\_pres\_temp\_wr.cpp creates the example data
file sfc\_pres\_temp.nc. The example program sfc\_pres\_temp\_rd.cpp
reads the data file.

  -------------------------------------------------------------------------- ---- --
  [2.2.4.1 sfc\_pres\_temp\_wr.cpp](#sfc_005fpres_005ftemp_005fwr_002ecpp)        
  [2.2.4.2 sfc\_pres\_temp\_rd.cpp](#sfc_005fpres_005ftemp_005frd_002ecpp)        
  -------------------------------------------------------------------------- ---- --

* * * * *

  ----------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp-in-C_002b_002b "Previous section in reading order")]   [[\>](#sfc_005fpres_005ftemp_005frd_002ecpp "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp-in-C_002b_002b "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.2.4.1 sfc\_pres\_temp\_wr.cpp 

* * * * *

  ----------------------------------------------------------------------------------- --------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp_005fwr_002ecpp "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#sfc_005fpres_005ftemp-in-C_002b_002b "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- --------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.2.4.2 sfc\_pres\_temp\_rd.cpp 

* * * * *

  ----------------------------------------------------------------------------------- -------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#sfc_005fpres_005ftemp_005frd_002ecpp "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D-in-C "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#Examples "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- -------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.3 The pres\_temp\_4D Example 
------------------------------

This example expands on the previous example by making our
two-dimensional data into four-dimensional data, adding a vertical level
axis and an unlimited time step axis.

Additionally, in this example the data are written and read one time
step at a time, as is typical in scientific applications that use the
unlimited dimension.

The sample data file created by pres\_temp\_4D\_wr can be examined with
the utility ncdump. The output is shown below. For more information on
ncdump see [(netcdf)NetCDF Utilities](netcdf.html#NetCDF-Utilities)
section ‘NetCDF Utilities’ in The NetCDF Users Guide.

  ------------------------------------------------------------------------------------------------- ---- --
  [2.3.1 pres\_temp\_4D\_wr.c and pres\_temp\_4D\_rd.c](#pres_005ftemp_005f4D-in-C)                      
  [2.3.2 pres\_temp\_4D\_wr.f and pres\_temp\_4D\_rd.f](#pres_005ftemp_005f4D-in-F77)                    
  [2.3.3 pres\_temp\_4D\_wr.f90 and pres\_temp\_4D\_rd.f90](#pres_005ftemp_005f4D-in-F90)                
  [2.3.4 pres\_temp\_4D\_wr.cpp and pres\_temp\_4D\_rd.cpp](#pres_005ftemp_005f4D-in-C_002b_002b)        
  ------------------------------------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------- ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D_005fwr_002ec "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------- ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.3.1 pres\_temp\_4D\_wr.c and pres\_temp\_4D\_rd.c 

These example programs can be found in the netCDF distribution, under
examples/C.

The example program pres\_temp\_4D\_wr.c creates the example data file
pres\_temp\_4D.nc. The example program pres\_temp\_4D\_rd.c reads the
data file.

  -------------------------------------------------------------------- ---- --
  [2.3.1.1 pres\_temp\_4D\_wr.c](#pres_005ftemp_005f4D_005fwr_002ec)        
  [2.3.1.2 pres\_temp\_4D\_rd.c](#pres_005ftemp_005f4D_005frd_002ec)        
  -------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------------ ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D-in-C "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D_005frd_002ec "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D-in-C "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.3.1.1 pres\_temp\_4D\_wr.c 

* * * * *

  -------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D_005fwr_002ec "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D-in-F77 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D-in-C "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- --------------------------------------------------------------------- ------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.3.1.2 pres\_temp\_4D\_rd.c 

* * * * *

  -------------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D_005frd_002ec "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D_005fwr_002ef "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.3.2 pres\_temp\_4D\_wr.f and pres\_temp\_4D\_rd.f 

These example programs can be found in the netCDF distribution, under
examples/F77.

The example program pres\_temp\_4D\_wr.f creates the example data file
pres\_temp\_4D.nc. The example program pres\_temp\_4D\_rd.f reads the
data file.

  -------------------------------------------------------------------- ---- --
  [2.3.2.1 pres\_temp\_4D\_wr.f](#pres_005ftemp_005f4D_005fwr_002ef)        
  [2.3.2.2 pres\_temp\_4D\_rd.f](#pres_005ftemp_005f4D_005frd_002ef)        
  -------------------------------------------------------------------- ---- --

* * * * *

  -------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D-in-F77 "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D_005frd_002ef "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D-in-F77 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.3.2.1 pres\_temp\_4D\_wr.f 

* * * * *

  -------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D_005fwr_002ef "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D-in-F90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D-in-F77 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.3.2.2 pres\_temp\_4D\_rd.f 

* * * * *

  -------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D_005frd_002ef "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D_005fwr_002ef90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.3.3 pres\_temp\_4D\_wr.f90 and pres\_temp\_4D\_rd.f90 

These example programs can be found in the netCDF distribution, under
examples/F90.

The example program pres\_temp\_4D\_wr.f90 creates the example data file
pres\_temp\_4D.nc. The example program pres\_temp\_4D\_rd.f90 reads the
data file.

  ------------------------------------------------------------------------ ---- --
  [2.3.3.1 pres\_temp\_4D\_wr.f90](#pres_005ftemp_005f4D_005fwr_002ef90)        
  [2.3.3.2 pres\_temp\_4D\_rd.f90](#pres_005ftemp_005f4D_005frd_002ef90)        
  ------------------------------------------------------------------------ ---- --

* * * * *

  -------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- --------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D-in-F90 "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D_005frd_002ef90 "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D-in-F90 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- --------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.3.3.1 pres\_temp\_4D\_wr.f90 

* * * * *

  ---------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- --------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D_005fwr_002ef90 "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D-in-C_002b_002b "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D-in-F90 "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- --------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.3.3.2 pres\_temp\_4D\_rd.f90 

* * * * *

  ---------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D_005frd_002ef90 "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D_005fwr_002ecpp "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 2.3.4 pres\_temp\_4D\_wr.cpp and pres\_temp\_4D\_rd.cpp 

These example programs can be found in the netCDF distribution, under
examples/CXX.

The example program pres\_temp\_4D\_wr.cpp creates the example data file
pres\_temp\_4D.nc. The example program pres\_temp\_4D\_rd.cpp reads the
data file.

  ------------------------------------------------------------------------ ---- --
  [2.3.4.1 pres\_temp\_4D\_wr.cpp](#pres_005ftemp_005f4D_005fwr_002ecpp)        
  [2.3.4.2 pres\_temp\_4D\_rd.cpp](#pres_005ftemp_005f4D_005frd_002ecpp)        
  ------------------------------------------------------------------------ ---- --

* * * * *

  ---------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- ----------------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D-in-C_002b_002b "Previous section in reading order")]   [[\>](#pres_005ftemp_005f4D_005frd_002ecpp "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D-in-C_002b_002b "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- --------------------------------------------------------------------- ----------------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.3.4.1 pres\_temp\_4D\_wr.cpp 

* * * * *

  ---------------------------------------------------------------------------------- ----------------------------------------------------------- --- --------------------------------------------------------------------- ----------------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D_005fwr_002ecpp "Previous section in reading order")]   [[\>](#Useful-Functions "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#pres_005ftemp_005f4D-in-C_002b_002b "Up section")]   [[\>\>](#Useful-Functions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------- ----------------------------------------------------------- --- --------------------------------------------------------------------- ----------------------------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 2.3.4.2 pres\_temp\_4D\_rd.cpp 

* * * * *

  ---------------------------------------------------------------------------------- --------------------------------------------------- --- --------------------------------------------------------------------- --------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#pres_005ftemp_005f4D_005frd_002ecpp "Previous section in reading order")]   [[\>](#Creation "Next section in reading order")]       [[\<\<](#Examples "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------- --------------------------------------------------- --- --------------------------------------------------------------------- --------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3. The Functions You Need in NetCDF-3 
=====================================

The netCDF-3 C and Fortran APIs each have over 100 functions, but most
users need only a handful. Listed below are the essential netCDF
functions for four important tasks in netCDF: creating new files,
reading existing files, learning about a netCDF file of unknown
structure, and reading and writing subsets of data.

In each case the functions are presented for each of the four language
APIs: C, Fortran 77, Fortran 90, and C++, with hyper-links to the
detailed documentation of each function.

  --------------------------------------------------------------------- ---- ------------------------------------------
  [3.1 Creating New Files and Metadata, an Overview](#Creation)              Creating netCDF files, adding metadata.
  [3.2 Reading NetCDF Files of Known Structure](#Reading)                    Reading netCDF files of known structure.
  [3.3 Reading NetCDF Files of Unknown Structure](#Inquiry-Functions)        Learning about an unknown netCDF file.
  [3.4 Reading and Writing Subsets of Data](#Subsets)                        Reading and writing Subsets of data.
  --------------------------------------------------------------------- ---- ------------------------------------------

* * * * *

  --------------------------------------------------------------- -------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Useful-Functions "Previous section in reading order")]   [[\>](#Creation-in-C "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Useful-Functions "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- -------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.1 Creating New Files and Metadata, an Overview 
------------------------------------------------

To construct a netCDF file you need to:

 `create the file`
:   Specify the name, optionally the format: classic (the default) or
    64bit-offset.

 `define metadata`
:   Specify the names and types of dimensions, data variables, and
    attributes.

 `write data`
:   Write arrays of data from program variables to the netCDF file.
    Arrays of data may be written all at once, or in subsets.

 `close the file`
:   Close the file to flush all buffers to the disk and free all
    resources allocated for this file.

  ----------------------------------------------------------------- ---- --
  [3.1.1 Creating a NetCDF File in C](#Creation-in-C)                    
  [3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)         
  [3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)         
  [3.1.4 Creating a NetCDF File in C++](#Creation-in-C_002b_002b)        
  ----------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------- ---------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Creation "Previous section in reading order")]   [[\>](#Creation-in-F77 "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Creation "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ---------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.1.1 Creating a NetCDF File in C 

Use nc\_create to create a file. Then use nc\_def\_dim to define each
shared dimension. The data variables are then specified with
nc\_def\_var. Any attributes are added with nc\_put\_att. Finally, call
nc\_enddef to tell the library that you are done defining the metadata,
and ready to start writing the data.

After all data are written to the file, call nc\_close to ensure that
all buffers are flushed, and any resources associated with the open file
are returned to the operating system.

For a very simple example, See section [simple\_xy\_wr.c and
simple\_xy\_rd.c](#simple_005fxy-in-C).

For a typical sequence of calls to the C versions of these functions,
see See [Creating a NetCDF Dataset:
(netcdf-c)Creating](netcdf-c.html#Creating) section ‘Creating a NetCDF
Dataset’ in The NetCDF C Interface Guide.

  ---------------------------------------------------------------------------------------------------------------------------------------------- --------------------------
  [(netcdf-c)nc\_create](netcdf-c.html#nc_005fcreate) section ‘nc\_create’ in The NetCDF C Interface Guide                                       create a new netCDF file
  [(netcdf-c)nc\_def\_dim](netcdf-c.html#nc_005fdef_005fdim) section ‘nc\_def\_dim’ in The NetCDF C Interface Guide                              define a dimension
  [(netcdf-c)nc\_def\_var](netcdf-c.html#nc_005fdef_005fvar) section ‘nc\_def\_var’ in The NetCDF C Interface Guide                              define a variable
  [(netcdf-c)nc\_put\_att\_ type](netcdf-c.html#nc_005fput_005fatt_005f-type) section ‘nc\_put\_att\_ type’ in The NetCDF C Interface Guide      write attributes
  [(netcdf-c)nc\_enddef](netcdf-c.html#nc_005fenddef) section ‘nc\_enddef’ in The NetCDF C Interface Guide                                       leave define mode
  [(netcdf-c)nc\_put\_vara\_ type](netcdf-c.html#nc_005fput_005fvara_005f-type) section ‘nc\_put\_vara\_ type’ in The NetCDF C Interface Guide   write arrays of data
  [(netcdf-c)nc\_close](netcdf-c.html#nc_005fclose) section ‘nc\_close’ in The NetCDF C Interface Guide                                          close a file
  ---------------------------------------------------------------------------------------------------------------------------------------------- --------------------------

* * * * *

  ------------------------------------------------------------ ---------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Creation-in-C "Previous section in reading order")]   [[\>](#Creation-in-F90 "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Creation "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ---------------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.1.2 Creating a NetCDF File in Fortran 77 

Use NF\_CREATE to create a file. Then use NF\_DEF\_DIM to define each
shared dimension. The data variables are then specified with
NF\_DEF\_VAR. Any attributes are added with NF\_PUT\_ATT. Finally, call
NF\_ENDDEF to tell the library that you are done defining the metadata,
and ready to start writing the data.

After all data are written to the file, call NF\_CLOSE to ensure that
all buffers are flushed, and any resources associated with the open file
are returned to the operating system.

For a typical sequence of calls see [Creating a NetCDF Dataset:
(netcdf-f77)Creating](netcdf-f77.html#Creating) section ‘Creating a
NetCDF Dataset’ in The NetCDF Fortran 77 Interface Guide.

Fortran users take note: the netCDF Fortran 77 API consists of wrappers
around the functions of the netCDF C library. There is no Fortran 77
code in netCDF except for these wrappers, and tests to ensure that the
wrappers work.

The name of each Fortran function shows the outline of the C function it
wraps (for example, NF\_CREATE is a wrapper around nc\_create).

  ----------------------------------------------------------------------------------------------------------------------------------------------------------- --------------------------
  [(netcdf-f77)NF\_CREATE](netcdf-f77.html#NF_005fCREATE) section ‘NF\_CREATE’ in The NetCDF Fortran 77 Interface Guide                                       create a new netCDF file
  [(netcdf-f77)NF\_DEF\_DIM](netcdf-f77.html#NF_005fDEF_005fDIM) section ‘NF\_DEF\_DIM’ in The NetCDF Fortran 77 Interface Guide                              define a dimension
  [(netcdf-f77)NF\_DEF\_VAR](netcdf-f77.html#NF_005fDEF_005fVAR) section ‘NF\_DEF\_VAR’ in The NetCDF Fortran 77 Interface Guide                              define a variable
  [(netcdf-f77)NF\_PUT\_ATT\_ type](netcdf-f77.html#NF_005fPUT_005fATT_005f-type) section ‘NF\_PUT\_ATT\_ type’ in The NetCDF Fortran 77 Interface Guide      write an attribute
  [(netcdf-f77)NF\_ENDDEF](netcdf-f77.html#NF_005fENDDEF) section ‘NF\_ENDDEF’ in The NetCDF Fortran 77 Interface Guide                                       end define mode
  [(netcdf-f77)NF\_PUT\_VARA\_ type](netcdf-f77.html#NF_005fPUT_005fVARA_005f-type) section ‘NF\_PUT\_VARA\_ type’ in The NetCDF Fortran 77 Interface Guide   write arrays of data
  [(netcdf-f77)NF\_CLOSE](netcdf-f77.html#NF_005fCLOSE) section ‘NF\_CLOSE’ in The NetCDF Fortran 77 Interface Guide                                          close the netCDF file
  ----------------------------------------------------------------------------------------------------------------------------------------------------------- --------------------------

* * * * *

  -------------------------------------------------------------- ------------------------------------------------------------------ --- ----------------------------------------------------------------------------- -------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Creation-in-F77 "Previous section in reading order")]   [[\>](#Creation-in-C_002b_002b "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Creation "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- ------------------------------------------------------------------ --- ----------------------------------------------------------------------------- -------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.1.3 Creating a NetCDF File in Fortran 90 

Use NF90\_CREATE to create a file. Then use NF90\_DEF\_DIM to define
each shared dimension. The data variables are then specified with
NF90\_DEF\_VAR. Any attributes are added with NF90\_PUT\_ATT. Finally,
call NF90\_ENDDEF to tell the library that you are done defining the
metadata, and ready to start writing the data.

After all data are written to the file, call NF90\_CLOSE to ensure that
all buffers are flushed, and any resources associated with the open file
are returned to the operating system.

For a typical sequence of calls see [Creating a NetCDF Dataset:
(netcdf-f90)Creating](netcdf-f90.html#Creating) section ‘Creating a
NetCDF Dataset’ in The NetCDF Fortran 90 Interface Guide.

The netCDF Fortran 90 API calls the Fortran 77 API, which in turn calls
the netCDF C library.

The name of each Fortran function shows the outline of the F77 function
it wraps (for example, NF90\_CREATE is a wrapper around NF\_CREATE). The
F77 functions are, in turn, wrappers around the C functions.

  ----------------------------------------------------------------------------------------------------------------------------------------------------------------- -----------------------
  [(netcdf-f90)NF90\_CREATE](netcdf-f90.html#NF90_005fCREATE) section ‘NF90\_CREATE’ in The NetCDF Fortran 90 Interface Guide                                       create a netCDF file
  [(netcdf-f90)NF90\_DEF\_DIM](netcdf-f90.html#NF90_005fDEF_005fDIM) section ‘NF90\_DEF\_DIM’ in The NetCDF Fortran 90 Interface Guide                              define a dimension
  [(netcdf-f90)NF90\_DEF\_VAR](netcdf-f90.html#NF90_005fDEF_005fVAR) section ‘NF90\_DEF\_VAR’ in The NetCDF Fortran 90 Interface Guide                              define a variable
  [(netcdf-f90)NF90\_PUT\_ATT\_ type](netcdf-f90.html#NF90_005fPUT_005fATT_005f-type) section ‘NF90\_PUT\_ATT\_ type’ in The NetCDF Fortran 90 Interface Guide      write an attribute
  [(netcdf-f90)NF90\_ENDDEF](netcdf-f90.html#NF90_005fENDDEF) section ‘NF90\_ENDDEF’ in The NetCDF Fortran 90 Interface Guide                                       end define mode
  [(netcdf-f90)NF90\_PUT\_VARA\_ type](netcdf-f90.html#NF90_005fPUT_005fVARA_005f-type) section ‘NF90\_PUT\_VARA\_ type’ in The NetCDF Fortran 90 Interface Guide   write arrays of data
  [(netcdf-f90)NF90\_CLOSE](netcdf-f90.html#NF90_005fCLOSE) section ‘NF90\_CLOSE’ in The NetCDF Fortran 90 Interface Guide                                          close the netCDF file
  ----------------------------------------------------------------------------------------------------------------------------------------------------------------- -----------------------

* * * * *

  -------------------------------------------------------------- -------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Creation-in-F90 "Previous section in reading order")]   [[\>](#Reading "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Creation "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- -------------------------------------------------- --- ----------------------------------------------------------------------------- -------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.1.4 Creating a NetCDF File in C++ 

Create an instance of the NcFile class to create a netCDF file. Use its
add\_dim and add\_var methods to add dimensions and variables. The
add\_att method is available for both NcFile and NcVar.

Use the NcError class to specify error handling behavior.

For an example creating a simple file see
[simple\_xy\_wr.cpp](#simple_005fxy_005fwr_002ecpp). For a more complex
example see
[pres\_temp\_4D\_wr.cpp](#pres_005ftemp_005f4D_005fwr_002ecpp).

  ---------------------------------------------------------------------------------------------------------------------- ----------------------------------------------
  [(netcdf-cxx)Class NcFile](netcdf-cxx.html#Class-NcFile) section ‘Class NcFile’ in The NetCDF C++ Interface Guide      a C++ class to manipulate netCDF files
  [(netcdf-cxx)Class NcDim](netcdf-cxx.html#Class-NcDim) section ‘Class NcDim’ in The NetCDF C++ Interface Guide         a C++ class to manipulate netCDF dimensions
  [(netcdf-cxx)Class NcVar](netcdf-cxx.html#Class-NcVar) section ‘Class NcVar’ in The NetCDF C++ Interface Guide         a C++ class to manipulate netCDF variables
  [(netcdf-cxx)Class NcAtt](netcdf-cxx.html#Class-NcAtt) section ‘Class NcAtt’ in The NetCDF C++ Interface Guide         a C++ class to manipulate netCDF attributes
  [(netcdf-cxx)Class NcError](netcdf-cxx.html#Class-NcError) section ‘Class NcError’ in The NetCDF C++ Interface Guide   a C++ class to control netCDF error handling
  ---------------------------------------------------------------------------------------------------------------------- ----------------------------------------------

* * * * *

  ---------------------------------------------------------------------- ------------------------------------------------------------------ --- ----------------------------------------------------------------------------- ---------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Creation-in-C_002b_002b "Previous section in reading order")]   [[\>](#Numbering-of-NetCDF-IDs "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Useful-Functions "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- ------------------------------------------------------------------ --- ----------------------------------------------------------------------------- ---------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.2 Reading NetCDF Files of Known Structure 
-------------------------------------------

To read a netCDF file of known structure, you need to:

 `open the file`
:   Specify the file name and whether you want read-write or read-only
    access.

 `read variable or attribute data`
:   Read the data or attributes of interest.

 `close the file`
:   Release all resources associated with this file.

Use ncdump to learn the structure of a file (use the -h option). For
more information about ncdump see [(netcdf)NetCDF
Utilities](netcdf.html#NetCDF-Utilities) section ‘NetCDF Utilities’ in
The NetCDF Users Guide.

* * * * *

  ------------------------------------------------------ ------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading "Previous section in reading order")]   [[\>](#Reading-in-C "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Reading "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------ ------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.2.1 Numbering of NetCDF IDs 

In C, Fortran 77, and Fortran 90, netCDF objects are identified by an
integer: the ID. NetCDF functions use this ID to identify the object.
It’s helpful for the programmer to understand these IDs.

Open data files, dimensions, variables, and attributes are each numbered
independently, and are always numbered in the order in which they were
defined. (They also appear in this order in ncdump output.) Numbering
starts with 0 in C, and 1 in Fortran 77/90.

For example, the first variable defined in a file will have an ID of 0
in C programs, and 1 in Fortran programs, and functions that apply to a
variable will need to know the ID of the variable you mean.

(The numbering of files is an exception: file IDs are assigned by the
operating system when a file is opened, and are not permanently
associated with the file. IDs for netCDF dimensions and variables are
persistent, but deleting an attribute changes subsequent attribute
numbers.)

Although netCDF refers to everything by an integer id (varid, dimid,
attnum), there are inquiry functions which, given a name, will return an
ID. For example, in the C API, nc\_inq\_varid will take a character
string (the name), and give back the ID of the variable of that name.
The variable ID is then used in subsequent calls (to read the data, for
example).

Other inquiry functions exist to further describe the file. (see section
[Reading NetCDF Files of Unknown Structure](#Inquiry-Functions)).

  --------------------------------------------------------------------- ---- --
  [3.2.2 Reading a Known NetCDF File in C](#Reading-in-C)                    
  [3.2.3 Reading a Known NetCDF File in Fortran 77](#Reading-in-F77)         
  [3.2.4 Reading a Known NetCDF File in Fortran 90](#Reading-in-F90)         
  [3.2.5 Reading a Known NetCDF File in C++](#Reading-in-C_002b_002b)        
  --------------------------------------------------------------------- ---- --

* * * * *

  ---------------------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Numbering-of-NetCDF-IDs "Previous section in reading order")]   [[\>](#Reading-in-F77 "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Reading "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.2.2 Reading a Known NetCDF File in C 

For a typical sequence of calls to these C functions see [Reading a
NetCDF Dataset with Known Names: (netcdf-c)Reading
Known](netcdf-c.html#Reading-Known) section ‘Reading a NetCDF Dataset
with Known Names’ in The NetCDF C Interface Guide.

  ---------------------------------------------------------------------------------------------------------------------------------------------- ---------------------
  [(netcdf-c)nc\_open](netcdf-c.html#nc_005fopen) section ‘nc\_open’ in The NetCDF C Interface Guide                                             open a netCDF file
  [(netcdf-c)nc\_get\_att](netcdf-c.html#nc_005fget_005fatt) section ‘nc\_get\_att’ in The NetCDF C Interface Guide                              read an attribute
  [(netcdf-c)nc\_get\_vara\_ type](netcdf-c.html#nc_005fget_005fvara_005f-type) section ‘nc\_get\_vara\_ type’ in The NetCDF C Interface Guide   read arrays of data
  [(netcdf-c)nc\_close](netcdf-c.html#nc_005fclose) section ‘nc\_close’ in The NetCDF C Interface Guide                                          close the file
  ---------------------------------------------------------------------------------------------------------------------------------------------- ---------------------

* * * * *

  ----------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading-in-C "Previous section in reading order")]   [[\>](#Reading-in-F90 "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Reading "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.2.3 Reading a Known NetCDF File in Fortran 77 

For a typical sequence of calls to these functions see [Reading a NetCDF
Dataset with Known Names: (netcdf-f77)Reading
Known](netcdf-f77.html#Reading-Known) section ‘Reading a NetCDF Dataset
with Known Names’ in The NetCDF Fortran 77 Interface Guide.

  ----------------------------------------------------------------------------------------------------------------------------------------------------------- ---------------------
  [(netcdf-f77)NF\_OPEN](netcdf-f77.html#NF_005fOPEN) section ‘NF\_OPEN’ in The NetCDF Fortran 77 Interface Guide                                             open a netCDF file
  [(netcdf-f77)NF\_GET\_ATT](netcdf-f77.html#NF_005fGET_005fATT) section ‘NF\_GET\_ATT’ in The NetCDF Fortran 77 Interface Guide                              read an attribute
  [(netcdf-f77)NF\_GET\_VARA\_ type](netcdf-f77.html#NF_005fGET_005fVARA_005f-type) section ‘NF\_GET\_VARA\_ type’ in The NetCDF Fortran 77 Interface Guide   read arrays of data
  [(netcdf-f77)NF\_CLOSE](netcdf-f77.html#NF_005fCLOSE) section ‘NF\_CLOSE’ in The NetCDF Fortran 77 Interface Guide                                          close the file
  ----------------------------------------------------------------------------------------------------------------------------------------------------------- ---------------------

* * * * *

  ------------------------------------------------------------- ----------------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading-in-F77 "Previous section in reading order")]   [[\>](#Reading-in-C_002b_002b "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Reading "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ----------------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.2.4 Reading a Known NetCDF File in Fortran 90 

For a typical sequence of calls to these functions see [Reading a NetCDF
Dataset with Known Names: (netcdf-f90)Reading
Known](netcdf-f90.html#Reading-Known) section ‘Reading a NetCDF Dataset
with Known Names’ in The NetCDF Fortran 90 Interface Guide.

  ----------------------------------------------------------------------------------------------------------------------------------------- ---------------------
  [(netcdf-f90)NF90\_OPEN](netcdf-f90.html#NF90_005fOPEN) section ‘NF90\_OPEN’ in The NetCDF Fortran 90 Interface Guide                     open a netCDF file
  [(netcdf-f90)NF90\_GET\_ATT](netcdf-f90.html#NF90_005fGET_005fATT) section ‘NF90\_GET\_ATT’ in The NetCDF Fortran 90 Interface Guide      read an attribute
  [(netcdf-f90)NF90\_GET\_VARA](netcdf-f90.html#NF90_005fGET_005fVARA) section ‘NF90\_GET\_VARA’ in The NetCDF Fortran 90 Interface Guide   read arrays of data
  [(netcdf-f90)NF90\_CLOSE](netcdf-f90.html#NF90_005fCLOSE) section ‘NF90\_CLOSE’ in The NetCDF Fortran 90 Interface Guide                  close the file
  ----------------------------------------------------------------------------------------------------------------------------------------- ---------------------

* * * * *

  ------------------------------------------------------------- ------------------------------------------------------------ --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading-in-F90 "Previous section in reading order")]   [[\>](#Inquiry-Functions "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Reading "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ------------------------------------------------------------ --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.2.5 Reading a Known NetCDF File in C++ 

  ------------------------------------------------------------------------------------------------------------------- ---------------------------------------------
  [(netcdf-cxx)Class NcFile](netcdf-cxx.html#Class-NcFile) section ‘Class NcFile’ in The NetCDF C++ Interface Guide   a C++ class to manipulate netCDF files
  [(netcdf-cxx)Class NcDim](netcdf-cxx.html#Class-NcDim) section ‘Class NcDim’ in The NetCDF C++ Interface Guide      a C++ class to manipulate netCDF dimensions
  [(netcdf-cxx)Class NcVar](netcdf-cxx.html#Class-NcVar) section ‘Class NcVar’ in The NetCDF C++ Interface Guide      a C++ class to manipulate netCDF variables
  [(netcdf-cxx)Class NcAtt](netcdf-cxx.html#Class-NcAtt) section ‘Class NcAtt’ in The NetCDF C++ Interface Guide      a C++ class to manipulate netCDF attributes
  ------------------------------------------------------------------------------------------------------------------- ---------------------------------------------

* * * * *

  --------------------------------------------------------------------- ------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading-in-C_002b_002b "Previous section in reading order")]   [[\>](#Inquiry-in-C "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Useful-Functions "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.3 Reading NetCDF Files of Unknown Structure 
---------------------------------------------

Perhaps you would like to write your software to handle more general
cases, so that you don’t have to adjust your source every time the grid
size changes, or a variable is added to the file.

There are inquiry functions that let you find out everything you need to
know about a file. These functions contain “inq” or “INQ” in their
names.

Using the inquiry functions, it is possible to write code that will read
and understand any netCDF file, whatever its contents. (For example,
ncdump does just that.)

  ------------------------------------------------------------------- ---- --
  [3.3.1 Inquiry in C](#Inquiry-in-C)                                      
  [3.3.2 Inquiry in Fortran 77](#Inquiry-in-F77)                           
  [3.3.3 Inquiry in Fortran 90](#Inquiry-in-F90)                           
  [3.3.4 Inquiry Functions in the C++ API](#Inquiry-in-C_002b_002b)        
  ------------------------------------------------------------------- ---- --

* * * * *

  ---------------------------------------------------------------- ------------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Inquiry-Functions "Previous section in reading order")]   [[\>](#NULL-Parameters-in-Inquiry-Functions "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Inquiry-Functions "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- ------------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.3.1 Inquiry in C 

First use nc\_inq, which will tell you how many variables and global
attributes there are in the file.

Start with global attribute 0, and proceed to natts - 1, the number of
global attributes minus one. The nc\_inq\_att function will tell you the
name, type, and length of each global attribute.

Then start with dimid 0, and proceed to dimid ndims - 1, calling
nc\_inq\_dim. This will tell you the name and length of each dimension,
and whether it is unlimited.

Then start with varid 0, and proceed to varid nvars - 1, calling
nc\_inq\_var. This will tell you the number of dimensions of this
variable, and their associated IDs. It will also get the name and type
of this variable, and whether there are any attributes attached. If
there are attributes attached, use the nc\_inq\_att function to get
their names, types, and lengths.

(To read an attribute, use the appropriate nc\_get\_att\_\<TYPE\>
function, like nc\_get\_att\_int() to get the data from an attribute
that is an array of integers.)

There are also functions that return an item’s ID, given its name. To
find IDs from the names, use functions nc\_inq\_dimid, nc\_inq\_attnum,
and nc\_inq\_varid.

For a typical sequence of calls to these functions see [Reading a netCDF
Dataset with Unknown Names: (netcdf-c)Reading
Unknown](netcdf-c.html#Reading-Unknown) section ‘Reading a netCDF
Dataset with Unknown Names’ in The NetCDF C Interface Guide.

* * * * *

  ----------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------------ ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Inquiry-in-C "Previous section in reading order")]   [[\>](#Inquiry-in-F77 "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Inquiry-in-C "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------------ ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 3.3.1.1 NULL Parameters in Inquiry Functions 

With any of the C inquiry functions, a NULL pointer can be used to
ignore a return parameter. Consider the nc\_inq function:

 

~~~~ {.example}
EXTERNL int
nc_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp);
~~~~

If you call this with NULL for the last three parameters, you can learn
the number of dimensions without bothering about the number of
variables, number of global attributes, and the ID of the unlimited
dimension.

For further convenience, we provide functions like nc\_inq\_ndims, which
only finds the number of dimensions, exactly as if you had called
nc\_inq, with NULLs in all parameters except ndimsp. (In fact, this is
just what the nc\_inq\_ndims functions does).

  ---------------------------------------------------------------------------------------------------------------------------------------- ---------------------------------------------------------------------------------------
  [(netcdf-c)nc\_inq](netcdf-c.html#nc_005finq) section ‘nc\_inq’ in The NetCDF C Interface Guide                                          Find number of dimensions, variables, and global attributes, and the unlimited dimid.
  [(netcdf-c)nc\_inq\_att](netcdf-c.html#nc_005finq_005fatt) section ‘nc\_inq\_att’ in The NetCDF C Interface Guide                        Find attribute name, type, and length.
  [(netcdf-c)nc\_inq\_dim Family](netcdf-c.html#nc_005finq_005fdim-Family) section ‘nc\_inq\_dim Family’ in The NetCDF C Interface Guide   Find dimension name and length.
  [(netcdf-c)nc\_inq\_var](netcdf-c.html#nc_005finq_005fvar) section ‘nc\_inq\_var’ in The NetCDF C Interface Guide                        Find variable name, type, num dimensions, dim IDs, and num attributes.
  [(netcdf-c)nc\_inq\_dimid](netcdf-c.html#nc_005finq_005fdimid) section ‘nc\_inq\_dimid’ in The NetCDF C Interface Guide                  Find dimension ID from its name.
  [(netcdf-c)nc\_inq\_varid](netcdf-c.html#nc_005finq_005fvarid) section ‘nc\_inq\_varid’ in The NetCDF C Interface Guide                  Find variable ID from its name.
  [(netcdf-c)nc\_inq\_format](netcdf-c.html#nc_005finq_005fformat) section ‘nc\_inq\_format’ in The NetCDF C Interface Guide               Find file format: classic or 64-bit offset
  [(netcdf-c)nc\_inq\_libvers](netcdf-c.html#nc_005finq_005flibvers) section ‘nc\_inq\_libvers’ in The NetCDF C Interface Guide            Find the netCDF version. (Currently No value for VERSION).
  ---------------------------------------------------------------------------------------------------------------------------------------- ---------------------------------------------------------------------------------------

* * * * *

  ----------------------------------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NULL-Parameters-in-Inquiry-Functions "Previous section in reading order")]   [[\>](#Inquiry-in-F90 "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Inquiry-Functions "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.3.2 Inquiry in Fortran 77 

First use NF\_INQ, which will tell you how many variables and global
attributes there are in the file. Then start with varid 1, and proceed
to varid nvars, calling NF\_INQ\_VAR.

For a typical sequence of calls to these functions see [Reading a netCDF
Dataset with Unknown Names: (netcdf-f77)Reading
Unknown](netcdf-f77.html#Reading-Unknown) section ‘Reading a netCDF
Dataset with Unknown Names’ in The NetCDF Fortran 77 Interface Guide.

  --------------------------------------------------------------------------------------------------------------------------------------------- ---------------------------------------------------------------------------------------
  [(netcdf-f77)NF\_INQ](netcdf-f77.html#NF_005fINQ) section ‘NF\_INQ’ in The NetCDF Fortran 77 Interface Guide.                                 Find number of dimensions, variables, and global attributes, and the unlimited dimid.
  [(netcdf-f77)NF\_INQ\_DIM](netcdf-f77.html#NF_005fINQ_005fDIM) section ‘NF\_INQ\_DIM’ in The NetCDF Fortran 77 Interface Guide.               Find dimension name and length.
  [(netcdf-f77)NF\_INQ\_VARID](netcdf-f77.html#NF_005fINQ_005fVARID) section ‘NF\_INQ\_VARID’ in The NetCDF Fortran 77 Interface Guide.         Find variable ID from its name.
  [(netcdf-f77)NF\_INQ\_VAR](netcdf-f77.html#NF_005fINQ_005fVAR) section ‘NF\_INQ\_VAR’ in The NetCDF Fortran 77 Interface Guide.               Find variable name, type, num dimensions, dim IDs, and num attributes.
  [(netcdf-f77)NF\_INQ\_DIMID](netcdf-f77.html#NF_005fINQ_005fDIMID) section ‘NF\_INQ\_DIMID’ in The NetCDF Fortran 77 Interface Guide.         Find dimension ID from its name.
  [(netcdf-f77)NF\_INQ\_DIM](netcdf-f77.html#NF_005fINQ_005fDIM) section ‘NF\_INQ\_DIM’ in The NetCDF Fortran 77 Interface Guide.               Find dimension name and length.
  [(netcdf-f77)NF\_INQ\_ATT](netcdf-f77.html#NF_005fINQ_005fATT) section ‘NF\_INQ\_ATT’ in The NetCDF Fortran 77 Interface Guide.               Find attribute name, type, and length.
  [(netcdf-f77)NF\_INQ\_FORMAT](netcdf-f77.html#NF_005fINQ_005fFORMAT) section ‘NF\_INQ\_FORMAT’ in The NetCDF Fortran 77 Interface Guide.      Find file format: classic or 64-bit offset
  [(netcdf-f77)NF\_INQ\_LIBVERS](netcdf-f77.html#NF_005fINQ_005fLIBVERS) section ‘NF\_INQ\_LIBVERS’ in The NetCDF Fortran 77 Interface Guide.   Find the netCDF version. (Currently No value for VERSION).
  --------------------------------------------------------------------------------------------------------------------------------------------- ---------------------------------------------------------------------------------------

* * * * *

  ------------------------------------------------------------- ----------------------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Inquiry-in-F77 "Previous section in reading order")]   [[\>](#Inquiry-in-C_002b_002b "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Inquiry-Functions "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ----------------------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.3.3 Inquiry in Fortran 90 

First use NF90\_INQ, which will tell you how many variables and global
attributes there are in the file. Then start with varid 1, and proceed
to varid nvars, calling NF90\_INQ\_VAR.

For a typical sequence of calls to these functions, see See [Reading a
netCDF Dataset with Unknown Names: (netcdf-f90)Reading
Unknown](netcdf-f90.html#Reading-Unknown) section ‘Reading a netCDF
Dataset with Unknown Names’ in The NetCDF Fortran 90 Interface Guide.

  --------------------------------------------------------------------------------------------------------------------------------------------------- ---------------------------------------------------------------------------------------
  [(netcdf-f90)NF90\_INQ](netcdf-f90.html#NF90_005fINQ) section ‘NF90\_INQ’ in The NetCDF Fortran 90 Interface Guide.                                 Find number of dimensions, variables, and global attributes, and the unlimited dimid.
  [(netcdf-f90)NF90\_INQ\_DIM](netcdf-f90.html#NF90_005fINQ_005fDIM) section ‘NF90\_INQ\_DIM’ in The NetCDF Fortran 90 Interface Guide.               Find dimension name and length.
  [(netcdf-f90)NF90\_INQ\_VARID](netcdf-f90.html#NF90_005fINQ_005fVARID) section ‘NF90\_INQ\_VARID’ in The NetCDF Fortran 90 Interface Guide.         Find variable ID from its name.
  [(netcdf-f90)NF90\_INQ\_VAR](netcdf-f90.html#NF90_005fINQ_005fVAR) section ‘NF90\_INQ\_VAR’ in The NetCDF Fortran 90 Interface Guide.               Find variable name, type, num dimensions, dim IDs, and num attributes.
  [(netcdf-f90)NF90\_INQ\_DIMID](netcdf-f90.html#NF90_005fINQ_005fDIMID) section ‘NF90\_INQ\_DIMID’ in The NetCDF Fortran 90 Interface Guide.         Find dimension ID from its name.
  [(netcdf-f90)NF90\_INQ\_DIM](netcdf-f90.html#NF90_005fINQ_005fDIM) section ‘NF90\_INQ\_DIM’ in The NetCDF Fortran 90 Interface Guide.               Find dimension name and length.
  [(netcdf-f90)NF90\_INQ\_ATT](netcdf-f90.html#NF90_005fINQ_005fATT) section ‘NF90\_INQ\_ATT’ in The NetCDF Fortran 90 Interface Guide.               Find attribute name, type, and length.
  [(netcdf-f90)NF90\_INQ\_FORMAT](netcdf-f90.html#NF90_005fINQ_005fFORMAT) section ‘NF90\_INQ\_FORMAT’ in The NetCDF Fortran 90 Interface Guide.      Find file format: classic or 64-bit offset
  [(netcdf-f90)NF90\_INQ\_LIBVERS](netcdf-f90.html#NF90_005fINQ_005fLIBVERS) section ‘NF90\_INQ\_LIBVERS’ in The NetCDF Fortran 90 Interface Guide.   Find the netCDF version. (Currently No value for VERSION).
  --------------------------------------------------------------------------------------------------------------------------------------------------- ---------------------------------------------------------------------------------------

* * * * *

  ------------------------------------------------------------- -------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Inquiry-in-F90 "Previous section in reading order")]   [[\>](#Subsets "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Inquiry-Functions "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- -------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.3.4 Inquiry Functions in the C++ API 

  ------------------------------------------------------------------------------------------------------------------- ---------------------------------------------
  [(netcdf-cxx)Class NcFile](netcdf-cxx.html#Class-NcFile) section ‘Class NcFile’ in The NetCDF C++ Interface Guide   a C++ class to manipulate netCDF files
  [(netcdf-cxx)Class NcDim](netcdf-cxx.html#Class-NcDim) section ‘Class NcDim’ in The NetCDF C++ Interface Guide      a C++ class to manipulate netCDF dimensions
  [(netcdf-cxx)Class NcVar](netcdf-cxx.html#Class-NcVar) section ‘Class NcVar’ in The NetCDF C++ Interface Guide      a C++ class to manipulate netCDF variables
  [(netcdf-cxx)Class NcAtt](netcdf-cxx.html#Class-NcAtt) section ‘Class NcAtt’ in The NetCDF C++ Interface Guide      a C++ class to manipulate netCDF attributes
  ------------------------------------------------------------------------------------------------------------------- ---------------------------------------------

* * * * *

  --------------------------------------------------------------------- ---------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Inquiry-in-C_002b_002b "Previous section in reading order")]   [[\>](#Subsetting-in-C "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Useful-Functions "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ---------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.4 Reading and Writing Subsets of Data 
---------------------------------------

Usually users are interested in reading or writing subsets of variables
in a netCDF data file. The netCDF APIs provide a variety of functions
for this purpose.

In the simplest case, you will use the same type for both file and
in-memory storage, but in some cases you may wish to use different
types. For example, you might have a netCDF file that contains integer
data, and you wish to read it into floating-point storage, converting
the data as it is read. The same sort of type conversion can be done
when writing the data.

To convert to a type while reading data, use the appropriate
nc\_get\_vara\_\<TYPE\> or NF\_GET\_VARA\_\<TYPE\> function. For
example, the C function nc\_get\_vara\_float(), and the Fortran function
NF\_GET\_VARA\_REAL will read netCDF data of any numeric type into a
floating-point array, automatically converting each element to the
desired type.

To convert from a type while writing data, use the appropriate
nc\_put\_vara\_\<TYPE\> or NF\_PUT\_VARA\_\<TYPE\> function. For
example, the C function nc\_put\_vara\_float(), and the Fortran function
NC\_PUT\_VARA\_REAL will write floating-point data into netCDF arrays,
automatically converting each element of the array to the type of the
netCDF variable.

The \<TYPE\> in the function name refers to the type of the in-memory
data, in both cases. They type of the file data is determined when the
netCDF variable is defined.

  -------------------------------------------------------------------------------- ---- --
  [3.4.1 Reading and Writing Subsets of Data in C](#Subsetting-in-C)                    
  [3.4.2 Reading and Writing Subsets of Data in Fortran 77](#Subsetting-in-F77)         
  [3.4.3 Reading and Writing Subsets of Data in Fortran 90](#Subsetting-in-F90)         
  [3.4.4 Reading and Writing Subsets of Data in C++](#Subsetting-in-C_002b_002b)        
  -------------------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------ ------------------------------------------------------------ --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Subsets "Previous section in reading order")]   [[\>](#Subsetting-in-F77 "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Subsets "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------ ------------------------------------------------------------ --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.4.1 Reading and Writing Subsets of Data in C 

The type of the data may be automatically converted on read or write.
For more information about type conversion see [(netcdf-c)Type
Conversion](netcdf-c.html#Type-Conversion) section ‘Type Conversion’ in
The NetCDF C Interface Guide.

  ----------------------------------- ----------------------------------------------------------------------------------------------------------------------------------------------
  Read the entire variable at once    [(netcdf-c)nc\_get\_var\_ type](netcdf-c.html#nc_005fget_005fvar_005f-type) section ‘nc\_get\_var\_ type’ in The NetCDF C Interface Guide
  Write the entire variable at once   [(netcdf-c)nc\_put\_var\_ type](netcdf-c.html#nc_005fput_005fvar_005f-type) section ‘nc\_put\_var\_ type’ in The NetCDF C Interface Guide
  Read just one value                 [(netcdf-c)nc\_get\_var1\_ type](netcdf-c.html#nc_005fget_005fvar1_005f-type) section ‘nc\_get\_var1\_ type’ in The NetCDF C Interface Guide
  Write just one value                [(netcdf-c)nc\_put\_var1\_ type](netcdf-c.html#nc_005fput_005fvar1_005f-type) section ‘nc\_put\_var1\_ type’ in The NetCDF C Interface Guide
  Read an array subset                [(netcdf-c)nc\_get\_vara\_ type](netcdf-c.html#nc_005fget_005fvara_005f-type) section ‘nc\_get\_vara\_ type’ in The NetCDF C Interface Guide
  Write an array subset               [(netcdf-c)nc\_put\_vara\_ type](netcdf-c.html#nc_005fput_005fvara_005f-type) section ‘nc\_put\_vara\_ type’ in The NetCDF C Interface Guide
  Read an array with strides          [(netcdf-c)nc\_get\_vars\_ type](netcdf-c.html#nc_005fget_005fvars_005f-type) section ‘nc\_get\_vars\_ type’ in The NetCDF C Interface Guide
  Write an array with strides         [(netcdf-c)nc\_put\_vars\_ type](netcdf-c.html#nc_005fput_005fvars_005f-type) section ‘nc\_put\_vars\_ type’ in The NetCDF C Interface Guide
  ----------------------------------- ----------------------------------------------------------------------------------------------------------------------------------------------

* * * * *

  -------------------------------------------------------------- ------------------------------------------------------------ --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Subsetting-in-C "Previous section in reading order")]   [[\>](#Subsetting-in-F90 "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Subsets "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- ------------------------------------------------------------ --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.4.2 Reading and Writing Subsets of Data in Fortran 77 

The type of the data may be automatically converted on read or write.
For more information about type conversion see [(netcdf-f77)Type
Conversion](netcdf-f77.html#Type-Conversion) section ‘Type Conversion’
in The NetCDF Fortran 77 Interface Guide.

  ----------------------------------- -----------------------------------------------------------------------------------------------------------------------------------------------------------
  Read the entire variable at once    [(netcdf-f77)NF\_GET\_VAR\_ type](netcdf-f77.html#NF_005fGET_005fVAR_005f-type) section ‘NF\_GET\_VAR\_ type’ in The NetCDF Fortran 77 Interface Guide
  Write the entire variable at once   [(netcdf-f77)NF\_PUT\_VAR\_ type](netcdf-f77.html#NF_005fPUT_005fVAR_005f-type) section ‘NF\_PUT\_VAR\_ type’ in The NetCDF Fortran 77 Interface Guide
  Read just one value                 [(netcdf-f77)NF\_GET\_VAR1\_ type](netcdf-f77.html#NF_005fGET_005fVAR1_005f-type) section ‘NF\_GET\_VAR1\_ type’ in The NetCDF Fortran 77 Interface Guide
  Write just one value                [(netcdf-f77)NF\_PUT\_VAR1\_ type](netcdf-f77.html#NF_005fPUT_005fVAR1_005f-type) section ‘NF\_PUT\_VAR1\_ type’ in The NetCDF Fortran 77 Interface Guide
  Read an array subset                [(netcdf-f77)NF\_GET\_VARA\_ type](netcdf-f77.html#NF_005fGET_005fVARA_005f-type) section ‘NF\_GET\_VARA\_ type’ in The NetCDF Fortran 77 Interface Guide
  Write an array subset               [(netcdf-f77)NF\_PUT\_VARA\_ type](netcdf-f77.html#NF_005fPUT_005fVARA_005f-type) section ‘NF\_PUT\_VARA\_ type’ in The NetCDF Fortran 77 Interface Guide
  Read an array with strides          [(netcdf-f77)NF\_GET\_VARS\_ type](netcdf-f77.html#NF_005fGET_005fVARS_005f-type) section ‘NF\_GET\_VARS\_ type’ in The NetCDF Fortran 77 Interface Guide
  Write an array with strides         [(netcdf-f77)NF\_PUT\_VARS\_ type](netcdf-f77.html#NF_005fPUT_005fVARS_005f-type) section ‘NF\_PUT\_VARS\_ type’ in The NetCDF Fortran 77 Interface Guide
  ----------------------------------- -----------------------------------------------------------------------------------------------------------------------------------------------------------

* * * * *

  ---------------------------------------------------------------- -------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Subsetting-in-F77 "Previous section in reading order")]   [[\>](#Subsetting-in-C_002b_002b "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Subsets "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- -------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.4.3 Reading and Writing Subsets of Data in Fortran 90 

The type of the data may be automatically converted on read or write.
For more information about type conversion see [(netcdf-f90)Type
Conversion](netcdf-f90.html#Type-Conversion) section ‘Type Conversion’
in The NetCDF Fortran 90 Interface Guide.

  ----------------------------------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------
  Read the entire variable at once    [(netcdf-f90)NF90\_GET\_VAR\_ type](netcdf-f90.html#NF90_005fGET_005fVAR_005f-type) section ‘NF90\_GET\_VAR\_ type’ in The NetCDF Fortran 90 Interface Guide
  Write the entire variable at once   [(netcdf-f90)NF90\_PUT\_VAR\_ type](netcdf-f90.html#NF90_005fPUT_005fVAR_005f-type) section ‘NF90\_PUT\_VAR\_ type’ in The NetCDF Fortran 90 Interface Guide
  Read just one value                 [(netcdf-f90)NF90\_GET\_VAR1\_ type](netcdf-f90.html#NF90_005fGET_005fVAR1_005f-type) section ‘NF90\_GET\_VAR1\_ type’ in The NetCDF Fortran 90 Interface Guide
  Write just one value                [(netcdf-f90)NF90\_PUT\_VAR1\_ type](netcdf-f90.html#NF90_005fPUT_005fVAR1_005f-type) section ‘NF90\_PUT\_VAR1\_ type’ in The NetCDF Fortran 90 Interface Guide
  Read an array subset                [(netcdf-f90)NF90\_GET\_VARA\_ type](netcdf-f90.html#NF90_005fGET_005fVARA_005f-type) section ‘NF90\_GET\_VARA\_ type’ in The NetCDF Fortran 90 Interface Guide
  Write an array subset               [(netcdf-f90)NF90\_PUT\_VARA\_ type](netcdf-f90.html#NF90_005fPUT_005fVARA_005f-type) section ‘NF90\_PUT\_VARA\_ type’ in The NetCDF Fortran 90 Interface Guide
  Read an array with strides          [(netcdf-f90)NF90\_GET\_VARS\_ type](netcdf-f90.html#NF90_005fGET_005fVARS_005f-type) section ‘NF90\_GET\_VARS\_ type’ in The NetCDF Fortran 90 Interface Guide
  Write an array with strides         [(netcdf-f90)NF90\_PUT\_VARS\_ type](netcdf-f90.html#NF90_005fPUT_005fVARS_005f-type) section ‘NF90\_PUT\_VARS\_ type’ in The NetCDF Fortran 90 Interface Guide
  ----------------------------------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------

* * * * *

  ---------------------------------------------------------------- ------------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Subsetting-in-F90 "Previous section in reading order")]   [[\>](#API_002dExtensions "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Subsets "Up section")]   [[\>\>](#API_002dExtensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- ------------------------------------------------------------- --- ----------------------------------------------------------------------------- ------------------------------- ---------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 3.4.4 Reading and Writing Subsets of Data in C++ 

To read a record of data at a time, use the set\_cur method of the NcVar
class to set the number of the record of interest, and then use the get
method to read the record.

  ---------------------------------------------------------------------------------------------------------------- -----------------------------------------------------------------------------------------------------------
  [(netcdf-cxx)Class NcVar](netcdf-cxx.html#Class-NcVar) section ‘Class NcVar’ in The NetCDF C++ Interface Guide   a C++ class to manipulate netCDF variables, use the set\_cur and get methods to read records from a file.
  ---------------------------------------------------------------------------------------------------------------- -----------------------------------------------------------------------------------------------------------

* * * * *

  ------------------------------------------------------------------------ ----------------------------------------------------------- --- ----------------------------------------------------------------------------- --------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Subsetting-in-C_002b_002b "Previous section in reading order")]   [[\>](#Interoperability "Next section in reading order")]       [[\<\<](#Useful-Functions "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ ----------------------------------------------------------- --- ----------------------------------------------------------------------------- --------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4. API Extensions Introduced with NetCDF-4 
==========================================

NetCDF-4 includes many advanced features. These features are only
available when working with files created in the netCDF format. (That
is, HDF5 files, created by netCDF, or simple-model HDF5 files).

  ----------------------------------------------------------------------------- ---- ----------------------------------------
  [4.1 Interoperability with HDF5](#Interoperability)                                Reading and writing HDF5 files.
  [4.2 Multiple Unlimited Dimensions](#Multiple_002dUnlimited_002dDimensions)        Use more than one unlimited dimension.
  [4.3 Groups](#Groups)                                                              Organizing data hierarchically.
  [4.4 Compound Types](#Compound_002dTypes)                                          Creating data type like C structs.
  [4.5 Opaque Types](#Opaque_002dTypes)                                              Creating a data type of known size.
  [4.6 Variable Length Arrays (VLEN)](#VLEN_002dType)                                Variable length arrays.
  [4.7 Strings](#Strings)                                                            Storing strings of data.
  [4.8 New Inquiry Functions](#New_002dinq_002dFunctions)                            Functions to help explore a file.
  [4.9 Parallel I/O with NetCDF](#Parallel)                                          How to get parallel I/O.
  [4.10 The Future of NetCDF](#Future)                                               What’s coming next!
  ----------------------------------------------------------------------------- ---- ----------------------------------------

* * * * *

  ----------------------------------------------------------------- ------------------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#API_002dExtensions "Previous section in reading order")]   [[\>](#Reading-and-Editing-NetCDF_002d4-Files-with-HDF5 "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- ------------------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.1 Interoperability with HDF5 
------------------------------

NetCDF-4 allows some interoperability with HDF5.

* * * * *

  --------------------------------------------------------------- ------------------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ---------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Interoperability "Previous section in reading order")]   [[\>](#Reading-and-Editing-HDF5-Files-with-NetCDF_002d4 "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Interoperability "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- ------------------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ---------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 4.1.1 Reading and Editing NetCDF-4 Files with HDF5 

The HDF5 Files produced by netCDF-4 are perfectly respectable HDF5
files, and can be read by any HDF5 application.

NetCDF-4 relies on several new features of HDF5, including dimension
scales. The HDF5 dimension scales feature adds a bunch of attributes to
the HDF5 file to keep track of the dimension information.

It is not just wrong, but wrong-headed, to modify these attributes
except with the HDF5 dimension scale API. If you do so, then you will
deserve what you get, which will be a mess.

Additionally, netCDF stores some extra information for dimensions
without dimension scale information. (That is, a dimension without an
associated coordinate variable). So HDF5 users should not write data to
a netCDF-4 file which extends any unlimited dimension.

Also there are some types allowed in HDF5, but not allowed in netCDF-4
(for example the time type). Using any such type in a netCDF-4 file will
cause the file to become unreadable to netCDF-4. So don’t do it.

NetCDF-4 ignores all HDF5 references. Can’t make head nor tail of them.
Also netCDF-4 assumes a strictly hierarchical group structure. No
looping, you weirdo!

Attributes can be added (they must be one of the netCDF-4 types),
modified, or even deleted, in HDF5.

* * * * *

  ----------------------------------------------------------------------------------------------- -------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ---------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading-and-Editing-NetCDF_002d4-Files-with-HDF5 "Previous section in reading order")]   [[\>](#Multiple_002dUnlimited_002dDimensions "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Interoperability "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------------------- -------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ---------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 4.1.2 Reading and Editing HDF5 Files with NetCDF-4 

Assuming a HDF5 file is written in accordance with the netCDF-4 rules
(i.e. no strange types, no looping groups), and assuming that \*every\*
dataset has a dimension scale attached to each dimension, the netCDF-4
API can be used to read and edit the file.

In HDF5 (version 1.8.0 and later), dimension scales are (generally) 1D
datasets, that hold dimension data. A multi-dimensional dataset can then
attach a dimension scale to any or all of its dimensions. For example, a
user might have 1D dimension scales for lat and lon, and a 2D dataset
which has lat attached to the first dimension, and lon to the second.

Dimension scales are vital to netCDF-4, which uses shared dimensions. If
you want to read a HDF5 file with netCDF-4, it must use dimension
scales, and one dimension scale must be attached to each dimension of
every dataset in the file.

* * * * *

  ----------------------------------------------------------------------------------------------- ------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading-and-Editing-HDF5-Files-with-NetCDF_002d4 "Previous section in reading order")]   [[\>](#Groups "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------------------- ------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.2 Multiple Unlimited Dimensions 
---------------------------------

With classic and 64-bit offset netCDF files, each variable may use at
most one unlimited dimension. With netCDF-4 format files, this
restriction is lifted.

Simply define as many unlimited dimensions as you wish, and use them in
a variable. When data are written to that variable, the dimensions will
be expanded as needed.

* * * * *

  ------------------------------------------------------------------------------------ ------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Multiple_002dUnlimited_002dDimensions "Previous section in reading order")]   [[\>](#Compound_002dTypes "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------ ------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.3 Groups 
----------

NetCDF-4 files can store attributes, variables, and dimensions in
hierarchical groups.

This allows the user to create a structure much like a Unix file system.
In netCDF, each group gets an ncid. Opening or creating a file returns
the ncid for the root group (which is named “/”). Groups can be added
with the nc\_def\_grp function. Get the number of groups, and their
ncids, with the nc\_inq\_grps function.

Dimensions are scoped such that they are visible to all child groups.
For example, you can define a dimension in the root group, and use its
dimension id when defining a variable in a sub-group.

Attributes defined as NC\_GLOBAL apply to the group, not the entire
file.

The degenerate case, in which only the root group is used, corresponds
exactly with the classic data mode, before groups were introduced.

* * * * *

  ----------------------------------------------------- ----------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Groups "Previous section in reading order")]   [[\>](#Opaque_002dTypes "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- ----------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.4 Compound Types 
------------------

In netCDF-4 files it’s possible to create a data type which corresponds
to a C struct. These are known as “compound” types (following HDF5
nomenclature).

That is, a netCDF compound type is a data structure which contains an
arbitrary collection of other data types, including other compound
types.

To define a new compound type, use nc\_def\_compound. Then call
nc\_insert\_compound for each type within the compound type.

Read and write arrays of compound data with the nc\_get\_vara and
nc\_put\_vara functions. These functions were actually part of the
netCDF-2 API, brought out of semi-retirement to handle user-defined
types in netCDF-4.

* * * * *

  ----------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Compound_002dTypes "Previous section in reading order")]   [[\>](#VLEN_002dType "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.5 Opaque Types 
----------------

Store blobs of bits in opaque types. Create an opaque type with
nc\_def\_opaque. Read and write them with nc\_get\_vara/nc\_put\_vara.

* * * * *

  --------------------------------------------------------------- -------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Opaque_002dTypes "Previous section in reading order")]   [[\>](#Strings "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- -------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.6 Variable Length Arrays (VLEN) 
---------------------------------

Create a VLEN type to store variable length arrays of a known base type.
Use nc\_def\_vlen to define a VLEN type, read and write them with
nc\_get\_vara/nc\_put\_vara.

* * * * *

  ------------------------------------------------------------ -------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#VLEN_002dType "Previous section in reading order")]   [[\>](#New_002dinq_002dFunctions "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ -------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.7 Strings 
-----------

Use the NC\_STRING type to store arrays of strings. Read and write them
with nc\_get\_vara/nc\_put\_vara.

* * * * *

  ------------------------------------------------------ --------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Strings "Previous section in reading order")]   [[\>](#Parallel "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------ --------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.8 New Inquiry Functions 
-------------------------

There are many new inquiry functions to allow a program to navigate a
completely unknown netCDF file.

To find the number To find all the dimensions visible from a group, use
nc\_inq\_dimids.

* * * * *

  ------------------------------------------------------------------------ ------------------------------------------------------------------------------------ --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#New_002dinq_002dFunctions "Previous section in reading order")]   [[\>](#Parallel-I_002fO-Choices-for-NetCDF-Users "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ ------------------------------------------------------------------------------------ --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.9 Parallel I/O with NetCDF 
----------------------------

Parallel I/O allows many processes to read/write netCDF data at the same
time. Used properly, it allows users to overcome I/O bottlenecks in high
performance computing environments.

* * * * *

  ------------------------------------------------------- ----------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Parallel "Previous section in reading order")]   [[\>](#Parallel-I_002fO-with-NetCDF_002d4 "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Parallel "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ----------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 4.9.1 Parallel I/O Choices for NetCDF Users 

Parallel read-only access can be achieved netCDF files using the netCDF
C/Fortran library. Each process can run a copy of the netCDF library and
open and read any subsets of the data in the file. This sort of “fseek
parallelism” will break down dramatically for any kind of writing.

There are two methods available to users for read/write parallel I/O
netCDF-4 or the parallel netCDF package from Argonne/Northwestern.
Unfortunately the two methods involve different APIs, and different
binary formats.

For parallel read/write access to classic and 64-bit offset data users
must use the parallel-netcdf library from Argonne/Northwestern
University. This is not a Unidata software package, but was developed
using the Unidata netCDF C library as a starting point. For more
information see the parallel netcdf web site:
http://www.mcs.anl.gov/parallel-netcdf.

For parallel read/write access to netCDF-4/HDF5 files users must use the
netCDF-4 API. The Argonne/Northwestern parallel netcdf package cannot
read netCDF-4/HDF5 files.

* * * * *

  ---------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Parallel-I_002fO-Choices-for-NetCDF-Users "Previous section in reading order")]   [[\>](#Building-NetCDF_002d4-for-Parallel-I_002fO "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Parallel "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 4.9.2 Parallel I/O with NetCDF-4 

NetCDF-4 provides access to HDF5 parallel I/O features for netCDF-4/HDF5
files. NetCDF classic and 64-bit offset format may not be opened or
created for use with parallel I/O. (They may be opened and created, but
parallel I/O is not available.)

A few functions have been added to the netCDF C API to handle parallel
I/O. These functions are also available in the Fortran 90 and Fortran 77
APIs.

* * * * *

  --------------------------------------------------------------------------------- ------------------------------------------------------------------------------------------ --- ------------------------------------------------------------------------------- ---------------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Parallel-I_002fO-with-NetCDF_002d4 "Previous section in reading order")]   [[\>](#Opening_002fCreating-Files-for-Parallel-I_002fO "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Parallel-I_002fO-with-NetCDF_002d4 "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ------------------------------------------------------------------------------------------ --- ------------------------------------------------------------------------------- ---------------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 4.9.2.1 Building NetCDF-4 for Parallel I/O 

You must build netCDF-4 properly to take advantage of parallel features.

For parallel I/O HDF5 must be built with –enable-parallel. Typically the
CC environment variable is set to mpicc. You must build HDF5 and
netCDF-4 with the same compiler and compiler options.

The netCDF configure script will detect the parallel capability of HDF5
and build the netCDF-4 parallel I/O features automatically. No configure
options to the netcdf configure are required. If the Fortran APIs are
desired set environmental variable FC to mpif90 (or some local variant.)

* * * * *

  ----------------------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ---------------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Building-NetCDF_002d4-for-Parallel-I_002fO "Previous section in reading order")]   [[\>](#Collective_002fIndependent-Access "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Parallel-I_002fO-with-NetCDF_002d4 "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ---------------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 4.9.2.2 Opening/Creating Files for Parallel I/O 

The nc\_open\_par and nc\_create\_par functions are used to create/open
a netCDF file with the C API. (Or use nf\_open\_par/nf\_create\_par from
Fortran 77).

For Fortran 90 users the nf90\_open and nf90\_create calls have been
modified to permit parallel I/O files to be opened/created using
optional parameters comm and info.

The parallel access associated with these functions is not a
characteristic of the data file, but the way it was opened.

* * * * *

  ---------------------------------------------------------------------------------------------- --------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ---------------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Opening_002fCreating-Files-for-Parallel-I_002fO "Previous section in reading order")]   [[\>](#simple_005fxy_005fpar-in-C "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Parallel-I_002fO-with-NetCDF_002d4 "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------------------- --------------------------------------------------------------------- --- ------------------------------------------------------------------------------- ---------------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 4.9.2.3 Collective/Independent Access 

Parallel file access is either collective (all processors must
participate) or independent (any processor may access the data without
waiting for others).

All netCDF metadata writing operations are collective. That is, all
creation of groups, types, variables, dimensions, or attributes.

Data reads and writes (ex. calls to nc\_put\_vara\_int and
nc\_get\_vara\_int) may be independent (the default) or collective. To
make writes to a variable collective, call the nc\_var\_par\_access
function (or nf\_var\_par\_access for Fortran 77 users, or
nf90\_var\_par\_access for Fortran 90 users).

The example program below demonstrates simple parallel writing and
reading of a netCDF file.

  -------------------------------------------------------------------------------------- ---- --
  [4.9.3 simple\_xy\_par\_wr.c and simple\_xy\_par\_rd.c](#simple_005fxy_005fpar-in-C)        
  -------------------------------------------------------------------------------------- ---- --

* * * * *

  -------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Collective_002fIndependent-Access "Previous section in reading order")]   [[\>](#simple_005fxy_005fpar_005fwr_002ef90 "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Parallel "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 4.9.3 simple\_xy\_par\_wr.c and simple\_xy\_par\_rd.c 

For this release, only a Fortran 90 language version of this example is
provided. Other APIs will be demonstrated in examples in future
releases.

In the simple\_xy\_par\_wr example program an num\_procs x num\_procs
array is written to the disk, where num\_proc is the number of
processors on which this program is run. Each processor writes one row
of length num\_proc.

In the simple\_xy\_par\_rd program the file is read in, and each
processor expects to read in a row with its own MPI rank stored. (The
read program must be run on no more processors than were used to create
the file.)

  -------------------------------------------------------------------------- ---- --
  [4.9.3.1 simple\_xy\_par\_wr.f90](#simple_005fxy_005fpar_005fwr_002ef90)        
  [4.9.3.2 simple\_xy\_par\_rd.f90](#simple_005fxy_005fpar_005frd_002ef90)        
  -------------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fpar-in-C "Previous section in reading order")]   [[\>](#simple_005fxy_005fpar_005frd_002ef90 "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fpar-in-C "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 4.9.3.1 simple\_xy\_par\_wr.f90 

* * * * *

  ----------------------------------------------------------------------------------- ------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fpar_005fwr_002ef90 "Previous section in reading order")]   [[\>](#Future "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fpar-in-C "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- ------------------------------------------------- --- ------------------------------------------------------------------------------- -------------------------------------------------- ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 4.9.3.2 simple\_xy\_par\_rd.f90 

* * * * *

  ----------------------------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fpar_005frd_002ef90 "Previous section in reading order")]   [[\>](#NetCDF_002d4-Examples "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#API_002dExtensions "Up section")]   [[\>\>](#NetCDF_002d4-Examples "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------- ------------------------------------------ ------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.10 The Future of NetCDF 
-------------------------

NetCDF continues under active development at Unidata (see
[http://www.unidata.ucar.edu](http://www.unidata.ucar.edu)).

The next few releases of netCDF will include:

1.  A new C++ API which has better error handling and handles netCDF-4
    advanced features, such as groups and compound types.
2.  Remote access to files stored on a DAP server.
3.  Bundled packaging with udunits and other useful tools.
4.  More documentation, more examples, more tests, and more fun!

* * * * *

  ----------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Future "Previous section in reading order")]   [[\>](#simple_005fnc4 "Next section in reading order")]       [[\<\<](#API_002dExtensions "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5. NetCDF-4 Examples 
====================

Any existing netCDF applications can be converted to generate
netCDF-4/HDF5 files. Simply change the file creation call to include the
correct mode flag.

For example, in one of the C examples which write a data file, change
the nc\_create call so that NC\_NETCDF4 is one of the flags set on the
create.

The corresponding read example will work without modification; netCDF
will notice that the file is a NetCDF-4/HDF5 file, and will read it
automatically, just as if it were a netCDF classic format file.

In the example in this section we show some of the advanced features of
netCDF-4. More examples will be added in future releases.

  ----------------------------------------------------------- ---- --
  [5.1 The simple\_nc4 Example](#simple_005fnc4)                   
  [5.2 The simple\_xy\_nc4 Example](#simple_005fxy_005fnc4)        
  ----------------------------------------------------------- ---- --

* * * * *

  -------------------------------------------------------------------- -------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF_002d4-Examples "Previous section in reading order")]   [[\>](#simple_005fnc4-in-C "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF_002d4-Examples "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- -------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.1 The simple\_nc4 Example 
---------------------------

This example, like the simple\_xy netCDF-3 example above, is an overly
simplified example which demonstrates how to use groups in a netCDF-4
file.

This example is only available in C for this version of netCDF-4. The
example creates and then reads the file “simple\_nc4.nc.”

The simple\_xy.nc data file contains two dimensions, “x” and “y”, two
groups, “grp1” and “grp2”, and two data variables, one in each group,
both named: “data.” One data variable is an unsigned 64-bit integer, the
other a user-defined compound type.

The example program simple\_nc4\_wr.c creates the example data file
simple\_nc4.nc. The example program simple\_nc4\_rd.c reads the data
file.

  ----------------------------------------------------------------------- ---- --
  [5.1.1 simple\_nc4\_wr.c and simple\_nc4\_rd.c](#simple_005fnc4-in-C)        
  ----------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fnc4 "Previous section in reading order")]   [[\>](#simple_005fnc4_005fwr_002ec "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fnc4 "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 5.1.1 simple\_nc4\_wr.c and simple\_nc4\_rd.c 

For this release, only a C language version of this example is provided.
Other APIs will be demonstrated in examples in future releases.

  ----------------------------------------------------------- ---- --
  [5.1.1.1 simple\_nc4\_wr.c](#simple_005fnc4_005fwr_002ec)        
  [5.1.1.2 simple\_nc4\_rd.c](#simple_005fnc4_005frd_002ec)        
  ----------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------ ---------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fnc4-in-C "Previous section in reading order")]   [[\>](#simple_005fnc4_005frd_002ec "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fnc4-in-C "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ ---------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 5.1.1.1 simple\_nc4\_wr.c 

* * * * *

  -------------------------------------------------------------------------- ---------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fnc4_005fwr_002ec "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4 "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fnc4-in-C "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ---------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 5.1.1.2 simple\_nc4\_rd.c 

* * * * *

  -------------------------------------------------------------------------- --------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fnc4_005frd_002ec "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4-in-C "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF_002d4-Examples "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- --------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.2 The simple\_xy\_nc4 Example 
-------------------------------

This example, like the simple\_xy netCDF-3 example above, is an overly
simplified example. It is based on the simple\_xy example, but used data
chunking, compression, and the fletcher32 filter.

(These are all HDF5 features. For more information see
http://hdfgroup.org/HDF5/).

This example is not yet available in C++. We hope to have the C++
example in a future release of netCDF.

The example creates and then reads the file “simple\_xy\_nc4.nc.”

The example program simple\_xy\_nc4\_wr.c creates the example data file
simple\_xy\_nc4.nc. The example program simple\_xy\_nc4\_rd.c reads the
data file.

  -------------------------------------------------------------------------------------------- ---- --
  [5.2.1 simple\_xy\_nc4\_wr.c and simple\_xy\_nc4\_rd.c](#simple_005fxy_005fnc4-in-C)              
  [5.2.2 simple\_xy\_nc4\_wr.f and simple\_xy\_nc4\_rd.f](#simple_005fxy_005fnc4-in-F77)            
  [5.2.3 simple\_xy\_nc4\_wr.f90 and simple\_xy\_nc4\_rd.f90](#simple_005fxy_005fnc4-in-F90)        
  -------------------------------------------------------------------------------------------- ---- --

* * * * *

  -------------------------------------------------------------------- ----------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4 "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4_005fwr_002ec "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4 "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ----------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 5.2.1 simple\_xy\_nc4\_wr.c and simple\_xy\_nc4\_rd.c 

This is just like the simple\_xy example, but with chunking and variable
compression.

  ---------------------------------------------------------------------- ---- --
  [5.2.1.1 simple\_xy\_nc4\_wr.c](#simple_005fxy_005fnc4_005fwr_002ec)        
  [5.2.1.2 simple\_xy\_nc4\_rd.c](#simple_005fxy_005fnc4_005frd_002ec)        
  ---------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- -------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4-in-C "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4_005frd_002ec "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4-in-C "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- -------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 5.2.1.1 simple\_xy\_nc4\_wr.c 

* * * * *

  --------------------------------------------------------------------------------- ----------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- -------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4_005fwr_002ec "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4-in-F77 "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4-in-C "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ----------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- -------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 5.2.1.2 simple\_xy\_nc4\_rd.c 

* * * * *

  --------------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4_005frd_002ec "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4_005fwr_002ef "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4 "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 5.2.2 simple\_xy\_nc4\_wr.f and simple\_xy\_nc4\_rd.f 

This is just like the simple\_xy example, but with chunking and variable
compression.

  ---------------------------------------------------------------------- ---- --
  [5.2.2.1 simple\_xy\_nc4\_wr.f](#simple_005fxy_005fnc4_005fwr_002ef)        
  [5.2.2.2 simple\_xy\_nc4\_rd.f](#simple_005fxy_005fnc4_005frd_002ef)        
  ---------------------------------------------------------------------- ---- --

* * * * *

  --------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ---------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4-in-F77 "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4_005frd_002ef "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4-in-F77 "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ----------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ---------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 5.2.2.1 simple\_xy\_nc4\_wr.f 

* * * * *

  --------------------------------------------------------------------------------- ----------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ---------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4_005fwr_002ef "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4-in-F90 "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4-in-F77 "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ----------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ---------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 5.2.2.2 simple\_xy\_nc4\_rd.f 

* * * * *

  --------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4_005frd_002ef "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4_005fwr_002ef90 "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4 "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- --------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 5.2.3 simple\_xy\_nc4\_wr.f90 and simple\_xy\_nc4\_rd.f90 

This is just like the simple\_xy example, but with chunking and variable
compression.

  -------------------------------------------------------------------------- ---- --
  [5.2.3.1 simple\_xy\_nc4\_wr.f90](#simple_005fxy_005fnc4_005fwr_002ef90)        
  [5.2.3.2 simple\_xy\_nc4\_rd.f90](#simple_005fxy_005fnc4_005frd_002ef90)        
  -------------------------------------------------------------------------- ---- --

* * * * *

  --------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ---------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4-in-F90 "Previous section in reading order")]   [[\>](#simple_005fxy_005fnc4_005frd_002ef90 "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4-in-F90 "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ---------------------------------------------------------------------------------- ---------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 5.2.3.1 simple\_xy\_nc4\_wr.f90 

* * * * *

  ----------------------------------------------------------------------------------- --------------------------------------------------------- --- ---------------------------------------------------------------------------------- ---------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4_005fwr_002ef90 "Previous section in reading order")]   [[\>](#Combined-Index "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#simple_005fxy_005fnc4-in-F90 "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- --------------------------------------------------------- --- ---------------------------------------------------------------------------------- ---------------------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

#### 5.2.3.2 simple\_xy\_nc4\_rd.f90 

* * * * *

  ----------------------------------------------------------------------------------- -------- --- ---------------------------------------------------------------------------------- --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#simple_005fxy_005fnc4_005frd_002ef90 "Previous section in reading order")]   [ \> ]       [[\<\<](#NetCDF_002d4-Examples "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [ \>\> ]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- -------- --- ---------------------------------------------------------------------------------- --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Index 
=====

Jump to:  

[**6**](#Index_cp_symbol-1)   \
 [**A**](#Index_cp_letter-A)   [**C**](#Index_cp_letter-C)  
[**D**](#Index_cp_letter-D)   [**E**](#Index_cp_letter-E)  
[**F**](#Index_cp_letter-F)   [**G**](#Index_cp_letter-G)  
[**I**](#Index_cp_letter-I)   [**L**](#Index_cp_letter-L)  
[**N**](#Index_cp_letter-N)   [**P**](#Index_cp_letter-P)  
[**R**](#Index_cp_letter-R)   [**S**](#Index_cp_letter-S)  
[**T**](#Index_cp_letter-T)   [**U**](#Index_cp_letter-U)  
[**V**](#Index_cp_letter-V)  

Index Entry

Section

* * * * *

6

[64-bit offset format](#index-64_002dbit-offset-format)

[1.9 A Note on NetCDF Versions and Formats](#Versions)

* * * * *

A

[attribute](#index-attribute)

[1.1 The Classic NetCDF Data Model](#Data-Model)

* * * * *

C

[C++, netCDF API](#index-C_002b_002b_002c-netCDF-API)

[1.7 The NetCDF Programming APIs](#APIs)

[C, netCDF API](#index-C_002c-netCDF-API)

[1.7 The NetCDF Programming APIs](#APIs)

[classic format](#index-classic-format)

[1.9 A Note on NetCDF Versions and Formats](#Versions)

[common data model](#index-common-data-model)

[1.2 The Common Data Model and NetCDF-4](#Common-Data-Model)

[compound types](#index-compound-types)

[1.2 The Common Data Model and NetCDF-4](#Common-Data-Model)

[creating files in C](#index-creating-files-in-C)

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

[creating files in C++](#index-creating-files-in-C_002b_002b)

[3.1.4 Creating a NetCDF File in C++](#Creation-in-C_002b_002b)

[creating files in Fortran](#index-creating-files-in-Fortran)

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

[creating files in Fortran](#index-creating-files-in-Fortran-1)

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

[creating netCDF files](#index-creating-netCDF-files)

[3.1 Creating New Files and Metadata, an Overview](#Creation)

* * * * *

D

[data model](#index-data-model)

[1.1 The Classic NetCDF Data Model](#Data-Model)

[dimension](#index-dimension)

[1.1 The Classic NetCDF Data Model](#Data-Model)

* * * * *

E

[example programs](#index-example-programs)

[2. Example Programs](#Examples)

* * * * *

F

[Fortran 77, netCDF API](#index-Fortran-77_002c-netCDF-API)

[1.7 The NetCDF Programming APIs](#APIs)

[Fortran 90, netCDF API](#index-Fortran-90_002c-netCDF-API)

[1.7 The NetCDF Programming APIs](#APIs)

* * * * *

G

[groups](#index-groups)

[1.2 The Common Data Model and NetCDF-4](#Common-Data-Model)

* * * * *

I

[inquiry functions](#index-inquiry-functions)

[3.2 Reading NetCDF Files of Known Structure](#Reading)

* * * * *

L

[language APIs for netCDF](#index-language-APIs-for-netCDF)

[1.7 The NetCDF Programming APIs](#APIs)

* * * * *

N

[ncdump](#index-ncdump)

[1.6 Tools for Manipulating NetCDF Files](#Tools)

[NcFile](#index-NcFile)

[3.2.5 Reading a Known NetCDF File in C++](#Reading-in-C_002b_002b)

[ncgen](#index-ncgen)

[1.6 Tools for Manipulating NetCDF Files](#Tools)

[nc\_close](#index-nc_005fclose)

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

[nc\_create](#index-nc_005fcreate)

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

[nc\_def\_dim](#index-nc_005fdef_005fdim)

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

[nc\_def\_var](#index-nc_005fdef_005fvar)

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

[nc\_enddef](#index-nc_005fenddef)

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

[nc\_get\_att](#index-nc_005fget_005fatt)

[3.2.2 Reading a Known NetCDF File in C](#Reading-in-C)

[nc\_get\_var1](#index-nc_005fget_005fvar1)

[3.4.1 Reading and Writing Subsets of Data in C](#Subsetting-in-C)

[nc\_get\_vara](#index-nc_005fget_005fvara)

[3.2.2 Reading a Known NetCDF File in C](#Reading-in-C)

[nc\_get\_varm](#index-nc_005fget_005fvarm)

[3.4.1 Reading and Writing Subsets of Data in C](#Subsetting-in-C)

[nc\_get\_vars](#index-nc_005fget_005fvars)

[3.4.1 Reading and Writing Subsets of Data in C](#Subsetting-in-C)

[nc\_open](#index-nc_005fopen)

[3.2.2 Reading a Known NetCDF File in C](#Reading-in-C)

[nc\_put\_att](#index-nc_005fput_005fatt)

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

[nc\_put\_var1](#index-nc_005fput_005fvar1)

[3.4.1 Reading and Writing Subsets of Data in C](#Subsetting-in-C)

[nc\_put\_vara](#index-nc_005fput_005fvara)

[3.1.1 Creating a NetCDF File in C](#Creation-in-C)

[nc\_put\_varm](#index-nc_005fput_005fvarm)

[3.4.1 Reading and Writing Subsets of Data in C](#Subsetting-in-C)

[nc\_put\_vars](#index-nc_005fput_005fvars)

[3.4.1 Reading and Writing Subsets of Data in C](#Subsetting-in-C)

[netCDF, definition](#index-netCDF_002c-definition)

[1. What is NetCDF?](#Intro)

[netCDF-4](#index-netCDF_002d4)

[4.10 The Future of NetCDF](#Future)

[netCDF-4 model extensions](#index-netCDF_002d4-model-extensions)

[1.2 The Common Data Model and NetCDF-4](#Common-Data-Model)

[NF90\_CLOSE](#index-NF90_005fCLOSE)

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

[NF90\_CREATE](#index-NF90_005fCREATE)

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

[NF90\_DEF\_DIM](#index-NF90_005fDEF_005fDIM)

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

[NF90\_DEF\_VAR](#index-NF90_005fDEF_005fVAR)

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

[NF90\_ENDDEF](#index-NF90_005fENDDEF)

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

[NF90\_GET\_ATT](#index-NF90_005fGET_005fATT)

[3.2.4 Reading a Known NetCDF File in Fortran 90](#Reading-in-F90)

[NF90\_GET\_VAR1](#index-NF90_005fGET_005fVAR1)

[3.4.3 Reading and Writing Subsets of Data in Fortran
90](#Subsetting-in-F90)

[NF90\_GET\_VARA](#index-NF90_005fGET_005fVARA)

[3.2.4 Reading a Known NetCDF File in Fortran 90](#Reading-in-F90)

[NF90\_GET\_VARM](#index-NF90_005fGET_005fVARM)

[3.4.3 Reading and Writing Subsets of Data in Fortran
90](#Subsetting-in-F90)

[NF90\_GET\_VARS](#index-NF90_005fGET_005fVARS)

[3.4.3 Reading and Writing Subsets of Data in Fortran
90](#Subsetting-in-F90)

[NF90\_OPEN](#index-NF90_005fOPEN)

[3.2.4 Reading a Known NetCDF File in Fortran 90](#Reading-in-F90)

[NF90\_PUT\_ATT\_ type](#index-NF90_005fPUT_005fATT_005f-type)

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

[NF90\_PUT\_VAR1](#index-NF90_005fPUT_005fVAR1)

[3.4.3 Reading and Writing Subsets of Data in Fortran
90](#Subsetting-in-F90)

[NF90\_PUT\_VARA](#index-NF90_005fPUT_005fVARA)

[3.1.3 Creating a NetCDF File in Fortran 90](#Creation-in-F90)

[NF90\_PUT\_VARM](#index-NF90_005fPUT_005fVARM)

[3.4.3 Reading and Writing Subsets of Data in Fortran
90](#Subsetting-in-F90)

[NF90\_PUT\_VARS](#index-NF90_005fPUT_005fVARS)

[3.4.3 Reading and Writing Subsets of Data in Fortran
90](#Subsetting-in-F90)

[NF\_CLOSE](#index-NF_005fCLOSE)

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

[NF\_CREATE](#index-NF_005fCREATE)

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

[NF\_DEF\_DIM](#index-NF_005fDEF_005fDIM)

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

[NF\_DEF\_VAR](#index-NF_005fDEF_005fVAR)

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

[NF\_ENDDEF](#index-NF_005fENDDEF)

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

[NF\_GET\_ATT](#index-NF_005fGET_005fATT)

[3.2.3 Reading a Known NetCDF File in Fortran 77](#Reading-in-F77)

[NF\_GET\_VAR1](#index-NF_005fGET_005fVAR1)

[3.4.2 Reading and Writing Subsets of Data in Fortran
77](#Subsetting-in-F77)

[NF\_GET\_VARA](#index-NF_005fGET_005fVARA)

[3.2.3 Reading a Known NetCDF File in Fortran 77](#Reading-in-F77)

[NF\_GET\_VARM](#index-NF_005fGET_005fVARM)

[3.4.2 Reading and Writing Subsets of Data in Fortran
77](#Subsetting-in-F77)

[NF\_GET\_VARS](#index-NF_005fGET_005fVARS)

[3.4.2 Reading and Writing Subsets of Data in Fortran
77](#Subsetting-in-F77)

[NF\_OPEN](#index-NF_005fOPEN)

[3.2.3 Reading a Known NetCDF File in Fortran 77](#Reading-in-F77)

[NF\_PUT\_ATT\_ type](#index-NF_005fPUT_005fATT_005f-type)

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

[NF\_PUT\_VAR1](#index-NF_005fPUT_005fVAR1)

[3.4.2 Reading and Writing Subsets of Data in Fortran
77](#Subsetting-in-F77)

[NF\_PUT\_VARA](#index-NF_005fPUT_005fVARA)

[3.1.2 Creating a NetCDF File in Fortran 77](#Creation-in-F77)

[NF\_PUT\_VARM](#index-NF_005fPUT_005fVARM)

[3.4.2 Reading and Writing Subsets of Data in Fortran
77](#Subsetting-in-F77)

[NF\_PUT\_VARS](#index-NF_005fPUT_005fVARS)

[3.4.2 Reading and Writing Subsets of Data in Fortran
77](#Subsetting-in-F77)

* * * * *

P

[perl, netCDF API](#index-perl_002c-netCDF-API)

[1.7 The NetCDF Programming APIs](#APIs)

[pres\_temp\_4D example](#index-pres_005ftemp_005f4D-example)

[2. Example Programs](#Examples)

[python, netCDF API](#index-python_002c-netCDF-API)

[1.7 The NetCDF Programming APIs](#APIs)

* * * * *

R

[reading netCDF files of known
structure](#index-reading-netCDF-files-of-known-structure)

[3.2 Reading NetCDF Files of Known Structure](#Reading)

[reading netCDF files with C](#index-reading-netCDF-files-with-C)

[3.2.2 Reading a Known NetCDF File in C](#Reading-in-C)

[reading netCDF files with
C++](#index-reading-netCDF-files-with-C_002b_002b)

[3.2.5 Reading a Known NetCDF File in C++](#Reading-in-C_002b_002b)

[reading netCDF files with Fortran
77](#index-reading-netCDF-files-with-Fortran-77)

[3.2.3 Reading a Known NetCDF File in Fortran 77](#Reading-in-F77)

[reading netCDF files with Fortran
90](#index-reading-netCDF-files-with-Fortran-90)

[3.2.4 Reading a Known NetCDF File in Fortran 90](#Reading-in-F90)

[ruby, netCDF API](#index-ruby_002c-netCDF-API)

[1.7 The NetCDF Programming APIs](#APIs)

* * * * *

S

[sfc\_pres\_temp example](#index-sfc_005fpres_005ftemp-example)

[2. Example Programs](#Examples)

[simple\_xy example](#index-simple_005fxy-example)

[2. Example Programs](#Examples)

[software for netCDF](#index-software-for-netCDF)

[1.6 Tools for Manipulating NetCDF Files](#Tools)

* * * * *

T

[third-party tools](#index-third_002dparty-tools)

[1.6 Tools for Manipulating NetCDF Files](#Tools)

[tools for manipulating netCDF](#index-tools-for-manipulating-netCDF)

[1.7 The NetCDF Programming APIs](#APIs)

* * * * *

U

[UCAR](#index-UCAR)

[1. What is NetCDF?](#Intro)

[Unidata](#index-Unidata)

[1. What is NetCDF?](#Intro)

[user-defined types](#index-user_002ddefined-types)

[1.2 The Common Data Model and NetCDF-4](#Common-Data-Model)

* * * * *

V

[V2 API](#index-V2-API)

[1.7 The NetCDF Programming APIs](#APIs)

[variable](#index-variable)

[1.1 The Classic NetCDF Data Model](#Data-Model)

* * * * *

Jump to:  

[**6**](#Index_cp_symbol-1)   \
 [**A**](#Index_cp_letter-A)   [**C**](#Index_cp_letter-C)  
[**D**](#Index_cp_letter-D)   [**E**](#Index_cp_letter-E)  
[**F**](#Index_cp_letter-F)   [**G**](#Index_cp_letter-G)  
[**I**](#Index_cp_letter-I)   [**L**](#Index_cp_letter-L)  
[**N**](#Index_cp_letter-N)   [**P**](#Index_cp_letter-P)  
[**R**](#Index_cp_letter-R)   [**S**](#Index_cp_letter-S)  
[**T**](#Index_cp_letter-T)   [**U**](#Index_cp_letter-U)  
[**V**](#Index_cp_letter-V)  

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

