\page netcdf-c-texi netCDF-C texi-based documentation.

  ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------- ------------ ------------------------------------ ----------------------------------

The NetCDF C Interface Guide {.settitle}
============================

This document describes the C interface to the netCDF library; it
applies to netCDF version No value for VERSION and was last updated on
No value for UPDATED.

For a complete description of the netCDF format and utilities see [The
NetCDF Users Guide: (netcdf)Top](netcdf.html#Top) section ‘Top’ in The
NetCDF Users Guide.

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

  

[2. Datasets](#Datasets)

  

[3. Groups](#Groups)

  

[4. Dimensions](#Dimensions)

  

[5. User Defined Data Types](#User-Defined-Data-Types)

  

[6. Variables](#Variables)

  

[7. Attributes](#Attributes)

  

[A. Summary of C Interface](#Summary-of-C-Interface)

  

[B. NetCDF 3 to NetCDF 4 Transition Guide](#NetCDF-3-Transition-Guide)

  

[C. NetCDF 2 to NetCDF 3 C Transition
Guide](#NetCDF-2-C-Transition-Guide)

  

[D. NetCDF-3 Error Codes](#NetCDF_002d3-Error-Codes)

  

[E. NetCDF-4 Error Codes](#NetCDF_002d4-Error-Codes)

  

[F. DAP Error Codes](#DAP-Error-Codes)

  

[G. Index](#Combined-Index)

  

~~~~ {.menu-comment}
 — The Detailed Node Listing —

Use of the NetCDF Library
~~~~

[1.1 Creating a NetCDF Dataset](#Creating)

  

[1.2 Reading a NetCDF Dataset with Known Names](#Reading-Known)

  

[1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)

  

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

  

[1.5 Error Handling](#Errors)

  

[1.6 Compiling and Linking with the NetCDF Library](#Compiling)

  

~~~~ {.menu-comment}
Datasets
~~~~

[2.1 NetCDF Library Interface Descriptions](#Interface-Descriptions)

  

What’s in the Function Documentation

[2.2 Parallel Access for NetCDF Files](#parallel-access)

  

[2.3 Get error message corresponding to error status:
nc\_strerror](#nc_005fstrerror)

  

Get Error Messages

[2.4 Get netCDF library version:
nc\_inq\_libvers](#nc_005finq_005flibvers)

  

Get netCDF library version

[2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)

  

Create a Dataset

[2.6 Create a NetCDF Dataset With Performance Options:
nc\_\_create](#nc_005f_005fcreate)

  

Create a Dataset with Performance Tuning

[2.7 Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)

  

[2.8 Open a NetCDF Dataset for Access: nc\_open](#nc_005fopen)

  

Open a Dataset

[2.9 Open a NetCDF Dataset for Access with Performance Tuning:
nc\_\_open](#nc_005f_005fopen)

  

Open a Dataset with Performance Tuning

[2.10 Open a NetCDF Dataset for Parallel Access](#nc_005fopen_005fpar)

  

[2.11 Put Open NetCDF Dataset into Define Mode:
nc\_redef](#nc_005fredef)

  

Put a Dataset into Define Mode

[2.12 Leave Define Mode: nc\_enddef](#nc_005fenddef)

  

Leave Define Mode

[2.13 Leave Define Mode with Performance Tuning:
nc\_\_enddef](#nc_005f_005fenddef)

  

Leave Define Mode with Performance Tuning

[2.14 Close an Open NetCDF Dataset: nc\_close](#nc_005fclose)

  

Close a Dataset

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

  

Inquire about a Dataset

[2.16 Synchronize an Open NetCDF Dataset to Disk:
nc\_sync](#nc_005fsync)

  

Synchronize a Dataset to Disk

[2.17 Back Out of Recent Definitions: nc\_abort](#nc_005fabort)

  

Back Out of Recent Definitions

[2.18 Set Fill Mode for Writes: nc\_set\_fill](#nc_005fset_005ffill)

  

Set Fill Mode for Writes

[2.19 Set Default Creation Format:
nc\_set\_default\_format](#nc_005fset_005fdefault_005fformat)

  

Change the default output format

[2.20 Set HDF5 Chunk Cache for Future File Opens/Creates:
nc\_set\_chunk\_cache](#nc_005fset_005fchunk_005fcache)

  

[2.21 Get the HDF5 Chunk Cache Settings for Future File Opens/Creates:
nc\_get\_chunk\_cache](#nc_005fget_005fchunk_005fcache)

  

~~~~ {.menu-comment}
Groups
~~~~

[3.1 Find a Group ID: nc\_inq\_ncid](#nc_005finq_005fncid)

  

[3.2 Get a List of Groups in a Group:
nc\_inq\_grps](#nc_005finq_005fgrps)

  

[3.3 Find all the Variables in a Group:
nc\_inq\_varids](#nc_005finq_005fvarids)

  

[3.4 Find all Dimensions Visible in a Group:
nc\_inq\_dimids](#nc_005finq_005fdimids)

  

[3.5 Find a Group’s Name: nc\_inq\_grpname](#nc_005finq_005fgrpname)

  

[3.6 Find a Group’s Full Name:
nc\_inq\_grpname\_full](#nc_005finq_005fgrpname_005ffull)

  

[3.7 Find the Length of a Group’s Full Name:
nc\_inq\_grpname\_len](#nc_005finq_005fgrpname_005flen)

  

[3.8 Find a Group’s Parent:
nc\_inq\_grp\_parent](#nc_005finq_005fgrp_005fparent)

  

[3.9 Find a Group by Name:
nc\_inq\_grp\_ncid](#nc_005finq_005fgrp_005fncid)

  

[3.10 Find a Group by its Fully-qualified Name:
nc\_inq\_grp\_full\_ncid](#nc_005finq_005fgrp_005ffull_005fncid)

  

[3.11 Create a New Group: nc\_def\_grp](#nc_005fdef_005fgrp)

  

~~~~ {.menu-comment}
Dimensions
~~~~

[4.1 Dimensions Introduction](#Dimensions-Introduction)

  

[4.2 Create a Dimension: nc\_def\_dim](#nc_005fdef_005fdim)

  

Create a Dimension

[4.3 Get a Dimension ID from Its Name:
nc\_inq\_dimid](#nc_005finq_005fdimid)

  

Get a Dimension ID from Its Name

[4.4 Inquire about a Dimension: nc\_inq\_dim
Family](#nc_005finq_005fdim-Family)

  

Inquire about a Dimension

[4.5 Rename a Dimension: nc\_rename\_dim](#nc_005frename_005fdim)

  

Rename a Dimension

[4.6 Find All Unlimited Dimension IDs:
nc\_inq\_unlimdims](#nc_005finq_005funlimdims)

  

~~~~ {.menu-comment}
User Defined Data Types
~~~~

[5.1 User Defined Types Introduction](#User-Defined-Types)

  

[5.2 Learn the IDs of All Types in Group:
nc\_inq\_typeids](#nc_005finq_005ftypeids)

  

[5.3 Find a Typeid from Group and Name:
nc\_inq\_typeid](#nc_005finq_005ftypeid)

  

[5.4 Learn About a User Defined Type:
nc\_inq\_type](#nc_005finq_005ftype)

  

[5.5 Learn About a User Defined Type:
nc\_inq\_user\_type](#nc_005finq_005fuser_005ftype)

  

[5.6 Compound Types Introduction](#Compound-Types)

  

[5.7 Creating a Compound Type:
nc\_def\_compound](#nc_005fdef_005fcompound)

  

[5.8 Inserting a Field into a Compound Type:
nc\_insert\_compound](#nc_005finsert_005fcompound)

  

[5.9 Inserting an Array Field into a Compound Type:
nc\_insert\_array\_compound](#nc_005finsert_005farray_005fcompound)

  

[5.10 Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound)

  

[5.11 Learn the Name of a Compound Type:
nc\_inq\_compound\_name](#nc_005finq_005fcompound_005fname)

  

[5.12 Learn the Size of a Compound Type:
nc\_inq\_compound\_size](#nc_005finq_005fcompound_005fsize)

  

[5.13 Learn the Number of Fields in a Compound Type:
nc\_inq\_compound\_nfields](#nc_005finq_005fcompound_005fnfields)

  

[5.14 Learn About a Field of a Compound Type:
nc\_inq\_compound\_field](#nc_005finq_005fcompound_005ffield)

  

[5.15 Find the Name of a Field in a Compound Type:
nc\_inq\_compound\_fieldname](#nc_005finq_005fcompound_005ffieldname)

  

[5.16 Get the FieldID of a Compound Type Field:
nc\_inq\_compound\_fieldindex](#nc_005finq_005fcompound_005ffieldindex)

  

[5.17 Get the Offset of a Field:
nc\_inq\_compound\_fieldoffset](#nc_005finq_005fcompound_005ffieldoffset)

  

[5.18 Find the Type of a Field:
nc\_inq\_compound\_fieldtype](#nc_005finq_005fcompound_005ffieldtype)

  

[5.19 Find the Number of Dimensions in an Array Field:
nc\_inq\_compound\_fieldndims](#nc_005finq_005fcompound_005ffieldndims)

  

[5.20 Find the Sizes of Dimensions in an Array Field:
nc\_inq\_compound\_fielddim\_sizes](#nc_005finq_005fcompound_005ffielddim_005fsizes)

  

[5.21 Variable Length Array Introduction](#Variable-Length-Array)

  

[5.22 Define a Variable Length Array (VLEN):
nc\_def\_vlen](#nc_005fdef_005fvlen)

  

[5.23 Learning about a Variable Length Array (VLEN) Type:
nc\_inq\_vlen](#nc_005finq_005fvlen)

  

[5.24 Releasing Memory for a Variable Length Array (VLEN) Type:
nc\_free\_vlen](#nc_005ffree_005fvlen)

  

[5.25 Releasing Memory for an Array of Variable Length Array (VLEN)
Type: nc\_free\_vlen](#nc_005ffree_005fvlens)

  

[5.26 Opaque Type Introduction](#Opaque-Type)

  

[5.27 Creating Opaque Types: nc\_def\_opaque](#nc_005fdef_005fopaque)

  

[5.28 Learn About an Opaque Type:
nc\_inq\_opaque](#nc_005finq_005fopaque)

  

[5.29 Enum Type Introduction](#Enum-Type)

  

[5.30 Creating a Enum Type: nc\_def\_enum](#nc_005fdef_005fenum)

  

[5.31 Inserting a Field into a Enum Type:
nc\_insert\_enum](#nc_005finsert_005fenum)

  

[5.32 Learn About a Enum Type: nc\_inq\_enum](#nc_005finq_005fenum)

  

[5.33 Learn the Name of a Enum Type:
nc\_inq\_enum\_member](#nc_005finq_005fenum_005fmember)

  

[5.34 Learn the Name of a Enum Type:
nc\_inq\_enum\_ident](#nc_005finq_005fenum_005fident)

  

~~~~ {.menu-comment}
Compound Types Introduction
~~~~

[5.7 Creating a Compound Type:
nc\_def\_compound](#nc_005fdef_005fcompound)

  

[5.8 Inserting a Field into a Compound Type:
nc\_insert\_compound](#nc_005finsert_005fcompound)

  

[5.10 Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound)

  

[5.15 Find the Name of a Field in a Compound Type:
nc\_inq\_compound\_fieldname](#nc_005finq_005fcompound_005ffieldname)

  

[5.16 Get the FieldID of a Compound Type Field:
nc\_inq\_compound\_fieldindex](#nc_005finq_005fcompound_005ffieldindex)

  

[5.17 Get the Offset of a Field:
nc\_inq\_compound\_fieldoffset](#nc_005finq_005fcompound_005ffieldoffset)

  

[5.18 Find the Type of a Field:
nc\_inq\_compound\_fieldtype](#nc_005finq_005fcompound_005ffieldtype)

  

~~~~ {.menu-comment}
Variables
~~~~

[6.1 Introduction](#Variable-Introduction)

  

[6.2 Language Types Corresponding to netCDF external data
types](#Variable-Types)

  

[6.3 NetCDF-3 Classic and 64-Bit Offset Data
Types](#NetCDF_002d3-Variable-Types)

  

[6.4 NetCDF-4 Atomic Types](#NetCDF_002d4-Atomic-Types)

  

[6.5 Create a Variable: `nc_def_var`](#nc_005fdef_005fvar)

  

Create a Variable

[6.6 Define Chunking Parameters for a Variable:
`nc_def_var_chunking`](#nc_005fdef_005fvar_005fchunking)

  

Set chunking parameters

[6.7 Learn About Chunking Parameters for a Variable:
`nc_inq_var_chunking`](#nc_005finq_005fvar_005fchunking)

  

Learn about chunking parameters

[6.8 Set HDF5 Chunk Cache for a Variable:
nc\_set\_var\_chunk\_cache](#nc_005fset_005fvar_005fchunk_005fcache)

  

[6.9 Get the HDF5 Chunk Cache Settings for a Variable:
nc\_get\_var\_chunk\_cache](#nc_005fget_005fvar_005fchunk_005fcache)

  

[6.10 Define Fill Parameters for a Variable:
`nc_def_var_fill`](#nc_005fdef_005fvar_005ffill)

  

[6.11 Learn About Fill Parameters for a Variable:
`nc_inq_var_fill`](#nc_005finq_005fvar_005ffill)

  

[6.12 Define Compression Parameters for a Variable:
`nc_def_var_deflate`](#nc_005fdef_005fvar_005fdeflate)

  

Set compression parameters

[6.13 Learn About Deflate Parameters for a Variable:
`nc_inq_var_deflate`](#nc_005finq_005fvar_005fdeflate)

  

Learn about compression parameters

[6.14 Learn About Szip Parameters for a Variable:
`nc_inq_var_szip`](#nc_005finq_005fvar_005fszip)

  

[6.15 Define Checksum Parameters for a Variable:
`nc_def_var_fletcher32`](#nc_005fdef_005fvar_005ffletcher32)

  

Set checksum filter

[6.16 Learn About Checksum Parameters for a Variable:
`nc_inq_var_fletcher32`](#nc_005finq_005fvar_005ffletcher32)

  

Learn if checksum filter is set

[6.17 Define Endianness of a Variable:
`nc_def_var_endian`](#nc_005fdef_005fvar_005fendian)

  

[6.18 Learn About Endian Parameters for a Variable:
`nc_inq_var_endian`](#nc_005finq_005fvar_005fendian)

  

[6.19 Get a Variable ID from Its Name:
nc\_inq\_varid](#nc_005finq_005fvarid)

  

Get a Variable ID from Its Name

[6.20 Get Information about a Variable from Its ID:
nc\_inq\_var](#nc_005finq_005fvar)

  

Get Information about a Variable from Its ID

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

  

Write a Single Data Value

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

  

Write an Entire Variable

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

  

Write an Array of Values

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

  

Write a Subsampled Array of Values

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

  

Write a Mapped Array of Values

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

  

Read a Single Data Value

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

  

Read an Entire Variable

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

  

Read an Array of Values

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

  

Read a Subsampled Array of Values

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

  

Read a Mapped Array of Values

[6.31 Reading and Writing Character String Values](#Strings)

  

[6.32 Releasing Memory for a NC\_STRING:
nc\_free\_string](#nc_005ffree_005fstring)

  

[6.33 Fill Values](#Fill-Values)

  

What’s Written Where there’s No Data?

[6.34 Rename a Variable: nc\_rename\_var](#nc_005frename_005fvar)

  

Rename a Variable

[6.35 Copy a Variable from One File to Another:
nc\_copy\_var](#nc_005fcopy_005fvar)

  

[6.36 Change between Collective and Independent Parallel Access:
nc\_var\_par\_access](#nc_005fvar_005fpar_005faccess)

  

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

  

~~~~ {.menu-comment}
Reading and Writing Character String Values
~~~~

[6.31.1 Reading and Writing Character String Values in the Classic
Model](#Classic-Strings)

  

[6.31.2 Reading and Writing Arrays of Strings](#Arrays-of-Strings)

  

~~~~ {.menu-comment}
Attributes
~~~~

[7.1 Introduction](#Attributes-Introduction)

  

[7.2 Create an Attribute: nc\_put\_att\_
type](#nc_005fput_005fatt_005f-type)

  

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

  

[7.4 Get Attribute’s Values:nc\_get\_att\_
type](#nc_005fget_005fatt_005f-type)

  

[7.5 Copy Attribute from One NetCDF to Another:
nc\_copy\_att](#nc_005fcopy_005fatt)

  

[7.6 Rename an Attribute: nc\_rename\_att](#nc_005frename_005fatt)

  

[7.7 Delete an Attribute: nc\_del\_att](#nc_005fdel_005fatt)

  

~~~~ {.menu-comment}
~~~~

* * * * *

  -------------------------------------------------- --------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Top "Previous section in reading order")]   [[\>](#Creating "Next section in reading order")]       [[\<\<](#Top "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Datasets "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------- --------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1. Use of the NetCDF Library {.chapter}
============================

You can use the netCDF library without knowing about all of the netCDF
interface. If you are creating a netCDF dataset, only a handful of
routines are required to define the necessary dimensions, variables, and
attributes, and to write the data to the netCDF dataset. (Even less is
needed if you use the ncgen utility to create the dataset before running
a program using netCDF library calls to write data.) Similarly, if you
are writing software to access data stored in a particular netCDF
object, only a small subset of the netCDF library is required to open
the netCDF dataset and access the data. Authors of generic applications
that access arbitrary netCDF datasets need to be familiar with more of
the netCDF library.

In this chapter we provide templates of common sequences of netCDF calls
needed for common uses. For clarity we present only the names of
routines; omit declarations and error checking; omit the type-specific
suffixes of routine names for variables and attributes; indent
statements that are typically invoked multiple times; and use ... to
represent arbitrary sequences of other statements. Full parameter lists
are described in later chapters.

  --------------------------------------------------------------------- ---- --
  [1.1 Creating a NetCDF Dataset](#Creating)                                 
  [1.2 Reading a NetCDF Dataset with Known Names](#Reading-Known)            
  [1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)        
  [1.4 Adding New Dimensions, Variables, Attributes](#Adding)                
  [1.5 Error Handling](#Errors)                                              
  [1.6 Compiling and Linking with the NetCDF Library](#Compiling)            
  --------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------------------ -------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Use-of-the-NetCDF-Library "Previous section in reading order")]   [[\>](#Reading-Known "Next section in reading order")]       [[\<\<](#Use-of-the-NetCDF-Library "Beginning of this chapter or previous chapter")]   [[Up](#Use-of-the-NetCDF-Library "Up section")]   [[\>\>](#Datasets "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ -------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.1 Creating a NetCDF Dataset {.section}
-----------------------------

Here is a typical sequence of netCDF calls used to create a new netCDF
dataset:

 

~~~~ {.example}
    nc_create           /* create netCDF dataset: enter define mode */
         ... 
       nc_def_dim       /* define dimensions: from name and length */
         ... 
       nc_def_var       /* define variables: from name, type, ... */
         ... 
       nc_put_att       /* put attribute: assign attribute values */
         ... 
    nc_enddef           /* end definitions: leave define mode */
         ... 
       nc_put_var       /* provide values for variables */
         ... 
    nc_close            /* close: save new netCDF dataset */
~~~~

Only one call is needed to create a netCDF dataset, at which point you
will be in the first of two netCDF modes. When accessing an open netCDF
dataset, it is either in define mode or data mode. In define mode, you
can create dimensions, variables, and new attributes, but you cannot
read or write variable data. In data mode, you can access data and
change existing attributes, but you are not permitted to create new
dimensions, variables, or attributes.

One call to nc\_def\_dim is needed for each dimension created.
Similarly, one call to nc\_def\_var is needed for each variable
creation, and one call to a member of the nc\_put\_att family is needed
for each attribute defined and assigned a value. To leave define mode
and enter data mode, call nc\_enddef.

Once in data mode, you can add new data to variables, change old values,
and change values of existing attributes (so long as the attribute
changes do not require more storage space). Single values may be written
to a netCDF variable with one of the members of the nc\_put\_var1
family, depending on what type of data you have to write. All the values
of a variable may be written at once with one of the members of the
nc\_put\_var family. Arrays or array cross-sections of a variable may be
written using members of the nc\_put\_vara family. Subsampled array
sections may be written using members of the nc\_put\_vars family.
Mapped array sections may be written using members of the nc\_put\_varm
family. (Subsampled and mapped access are general forms of data access
that are explained later.)

Finally, you should explicitly close all netCDF datasets that have been
opened for writing by calling nc\_close. By default, access to the file
system is buffered by the netCDF library. If a program terminates
abnormally with netCDF datasets open for writing, your most recent
modifications may be lost. This default buffering of data is disabled by
setting the NC\_SHARE flag when opening the dataset. But even if this
flag is set, changes to attribute values or changes made in define mode
are not written out until nc\_sync or nc\_close is called.

* * * * *

  ------------------------------------------------------- ---------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Creating "Previous section in reading order")]   [[\>](#Reading-Unknown "Next section in reading order")]       [[\<\<](#Use-of-the-NetCDF-Library "Beginning of this chapter or previous chapter")]   [[Up](#Use-of-the-NetCDF-Library "Up section")]   [[\>\>](#Datasets "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ---------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.2 Reading a NetCDF Dataset with Known Names {.section}
---------------------------------------------

Here we consider the case where you know the names of not only the
netCDF datasets, but also the names of their dimensions, variables, and
attributes. (Otherwise you would have to do "inquire" calls.) The order
of typical C calls to read data from those variables in a netCDF dataset
is:

 

~~~~ {.example}
    nc_open                /* open existing netCDF dataset */
         ... 
       nc_inq_dimid        /* get dimension IDs */
         ... 
       nc_inq_varid        /* get variable IDs */
         ... 
       nc_get_att          /* get attribute values */
         ... 
       nc_get_var          /* get values of variables */
         ... 
    nc_close               /* close netCDF dataset */
~~~~

First, a single call opens the netCDF dataset, given the dataset name,
and returns a netCDF ID that is used to refer to the open netCDF dataset
in all subsequent calls.

Next, a call to nc\_inq\_dimid for each dimension of interest gets the
dimension ID from the dimension name. Similarly, each required variable
ID is determined from its name by a call to nc\_inq\_varid Once variable
IDs are known, variable attribute values can be retrieved using the
netCDF ID, the variable ID, and the desired attribute name as input to a
member of the nc\_get\_att family (typically nc\_get\_att\_text or
nc\_get\_att\_double) for each desired attribute. Variable data values
can be directly accessed from the netCDF dataset with calls to members
of the nc\_get\_var1 family for single values, the nc\_get\_var family
for entire variables, or various other members of the nc\_get\_vara,
nc\_get\_vars, or nc\_get\_varm families for array, subsampled or mapped
access.

Finally, the netCDF dataset is closed with nc\_close. There is no need
to close a dataset open only for reading.

* * * * *

  ------------------------------------------------------------ ------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading-Known "Previous section in reading order")]   [[\>](#Adding "Next section in reading order")]       [[\<\<](#Use-of-the-NetCDF-Library "Beginning of this chapter or previous chapter")]   [[Up](#Use-of-the-NetCDF-Library "Up section")]   [[\>\>](#Datasets "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.3 Reading a netCDF Dataset with Unknown Names {.section}
-----------------------------------------------

It is possible to write programs (e.g., generic software) which do such
things as processing every variable, without needing to know in advance
the names of these variables. Similarly, the names of dimensions and
attributes may be unknown.

Names and other information about netCDF objects may be obtained from
netCDF datasets by calling inquire functions. These return information
about a whole netCDF dataset, a dimension, a variable, or an attribute.
The following template illustrates how they are used:

 

~~~~ {.example}
    nc_open                   /* open existing netCDF dataset */
      ... 
    nc_inq                    /* find out what is in it */
         ... 
       nc_inq_dim             /* get dimension names, lengths */
         ... 
       nc_inq_var             /* get variable names, types, shapes */
            ... 
          nc_inq_attname      /* get attribute names */
            ... 
          nc_inq_att          /* get attribute types and lengths */
            ... 
          nc_get_att          /* get attribute values */
            ... 
       nc_get_var             /* get values of variables */
         ... 
    nc_close                  /* close netCDF dataset */
~~~~

As in the previous example, a single call opens the existing netCDF
dataset, returning a netCDF ID. This netCDF ID is given to the nc\_inq
routine, which returns the number of dimensions, the number of
variables, the number of global attributes, and the ID of the unlimited
dimension, if there is one.

All the inquire functions are inexpensive to use and require no I/O,
since the information they provide is stored in memory when a netCDF
dataset is first opened.

Dimension IDs use consecutive integers, beginning at 0. Also dimensions,
once created, cannot be deleted. Therefore, knowing the number of
dimension IDs in a netCDF dataset means knowing all the dimension IDs:
they are the integers 0, 1, 2, ...up to the number of dimensions. For
each dimension ID, a call to the inquire function nc\_inq\_dim returns
the dimension name and length.

Variable IDs are also assigned from consecutive integers 0, 1, 2, ... up
to the number of variables. These can be used in nc\_inq\_var calls to
find out the names, types, shapes, and the number of attributes assigned
to each variable.

Once the number of attributes for a variable is known, successive calls
to nc\_inq\_attname return the name for each attribute given the netCDF
ID, variable ID, and attribute number. Armed with the attribute name, a
call to nc\_inq\_att returns its type and length. Given the type and
length, you can allocate enough space to hold the attribute values. Then
a call to a member of the nc\_get\_att family returns the attribute
values.

Once the IDs and shapes of netCDF variables are known, data values can
be accessed by calling a member of the nc\_get\_var1 family for single
values, or members of the nc\_get\_var, nc\_get\_vara, nc\_get\_vars, or
nc\_get\_varm for various kinds of array access.

* * * * *

  -------------------------------------------------------------- ------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Reading-Unknown "Previous section in reading order")]   [[\>](#Errors "Next section in reading order")]       [[\<\<](#Use-of-the-NetCDF-Library "Beginning of this chapter or previous chapter")]   [[Up](#Use-of-the-NetCDF-Library "Up section")]   [[\>\>](#Datasets "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- ------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.4 Adding New Dimensions, Variables, Attributes {.section}
------------------------------------------------

An existing netCDF dataset can be extensively altered. New dimensions,
variables, and attributes can be added or existing ones renamed, and
existing attributes can be deleted. Existing dimensions, variables, and
attributes can be renamed. The following code template lists a typical
sequence of calls to add new netCDF components to an existing dataset:

 

~~~~ {.example}
    nc_open             /* open existing netCDF dataset */
      ... 
    nc_redef            /* put it into define mode */
        ... 
      nc_def_dim        /* define additional dimensions (if any) */
        ... 
      nc_def_var        /* define additional variables (if any) */
        ... 
      nc_put_att        /* define additional attributes (if any) */
        ... 
    nc_enddef           /* check definitions, leave define mode */
        ... 
      nc_put_var        /* provide values for new variables */
        ... 
    nc_close            /* close netCDF dataset */
~~~~

A netCDF dataset is first opened by the nc\_open call. This call puts
the open dataset in data mode, which means existing data values can be
accessed and changed, existing attributes can be changed (so long as
they do not grow), but nothing can be added. To add new netCDF
dimensions, variables, or attributes you must enter define mode, by
calling nc\_redef. In define mode, call nc\_def\_dim to define new
dimensions, nc\_def\_var to define new variables, and a member of the
nc\_put\_att family to assign new attributes to variables or enlarge old
attributes.

You can leave define mode and reenter data mode, checking all the new
definitions for consistency and committing the changes to disk, by
calling nc\_enddef. If you do not wish to reenter data mode, just call
nc\_close, which will have the effect of first calling nc\_enddef.

Until the nc\_enddef call, you may back out of all the redefinitions
made in define mode and restore the previous state of the netCDF dataset
by calling nc\_abort. You may also use the nc\_abort call to restore the
netCDF dataset to a consistent state if the call to nc\_enddef fails. If
you have called nc\_close from definition mode and the implied call to
nc\_enddef fails, nc\_abort will automatically be called to close the
netCDF dataset and leave it in its previous consistent state (before you
entered define mode).

For netCDF-4/HDF5 format files, define mode is still important, but the
user does not have to called nc\_enddef - it is called automatically
when needed. It may also be called by the user.

In netCDF-4/HDF5 files, there are some settings which can only be
modified during the very first define mode of the file. For example the
compression level of a variable may be set only after the nc\_def\_var
call and before the next nc\_enddef call, whether it is called by the
user explicitly, or when the user tries to read or write some data.

At most one process should have a netCDF dataset open for writing at one
time. The library is designed to provide limited support for multiple
concurrent readers with one writer, via disciplined use of the nc\_sync
function and the NC\_SHARE flag. If a writer makes changes in define
mode, such as the addition of new variables, dimensions, or attributes,
some means external to the library is necessary to prevent readers from
making concurrent accesses and to inform readers to call nc\_sync before
the next access.

* * * * *

  ----------------------------------------------------- ---------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Adding "Previous section in reading order")]   [[\>](#Compiling "Next section in reading order")]       [[\<\<](#Use-of-the-NetCDF-Library "Beginning of this chapter or previous chapter")]   [[Up](#Use-of-the-NetCDF-Library "Up section")]   [[\>\>](#Datasets "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- ---------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.5 Error Handling {.section}
------------------

The netCDF library provides the facilities needed to handle errors in a
flexible way. Each netCDF function returns an integer status value. If
the returned status value indicates an error, you may handle it in any
way desired, from printing an associated error message and exiting to
ignoring the error indication and proceeding (not recommended!). For
simplicity, the examples in this guide check the error status and call a
separate function, handle\_err(), to handle any errors. One possible
definition of handle\_err() can be found within the documentation of
nc\_strerror (see section [Get error message corresponding to error
status: nc\_strerror](#nc_005fstrerror)).

The nc\_strerror function is available to convert a returned integer
error status into an error message string.

Occasionally, low-level I/O errors may occur in a layer below the netCDF
library. For example, if a write operation causes you to exceed disk
quotas or to attempt to write to a device that is no longer available,
you may get an error from a layer below the netCDF library, but the
resulting write error will still be reflected in the returned status
value.

* * * * *

  ----------------------------------------------------- --------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Errors "Previous section in reading order")]   [[\>](#Datasets "Next section in reading order")]       [[\<\<](#Use-of-the-NetCDF-Library "Beginning of this chapter or previous chapter")]   [[Up](#Use-of-the-NetCDF-Library "Up section")]   [[\>\>](#Datasets "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- --------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

1.6 Compiling and Linking with the NetCDF Library {.section}
-------------------------------------------------

Details of how to compile and link a program that uses the netCDF C or
FORTRAN interfaces differ, depending on the operating system, the
available compilers, where the netCDF library and include files are
installed, and whether or not you are using shared libraries.
Nevertheless, we provide here examples of how to compile and link a
program that uses the netCDF library on a Unix platform, so that you can
adjust these examples to fit your installation.

Every C file that references netCDF functions or constants must contain
an appropriate \#include statement before the first such reference:

 

~~~~ {.example}
#include <netcdf.h>
~~~~

Unless the netcdf.h file is installed in a standard directory where the
C compiler always looks, you must use the -I option when invoking the
compiler, to specify a directory where netcdf.h is installed, for
example:

 

~~~~ {.example}
cc -c -I/usr/local/netcdf/include myprogram.c
~~~~

Alternatively, you could specify an absolute path name in the \#include
statement, but then your program would not compile on another platform
where netCDF is installed in a different location.

Unless the netCDF library is installed in a standard directory where the
linker always looks, you must use the -L and -l options to link an
object file that uses the netCDF library.

If the netCDF library was configured with the –enable-shared flag, and
the operating system supports shared libraries, then it should be
possible to link an application program using a relatively simple
command. For example:

 

~~~~ {.example}
cc -o myprogram myprogram.o -L/usr/local/netcdf/lib -lnetcdf
~~~~

It should be noted that on some operating systems, when using shared
libraries, the application itself may need to be compiled using some
form of PIC (position independent code) flag; the particular flag will
depend on the C compiler used. You should try it first without any PIC
flag, and if that fails, then check with the system administrator about
the proper form of PIC flag to use.

In addition, for some C compilers (e.g. Sun’s cc compiler) it is
necessary to specify runtime paths to the relevant libnetcdf.so. This
can be accomplished in one of two ways.

1.  Add the path to the directory containing libnetcdf.so to the
    LD\_LIBRARY\_PATH environment variable. This path is searched at
    runtime to locate any needed shared library. This might be
    accomplished, for example, by the following shell command (assuming
    that libnetcdf.so is in /usr/local/netcdf/lib).

     

    ~~~~ {.example}
    LD_LIBRARY_PATH="/usr/local/netcdf/lib:$LD_LIBRARY_PATH"
    export LD_LIBRARY_PATH
    ~~~~

2.  Set the so-called runtime path when the application is linked so
    that the absolute paths of all needed shared libraries is included
    in the application binary. For gcc under Linus, this is usually
    automatic. For C compilers on Solaris (and probably other operating
    systems) the runtime path must be specified at link time. The
    command in this case might look like this.

     

    ~~~~ {.example}
    cc -o myprogram myprogram.o -L/usr/local/netcdf/lib -lnetcdf -R/usr/local/netcdf/lib
    ~~~~

    Note that the -R flag is also C compiler dependent. For gcc and
    Linux, for example, the specification is usually of this form.

     

    ~~~~ {.example}
    cc ...  -Wl,-rpath,/usr/local/netcdf/lib
    ~~~~

    Other compilers may use other flags to specify this. Check with the
    local system administrator.

If shared libraries are not supported or are not being used for some
reason, then it is necessary to include all the dependent libraries in
the compile command. For example, for a netCDF-4 enabled library, it
will be necessary to link with two HDF5 libraries, at least one
compression library, and (on some systems) the math library.

 

~~~~ {.example}
cc -o myprogram myprogram.o -L/usr/local/netcdf/lib -L/usr/local/hdf5/lib -lnetcdf -lhdf5_hl -lhdf5 -lz
~~~~

Other configuration features (e.g. DAP support or parallel IO) may
require additional libraries.

A complete list of necessary libraries can be obtained by executing the
“nc-config –libs” command. For example:

 

~~~~ {.example}
./nc-config --libs
~~~~

might return something like this:

 

~~~~ {.example}
-L/tmp/install/spock/lib -lnetcdf -L/upc/share/stdinstall/local/spock/lib
-lhdf5_hl -lhdf5 -L/upc/share/stdinstall/local/spock/lib -lz -lm
-L/upc/share/stdinstall/local/spock/lib -lcurl -L/usr/kerberos/lib64
-L/upc/share/stdinstall/local/spock/lib
-lidn -lssl -lcrypto -lldap -lrt -lssl -lcrypto -ldl -lz -lz
~~~~

Obviously there is some redundancy in this list, so it can be reduced
somewhat to produce this slightly simpler list.

 

~~~~ {.example}
-L/tmp/install/spock/lib -lnetcdf
-L/upc/share/stdinstall/local/spock/lib -lhdf5 -lhdf5_hl -lz -lcurl
-L/usr/kerberos/lib64 -lcrypto -lssl
-ldl -lidn -lldap -lm -lrt 
~~~~

* * * * *

  -------------------------------------------------------- ----------------------------------------------------------------- --- -------------------------------------------------------------------------------------- --------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Compiling "Previous section in reading order")]   [[\>](#Interface-Descriptions "Next section in reading order")]       [[\<\<](#Use-of-the-NetCDF-Library "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------- ----------------------------------------------------------------- --- -------------------------------------------------------------------------------------- --------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2. Datasets {.chapter}
===========

This chapter presents the interfaces of the netCDF functions that deal
with a netCDF dataset or the whole netCDF library.

A netCDF dataset that has not yet been opened can only be referred to by
its dataset name. Once a netCDF dataset is opened, it is referred to by
a netCDF ID, which is a small non-negative integer returned when you
create or open the dataset. A netCDF ID is much like a file descriptor
in C or a logical unit number in FORTRAN. In any single program, the
netCDF IDs of distinct open netCDF datasets are distinct. A single
netCDF dataset may be opened multiple times and will then have multiple
distinct netCDF IDs; however at most one of the open instances of a
single netCDF dataset should permit writing. When an open netCDF dataset
is closed, the ID is no longer associated with a netCDF dataset.

Functions that deal with the netCDF library include:

-   Get version of library.
-   Get error message corresponding to a returned error code.

The operations supported on a netCDF dataset as a single object are:

-   Create, given dataset name and whether to overwrite or not.
-   Open for access, given dataset name and read or write intent.
-   Put into define mode, to add dimensions, variables, or attributes.
-   Take out of define mode, checking consistency of additions.
-   Close, writing to disk if required.
-   Inquire about the number of dimensions, number of variables, number
    of global attributes, and ID of the unlimited dimension, if any.
-   Synchronize to disk to make sure it is current.
-   Set and unset nofill mode for optimized sequential writes.
-   After a summary of conventions used in describing the netCDF
    interfaces, the rest of this chapter presents a detailed description
    of the interfaces for these operations.

  -------------------------------------------------------------------------------------------------------------------------------- ---- -------------------------------------------
  [2.1 NetCDF Library Interface Descriptions](#Interface-Descriptions)                                                                  What’s in the Function Documentation
  [2.2 Parallel Access for NetCDF Files](#parallel-access)                                                                              
  [2.3 Get error message corresponding to error status: nc\_strerror](#nc_005fstrerror)                                                 Get Error Messages
  [2.4 Get netCDF library version: nc\_inq\_libvers](#nc_005finq_005flibvers)                                                           Get netCDF library version
  [2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)                                                                             Create a Dataset
  [2.6 Create a NetCDF Dataset With Performance Options: nc\_\_create](#nc_005f_005fcreate)                                             Create a Dataset with Performance Tuning
  [2.7 Create a NetCDF Dataset With Performance Options: nc\_create\_par](#nc_005fcreate_005fpar)                                       
  [2.8 Open a NetCDF Dataset for Access: nc\_open](#nc_005fopen)                                                                        Open a Dataset
  [2.9 Open a NetCDF Dataset for Access with Performance Tuning: nc\_\_open](#nc_005f_005fopen)                                         Open a Dataset with Performance Tuning
  [2.10 Open a NetCDF Dataset for Parallel Access](#nc_005fopen_005fpar)                                                                
  [2.11 Put Open NetCDF Dataset into Define Mode: nc\_redef](#nc_005fredef)                                                             Put a Dataset into Define Mode
  [2.12 Leave Define Mode: nc\_enddef](#nc_005fenddef)                                                                                  Leave Define Mode
  [2.13 Leave Define Mode with Performance Tuning: nc\_\_enddef](#nc_005f_005fenddef)                                                   Leave Define Mode with Performance Tuning
  [2.14 Close an Open NetCDF Dataset: nc\_close](#nc_005fclose)                                                                         Close a Dataset
  [2.15 Inquire about an Open NetCDF Dataset: nc\_inq Family](#nc_005finq-Family)                                                       Inquire about a Dataset
  [2.16 Synchronize an Open NetCDF Dataset to Disk: nc\_sync](#nc_005fsync)                                                             Synchronize a Dataset to Disk
  [2.17 Back Out of Recent Definitions: nc\_abort](#nc_005fabort)                                                                       Back Out of Recent Definitions
  [2.18 Set Fill Mode for Writes: nc\_set\_fill](#nc_005fset_005ffill)                                                                  Set Fill Mode for Writes
  [2.19 Set Default Creation Format: nc\_set\_default\_format](#nc_005fset_005fdefault_005fformat)                                      Change the default output format
  [2.20 Set HDF5 Chunk Cache for Future File Opens/Creates: nc\_set\_chunk\_cache](#nc_005fset_005fchunk_005fcache)                     
  [2.21 Get the HDF5 Chunk Cache Settings for Future File Opens/Creates: nc\_get\_chunk\_cache](#nc_005fget_005fchunk_005fcache)        
  -------------------------------------------------------------------------------------------------------------------------------- ---- -------------------------------------------

* * * * *

  ------------------------------------------------------- ---------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Datasets "Previous section in reading order")]   [[\>](#parallel-access "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ---------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.1 NetCDF Library Interface Descriptions {.section}
-----------------------------------------

Each interface description for a particular netCDF function in this and
later chapters contains:

-   a description of the purpose of the function;
-   a C function prototype that presents the type and order of the
    formal parameters to the function;
-   a description of each formal parameter in the C interface;
-   a list of possible error conditions; and
-   an example of a C program fragment calling the netCDF function (and
    perhaps other netCDF functions).

The examples follow a simple convention for error handling, always
checking the error status returned from each netCDF function call and
calling a handle\_error function in case an error was detected. For an
example of such a function, see [Get error message corresponding to
error status: nc\_strerror](#nc_005fstrerror).

* * * * *

  --------------------------------------------------------------------- ---------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Interface-Descriptions "Previous section in reading order")]   [[\>](#nc_005fstrerror "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ---------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.2 Parallel Access for NetCDF Files {.section}
------------------------------------

To use parallel access, open or create the file with nc\_open\_par (see
[Open a NetCDF Dataset for Parallel Access](#nc_005fopen_005fpar)) or
nc\_create\_par (see [Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)).

The mode flag NC\_PNETCDF will be automatically turned on for classic or
64-bit offset files opened or created with the parallel access
functions.

The following example shows the creation of a file using parallel access
with a netCDF-4/HDF5 file, and how a program might write data to such a
file.

 

~~~~ {.example}
#include "netcdf.h"
#include <mpi.h>
#include <assert.h>
#include "hdf5.h"
#include <string.h>
#include <stdlib.h>

#define BAIL(e) do { \
printf("Bailing out in file %s, line %d, error:%s.\n", __FILE__, __LINE__, nc_strerror(e)); \
return e; \
} while (0) 

#define FILE "test_par.nc"
#define NDIMS 2
#define DIMSIZE 24
#define QTR_DATA (DIMSIZE*DIMSIZE/4)
#define NUM_PROC 4

int
main(int argc, char **argv)
{
    /* MPI stuff. */
    int mpi_namelen;            
    char mpi_name[MPI_MAX_PROCESSOR_NAME];
    int mpi_size, mpi_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* Netcdf-4 stuff. */
    int ncid, v1id, dimids[NDIMS];
    size_t start[NDIMS], count[NDIMS];

    int data[DIMSIZE*DIMSIZE], j, i, res;

    /* Initialize MPI. */
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Get_processor_name(mpi_name, &mpi_namelen);
    printf("mpi_name: %s size: %d rank: %d\n", mpi_name, 
           mpi_size, mpi_rank);

    /* Create a parallel netcdf-4 file. */
    if ((res = nc_create_par(FILE, NC_NETCDF4|NC_MPIIO, comm, 
                             info, &ncid)))
        BAIL(res);

    /* Create two dimensions. */
    if ((res = nc_def_dim(ncid, "d1", DIMSIZE, dimids)))
       BAIL(res);
    if ((res = nc_def_dim(ncid, "d2", DIMSIZE, &dimids[1])))
       BAIL(res);

    /* Create one var. */
    if ((res = nc_def_var(ncid, "v1", NC_INT, NDIMS, dimids, &v1id)))
       BAIL(res);

    if ((res = nc_enddef(ncid)))
       BAIL(res);

    /* Set up slab for this process. */
    start[0] = mpi_rank * DIMSIZE/mpi_size;
    start[1] = 0;
    count[0] = DIMSIZE/mpi_size;
    count[1] = DIMSIZE;
    printf("mpi_rank=%d start[0]=%d start[1]=%d count[0]=%d count[1]=%d\n",
           mpi_rank, start[0], start[1], count[0], count[1]);

    /* Create phony data. We're going to write a 24x24 array of ints,
       in 4 sets of 144. */
    printf("mpi_rank*QTR_DATA=%d (mpi_rank+1)*QTR_DATA-1=%d\n",
           mpi_rank*QTR_DATA, (mpi_rank+1)*QTR_DATA);
    for (i=mpi_rank*QTR_DATA; i<(mpi_rank+1)*QTR_DATA; i++)
       data[i] = mpi_rank;

    /*if ((res = nc_var_par_access(ncid, v1id, NC_COLLECTIVE)))
      BAIL(res);*/
    if ((res = nc_var_par_access(ncid, v1id, NC_INDEPENDENT)))
       BAIL(res);

    /* Write slabs of phony data. */
    if ((res = nc_put_vara_int(ncid, v1id, start, count, 
                               &data[mpi_rank*QTR_DATA])))
       BAIL(res);

    /* Close the netcdf file. */
    if ((res = nc_close(ncid)))
        BAIL(res);
    
    /* Shut down MPI. */
    MPI_Finalize();

    return 0;
}
~~~~

* * * * *

  -------------------------------------------------------------- ----------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#parallel-access "Previous section in reading order")]   [[\>](#nc_005finq_005flibvers "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- ----------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.3 Get error message corresponding to error status: nc\_strerror {.section}
-----------------------------------------------------------------

The function nc\_strerror returns a static reference to an error message
string corresponding to an integer netCDF error status or to a system
error number, presumably returned by a previous call to some other
netCDF function. The list of netCDF error status codes is available in
the appropriate include file for each language binding.

Usage {.heading}
-----

 

~~~~ {.example}
const char * nc_strerror(int ncerr);
~~~~

 `ncerr`
:   An error status that might have been returned from a previous call
    to some netCDF function.

Errors {.heading}
------

If you provide an invalid integer error status that does not correspond
to any netCDF error message or or to any system error message (as
understood by the system strerror function), nc\_strerror returns a
string indicating that there is no such error status.

Example {.heading}
-------

Here is an example of a simple error handling function that uses
nc\_strerror to print the error message corresponding to the netCDF
error status returned from any netCDF function call and then exit:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
void handle_error(int status) {
if (status != NC_NOERR) {
   fprintf(stderr, "%s\n", nc_strerror(status));
   exit(-1);
   }
}
~~~~

* * * * *

  -------------------------------------------------------------- -------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fstrerror "Previous section in reading order")]   [[\>](#nc_005fcreate "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- -------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.4 Get netCDF library version: nc\_inq\_libvers {.section}
------------------------------------------------

The function nc\_inq\_libvers returns a string identifying the version
of the netCDF library, and when it was built.

Usage {.heading}
-----

 

~~~~ {.example}
const char * nc_inq_libvers(void);
~~~~

Errors {.heading}
------

This function takes no arguments, and thus no errors are possible in its
invocation.

Example {.heading}
-------

Here is an example using nc\_inq\_libvers to print the version of the
netCDF library with which the program is linked:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
   printf("%s\n", nc_inq_libvers());
~~~~

* * * * *

  --------------------------------------------------------------------- ------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005flibvers "Previous section in reading order")]   [[\>](#nc_005f_005fcreate "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.5 Create a NetCDF Dataset: nc\_create {.section}
---------------------------------------

This function creates a new netCDF dataset, returning a netCDF ID that
can subsequently be used to refer to the netCDF dataset in other netCDF
function calls. The new netCDF dataset opened for write access and
placed in define mode, ready for you to add dimensions, variables, and
attributes.

A creation mode flag specifies:

-   whether to overwrite any existing dataset with the same name,
-   whether access to the dataset is shared,
-   whether this file should be in netCDF classic format (the default),
    the new 64-bit offset format (use NC\_64BIT\_OFFSET), or NC\_NETCDF4
    for a netCDF-4/HDF5 file.

Usage {.heading}
-----

NOTE: When creating a netCDF-4 file HDF5 error reporting is turned off,
if it is on. This doesn’t stop the HDF5 error stack from recording the
errors, it simply stops their display to the user through stderr.

 

~~~~ {.example}
int nc_create (const char* path, int cmode, int *ncidp);
~~~~

 `path`
:   The file name of the new netCDF dataset.

 `cmode`
:   The creation mode flag. The following flags are available:
    NC\_NOCLOBBER, NC\_SHARE, NC\_64BIT\_OFFSET, NC\_NETCDF4,
    NC\_CLASSIC\_MODEL.

    Setting NC\_NOCLOBBER means you do not want to clobber (overwrite)
    an existing dataset; an error (NC\_EEXIST) is returned if the
    specified dataset already exists.

    The NC\_SHARE flag is appropriate when one process may be writing
    the dataset and one or more other processes reading the dataset
    concurrently; it means that dataset accesses are not buffered and
    caching is limited. Since the buffering scheme is optimized for
    sequential access, programs that do not access data sequentially may
    see some performance improvement by setting the NC\_SHARE flag. This
    flag is ignored for netCDF-4 files. (See below.)

    Setting NC\_64BIT\_OFFSET causes netCDF to create a 64-bit offset
    format file, instead of a netCDF classic format file. The 64-bit
    offset format imposes far fewer restrictions on very large (i.e.
    over 2 GB) data files. See [(netcdf)Large File
    Support](netcdf.html#Large-File-Support) section ‘Large File
    Support’ in The NetCDF Users Guide.

    A zero value (defined for convenience as NC\_CLOBBER) specifies the
    default behavior: overwrite any existing dataset with the same file
    name and buffer and cache accesses for efficiency. The dataset will
    be in netCDF classic format. See [(netcdf)NetCDF Classic Format
    Limitations](netcdf.html#NetCDF-Classic-Format-Limitations) section
    ‘NetCDF Classic Format Limitations’ in The NetCDF Users Guide.

    Setting NC\_NETCDF4 causes netCDF to create a HDF5/NetCDF-4 file.

    Setting NC\_CLASSIC\_MODEL causes netCDF to enforce the classic data
    model in this file. (This only has effect for netCDF-4/HDF5 files,
    as classic and 64-bit offset files always use the classic model.)
    When used with NC\_NETCDF4, this flag ensures that the resulting
    netCDF-4/HDF5 file may never contain any new constructs from the
    enhanced data model. That is, it cannot contain groups, user defined
    types, multiple unlimited dimensions, or new atomic types. The
    advantage of this restriction is that such files are guaranteed to
    work with existing netCDF software.

 `ncidp`
:   Pointer to location where returned netCDF ID is to be stored.

Errors {.heading}
------

nc\_create returns the value NC\_NOERR if no errors occurred. Possible
causes of errors include:

-   Passing a dataset name that includes a directory that does not
    exist.
-   Specifying a dataset name of a file that exists and also specifying
    NC\_NOCLOBBER.
-   Specifying a meaningless value for the creation mode.
-   Attempting to create a netCDF dataset in a directory where you don’t
    have permission to create files.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_ENOMEM`
:   System out of memory.

 `NC_EHDFERR`
:   HDF5 error (netCDF-4 files only).

 `NC_EFILEMETA`
:   Error writing netCDF-4 file-level metadata in HDF5 file. (netCDF-4
    files only).

Examples {.heading}
--------

In this example we create a netCDF dataset named foo.nc; we want the
dataset to be created in the current directory only if a dataset with
that name does not already exist:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

In this example we create a netCDF dataset named foo\_large.nc. It will
be in the 64-bit offset format.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_create("foo_large.nc", NC_NOCLOBBER|NC_64BIT_OFFSET, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

In this example we create a netCDF dataset named foo\_HDF5.nc. It will
be in the HDF5 format.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_create("foo_HDF5.nc", NC_NOCLOBBER|NC_NETCDF4, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

In this example we create a netCDF dataset named foo\_HDF5\_classic.nc.
It will be in the HDF5 format, but will not allow the use of any
netCDF-4 advanced features. That is, it will conform to the classic
netCDF-3 data model.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_create("foo_HDF5_classic.nc", NC_NOCLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

A variant of nc\_create, nc\_\_create (note the double underscore)
allows users to specify two tuning parameters for the file that it is
creating. These tuning parameters are not written to the data file, they
are only used for so long as the file remains open after an
nc\_\_create. See section [Create a NetCDF Dataset With Performance
Options: nc\_\_create](#nc_005f_005fcreate).

* * * * *

  ------------------------------------------------------------ ---------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fcreate "Previous section in reading order")]   [[\>](#nc_005fcreate_005fpar "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ---------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.6 Create a NetCDF Dataset With Performance Options: nc\_\_create {.section}
------------------------------------------------------------------

This function is a variant of nc\_create, nc\_\_create (note the double
underscore) allows users to specify two tuning parameters for the file
that it is creating. These tuning parameters are not written to the data
file, they are only used for so long as the file remains open after an
nc\_\_create.

This function creates a new netCDF dataset, returning a netCDF ID that
can subsequently be used to refer to the netCDF dataset in other netCDF
function calls. The new netCDF dataset opened for write access and
placed in define mode, ready for you to add dimensions, variables, and
attributes.

A creation mode flag specifies whether to overwrite any existing dataset
with the same name and whether access to the dataset is shared, and
whether this file should be in netCDF classic format (the default), or
the new 64-bit offset format.

Usage {.heading}
-----

 

~~~~ {.example}
int nc__create(const char *path, int cmode, size_t initialsz,
               size_t *bufrsizehintp, int *ncidp);
~~~~

 `path`
:   The file name of the new netCDF dataset.

 `cmode`
:   The creation mode flag. The following flags are available:
    NC\_NOCLOBBER, NC\_SHARE, and NC\_64BIT\_OFFSET, NC\_NETCDF4,
    NC\_CLASSIC\_MODEL.

    Setting NC\_NOCLOBBER means you do not want to clobber (overwrite)
    an existing dataset; an error (NC\_EEXIST) is returned if the
    specified dataset already exists.

    The NC\_SHARE flag is appropriate when one process may be writing
    the dataset and one or more other processes reading the dataset
    concurrently; it means that dataset accesses are not buffered and
    caching is limited. Since the buffering scheme is optimized for
    sequential access, programs that do not access data sequentially may
    see some performance improvement by setting the NC\_SHARE flag. This
    flag is ignored for netCDF-4 files. (See below.)

    Setting NC\_64BIT\_OFFSET causes netCDF to create a 64-bit offset
    format file, instead of a netCDF classic format file. The 64-bit
    offset format imposes far fewer restrictions on very large (i.e.
    over 2 GB) data files. See [(netcdf)Large File
    Support](netcdf.html#Large-File-Support) section ‘Large File
    Support’ in The NetCDF Users Guide.

    A zero value (defined for convenience as NC\_CLOBBER) specifies the
    default behavior: overwrite any existing dataset with the same file
    name and buffer and cache accesses for efficiency. The dataset will
    be in netCDF classic format. See [(netcdf)NetCDF Classic Format
    Limitations](netcdf.html#NetCDF-Classic-Format-Limitations) section
    ‘NetCDF Classic Format Limitations’ in The NetCDF Users Guide.

    Setting NC\_NETCDF4 causes netCDF to create a HDF5/NetCDF-4 file.

    Setting NC\_CLASSIC\_MODEL causes netCDF to enforce the classic data
    model in this file. (This only has effect for netCDF-4/HDF5 files,
    as classic and 64-bit offset files always use the classic model.)
    When used with NC\_NETCDF4, this flag ensures that the resulting
    netCDF-4/HDF5 file may never contain any new constructs from the
    enhanced data model. That is, it cannot contain groups, user defined
    types, multiple unlimited dimensions, or new atomic types. The
    advantage of this restriction is that such files are guaranteed to
    work with existing netCDF software.

 `initialsz`
:   On some systems, and with custom I/O layers, it may be advantageous
    to set the size of the output file at creation time. This parameter
    sets the initial size of the file at creation time.

 `bufrsizehintp`
:   The argument referenced by bufrsizehintp controls a space versus
    time tradeoff, memory allocated in the netcdf library versus number
    of system calls.

    Because of internal requirements, the value may not be set to
    exactly the value requested. The actual value chosen is returned by
    reference.

    Using the value NC\_SIZEHINT\_DEFAULT causes the library to choose a
    default. How the system chooses the default depends on the system.
    On many systems, the "preferred I/O block size" is available from
    the stat() system call, struct stat member st\_blksize. If this is
    available it is used. Lacking that, twice the system pagesize is
    used.

    Lacking a call to discover the system pagesize, we just set default
    bufrsize to 8192.

    The bufrsize is a property of a given open netcdf descriptor ncid,
    it is not a persistent property of the netcdf dataset.

 `ncidp`
:   Pointer to location where returned netCDF ID is to be stored.

Errors {.heading}
------

nc\_create returns the value NC\_NOERR if no errors occurred. Possible
causes of errors include:

-   Passing a dataset name that includes a directory that does not
    exist.
-   Specifying a dataset name of a file that exists and also specifying
    NC\_NOCLOBBER.
-   Specifying a meaningless value for the creation mode.
-   Attempting to create a netCDF dataset in a directory where you don’t
    have permission to create files.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_ENOMEM`
:   System out of memory.

 `NC_EHDFERR`
:   HDF5 error (netCDF-4 files only).

 `NC_EFILEMETA`
:   Error writing netCDF-4 file-level metadata in HDF5 file. (netCDF-4
    files only).

Examples {.heading}
--------

In this example we create a netCDF dataset named foo.nc; we want the
dataset to be created in the current directory only if a dataset with
that name does not already exist:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

In this example we create a netCDF dataset named foo\_large.nc; we want
the dataset to be created in the current directory only if a dataset
with that name does not already exist. We also specify that bufrsize and
initial size for the file.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
int intialsz = 2048;
int *bufrsize;
   ... 
*bufrsize = 1024;
status = nc__create("foo.nc", NC_NOCLOBBER, initialsz, bufrsize, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ----------------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005f_005fcreate "Previous section in reading order")]   [[\>](#nc_005fopen "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.7 Create a NetCDF Dataset With Performance Options: nc\_create\_par {.section}
---------------------------------------------------------------------

This function is a variant of nc\_create, nc\_create\_par allows users
to open a file on a MPI/IO or MPI/Posix parallel file system.

The parallel parameters are not written to the data file, they are only
used for so long as the file remains open after an nc\_create\_par.

This function creates a new netCDF dataset, returning a netCDF ID that
can subsequently be used to refer to the netCDF dataset in other netCDF
function calls. The new netCDF dataset opened for write access and
placed in define mode, ready for you to add dimensions, variables, and
attributes.

If the NC\_NETCDF4 flag is used, the HDF5 library is used for parallel
I/O. If not, the parallel-netcdf library is used.

When a file is created for parallel access, independent operations are
the default. To use independent access on a variable, See section
[Change between Collective and Independent Parallel Access:
nc\_var\_par\_access](#nc_005fvar_005fpar_005faccess).

Usage {.heading}
-----

 

~~~~ {.example}
int nc_create_par(const char *path, int cmode, MPI_Comm comm,
                  MPI_Info info, int ncidp);
~~~~

 `path`
:   The file name of the new netCDF dataset.

 `cmode`
:   Either the NC\_MPIIO or NC\_MPIPOSIX flags may be present if the
    NC\_NETCDF4 flag is used.

    The NC\_SHARE flag is ignored.

 `comm`
:   The MPI\_Comm object returned by the MPI layer.

 `info`
:   The MPI\_Info object returned by the MPI layer, if MPI/IO is being
    used, or 0 if MPI/Posix is being used.

 `ncidp`
:   Pointer to location where returned netCDF ID is to be stored.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_EPARINIT`
:   Could not initialize parallel access at the HDF5 layer.

 `NC_EFILEMETA`
:   HDF5 layer cannot handle create or root group open.

 `NC_EEXIST`
:   Specifying a dataset name of a file that exists and also specifying
    NC\_NOCLOBBER.

 `NC_EINVAL`
:   Bad value for the creation mode.

 `NC_EHDFERR`
:   Unexpected error from the HDF5 layer.

Examples {.heading}
--------

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
*bufrsize = 1024;
status = nc__create("foo.nc", NC_NOCLOBBER, initialsz, bufrsize, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  -------------------------------------------------------------------- ----------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fcreate_005fpar "Previous section in reading order")]   [[\>](#nc_005f_005fopen "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ----------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.8 Open a NetCDF Dataset for Access: nc\_open {.section}
----------------------------------------------

The function nc\_open opens an existing netCDF dataset for access. It
determines the underlying file format automatically. Use the same call
to open a netCDF classic, 64-bit offset, or netCDF-4 file.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_open (const char *path, int omode, int *ncidp);
~~~~

 `path`
:   File name for netCDF dataset to be opened. When DAP support is
    enabled, then the path may be an OPeNDAP URL rather than a file
    path.

 `omode`
:   A zero value (or NC\_NOWRITE) specifies the default behavior: open
    the dataset with read-only access, buffering and caching accesses
    for efficiency

    Otherwise, the open mode is NC\_WRITE, NC\_SHARE, or
    NC\_WRITE|NC\_SHARE. Setting the NC\_WRITE flag opens the dataset
    with read-write access. ("Writing" means any kind of change to the
    dataset, including appending or changing data, adding or renaming
    dimensions, variables, and attributes, or deleting attributes.)

    The NC\_SHARE flag is only used for netCDF classic and 64-bit offset
    files. It is appropriate when one process may be writing the dataset
    and one or more other processes reading the dataset concurrently; it
    means that dataset accesses are not buffered and caching is limited.
    Since the buffering scheme is optimized for sequential access,
    programs that do not access data sequentially may see some
    performance improvement by setting the NC\_SHARE flag.

    It is not necessary to pass any information about the format of the
    file being opened. The file type will be detected automatically by
    the netCDF library.

    If a the path is a DAP URL, then the open mode is read-only. Setting
    NC\_WRITE will be ignored.

 `ncidp`
:   Pointer to location where returned netCDF ID is to be stored.

Errors {.heading}
------

When opening a netCDF-4 file HDF5 error reporting is turned off, if it
is on. This doesn’t stop the HDF5 error stack from recording the errors,
it simply stops their display to the user through stderr.

nc\_open returns the value NC\_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

-   The specified netCDF dataset does not exist.
-   A meaningless mode was specified.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_NOMEM`
:   Out of memory.

 `NC_EHDFERR`
:   HDF5 error. (NetCDF-4 files only.)

 `NC_EDIMMETA`
:   Error in netCDF-4 dimension metadata. (NetCDF-4 files only.)

 `NC_ENOCOMPOIND`
:   (NetCDF-4 files only.)

Example {.heading}
-------

Here is an example using nc\_open to open an existing netCDF dataset
named foo.nc for read-only, non-shared access:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_open("foo.nc", 0, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------- -------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fopen "Previous section in reading order")]   [[\>](#nc_005fopen_005fpar "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- -------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.9 Open a NetCDF Dataset for Access with Performance Tuning: nc\_\_open {.section}
------------------------------------------------------------------------

A function opens a netCDF dataset for access with an additional
performance tuning parameter. When DAP support is enabled, it is
possible to open a DAP data source through this interface, but it is
deprecated because all of the performance tuning parameters are ignored.
The standard nc\_open interface should be used instead.

Usage {.heading}
-----

 

~~~~ {.example}
int nc__open(const char *path, int mode, size_t *bufrsizehintp, int *ncidp);
~~~~

 `path`
:   File name for netCDF dataset to be opened.

 `omode`
:   A zero value (or NC\_NOWRITE) specifies the default behavior: open
    the dataset with read-only access, buffering and caching accesses
    for efficiency

    Otherwise, the open mode is NC\_WRITE, NC\_SHARE, or
    NC\_WRITE|NC\_SHARE. Setting the NC\_WRITE flag opens the dataset
    with read-write access. ("Writing" means any kind of change to the
    dataset, including appending or changing data, adding or renaming
    dimensions, variables, and attributes, or deleting attributes.) The
    NC\_SHARE flag is appropriate when one process may be writing the
    dataset and one or more other processes reading the dataset
    concurrently; it means that dataset accesses are not buffered and
    caching is limited. Since the buffering scheme is optimized for
    sequential access, programs that do not access data sequentially may
    see some performance improvement by setting the NC\_SHARE flag.

 `bufrsizehintp`
:   The argument referenced by bufrsizehintp controls a space versus
    time tradeoff, memory allocated in the netcdf library versus number
    of system calls.

    Because of internal requirements, the value may not be set to
    exactly the value requested. The actual value chosen is returned by
    reference.

    Using the value NC\_SIZEHINT\_DEFAULT causes the library to choose a
    default. How the system chooses the default depends on the system.
    On many systems, the "preferred I/O block size" is available from
    the stat() system call, struct stat member st\_blksize. If this is
    available it is used. Lacking that, twice the system pagesize is
    used.

    Lacking a call to discover the system pagesize, we just set default
    bufrsize to 8192.

    The bufrsize is a property of a given open netcdf descriptor ncid,
    it is not a persistent property of the netcdf dataset.

 `ncidp`
:   Pointer to location where returned netCDF ID is to be stored.

Errors {.heading}
------

nc\_\_open returns the value NC\_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

-   The specified netCDF dataset does not exist.
-   A meaningless mode was specified.

Example {.heading}
-------

Here is an example using nc\_\_open to open an existing netCDF dataset
named foo.nc for read-only, non-shared access:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
size_t bufrsize;
   ... 
*bufrsize = 1024;
status = nc_open("foo.nc", 0, &bufrsize, &ncid);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  --------------------------------------------------------------- ------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005f_005fopen "Previous section in reading order")]   [[\>](#nc_005fredef "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- ------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.10 Open a NetCDF Dataset for Parallel Access {.section}
----------------------------------------------

This function opens a netCDF-4 dataset for parallel access.

For netcdf-4/HDF5 files, the HDF5 library parallel I/O is used. This
opens the file using either MPI-IO or MPI-POSIX.

DAP access is not allowed with parallel I/O.

When netCDF opens a file for parallel access, independent operations are
the default. To use independent access on a variable, See section
[Change between Collective and Independent Parallel Access:
nc\_var\_par\_access](#nc_005fvar_005fpar_005faccess).

Usage {.heading}
-----

 

~~~~ {.example}
int nc_open_par(const char *path, int mode, MPI_Comm comm, 
                MPI_Info info, int *ncidp);
~~~~

 `path`
:   File name for netCDF dataset to be opened.

 `omode`
:   Either the NC\_MPIIO or NC\_MPIPOSIX flags may be present for a
    netCDF-4/HDF5 file.

    The flag NC\_WRITE opens the dataset with read-write access.
    ("Writing" means any kind of change to the dataset, including
    appending or changing data, adding or renaming dimensions,
    variables, and attributes, or deleting attributes.)

    All other flags are ignored or not allowed. The NC\_NETCDF4 flag is
    not required, as the file type is detected when the file is opened.

 `comm`
:   MPI\_Comm object returned by the MPI layer.

 `info`
:   MPI\_Info object returned by the MPI layer, or NULL if MPI-POSIX
    access is desired.

 `ncidp`
:   Pointer to location where returned netCDF ID is to be stored.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

``
:   The specified netCDF dataset does not exist.

``
:   A meaningless mode was specified.

Example {.heading}
-------

Here is an example (from nc\_test4/tst\_parallel2.c) using
nc\_open\_par.

 

~~~~ {.example}
    /* Reopen the file and check it. */
    if (nc_open_par(file_name, NC_NOWRITE, comm, info, &ncid)) ERR;

    /* Read all the slabs this process is responsible for. */
    for (i = 0; i < NUM_SLABS / mpi_size; i++)
    {
       start[0] = NUM_SLABS / mpi_size * mpi_rank + i;
       /* Read one slab of data. */
       if (nc_get_vara_int(ncid, varid, start, count, data_in)) ERR;
    }
~~~~

* * * * *

  ------------------------------------------------------------------ -------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fopen_005fpar "Previous section in reading order")]   [[\>](#nc_005fenddef "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ -------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.11 Put Open NetCDF Dataset into Define Mode: nc\_redef {.section}
--------------------------------------------------------

The function nc\_redef puts an open netCDF dataset into define mode, so
dimensions, variables, and attributes can be added or renamed and
attributes can be deleted.

Usage {.heading}
-----

For netCDF-4 files (i.e. files created with NC\_NETCDF4 in the cmode,
see section [Create a NetCDF Dataset: nc\_create](#nc_005fcreate)), it
is not necessary to call nc\_redef unless the file was also created with
NC\_STRICT\_NC3. For straight-up netCDF-4 files, nc\_redef is called
automatically, as needed.

For all netCDF-4 files, the root ncid must be used. This is the ncid
returned by nc\_open and nc\_create, and points to the root of the
hierarchy tree for netCDF-4 files.

 

~~~~ {.example}
int nc_redef(int ncid);
~~~~

 `ncid`
:   netCDF ID, from a previous call to nc\_open or nc\_create.

Errors {.heading}
------

nc\_redef returns the value NC\_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

-   The specified netCDF dataset is already in define mode. This error
    code will only be returned for classic and 64-bit offset format
    files.
-   The specified netCDF dataset was opened for read-only.
-   The specified netCDF ID does not refer to an open netCDF dataset.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad ncid.

 `NC_EBADGRPID`
:   The ncid must refer to the root group of the file, that is, the
    group returned by nc\_open or nc\_create. (see section [Open a
    NetCDF Dataset for Access: nc\_open](#nc_005fopen) see section
    [Create a NetCDF Dataset: nc\_create](#nc_005fcreate)).

 `NC_EINDEFINE`
:   Already in define mode.

 `NC_EPERM`
:   File is read-only.

Example {.heading}
-------

Here is an example using nc\_redef to open an existing netCDF dataset
named foo.nc and put it into define mode:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);  /* open dataset */
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_redef(ncid);                      /* put in define mode */
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ----------------------------------------------------------- ------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fredef "Previous section in reading order")]   [[\>](#nc_005f_005fenddef "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- ------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.12 Leave Define Mode: nc\_enddef {.section}
----------------------------------

The function nc\_enddef takes an open netCDF dataset out of define mode.
The changes made to the netCDF dataset while it was in define mode are
checked and committed to disk if no problems occurred. Non-record
variables may be initialized to a "fill value" as well. See section [Set
Fill Mode for Writes: nc\_set\_fill](#nc_005fset_005ffill). The netCDF
dataset is then placed in data mode, so variable data can be read or
written.

It’s not necessary to call nc\_enddef for netCDF-4 files. With netCDF-4
files, nc\_enddef is called when needed by the netcdf-4 library. User
calls to nc\_enddef for netCDF-4 files still flush the metadata to disk.

This call may involve copying data under some circumstances. For a more
extensive discussion see [(netcdf)File Structure and
Performance](netcdf.html#File-Structure-and-Performance) section ‘File
Structure and Performance’ in The NetCDF Users Guide.

For netCDF-4/HDF5 format files there are some variable settings (the
compression, endianness, fletcher32 error correction, and fill value)
which must be set (if they are going to be set at all) between the
nc\_def\_var and the next nc\_enddef. Once the nc\_enddef is called,
these settings can no longer be changed for a variable.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_enddef(int ncid);
~~~~

 `ncid`
:   NetCDF ID, from a previous call to nc\_open or nc\_create. If you
    use a group id, the enddef will apply to the entire file. That all,
    the enddef will not just end define mode in one group, but in the
    entire file.

Errors {.heading}
------

nc\_enddef returns the value NC\_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

-   The specified netCDF dataset is not in define mode.
-   The specified netCDF ID does not refer to an open netCDF dataset.
-   The size of one or more variables exceed the size constraints for
    whichever variant of the file format is in use). See [(netcdf)Large
    File Support](netcdf.html#Large-File-Support) section ‘Large File
    Support’ in The NetCDF Users Guide.

Example {.heading}
-------

Here is an example using nc\_enddef to finish the definitions of a new
netCDF dataset named foo.nc and put it into data mode:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
if (status != NC_NOERR) handle_error(status);

   ...       /* create dimensions, variables, attributes */

status = nc_enddef(ncid);  /*leave define mode*/
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ------------------------------------------------------------ ------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fenddef "Previous section in reading order")]   [[\>](#nc_005fclose "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.13 Leave Define Mode with Performance Tuning: nc\_\_enddef {.section}
------------------------------------------------------------

The function nc\_\_enddef takes an open netCDF dataset out of define
mode. The changes made to the netCDF dataset while it was in define mode
are checked and committed to disk if no problems occurred. Non-record
variables may be initialized to a "fill value" as well. See section [Set
Fill Mode for Writes: nc\_set\_fill](#nc_005fset_005ffill). The netCDF
dataset is then placed in data mode, so variable data can be read or
written.

This call may involve copying data under some circumstances. For a more
extensive discussion see [(netcdf)File Structure and
Performance](netcdf.html#File-Structure-and-Performance) section ‘File
Structure and Performance’ in The NetCDF Users Guide.

Caution: this function exposes internals of the netcdf version 1 file
format. Users should use nc\_enddef in most circumstances. This function
may not be available on future netcdf implementations.

The current netcdf file format has three sections, the "header" section,
the data section for fixed size variables, and the data section for
variables which have an unlimited dimension (record variables).

The header begins at the beginning of the file. The index (offset) of
the beginning of the other two sections is contained in the header.
Typically, there is no space between the sections. This causes copying
overhead to accrue if one wishes to change the size of the sections, as
may happen when changing names of things, text attribute values, adding
attributes or adding variables. Also, for buffered i/o, there may be
advantages to aligning sections in certain ways.

The minfree parameters allow one to control costs of future calls to
nc\_redef, nc\_enddef by requesting that minfree bytes be available at
the end of the section.

The align parameters allow one to set the alignment of the beginning of
the corresponding sections. The beginning of the section is rounded up
to an index which is a multiple of the align parameter. The flag value
ALIGN\_CHUNK tells the library to use the bufrsize (see above) as the
align parameter. It has nothing to do with the chunking
(multidimensional tiling) features of netCDF-4.

The file format requires mod 4 alignment, so the align parameters are
silently rounded up to multiples of 4. The usual call,

 

~~~~ {.example}
nc_enddef(ncid);
~~~~

is equivalent to

 

~~~~ {.example}
nc__enddef(ncid, 0, 4, 0, 4);
~~~~

The file format does not contain a "record size" value, this is
calculated from the sizes of the record variables. This unfortunate fact
prevents us from providing minfree and alignment control of the
"records" in a netcdf file. If you add a variable which has an unlimited
dimension, the third section will always be copied with the new variable
added.

Usage {.heading}
-----

 

~~~~ {.example}
int nc__enddef(int ncid, size_t h_minfree, size_t v_align,
               size_t v_minfree, size_t r_align);
~~~~

 `ncid`
:   NetCDF ID, from a previous call to nc\_open or nc\_create.

 `h_minfree`
:   Sets the pad at the end of the "header" section.

 `v_align`
:   Controls the alignment of the beginning of the data section for
    fixed size variables.

 `v_minfree`
:   Sets the pad at the end of the data section for fixed size
    variables.

 `r_align`
:   Controls the alignment of the beginning of the data section for
    variables which have an unlimited dimension (record variables).

Errors {.heading}
------

nc\_\_enddef returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The specified netCDF dataset is not in define mode.
-   The specified netCDF ID does not refer to an open netCDF dataset.
-   The size of one or more variables exceed the size constraints for
    whichever variant of the file format is in use). See [(netcdf)Large
    File Support](netcdf.html#Large-File-Support) section ‘Large File
    Support’ in The NetCDF Users Guide.

Example {.heading}
-------

Here is an example using nc\_enddef to finish the definitions of a new
netCDF dataset named foo.nc and put it into data mode:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
if (status != NC_NOERR) handle_error(status);

   ...       /* create dimensions, variables, attributes */

status = nc_enddef(ncid);  /*leave define mode*/
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ----------------------------------------------------------------- ------------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005f_005fenddef "Previous section in reading order")]   [[\>](#nc_005finq-Family "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- ------------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.14 Close an Open NetCDF Dataset: nc\_close {.section}
--------------------------------------------

The function nc\_close closes an open netCDF dataset.

If the dataset in define mode, nc\_enddef will be called before closing.
(In this case, if nc\_enddef returns an error, nc\_abort will
automatically be called to restore the dataset to the consistent state
before define mode was last entered.) After an open netCDF dataset is
closed, its netCDF ID may be reassigned to the next netCDF dataset that
is opened or created.

Usage {.heading}
-----

For netCDF-4 files, the ncid of the root group must be passed into
nc\_close.

 

~~~~ {.example}
int nc_close(int ncid);
~~~~

 `ncid`
:   NetCDF ID, from a previous call to nc\_open or nc\_create.

Errors {.heading}
------

nc\_close returns the value NC\_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

-   Define mode was entered and the automatic call made to nc\_enddef
    failed.
-   The specified netCDF ID does not refer to an open netCDF dataset.

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Invalid id passed.

 `NC_EBADGRPID`
:   ncid did not contain the root group id of this file. (NetCDF-4
    only).

Example {.heading}
-------

Here is an example using nc\_close to finish the definitions of a new
netCDF dataset named foo.nc and release its netCDF ID:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
if (status != NC_NOERR) handle_error(status);

   ...       /* create dimensions, variables, attributes */

status = nc_close(ncid);       /* close netCDF dataset */
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ----------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fclose "Previous section in reading order")]   [[\>](#nc_005fsync "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.15 Inquire about an Open NetCDF Dataset: nc\_inq Family {.section}
---------------------------------------------------------

Members of the nc\_inq family of functions return information about an
open netCDF dataset, given its netCDF ID. Dataset inquire functions may
be called from either define mode or data mode. The first function,
nc\_inq, returns values for the number of dimensions, the number of
variables, the number of global attributes, and the dimension ID of the
dimension defined with unlimited length, if any. Most of the other
functions in the family each return just one of these items of
information.

For C, these functions include nc\_inq, nc\_inq\_ndims, nc\_inq\_nvars,
nc\_inq\_natts, and nc\_inq\_unlimdim. An additional function,
nc\_inq\_format, returns the (rarely needed) format version. Another
function, nc\_inq\_path, returns the file name or URL with which a file
was opened or created.

No I/O is performed when these functions are called, since the required
information is available in memory for each open netCDF dataset.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq          (int ncid, int *ndimsp, int *nvarsp, int *ngattsp,
                     int *unlimdimidp);
int nc_inq_ndims    (int ncid, int *ndimsp);
int nc_inq_nvars    (int ncid, int *nvarsp);
int nc_inq_natts    (int ncid, int *ngattsp);
int nc_inq_unlimdim (int ncid, int *unlimdimidp);
int nc_inq_format   (int ncid, int *formatp);
int nc_inq_path     (int ncid, size_t *pathlenp, char *path);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `ndimsp`
:   Pointer to location for returned number of dimensions defined for
    this netCDF dataset.

 `nvarsp`
:   Pointer to location for returned number of variables defined for
    this netCDF dataset.

 `ngattsp`
:   Pointer to location for returned number of global attributes defined
    for this netCDF dataset.

 `unlimdimidp`
:   Pointer to location for returned ID of the unlimited dimension, if
    there is one for this netCDF dataset. If no unlimited length
    dimension has been defined, -1 is returned.

 `formatp`
:   Pointer to location for returned format version, one of
    NC\_FORMAT\_CLASSIC, NC\_FORMAT\_64BIT, NC\_FORMAT\_NETCDF4,
    NC\_FORMAT\_NETCDF4\_CLASSIC.

 `pathlenp`
:   Pointer to location for returned length of the string specifying the
    path with which a file was opened or created. For data on remote
    servers, this is the length of the URL string provided to open a
    connection to the data. If NULL, this argument is ignored. returned.

 `path`
:   Pointer to string large enough to contain the returned path with
    which the specified open netCDF dataset was opened or created. If
    NULL, this argument is ignored.

Errors {.heading}
------

All members of the nc\_inq family return the value NC\_NOERR if no
errors occurred. Otherwise, the returned status indicates an error.
Possible causes of errors include:

-   The specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_inq to find out about a netCDF dataset
named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status, ncid, ndims, nvars, ngatts, unlimdimid;
   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------------- ------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq-Family "Previous section in reading order")]   [[\>](#nc_005fabort "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- ------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.16 Synchronize an Open NetCDF Dataset to Disk: nc\_sync {.section}
---------------------------------------------------------

The function nc\_sync offers a way to synchronize the disk copy of a
netCDF dataset with in-memory buffers. There are two reasons you might
want to synchronize after writes:

-   To minimize data loss in case of abnormal termination, or
-   To make data available to other processes for reading immediately
    after it is written. But note that a process that already had the
    dataset open for reading would not see the number of records
    increase when the writing process calls nc\_sync; to accomplish
    this, the reading process must call nc\_sync.

This function is backward-compatible with previous versions of the
netCDF library. The intent was to allow sharing of a netCDF dataset
among multiple readers and one writer, by having the writer call
nc\_sync after writing and the readers call nc\_sync before each read.
For a writer, this flushes buffers to disk. For a reader, it makes sure
that the next read will be from disk rather than from previously cached
buffers, so that the reader will see changes made by the writing process
(e.g., the number of records written) without having to close and reopen
the dataset. If you are only accessing a small amount of data, it can be
expensive in computer resources to always synchronize to disk after
every write, since you are giving up the benefits of buffering.

An easier way to accomplish sharing (and what is now recommended) is to
have the writer and readers open the dataset with the NC\_SHARE flag,
and then it will not be necessary to call nc\_sync at all. However, the
nc\_sync function still provides finer granularity than the NC\_SHARE
flag, if only a few netCDF accesses need to be synchronized among
processes.

It is important to note that changes to the ancillary data, such as
attribute values, are not propagated automatically by use of the
NC\_SHARE flag. Use of the nc\_sync function is still required for this
purpose.

Sharing datasets when the writer enters define mode to change the data
schema requires extra care. In previous releases, after the writer left
define mode, the readers were left looking at an old copy of the
dataset, since the changes were made to a new copy. The only way readers
could see the changes was by closing and reopening the dataset. Now the
changes are made in place, but readers have no knowledge that their
internal tables are now inconsistent with the new dataset schema. If
netCDF datasets are shared across redefinition, some mechanism external
to the netCDF library must be provided that prevents access by readers
during redefinition and causes the readers to call nc\_sync before any
subsequent access.

When calling nc\_sync, the netCDF dataset must be in data mode. A netCDF
dataset in define mode is synchronized to disk only when nc\_enddef is
called. A process that is reading a netCDF dataset that another process
is writing may call nc\_sync to get updated with the changes made to the
data by the writing process (e.g., the number of records written),
without having to close and reopen the dataset.

Data is automatically synchronized to disk when a netCDF dataset is
closed, or whenever you leave define mode.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_sync(int ncid);
~~~~

 `ncid`
:   NetCDF ID, from a previous call to nc\_open or nc\_create.

Errors {.heading}
------

nc\_sync returns the value NC\_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

-   The netCDF dataset is in define mode.
-   The specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_sync to synchronize the disk writes of a
netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status;
int ncid;
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);  /* open for writing */
if (status != NC_NOERR) handle_error(status);

   ...           /* write data or change attributes */

status = nc_sync(ncid);      /* synchronize to disk */
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------- -------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fsync "Previous section in reading order")]   [[\>](#nc_005fset_005ffill "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- -------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.17 Back Out of Recent Definitions: nc\_abort {.section}
----------------------------------------------

You no longer need to call this function, since it is called
automatically by nc\_close in case the dataset is in define mode and
something goes wrong with committing the changes. The function nc\_abort
just closes the netCDF dataset, if not in define mode. If the dataset is
being created and is still in define mode, the dataset is deleted. If
define mode was entered by a call to nc\_redef, the netCDF dataset is
restored to its state before definition mode was entered and the dataset
is closed.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_abort(int ncid);
~~~~

 `ncid`
:   NetCDF ID, from a previous call to nc\_open or nc\_create.

Errors {.heading}
------

nc\_abort returns the value NC\_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

-   When called from define mode while creating a netCDF dataset,
    deletion of the dataset failed.
-   The specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_abort to back out of redefinitions of a
dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int ncid, status, latid;
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);/* open for writing */
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_redef(ncid);                    /* enter define mode */
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_def_dim(ncid, "lat", 18L, &latid);
if (status != NC_NOERR) {
   handle_error(status);
   status = nc_abort(ncid);                 /* define failed, abort */
   if (status != NC_NOERR) handle_error(status);
}
~~~~

* * * * *

  ----------------------------------------------------------- ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fabort "Previous section in reading order")]   [[\>](#nc_005fset_005fdefault_005fformat "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- ---------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.18 Set Fill Mode for Writes: nc\_set\_fill {.section}
--------------------------------------------

This function is intended for advanced usage, to optimize writes under
some circumstances described below. The function nc\_set\_fill sets the
fill mode for a netCDF dataset open for writing and returns the current
fill mode in a return parameter. The fill mode can be specified as
either NC\_FILL or NC\_NOFILL. The default behavior corresponding to
NC\_FILL is that data is pre-filled with fill values, that is fill
values are written when you create non-record variables or when you
write a value beyond data that has not yet been written. This makes it
possible to detect attempts to read data before it was written. For more
information on the use of fill values see [Fill Values](#Fill-Values).
For information about how to define your own fill values see
[(netcdf)Attribute Conventions](netcdf.html#Attribute-Conventions)
section ‘Attribute Conventions’ in NetCDF Users’ Guide.

The behavior corresponding to NC\_NOFILL overrides the default behavior
of prefilling data with fill values. This may enhance performance,
because it avoids the duplicate writes that occur when the netCDF
library writes fill values that are later overwritten with data, at the
cost of losing the ability to later detect access of values that were
never written.

A value indicating which mode the netCDF dataset was already in is
returned. You can use this value to temporarily change the fill mode of
an open netCDF dataset and then restore it to the previous mode.

After you turn on NC\_NOFILL mode for an open netCDF dataset, you must
be certain to write valid data in all the positions that will later be
read. Note that nofill mode is only a transient property of a netCDF
dataset open for writing: if you close and reopen the dataset, it will
revert to the default behavior. You can also revert to the default
behavior by calling nc\_set\_fill again to explicitly set the fill mode
to NC\_FILL.

There are three situations where it might be advantageous to set nofill
mode:

1.  Creating and initializing a netCDF dataset. In this case, you could
    set nofill mode before calling nc\_enddef and then write completely
    all non-record variables and the initial records of all the record
    variables you want to initialize.
2.  Extending an existing record-oriented netCDF dataset. Set nofill
    mode after opening the dataset for writing, then append the
    additional records to the dataset completely, leaving no intervening
    unwritten records.
3.  Adding new variables that you are going to initialize to an existing
    netCDF dataset. Set nofill mode before calling nc\_enddef then write
    all the new variables completely.

If the netCDF dataset has an unlimited dimension and the last record was
written while in nofill mode, then the dataset may be shorter than if
nofill mode was not set, but this will be completely transparent if you
access the data only through the netCDF interfaces.

The use of this feature may not be available (or even needed) in future
releases. Programmers are cautioned against heavy reliance upon this
feature.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_set_fill (int ncid, int fillmode, int *old_modep);
~~~~

 `ncid`
:   NetCDF ID, from a previous call to nc\_open or nc\_create.

 `fillmode`
:   Desired fill mode for the dataset, either NC\_NOFILL or NC\_FILL.

 `old_modep`
:   Pointer to location for returned current fill mode of the dataset
    before this call, either NC\_NOFILL or NC\_FILL.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   The specified netCDF ID does not refer to an open netCDF dataset.

 `NC_EPERM`
:   The specified netCDF ID refers to a dataset open for read-only
    access.

 `NC_EINVAL`
:   The fill mode argument is neither NC\_NOFILL nor NC\_FILL.

Example {.heading}
-------

Here is an example using nc\_set\_fill to set nofill mode for subsequent
writes of a netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int ncid, status, old_fill_mode;
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);  /* open for writing */
if (status != NC_NOERR) handle_error(status);

   ...           /* write data with default prefilling behavior */

status = nc_set_fill(ncid, NC_NOFILL, &old_fill_mode); /* set nofill */
if (status != NC_NOERR) handle_error(status);

   ...           /* write data with no prefilling */
~~~~

* * * * *

  ------------------------------------------------------------------ ------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fset_005ffill "Previous section in reading order")]   [[\>](#nc_005fset_005fchunk_005fcache "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ ------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.19 Set Default Creation Format: nc\_set\_default\_format {.section}
----------------------------------------------------------

This function is intended for advanced users.

Starting in version 3.6, netCDF introduced a new data format, the first
change in the underlying binary data format since the netCDF interface
was released. The new format, 64-bit offset format, was introduced to
greatly relax the limitations on creating very large files.

Users are warned that creating files in the 64-bit offset format makes
them unreadable by the netCDF library prior to version 3.6.0. For
reasons of compatibility, users should continue to create files in
netCDF classic format.

Users who do want to use 64-bit offset format files can create them
directory from nc\_create, using the proper cmode flag. (see section
[Create a NetCDF Dataset: nc\_create](#nc_005fcreate)).

The function nc\_set\_default\_format allows the user to change the
format of the netCDF file to be created by future calls to nc\_create
(or nc\_\_create) without changing the cmode flag.

This allows the user to convert a program to use 64-bit offset formation
without changing all calls the nc\_create. See [(netcdf)Large File
Support](netcdf.html#Large-File-Support) section ‘Large File Support’ in
The NetCDF Users Guide.

Once the default format is set, all future created files will be in the
desired format.

Two constants are provided in the netcdf.h file to be used with this
function, NC\_FORMAT\_64BIT and NC\_FORMAT\_CLASSIC.

If a non-NULL pointer is provided, it is assumed to point to an int,
where the existing default format will be written.

Using nc\_create with a cmode including NC\_64BIT\_OFFSET overrides the
default format, and creates a 64-bit offset file.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_set_default_format(int format, int *old_formatp);
~~~~

 `format`
:   Valid formats include NC\_FORMAT\_CLASSIC (the default),
    NC\_FORMAT\_64BIT, and, if –enable-netcdf-4 was used during
    configure, NC\_FORMAT\_NETCDF4 and NC\_FORMAT\_NETCDF4\_CLASSIC

 `old_formatp`
:   Either NULL (in which case it will be ignored), or a pointer to an
    int where the existing default format (i.e. before being changed to
    the new format) will be written. This allows you to get the existing
    default format while setting a new default format.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_EINVAL`
:   Invalid format. Valid formats include NC\_FORMAT\_CLASSIC,
    NC\_FORMAT\_64BIT, and, if –enable-netcdf-4 was used during
    configure, NC\_FORMAT\_NETCDF4 and NC\_FORMAT\_NETCDF4\_CLASSIC.
    Trying to set the default format to something else will result in an
    invalid argument error.

Example {.heading}
-------

Here is an example using nc\_set\_default\_format to create the same
file in four formats with the same nc\_create call (from
libsrc4/tst\_utf8.c):

 

~~~~ {.example}
#include <netcdf.h>
   ... 
      int ncid, varid, dimids[NDIMS];
      int f;

      for (f = NC_FORMAT_CLASSIC; f < NC_FORMAT_NETCDF4_CLASSIC; f++)
      {
         if (nc_set_default_format(f, NULL)) ERR;
         if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
   ... 
~~~~

* * * * *

  -------------------------------------------------------------------------------- ------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fset_005fdefault_005fformat "Previous section in reading order")]   [[\>](#nc_005fget_005fchunk_005fcache "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- ------------------------------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.20 Set HDF5 Chunk Cache for Future File Opens/Creates: nc\_set\_chunk\_cache {.section}
------------------------------------------------------------------------------

This function changes the default chunk cache settings in the HDF5
library for all variables in the file. The settings apply for subsequent
file opens/creates. This function does not change the chunk cache
settings of already open files.

For more information, see the documentation for the H5Pset\_cache()
function in the HDF5 library at the HDF5 website:
[http://hdfgroup.org/HDF5/](http://hdfgroup.org/HDF5/).

Usage {.heading}
-----

 

~~~~ {.example}
int nc_set_chunk_cache(size_t size, size_t nelems, float preemption);
~~~~

 `size`
:   The total size of the raw data chunk cache, in bytes. This should be
    big enough to hold multiple chunks of data.

 `nelems`
:   The number of chunk slots in the raw data chunk cache hash table.
    This should be a prime number larger than the number of chunks that
    will be in the cache.

 `preemption`
:   The preemtion value must be between 0 and 1 inclusive and indicates
    how much chunks that have been fully read are favored for
    preemption. A value of zero means fully read chunks are treated no
    differently than other chunks (the preemption is strictly LRU) while
    a value of one means fully read chunks are always preempted before
    other chunks.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_EINVAL`
:   Preemption must be between zero and one (inclusive).

Example {.heading}
-------

This example is from libsrc4/tst\_files.c:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define NEW_CACHE_SIZE 32000000
#define NEW_CACHE_NELEMS 2000
#define NEW_CACHE_PREEMPTION .75

   /* Change chunk cache. */
   if (nc_set_chunk_cache(NEW_CACHE_SIZE, NEW_CACHE_NELEMS, 
              NEW_CACHE_PREEMPTION)) ERR;

   /* Create a file with two dims, two vars, and two atts. */
   if (nc_create(FILE_NAME, cflags|NC_CLOBBER, &ncid)) ERR;

   ... 
~~~~

* * * * *

  ----------------------------------------------------------------------------- ------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fset_005fchunk_005fcache "Previous section in reading order")]   [[\>](#Groups "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Datasets "Up section")]   [[\>\>](#Groups "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------- ------------------------------------------------- --- --------------------------------------------------------------------- -------------------------------- ---------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

2.21 Get the HDF5 Chunk Cache Settings for Future File Opens/Creates: nc\_get\_chunk\_cache {.section}
-------------------------------------------------------------------------------------------

This function gets the chunk cache settings for the HDF5 library. The
settings apply for subsequent file opens/creates.

This affects the per-file chunk cache which the HDF5 layer maintains.
The chunk cache size can be tuned for better performance.

For more information, see the documentation for the H5Pget\_cache()
function in the HDF5 library at the HDF5 website:
[http://hdfgroup.org/HDF5/](http://hdfgroup.org/HDF5/).

Usage {.heading}
-----

 

~~~~ {.example}
int nc_get_chunk_cache(size_t *sizep, size_t *nelemsp, float *preemptionp);
~~~~

 `sizep`
:   The total size of the raw data chunk cache will be put here. If
    NULL, will be ignored.

 `nelemsp`
:   The number of chunk slots in the raw data chunk cache hash table
    will be put here. If NULL, will be ignored.

 `preemptionp`
:   The preemption will be put here. The preemtion value is between 0
    and 1 inclusive and indicates how much chunks that have been fully
    read are favored for preemption. A value of zero means fully read
    chunks are treated no differently than other chunks (the preemption
    is strictly LRU) while a value of one means fully read chunks are
    always preempted before other chunks. If NULL, will be ignored.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

Example {.heading}
-------

This example is from libsrc4/tst\_files.c:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
   /* Retrieve the chunk cache settings, just for fun. */
   if (nc_get_chunk_cache(&cache_size_in, &cache_nelems_in, 
              &cache_preemption_in)) ERR;
   if (cache_size_in != NEW_CACHE_SIZE || cache_nelems_in != NEW_CACHE_NELEMS ||
       cache_preemption_in != NEW_CACHE_PREEMPTION) ERR;

   ... 
~~~~

* * * * *

  ----------------------------------------------------------------------------- -------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fget_005fchunk_005fcache "Previous section in reading order")]   [[\>](#nc_005finq_005fncid "Next section in reading order")]       [[\<\<](#Datasets "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------- -------------------------------------------------------------- --- --------------------------------------------------------------------- --------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3. Groups {.chapter}
=========

NetCDF-4 added support for hierarchical groups within netCDF datasets.

Groups are identified with a ncid, which identifies both the open file,
and the group within that file. When a file is opened with nc\_open or
nc\_create, the ncid for the root group of that file is provided. Using
that as a starting point, users can add new groups, or list and navigate
existing groups.

All netCDF calls take a ncid which determines where the call will take
its action. For example, the nc\_def\_var function takes a ncid as its
first parameter. It will create a variable in whichever group its ncid
refers to. Use the root ncid provided by nc\_create or nc\_open to
create a variable in the root group. Or use nc\_def\_grp to create a
group and use its ncid to define a variable in the new group.

Variable are only visible in the group in which they are defined. The
same applies to attributes. “Global” attributes are associated with the
group whose ncid is used.

Dimensions are visible in their groups, and all child groups.

Group operations are only permitted on netCDF-4 files - that is, files
created with the HDF5 flag in nc\_create. (see section [Create a NetCDF
Dataset: nc\_create](#nc_005fcreate)). Groups are not compatible with
the netCDF classic data model, so files created with the
NC\_CLASSIC\_MODEL file cannot contain groups (except the root group).

  ------------------------------------------------------------------------------------------------------------------ ---- --
  [3.1 Find a Group ID: nc\_inq\_ncid](#nc_005finq_005fncid)                                                              
  [3.2 Get a List of Groups in a Group: nc\_inq\_grps](#nc_005finq_005fgrps)                                              
  [3.3 Find all the Variables in a Group: nc\_inq\_varids](#nc_005finq_005fvarids)                                        
  [3.4 Find all Dimensions Visible in a Group: nc\_inq\_dimids](#nc_005finq_005fdimids)                                   
  [3.5 Find a Group’s Name: nc\_inq\_grpname](#nc_005finq_005fgrpname)                                                    
  [3.6 Find a Group’s Full Name: nc\_inq\_grpname\_full](#nc_005finq_005fgrpname_005ffull)                                
  [3.7 Find the Length of a Group’s Full Name: nc\_inq\_grpname\_len](#nc_005finq_005fgrpname_005flen)                    
  [3.8 Find a Group’s Parent: nc\_inq\_grp\_parent](#nc_005finq_005fgrp_005fparent)                                       
  [3.9 Find a Group by Name: nc\_inq\_grp\_ncid](#nc_005finq_005fgrp_005fncid)                                            
  [3.10 Find a Group by its Fully-qualified Name: nc\_inq\_grp\_full\_ncid](#nc_005finq_005fgrp_005ffull_005fncid)        
  [3.11 Create a New Group: nc\_def\_grp](#nc_005fdef_005fgrp)                                                            
  ------------------------------------------------------------------------------------------------------------------ ---- --

* * * * *

  ----------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Groups "Previous section in reading order")]   [[\>](#nc_005finq_005fgrps "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.1 Find a Group ID: nc\_inq\_ncid {.section}
----------------------------------

Given an ncid and group name (NULL or "" gets root group), return ncid
of the named group.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_ncid(int ncid, const char *name, int *grp_ncid);
~~~~

 `ncid`
:   The group id for this operation.

 `name`
:   A char array that holds the name of the desired group.

 `grp_ncid`
:   An int pointer that will receive the group id, if the group is
    found.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

 

~~~~ {.example}
   int root_ncid, child_ncid;
   char file[] = "nc4_test.nc";

   /* Open the file. */
   if ((res = nc_open(file, NC_NOWRITE, &root_ncid)))
      return res;

   /* Get the ncid of an existing group. */
   if ((res = nc_inq_ncid(root_ncid, "group1", &child_ncid)))
      return res;
~~~~

* * * * *

  ------------------------------------------------------------------ ---------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fncid "Previous section in reading order")]   [[\>](#nc_005finq_005fvarids "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ ---------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.2 Get a List of Groups in a Group: nc\_inq\_grps {.section}
--------------------------------------------------

Given a location id, return the number of groups it contains, and an
array of their ncids.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_grps(int ncid, int *numgrps, int *ncids);
~~~~

 `ncid`
:   The group id for this operation.

 `numgrps`
:   Pointer to an int which will get number of groups in this group. If
    NULL, it’s ignored.

 `ncids`
:   Pointer to a already allocated array of ints which will receive the
    ids of all the groups in this group. If NULL, it’s ignored. Call
    this function with NULL for ncids parameter to find out how many
    groups there are.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

 

~~~~ {.example}
   int root_ncid, numgrps;
   int *ncids;
   char file[] = "nc4_test.nc";

   /* Open the file. */
   if ((res = nc_open(file, NC_NOWRITE, &root_ncid)))
      return res;

   /* Get a list of ncids for the root group. (That is, find out of
      there are any groups already defined. */
   if ((res = nc_inq_grps(root_ncid, &numgrps, NULL)))
      return res;
   ncids = malloc(sizeof(int) * numgrps);
   if ((res = nc_inq_grps(root_ncid, NULL, ncids)))
      return res;
~~~~

* * * * *

  ------------------------------------------------------------------ ---------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fgrps "Previous section in reading order")]   [[\>](#nc_005finq_005fdimids "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ ---------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.3 Find all the Variables in a Group: nc\_inq\_varids {.section}
------------------------------------------------------

Find all varids for a location.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_varids(int ncid, int *nvars, int *varids);
~~~~

 `ncid`
:   The group id for this operation.

 `nvars`
:   The integer pointed to by this parameter will get the number of
    variable IDs found.

 `varids`
:   An already allocated array to store the list of varids. Ignored if
    NULL.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from libsrc4/tst\_vars.c.

 

~~~~ {.example}
      int nvars_in, varids_in[2];

      ...

      /* Open the file and make sure nc_inq_varids yeilds correct
       * result. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------- ----------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvarids "Previous section in reading order")]   [[\>](#nc_005finq_005fgrpname "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ----------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.4 Find all Dimensions Visible in a Group: nc\_inq\_dimids {.section}
-----------------------------------------------------------

Find all dimids for a location. This finds all dimensions in a group, or
any of its parents.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_dimids(int ncid, int *ndims, int *dimids, int include_parents);
~~~~

 `ncid`
:   The group id for this operation.

 `ndims`
:   If non-zero, pointer to location for returned number of dimensions
    defined for this location, including dimensions inherited from
    parent groups if include\_parents is non-zero.

 `dimids`
:   An already allocated array of ints when the dimids of the visible
    dimensions will be stashed. Use nc\_inq\_ndims to find out how many
    dims are visible from this group. (see section [Inquire about an
    Open NetCDF Dataset: nc\_inq Family](#nc_005finq-Family)).

 `include_parents`
:   If non-zero then all the dimensions in all parent groups will also
    be retrieved.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from libsrc4/tst\_dims.c.

 

~~~~ {.example}
      int ncid, dimid;
      int ndims_in, dimids_in[MAX_DIMS];

      ...

      /* Open the file and make sure nc_inq_dimids yeilds correct
       * result. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------- -------------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fdimids "Previous section in reading order")]   [[\>](#nc_005finq_005fgrpname_005ffull "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- -------------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.5 Find a Group’s Name: nc\_inq\_grpname {.section}
-----------------------------------------

Given a group ID find its name. (Root group is named "/"). See section
[Find a Group’s Full Name:
nc\_inq\_grpname\_full](#nc_005finq_005fgrpname_005ffull).

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_grpname(int ncid, char *name);
~~~~

`ncid`

The group id for this operation.

`name`

Pointer to allocated space of correct length. The name of the group will
be copied there. The name will be less than NC\_MAX\_NAME, not including
a terminating NULL byte.

``

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from libsrc4/tst\_grps.c.

 

~~~~ {.example}
      int grpid_in[MAX_SIBLING_GROUPS];
      char name_in[NC_MAX_NAME + 1];

      ...

      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
~~~~

* * * * *

  --------------------------------------------------------------------- ------------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fgrpname "Previous section in reading order")]   [[\>](#nc_005finq_005fgrpname_005flen "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ------------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.6 Find a Group’s Full Name: nc\_inq\_grpname\_full {.section}
----------------------------------------------------

Given ncid, find complete name of group. (Root group is named "/", a
full "path" for each group is provided in the name, with groups
separated with a forward slash / as in Unix directory names. For example
"/group1/subgrp1/subsubgrp1")

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_grpname_full(int ncid, size_t *lenp, char *full_name);
~~~~

`ncid`

The group id for this operation.

`lenp`

Pointer to a size\_t which will get the length. (Note that this does not
include the NULL terminator byte.)

`full_name`

Pointer to allocated space of correct length. The name of the group will
be copied there. To find the required length, call nc\_inq\_grpname\_len
call this function with NULL for the full\_name parameter, or call (see
section [Find the Length of a Group’s Full Name:
nc\_inq\_grpname\_len](#nc_005finq_005fgrpname_005flen)).

``

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from the test program libsrc4/tst\_grps.c.

 

~~~~ {.example}
      int grpid_in[MAX_SIBLING_GROUPS];
      char full_name_in[NC_MAX_NAME * 10]; 
      size_t len;

      ...

      if (nc_inq_grpname_full(grpid_in[0], &len, full_name_in)) ERR;
~~~~

* * * * *

  ------------------------------------------------------------------------------ ------------------------------------------------------------------------ --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fgrpname_005ffull "Previous section in reading order")]   [[\>](#nc_005finq_005fgrp_005fparent "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------ ------------------------------------------------------------------------ --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.7 Find the Length of a Group’s Full Name: nc\_inq\_grpname\_len {.section}
-----------------------------------------------------------------

Given ncid, find len of the full name, as returned by
nc\_inq\_grpname\_full See section [Find a Group’s Full Name:
nc\_inq\_grpname\_full](#nc_005finq_005fgrpname_005ffull). (Root group
is named "/", with length 1.)

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_grpname_len(int ncid, size_t *lenp);
~~~~

 `ncid`
:   The group id for this operation.

 `lenp`
:   Pointer to an int where the length will be placed. Length does not
    include NULL terminator.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from libsrc4/tst\_grps.c.

 

~~~~ {.example}
      int ncid;
      size_t len;

      ...

      if (nc_inq_grpname_len(ncid, &len)) ERR;
~~~~

* * * * *

  ----------------------------------------------------------------------------- ---------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fgrpname_005flen "Previous section in reading order")]   [[\>](#nc_005finq_005fgrp_005fncid "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------- ---------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.8 Find a Group’s Parent: nc\_inq\_grp\_parent {.section}
-----------------------------------------------

Given ncid, find the ncid of the parent group.

When used with the root group, this function returns the NC\_ENOGRP
error (since the root group has no parent.)

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_grp_parent(int ncid, int *parent_ncid);
~~~~

`ncid`

The group id.

`parent_ncid`

Pointer to an int. The ncid of the group will be copied there.

``

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOGRP`
:   No parent group found (i.e. this is the root group).

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

 

~~~~ {.example}
   if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
   if (nc_def_grp(ncid, HENRY_VII, &henry_vii_id)) ERR;

   if (nc_inq_grp_parent(henry_vii_id, &parent_ncid)) ERR;
   if (parent_ncid != ncid) ERR;
   if (nc_close(ncid)) ERR;
~~~~

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fgrp_005fparent "Previous section in reading order")]   [[\>](#nc_005finq_005fgrp_005ffull_005fncid "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.9 Find a Group by Name: nc\_inq\_grp\_ncid {.section}
--------------------------------------------

Given a group location id and the name of an immediate subgroup, find
the location id of the named subgroup.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_grp_ncid(int ncid, const char *grp_name, int *grp_ncid);
~~~~

 `ncid`
:   The location id of the group in which to look.

 `grp_name`
:   The name of a subgroup.

 `grp_ncid`
:   A pointer to int which will get the group id of the named subgroup,
    if it is found.

Return Codes {.heading}
------------

The following return codes may be returned by this function.

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_EINVAL`
:   No name provided or name longer than NC\_MAX\_NAME.

 `NC_ENOGRP`
:   Named subgroup not found.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from test program libsrc4/tst\_grps.c.

 

~~~~ {.example}
      /* Reopen and recheck. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      
      if (nc_inq_grp_ncid(ncid, SCI_FI, &g1id)) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------------- ------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fgrp_005fncid "Previous section in reading order")]   [[\>](#nc_005fdef_005fgrp "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.10 Find a Group by its Fully-qualified Name: nc\_inq\_grp\_full\_ncid {.section}
-----------------------------------------------------------------------

Given a fully qualified group name and an ncid, find the ncid of the
group id.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_grp_full_ncid(int ncid, char *full_name, int *grp_ncid);
~~~~

 `ncid`
:   The group id to look in.

 `full_name`
:   The fully-qualified group name.

 `grp_ncid`
:   A pointer to int which will get the group id, if it is found.

Return Codes {.heading}
------------

The following return codes may be returned by this function.

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_EINVAL`
:   No name provided or name longer than NC\_MAX\_NAME.

 `NC_ENOGRP`
:   Named group not found.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from test program libsrc4/tst\_grps.c.

 

~~~~ {.example}
      /* Reopen and recheck. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      
      if (nc_inq_grp_ncid(ncid, SCI_FI, &g1id)) ERR;
~~~~

* * * * *

  ----------------------------------------------------------------------------------- ----------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fgrp_005ffull_005fncid "Previous section in reading order")]   [[\>](#Dimensions "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Groups "Up section")]   [[\>\>](#Dimensions "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- ----------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

3.11 Create a New Group: nc\_def\_grp {.section}
-------------------------------------

Create a group. Its location id is returned in the new\_ncid pointer.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_def_grp(int parent_ncid, const char *name, int *new_ncid);
~~~~

 `parent_ncid`
:   The group id of the parent group.

 `name`
:   The name of the new group. A group cannot have the same name as a
    variable within the same parent group.

 `new_ncid`
:   A pointer to an int. The ncid of the new group will be placed there.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENAMEINUSE`
:   That name is in use. Group names must be unique within a group, and
    must not be the same as any variable or type in the group.

 `NC_EMAXNAME`
:   Name exceed max length NC\_MAX\_NAME.

 `NC_EBADNAME`
:   Name contains illegal characters.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

 `NC_EPERM`
:   Attempt to write to a read-only file.

 `NC_ENOTINDEFINE`
:   Not in define mode.

Example {.heading}
-------

 

~~~~ {.example}
      int ncid, a1_ncid;
      char grpname[] = "assimilation1";

      /* Create a group. */
      if ((res = nc_def_grp(ncid, grpname, &a1_ncid)))
         return res;
~~~~

* * * * *

  ----------------------------------------------------------------- ------------------------------------------------------------------ --- ------------------------------------------------------------------- --------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fgrp "Previous section in reading order")]   [[\>](#Dimensions-Introduction "Next section in reading order")]       [[\<\<](#Groups "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#User-Defined-Data-Types "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- ------------------------------------------------------------------ --- ------------------------------------------------------------------- --------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4. Dimensions {.chapter}
=============

  --------------------------------------------------------------------------------------- ---- ----------------------------------
  [4.1 Dimensions Introduction](#Dimensions-Introduction)                                      
  [4.2 Create a Dimension: nc\_def\_dim](#nc_005fdef_005fdim)                                  Create a Dimension
  [4.3 Get a Dimension ID from Its Name: nc\_inq\_dimid](#nc_005finq_005fdimid)                Get a Dimension ID from Its Name
  [4.4 Inquire about a Dimension: nc\_inq\_dim Family](#nc_005finq_005fdim-Family)             Inquire about a Dimension
  [4.5 Rename a Dimension: nc\_rename\_dim](#nc_005frename_005fdim)                            Rename a Dimension
  [4.6 Find All Unlimited Dimension IDs: nc\_inq\_unlimdims](#nc_005finq_005funlimdims)        
  --------------------------------------------------------------------------------------- ---- ----------------------------------

* * * * *

  --------------------------------------------------------- ------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Dimensions "Previous section in reading order")]   [[\>](#nc_005fdef_005fdim "Next section in reading order")]       [[\<\<](#Dimensions "Beginning of this chapter or previous chapter")]   [[Up](#Dimensions "Up section")]   [[\>\>](#User-Defined-Data-Types "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------- ------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.1 Dimensions Introduction {.section}
---------------------------

Dimensions for a netCDF dataset are defined when it is created, while
the netCDF dataset is in define mode. Additional dimensions may be added
later by reentering define mode. A netCDF dimension has a name and a
length. In a netCDF classic or 64-bit offset file, at most one dimension
can have the unlimited length, which means variables using this
dimension can grow along this dimension. In a netCDF-4 file multiple
unlimited dimensions are supported.

There is a suggested limit (100) to the number of dimensions that can be
defined in a single netCDF dataset. The limit is the value of the
predefined macro NC\_MAX\_DIMS. The purpose of the limit is to make
writing generic applications simpler. They need only provide an array of
NC\_MAX\_DIMS dimensions to handle any netCDF dataset. The
implementation of the netCDF library does not enforce this advisory
maximum, so it is possible to use more dimensions, if necessary, but
netCDF utilities that assume the advisory maximums may not be able to
handle the resulting netCDF datasets.

Ordinarily, the name and length of a dimension are fixed when the
dimension is first defined. The name may be changed later, but the
length of a dimension (other than the unlimited dimension) cannot be
changed without copying all the data to a new netCDF dataset with a
redefined dimension length.

Dimension lengths in the C interface are type size\_t rather than type
int to make it possible to access all the data in a netCDF dataset on a
platform that only supports a 16-bit int data type, for example MSDOS.
If dimension lengths were type int instead, it would not be possible to
access data from variables with a dimension length greater than a 16-bit
int can accommodate.

A netCDF dimension in an open netCDF dataset is referred to by a small
integer called a dimension ID. In the C interface, dimension IDs are 0,
1, 2, ..., in the order in which the dimensions were defined.

Operations supported on dimensions are:

-   Create a dimension, given its name and length.
-   Get a dimension ID from its name.
-   Get a dimension’s name and length from its ID.
-   Rename a dimension.

* * * * *

  ---------------------------------------------------------------------- --------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Dimensions-Introduction "Previous section in reading order")]   [[\>](#nc_005finq_005fdimid "Next section in reading order")]       [[\<\<](#Dimensions "Beginning of this chapter or previous chapter")]   [[Up](#Dimensions "Up section")]   [[\>\>](#User-Defined-Data-Types "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- --------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.2 Create a Dimension: nc\_def\_dim {.section}
------------------------------------

The function nc\_def\_dim adds a new dimension to an open netCDF dataset
in define mode. It returns (as an argument) a dimension ID, given the
netCDF ID, the dimension name, and the dimension length. At most one
unlimited length dimension, called the record dimension, may be defined
for each classic or 64-bit offset netCDF dataset. NetCDF-4 datasets may
have multiple unlimited dimensions.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_def_dim (int ncid, const char *name, size_t len, int *dimidp);
~~~~

 `ncid`
:   NetCDF group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, etc.

 `name`
:   Dimension name.

 `len`
:   Length of dimension; that is, number of values for this dimension as
    an index to variables that use it. This should be either a positive
    integer (of type size\_t) or the predefined constant NC\_UNLIMITED.

 `dimidp`
:   Pointer to location for returned dimension ID.

Errors {.heading}
------

nc\_def\_dim returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The netCDF dataset is not in definition mode.
-   The specified dimension name is the name of another existing
    dimension.
-   The specified length is not greater than zero.
-   The specified length is unlimited, but there is already an unlimited
    length dimension defined for this netCDF dataset.
-   The specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_def\_dim to create a dimension named lat of
length 18 and a unlimited dimension named rec in a new netCDF dataset
named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status, ncid, latid, recid;
   ... 
status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_def_dim(ncid, "lat", 18L, &latid);
if (status != NC_NOERR) handle_error(status);
status = nc_def_dim(ncid, "rec", NC_UNLIMITED, &recid);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ----------------------------------------------------------------- -------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fdim "Previous section in reading order")]   [[\>](#nc_005finq_005fdim-Family "Next section in reading order")]       [[\<\<](#Dimensions "Beginning of this chapter or previous chapter")]   [[Up](#Dimensions "Up section")]   [[\>\>](#User-Defined-Data-Types "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- -------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.3 Get a Dimension ID from Its Name: nc\_inq\_dimid {.section}
----------------------------------------------------

The function nc\_inq\_dimid returns (as an argument) the ID of a netCDF
dimension, given the name of the dimension. If ndims is the number of
dimensions defined for a netCDF dataset, each dimension has an ID
between 0 and ndims-1.

Usage {.heading}
-----

When searching for a dimension, the specified group is searched, and
then its parent group, and then its grandparent group, etc., up to the
root group.

 

~~~~ {.example}
int nc_inq_dimid (int ncid, const char *name, int *dimidp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `name`
:   Dimension name.

 `dimidp`
:   Pointer to location for the returned dimension ID.

Errors {.heading}
------

nc\_inq\_dimid returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

The name that was specified is not the name of a dimension in the netCDF
dataset. The specified netCDF ID does not refer to an open netCDF
dataset.

Example {.heading}
-------

Here is an example using nc\_inq\_dimid to determine the dimension ID of
a dimension named lat, assumed to have been defined previously in an
existing netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status, ncid, latid;
   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);  /* open for reading */
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_dimid(ncid, "lat", &latid);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ------------------------------------------------------------------- ---------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fdimid "Previous section in reading order")]   [[\>](#nc_005frename_005fdim "Next section in reading order")]       [[\<\<](#Dimensions "Beginning of this chapter or previous chapter")]   [[Up](#Dimensions "Up section")]   [[\>\>](#User-Defined-Data-Types "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------- ---------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.4 Inquire about a Dimension: nc\_inq\_dim Family {.section}
--------------------------------------------------

This family of functions returns information about a netCDF dimension.
Information about a dimension includes its name and its length. The
length for the unlimited dimension, if any, is the number of records
written so far.

The functions in this family include nc\_inq\_dim, nc\_inq\_dimname, and
nc\_inq\_dimlen. The function nc\_inq\_dim returns all the information
about a dimension; the other functions each return just one item of
information.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_dim     (int ncid, int dimid, char* name, size_t* lengthp);
int nc_inq_dimname (int ncid, int dimid, char *name);
int nc_inq_dimlen  (int ncid, int dimid, size_t *lengthp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `dimid`
:   Dimension ID, from a previous call to nc\_inq\_dimid or
    nc\_def\_dim.

 `name`
:   Returned dimension name. The caller must allocate space for the
    returned name. The maximum possible length, in characters, of a
    dimension name is given by the predefined constant NC\_MAX\_NAME.
    (This doesn’t include the null terminator, so declare your array to
    be size NC\_MAX\_NAME+1). The returned character array will be
    null-terminated.

 `lengthp`
:   Pointer to location for returned length of dimension. For the
    unlimited dimension, this is the number of records written so far.

Errors {.heading}
------

These functions return the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The dimension ID is invalid for the specified netCDF dataset.
-   The specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_inq\_dim to determine the length of a
dimension named lat, and the name and current maximum length of the
unlimited dimension for an existing netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status, ncid, latid, recid;
size_t latlength, recs;
char recname[NC_MAX_NAME+1];
   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);  /* open for reading */
if (status != NC_NOERR) handle_error(status);
status = nc_inq_unlimdim(ncid, &recid); /* get ID of unlimited dimension */
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_dimid(ncid, "lat", &latid);  /* get ID for lat dimension */
if (status != NC_NOERR) handle_error(status);
status = nc_inq_dimlen(ncid, latid, &latlength); /* get lat length */
if (status != NC_NOERR) handle_error(status);
/* get unlimited dimension name and current length */
status = nc_inq_dim(ncid, recid, recname, &recs);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ------------------------------------------------------------------------ ------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fdim-Family "Previous section in reading order")]   [[\>](#nc_005finq_005funlimdims "Next section in reading order")]       [[\<\<](#Dimensions "Beginning of this chapter or previous chapter")]   [[Up](#Dimensions "Up section")]   [[\>\>](#User-Defined-Data-Types "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ ------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.5 Rename a Dimension: nc\_rename\_dim {.section}
---------------------------------------

The function nc\_rename\_dim renames an existing dimension in a netCDF
dataset open for writing. You cannot rename a dimension to have the same
name as another dimension.

For netCDF classic and 64-bit offset files, if the new name is longer
than the old name, the netCDF dataset must be in define mode.

For netCDF-4 files the dataset is switched to define more for the
rename, regardless of the name length.

Usage {.heading}
-----

int nc\_rename\_dim(int ncid, int dimid, const char\* name);

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `dimid`
:   Dimension ID, from a previous call to nc\_inq\_dimid or
    nc\_def\_dim.

 `name`
:   New dimension name.

Errors {.heading}
------

nc\_rename\_dim returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The new name is the name of another dimension.
-   The dimension ID is invalid for the specified netCDF dataset.
-   The specified netCDF ID does not refer to an open netCDF dataset.
-   The new name is longer than the old name and the netCDF dataset is
    not in define mode.

Example {.heading}
-------

Here is an example using nc\_rename\_dim to rename the dimension lat to
latitude in an existing netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int status, ncid, latid;
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);  /* open for writing */
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_redef(ncid);  /* put in define mode to rename dimension */
if (status != NC_NOERR) handle_error(status);
status = nc_inq_dimid(ncid, "lat", &latid);
if (status != NC_NOERR) handle_error(status);
status = nc_rename_dim(ncid, latid, "latitude");
if (status != NC_NOERR) handle_error(status);
status = nc_enddef(ncid); /* leave define mode */
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  -------------------------------------------------------------------- ------------------------------------------------------------------ --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005frename_005fdim "Previous section in reading order")]   [[\>](#User-Defined-Data-Types "Next section in reading order")]       [[\<\<](#Dimensions "Beginning of this chapter or previous chapter")]   [[Up](#Dimensions "Up section")]   [[\>\>](#User-Defined-Data-Types "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ------------------------------------------------------------------ --- ----------------------------------------------------------------------- ---------------------------------- --------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

4.6 Find All Unlimited Dimension IDs: nc\_inq\_unlimdims {.section}
--------------------------------------------------------

In netCDF-4 files, it’s possible to have multiple unlimited dimensions.
This function returns a list of the unlimited dimension ids visible in a
group.

Dimensions are visible in a group if they have been defined in that
group, or any ancestor group.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_unlimdims(int ncid, int *nunlimdimsp, int *unlimdimidsp);
~~~~

 `ncid`
:   NetCDF group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, etc.

 `nunlimdimsp`
:   A pointer to an int which will get the number of visible unlimited
    dimensions. Ignored if NULL.

 `unlimdimidsp`
:   A pointer to an already allocated array of int which will get the
    ids of all visible unlimited dimensions. Ignored if NULL. To
    allocate the correct length for this array, call nc\_inq\_unlimdims
    with a NULL for this parameter and use the nunlimdimsp parameter to
    get the number of visible unlimited dimensions.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag HDF5. (see section [Open a NetCDF Dataset for
    Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

 

~~~~ {.example}
   int root_ncid, num_unlimdims, unlimdims[NC_MAX_DIMS];
   char file[] = "nc4_test.nc";
   int res;

   /* Open the file. */
   if ((res = nc_open(file, NC_NOWRITE, &root_ncid)))
      return res;

   /* Find out if there are any unlimited dimensions in the root
      group. */
   if ((res = nc_inq_unlimdims(root_ncid, &num_unlimdims, unlimdims)))
      return res;

   printf("nc_inq_unlimdims reports %d unlimited dimensions\n", num_unlimdims);
~~~~

* * * * *

  ----------------------------------------------------------------------- ------------------------------------------------------------- --- ----------------------------------------------------------------------- --------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005funlimdims "Previous section in reading order")]   [[\>](#User-Defined-Types "Next section in reading order")]       [[\<\<](#Dimensions "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------- ------------------------------------------------------------- --- ----------------------------------------------------------------------- --------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5. User Defined Data Types {.chapter}
==========================

  -------------------------------------------------------------------------------------------------------------------------------------------- ---- --
  [5.1 User Defined Types Introduction](#User-Defined-Types)                                                                                        
  [5.2 Learn the IDs of All Types in Group: nc\_inq\_typeids](#nc_005finq_005ftypeids)                                                              
  [5.3 Find a Typeid from Group and Name: nc\_inq\_typeid](#nc_005finq_005ftypeid)                                                                  
  [5.4 Learn About a User Defined Type: nc\_inq\_type](#nc_005finq_005ftype)                                                                        
  [5.5 Learn About a User Defined Type: nc\_inq\_user\_type](#nc_005finq_005fuser_005ftype)                                                         
  [5.6 Compound Types Introduction](#Compound-Types)                                                                                                
  [5.7 Creating a Compound Type: nc\_def\_compound](#nc_005fdef_005fcompound)                                                                       
  [5.8 Inserting a Field into a Compound Type: nc\_insert\_compound](#nc_005finsert_005fcompound)                                                   
  [5.9 Inserting an Array Field into a Compound Type: nc\_insert\_array\_compound](#nc_005finsert_005farray_005fcompound)                           
  [5.10 Learn About a Compound Type: nc\_inq\_compound](#nc_005finq_005fcompound)                                                                   
  [5.11 Learn the Name of a Compound Type: nc\_inq\_compound\_name](#nc_005finq_005fcompound_005fname)                                              
  [5.12 Learn the Size of a Compound Type: nc\_inq\_compound\_size](#nc_005finq_005fcompound_005fsize)                                              
  [5.13 Learn the Number of Fields in a Compound Type: nc\_inq\_compound\_nfields](#nc_005finq_005fcompound_005fnfields)                            
  [5.14 Learn About a Field of a Compound Type: nc\_inq\_compound\_field](#nc_005finq_005fcompound_005ffield)                                       
  [5.15 Find the Name of a Field in a Compound Type: nc\_inq\_compound\_fieldname](#nc_005finq_005fcompound_005ffieldname)                          
  [5.16 Get the FieldID of a Compound Type Field: nc\_inq\_compound\_fieldindex](#nc_005finq_005fcompound_005ffieldindex)                           
  [5.17 Get the Offset of a Field: nc\_inq\_compound\_fieldoffset](#nc_005finq_005fcompound_005ffieldoffset)                                        
  [5.18 Find the Type of a Field: nc\_inq\_compound\_fieldtype](#nc_005finq_005fcompound_005ffieldtype)                                             
  [5.19 Find the Number of Dimensions in an Array Field: nc\_inq\_compound\_fieldndims](#nc_005finq_005fcompound_005ffieldndims)                    
  [5.20 Find the Sizes of Dimensions in an Array Field: nc\_inq\_compound\_fielddim\_sizes](#nc_005finq_005fcompound_005ffielddim_005fsizes)        
  [5.21 Variable Length Array Introduction](#Variable-Length-Array)                                                                                 
  [5.22 Define a Variable Length Array (VLEN): nc\_def\_vlen](#nc_005fdef_005fvlen)                                                                 
  [5.23 Learning about a Variable Length Array (VLEN) Type: nc\_inq\_vlen](#nc_005finq_005fvlen)                                                    
  [5.24 Releasing Memory for a Variable Length Array (VLEN) Type: nc\_free\_vlen](#nc_005ffree_005fvlen)                                            
  [5.25 Releasing Memory for an Array of Variable Length Array (VLEN) Type: nc\_free\_vlen](#nc_005ffree_005fvlens)                                 
  [5.26 Opaque Type Introduction](#Opaque-Type)                                                                                                     
  [5.27 Creating Opaque Types: nc\_def\_opaque](#nc_005fdef_005fopaque)                                                                             
  [5.28 Learn About an Opaque Type: nc\_inq\_opaque](#nc_005finq_005fopaque)                                                                        
  [5.29 Enum Type Introduction](#Enum-Type)                                                                                                         
  [5.30 Creating a Enum Type: nc\_def\_enum](#nc_005fdef_005fenum)                                                                                  
  [5.31 Inserting a Field into a Enum Type: nc\_insert\_enum](#nc_005finsert_005fenum)                                                              
  [5.32 Learn About a Enum Type: nc\_inq\_enum](#nc_005finq_005fenum)                                                                               
  [5.33 Learn the Name of a Enum Type: nc\_inq\_enum\_member](#nc_005finq_005fenum_005fmember)                                                      
  [5.34 Learn the Name of a Enum Type: nc\_inq\_enum\_ident](#nc_005finq_005fenum_005fident)                                                        
  -------------------------------------------------------------------------------------------------------------------------------------------- ---- --

* * * * *

  ---------------------------------------------------------------------- ----------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#User-Defined-Data-Types "Previous section in reading order")]   [[\>](#nc_005finq_005ftypeids "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- ----------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.1 User Defined Types Introduction {.section}
-----------------------------------

NetCDF-4 has added support for four different user defined data types.
User defined type may only be used in files created with the NC\_NETCDF4
and without NC\_CLASSIC\_MODEL.

 `compound type`
:   Like a C struct, a compound type is a collection of types, including
    other user defined types, in one package.

 `variable length array type`
:   The variable length array may be used to store ragged arrays.

 `opaque type`
:   This type has only a size per element, and no other type
    information.

 `enum type`
:   Like an enumeration in C, this type lets you assign text values to
    integer values, and store the integer values.

Users may construct user defined type with the various nc\_def\_\*
functions described in this section. They may learn about user defined
types by using the nc\_inq\_ functions defined in this section.

Once types are constructed, define variables of the new type with
nc\_def\_var (see section [Create a Variable:
`nc_def_var`](#nc_005fdef_005fvar)). Write to them with nc\_put\_var1,
nc\_put\_var, nc\_put\_vara, or nc\_put\_vars (see section
[Variables](#Variables)). Read data of user-defined type with
nc\_get\_var1, nc\_get\_var, nc\_get\_vara, or nc\_get\_vars (see
section [Variables](#Variables)).

Create attributes of the new type with nc\_put\_att (see section [Create
an Attribute: nc\_put\_att\_ type](#nc_005fput_005fatt_005f-type)). Read
attributes of the new type with nc\_get\_att (see section [Get
Attribute’s Values:nc\_get\_att\_ type](#nc_005fget_005fatt_005f-type)).

* * * * *

  ----------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#User-Defined-Types "Previous section in reading order")]   [[\>](#nc_005finq_005ftypeid "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.2 Learn the IDs of All Types in Group: nc\_inq\_typeids {.section}
---------------------------------------------------------

Learn the number of types defined in a group, and their IDs.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_typeids(int ncid, int *ntypes, int *typeids);
~~~~

 `ncid`
:   The group id.

 `ntypes`
:   A pointer to int which will get the number of types defined in the
    group. If NULL, ignored.

 `typeids`
:   A pointer to an int array which will get the typeids. If NULL,
    ignored.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

Example {.heading}
-------

The following example is from the test program libsrc4/tst\_enums.c.

 

~~~~ {.example}
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Get type info. */
      if (nc_inq_typeids(ncid, &ntypes, typeids)) ERR;
      if (ntypes != 1 || !typeids[0]) ERR;
~~~~

* * * * *

  --------------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005ftypeids "Previous section in reading order")]   [[\>](#nc_005finq_005ftype "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.3 Find a Typeid from Group and Name: nc\_inq\_typeid {.section}
------------------------------------------------------

Given a group ID and a type name, find the ID of the type. If the type
is not found in the group, then the parents are searched. If still not
found, the entire file is searched.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_typeid(int ncid, const char *name, nc_type *typeidp);
~~~~

 `ncid`
:   The group id.

 `name`
:   The name of a type.

 `typeidp`
:   A pointer to an int which will get the typeid.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad ncid.

 `NC_EBADTYPE`
:   Can’t find type.

Example {.heading}
-------

The following example is from the test program libsrc4/tst\_vars.c. It
tests that the correct names are given for atomic types.

 

~~~~ {.example}
  /* Check inquire of atomic types */
     if (nc_inq_type(ncid, NC_BYTE, name_in, &size_in)) ERR;
     if (strcmp(name_in, "byte") || size_in != sizeof(char)) ERR;
     if (nc_inq_type(ncid, NC_CHAR, name_in, &size_in)) ERR;
     if (strcmp(name_in, "char") || size_in != sizeof(char)) ERR;
     if (nc_inq_type(ncid, NC_SHORT, name_in, &size_in)) ERR;
     if (strcmp(name_in, "short") || size_in != sizeof(short)) ERR;
     if (nc_inq_type(ncid, NC_INT, name_in, &size_in)) ERR;
     if (strcmp(name_in, "int") || size_in != sizeof(int)) ERR;
     if (nc_inq_type(ncid, NC_FLOAT, name_in, &size_in)) ERR;
     if (strcmp(name_in, "float") || size_in != sizeof(float)) ERR;
     if (nc_inq_type(ncid, NC_DOUBLE, name_in, &size_in)) ERR;
     if (strcmp(name_in, "double") || size_in != sizeof(double)) ERR;
     if (nc_inq_type(ncid, NC_UBYTE, name_in, &size_in)) ERR;
     if (strcmp(name_in, "ubyte") || size_in != sizeof(unsigned char)) ERR;
     if (nc_inq_type(ncid, NC_USHORT, name_in, &size_in)) ERR;
     if (strcmp(name_in, "ushort") || size_in != sizeof(unsigned short)) ERR;
     if (nc_inq_type(ncid, NC_UINT, name_in, &size_in)) ERR;
     if (strcmp(name_in, "uint") || size_in != sizeof(unsigned int)) ERR;
     if (nc_inq_type(ncid, NC_INT64, name_in, &size_in)) ERR;
     if (strcmp(name_in, "int64") || size_in != sizeof(long long)) ERR;
     if (nc_inq_type(ncid, NC_UINT64, name_in, &size_in)) ERR;
     if (strcmp(name_in, "uint64") || size_in != sizeof(unsigned long long)) ERR;
     if (nc_inq_type(ncid, NC_STRING, name_in, &size_in)) ERR;
     if (strcmp(name_in, "string") || size_in != 0) ERR;
     if (xtype_in != NC_SHORT) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------- ----------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005ftypeid "Previous section in reading order")]   [[\>](#nc_005finq_005fuser_005ftype "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ----------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.4 Learn About a User Defined Type: nc\_inq\_type {.section}
--------------------------------------------------

Given an ncid and a typeid, get the information about a type. This
function will work on any type, including atomic and any user defined
type, whether compound, opaque, enumeration, or variable length array.

For even more information about a user defined type [Learn About a User
Defined Type: nc\_inq\_user\_type](#nc_005finq_005fuser_005ftype).

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_type(int ncid, nc_type xtype, char *name, size_t *sizep);
~~~~

 `ncid`
:   The ncid for the group containing the type (ignored for atomic
    types).

 `xtype`
:   The typeid for this type, as returned by nc\_def\_compound,
    nc\_def\_opaque, nc\_def\_enum, nc\_def\_vlen, or nc\_inq\_var, or
    as found in netcdf.h in the list of atomic types (NC\_CHAR, NC\_INT,
    etc.).

 `name`
:   If non-NULL, the name of the user defined type will be copied here.
    It will be NC\_MAX\_NAME bytes or less. For atomic types, the type
    name from CDL will be given.

 `sizep`
:   If non-NULL, the (in-memory) size of the type in bytes will be
    copied here. VLEN type size is the size of nc\_vlen\_t. String size
    is returned as the size of a character pointer. The size may be used
    to malloc space for the data, no matter what the type.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad typeid.

 `NC_ENOTNC4`
:   Seeking a user-defined type in a netCDF-3 file.

 `NC_ESTRICTNC3`
:   Seeking a user-defined type in a netCDF-4 file for which classic
    model has been turned on.

 `NC_EBADGRPID`
:   Bad group ID in ncid.

 `NC_EBADID`
:   Type ID not found.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from the test program tst\_enums.c, and it uses all the
possible inquiry functions on an enum type.

 

~~~~ {.example}
      /* Check it out. */
      if (nc_inq_user_type(ncid, typeids[0], name_in, &base_size_in, &base_nc_type_in,
                           &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != sizeof(int) ||
          base_nc_type_in != NC_INT || nfields_in != NUM_MEMBERS || class_in != NC_ENUM) ERR;
      if (nc_inq_type(ncid, typeids[0], name_in, &base_size_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != sizeof(int)) ERR;
      if (nc_inq_enum(ncid, typeids[0], name_in, &base_nc_type, &base_size_in, &num_members)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_nc_type != NC_INT || num_members != NUM_MEMBERS) ERR;
      for (i = 0; i < NUM_MEMBERS; i++)
      {
         if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
         if (strcmp(name_in, member_name[i]) || value_in != member_value[i]) ERR;
         if (nc_inq_enum_ident(ncid, typeid, member_value[i], name_in)) ERR;
         if (strcmp(name_in, member_name[i])) ERR;
      }

      if (nc_close(ncid)) ERR; 
~~~~

* * * * *

  ------------------------------------------------------------------ --------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005ftype "Previous section in reading order")]   [[\>](#Compound-Types "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ --------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.5 Learn About a User Defined Type: nc\_inq\_user\_type {.section}
--------------------------------------------------------

Given an ncid and a typeid, get the information about a user defined
type. This function will work on any user defined type, whether
compound, opaque, enumeration, or variable length array.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_user_type(int ncid, nc_type xtype, char *name, size_t *sizep, 
                 nc_type *base_nc_typep, size_t *nfieldsp, int *classp);
~~~~

 `ncid`
:   The ncid for the group containing the user defined type.

 `xtype`
:   The typeid for this type, as returned by nc\_def\_compound,
    nc\_def\_opaque, nc\_def\_enum, nc\_def\_vlen, or nc\_inq\_var.

 `name`
:   If non-NULL, the name of the user defined type will be copied here.
    It will be NC\_MAX\_NAME bytes or less.

 `sizep`
:   If non-NULL, the (in-memory) size of the type in bytes will be
    copied here. VLEN type size is the size of nc\_vlen\_t. String size
    is returned as the size of a character pointer. The size may be used
    to malloc space for the data, no matter what the type.

 `nfieldsp`
:   If non-NULL, the number of fields will be copied here for enum and
    compound types.

 `classp`
:   Return the class of the user defined type, NC\_VLEN, NC\_OPAQUE,
    NC\_ENUM, or NC\_COMPOUND.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad typeid.

 `NC_EBADFIELDID`
:   Bad fieldid.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

 

~~~~ {.example}
      /* Create a file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Create an enum type. */
      if (nc_def_enum(ncid, NC_INT, TYPE_NAME, &typeid)) ERR;
      for (i = 0; i < NUM_MEMBERS; i++)
         if (nc_insert_enum(ncid, typeid, member_name[i], 
                            &member_value[i])) ERR;

      /* Check it out. */
      if (nc_inq_user_type(ncid, typeid, name_in, &base_size_in, &base_nc_type_in,
                           &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != sizeof(int) ||
          base_nc_type_in != NC_INT || nfields_in != NUM_MEMBERS || class_in != NC_ENUM) ERR;
~~~~

* * * * *

  --------------------------------------------------------------------------- ------------------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fuser_005ftype "Previous section in reading order")]   [[\>](#nc_005fdef_005fcompound "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ------------------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.6 Compound Types Introduction {.section}
-------------------------------

Compound data types can be defined for netCDF-4/HDF5 format files. A
compound datatype is similar to a struct in C and contains a collection
of one or more atomic or user-defined types. The netCDF-4 compound data
must comply with the properties and constraints of the HDF5 compound
data type in terms of which it is implemented.

In summary these are:

-   It has a fixed total size.
-   It consists of zero or more named members that do not overlap with
    other members.
-   Each member has a name distinct from other members.
-   Each member has its own datatype.
-   Each member is referenced by an index number between zero and N-1,
    where N is the number of members in the compound datatype.
-   Each member has a fixed byte offset, which is the first byte
    (smallest byte address) of that member in the compound datatype.
-   In addition to other other user-defined data types or atomic
    datatypes, a member can be a small fixed-size array of any type with
    up to four fixed-size “dimensions” (not associated with named netCDF
    dimensions).

Currently there is an HDF5 restriction on total size of a compound type
to not larger than 2\^16 = 65536 bytes. Attempts to exceed this limit
result in an NC\_EHDFERR error.

Compound types are not supported in classic or 64-bit offset format
files.

To write data in a compound type, first use nc\_def\_compound to create
the type, multiple calls to nc\_insert\_compound to add to the compound
type, and then write data with the appropriate nc\_put\_var1,
nc\_put\_vara, nc\_put\_vars, or nc\_put\_varm call.

To read data written in a compound type, you must know its structure.
Use the nc\_inq\_compound functions to learn about the compound type.

  -------------------------------------------------------------------------------------------------------------------------- ---- --
  [5.7 Creating a Compound Type: nc\_def\_compound](#nc_005fdef_005fcompound)                                                     
  [5.8 Inserting a Field into a Compound Type: nc\_insert\_compound](#nc_005finsert_005fcompound)                                 
  [5.10 Learn About a Compound Type: nc\_inq\_compound](#nc_005finq_005fcompound)                                                 
  [5.15 Find the Name of a Field in a Compound Type: nc\_inq\_compound\_fieldname](#nc_005finq_005fcompound_005ffieldname)        
  [5.16 Get the FieldID of a Compound Type Field: nc\_inq\_compound\_fieldindex](#nc_005finq_005fcompound_005ffieldindex)         
  [5.17 Get the Offset of a Field: nc\_inq\_compound\_fieldoffset](#nc_005finq_005fcompound_005ffieldoffset)                      
  [5.18 Find the Type of a Field: nc\_inq\_compound\_fieldtype](#nc_005finq_005fcompound_005ffieldtype)                           
  -------------------------------------------------------------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------------- --------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Compound-Types "Previous section in reading order")]   [[\>](#nc_005finsert_005fcompound "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- --------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.7 Creating a Compound Type: nc\_def\_compound {.section}
-----------------------------------------------

Create a compound type. Provide an ncid, a name, and a total size (in
bytes) of one element of the completed compound type.

After calling this function, fill out the type with repeated calls to
nc\_insert\_compound (see section [Inserting a Field into a Compound
Type: nc\_insert\_compound](#nc_005finsert_005fcompound)). Call
nc\_insert\_compound once for each field you wish to insert into the
compound type.

Note that there does not seem to be a way to read such types into
structures in Fortran 90 (and there are no structures in Fortran 77).

Usage {.heading}
-----

 

~~~~ {.example}
int nc_def_compound(int ncid, size_t size, const char *name, nc_type *typeidp);
~~~~

 `ncid`
:   The groupid where this compound type will be created.

 `size`
:   The size, in bytes, of the compound type.

 `name`
:   The name of the new compound type.

 `typeidp`
:   A pointer to an nc\_type. The typeid of the new type will be placed
    there.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENAMEINUSE`
:   That name is in use.

 `NC_EMAXNAME`
:   Name exceeds max length NC\_MAX\_NAME.

 `NC_EBADNAME`
:   Name contains illegal characters.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag NC\_NETCDF4. (see section [Open a NetCDF Dataset
    for Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

 `NC_EPERM`
:   Attempt to write to a read-only file.

 `NC_ENOTINDEFINE`
:   Not in define mode.

Example {.heading}
-------

 

~~~~ {.example}
      struct s1 
      {
            int i1;
            int i2;
      };
      struct s1 data[DIM_LEN], data_in[DIM_LEN];

      /* Create a file with a compound type. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct s1), SVC_REC, &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, BATTLES_WITH_KLINGONS, 
                             HOFFSET(struct s1, i1), NC_INT)) ERR;
      if (nc_insert_compound(ncid, typeid, DATES_WITH_ALIENS, 
                             HOFFSET(struct s1, i2), NC_INT)) ERR;
      if (nc_def_dim(ncid, STARDATE, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, SERVICE_RECORD, typeid, 1, dimids, &varid)) ERR;
      if (nc_put_var(ncid, varid, data)) ERR;
      if (nc_close(ncid)) ERR;
~~~~

* * * * *

  ---------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fcompound "Previous section in reading order")]   [[\>](#nc_005finsert_005farray_005fcompound "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- ------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.8 Inserting a Field into a Compound Type: nc\_insert\_compound {.section}
----------------------------------------------------------------

Insert a named field into a compound type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_insert_compound(int ncid, nc_type typeid, const char *name, size_t offset, 
                       nc_type field_typeid);
~~~~

 `ncid`
:   The groupid where a field for this compound type will be inserted.

 `typeid`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `name`
:   The name of the new field.

 `offset`
:   Offset in byte from the beginning of the compound type for this
    field.

 `field_typeid`
:   The type of the field to be inserted.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENAMEINUSE`
:   That name is in use. Field names must be unique within a compound
    type.

 `NC_EMAXNAME`
:   Name exceed max length NC\_MAX\_NAME.

 `NC_EBADNAME`
:   Name contains illegal characters.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag NC\_NETCDF4. (see section [Open a NetCDF Dataset
    for Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

 `NC_ENOTINDEFINE`
:   Not in define mode.

Example {.heading}
-------

* * * * *

  ------------------------------------------------------------------------- ------------------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finsert_005fcompound "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- ------------------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.9 Inserting an Array Field into a Compound Type: nc\_insert\_array\_compound {.section}
------------------------------------------------------------------------------

Insert a named field into a compound type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_insert_array_compound(int ncid, nc_type xtype, const char *name, 
                         size_t offset, nc_type field_typeid,
                         int ndims, const int *dim_sizes);
~~~~

 `ncid`
:   The ID of the file that contains the array type and the compound
    type.

 `xtype`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `name`
:   The name of the new field.

 `offset`
:   Offset in byte from the beginning of the compound type for this
    field.

 `field_typeid`
:   The base type of the array to be inserted.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENAMEINUSE`
:   That name is in use. Field names must be unique within a compound
    type.

 `NC_EMAXNAME`
:   Name exceed max length NC\_MAX\_NAME.

 `NC_EBADNAME`
:   Name contains illegal characters.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag NC\_NETCDF4. (see section [Open a NetCDF Dataset
    for Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

 `NC_ENOTINDEFINE`
:   Not in define mode.

 `NC_ETYPEDEFINED`
:   Attempt to change type that has already been committed. The first
    time the file leaves define mode, all defined types are committed,
    and can’t be changed. If you wish to add an array to a compound
    type, you must do so before the compound type is committed.

Example {.heading}
-------

This example comes from the test file libsrc4/tst\_compounds.c, which
writes data about some Star Fleet officers who are known to use netCDF
data.

 

~~~~ {.example}
   /* Since some aliens exists in different, or more than one,
    * dimensions, StarFleet keeps track of the dimensional abilities
    * of everyone on 7 dimensions. */
#define NUM_DIMENSIONS 7
   struct dim_rec
   {
         int starfleet_id;
         int abilities[NUM_DIMENSIONS];
   };
   struct dim_rec dim_data_out[DIM_LEN], dim_data_in[DIM_LEN];
   
   /* Create some phoney data. */   
   for (i=0; i<DIM_LEN; i++)
   {
      /* snip */
      /* Dimensional data. */
      dim_data_out[i].starfleet_id = i;
      for (j = 0; j < NUM_DIMENSIONS; j++)
         dim_data_out[i].abilities[j] = j;
      /* snip */
   }

   printf("*** testing compound variable containing an array of ints...");
   {
      nc_type field_typeid;
      int dim_sizes[] = {NUM_DIMENSIONS};

      /* Create a file with a compound type which contains an array of
       * int. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct dim_rec), "SFDimRec", &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, "starfleet_id", 
                             HOFFSET(struct dim_rec, starfleet_id), NC_INT)) ERR;
      if (nc_insert_array_compound(ncid, typeid, "abilities", 
                             HOFFSET(struct dim_rec, abilities), NC_INT, 1, dim_sizes)) ERR;
      if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_typeid, 
                                &field_ndims, field_sizes)) ERR;
      if (strcmp(name, "abilities") || offset != 4 || field_typeid != NC_INT || 
          field_ndims != 1 || field_sizes[0] != dim_sizes[0]) ERR;
      if (nc_def_dim(ncid, STARDATE, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, "dimension_data", typeid, 1, dimids, &varid)) ERR;
      if (nc_put_var(ncid, varid, dim_data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and take a look. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, name, &xtype, &ndims, dimids, &natts)) ERR;
      if (strcmp(name, "dimension_data") || ndims != 1 || natts != 0 || dimids[0] != 0) ERR;
      if (nc_inq_compound(ncid, xtype, name, &size, &nfields)) ERR;
      if (nfields != 2 || size != sizeof(struct dim_rec) || strcmp(name, "SFDimRec")) ERR;
      if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_typeid, 
                                &field_ndims, field_sizes)) ERR;
      if (strcmp(name, "abilities") || offset != 4 || field_typeid != NC_INT || 
          field_ndims != 1 || field_sizes[0] != NUM_DIMENSIONS) ERR;
      if (nc_get_var(ncid, varid, dim_data_in)) ERR;
      for (i=0; i<DIM_LEN; i++)
      {
         if (dim_data_in[i].starfleet_id != dim_data_out[i].starfleet_id) ERR;
         for (j = 0; j < NUM_DIMENSIONS; j++)
            if (dim_data_in[i].abilities[j] != dim_data_out[i].abilities[j]) ERR;
      }
      if (nc_close(ncid)) ERR;
   }
~~~~

* * * * *

  ----------------------------------------------------------------------------------- --------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finsert_005farray_005fcompound "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005fname "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------- --------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.10 Learn About a Compound Type: nc\_inq\_compound {.section}
---------------------------------------------------

Get the number of fields, len, and name of a compound type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound(int ncid, nc_type xtype, char *name, size_t *sizep, 
                    size_t *nfieldsp);
~~~~

 `ncid`
:   The ID of any group in the file that contains the compound type.

 `xtype`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `name`
:   Pointer to an already allocated char array which will get the name,
    as a null terminated string. It will have a maximum length of
    NC\_MAX\_NAME + 1. Ignored if NULL.

 `sizep`
:   A pointer to a size\_t. The size of the compound type will be put
    here. Ignored if NULL.

 `nfieldsp`
:   A pointer to a size\_t. The number of fields in the compound type
    will be placed here. Ignored if NULL.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Couldn’t find this ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4/HDF5 file.

 `NC_ESTRICTNC3`
:   A netCDF-4/HDF5 file, but with CLASSIC\_MODEL. No user defined types
    are allowed in the classic model.

 `NC_EBADTYPE`
:   This type not a compound type.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

The following example is from the test program libsrc4/tst\_compounds.c.
See also the example for See section [Inserting an Array Field into a
Compound Type:
nc\_insert\_array\_compound](#nc_005finsert_005farray_005fcompound).

 

~~~~ {.example}
#define BATTLES_WITH_KLINGONS "Number_of_Battles_with_Klingons"
#define DATES_WITH_ALIENS "Dates_with_Alien_Hotties"

   struct s1 
   {
         int i1;
         int i2;
   };

   /* Create a file with a compound type. */
   if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
   if (nc_def_compound(ncid, sizeof(struct s1), SVC_REC, &typeid)) ERR;
   if (nc_inq_compound(ncid, typeid, name, &size, &nfields)) ERR;
   if (nfields) ERR;
   if (nc_insert_compound(ncid, typeid, BATTLES_WITH_KLINGONS, 
                     HOFFSET(struct s1, i1), NC_INT)) ERR;
   if (nc_insert_compound(ncid, typeid, DATES_WITH_ALIENS, 
                     HOFFSET(struct s1, i2), NC_INT)) ERR;

   /* Check the compound type. */                             
   if (nc_inq_compound(ncid, xtype, name, &size, &nfields)) ERR;
   if (nfields != 2 || size != 8 || strcmp(name, SVC_REC)) ERR;
   if (nc_inq_compound_name(ncid, xtype, name)) ERR;
   if (strcmp(name, SVC_REC)) ERR;
   if (nc_inq_compound_size(ncid, xtype, &size)) ERR;
   if (size != 8) ERR;
   if (nc_inq_compound_nfields(ncid, xtype, &nfields)) ERR;
   if (nfields != 2) ERR;
   if (nc_inq_compound_field(ncid, xtype, 0, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
   if (strcmp(name, BATTLES_WITH_KLINGONS) || offset != 0 || (field_xtype != NC_INT || field_ndims != 0)) ERR;
   if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
   if (strcmp(name, DATES_WITH_ALIENS) || offset != 4 || field_xtype != NC_INT) ERR;
   if (nc_inq_compound_fieldname(ncid, xtype, 1, name)) ERR;
   if (strcmp(name, DATES_WITH_ALIENS)) ERR;
   if (nc_inq_compound_fieldindex(ncid, xtype, BATTLES_WITH_KLINGONS, &fieldid)) ERR;
   if (fieldid != 0) ERR;
   if (nc_inq_compound_fieldoffset(ncid, xtype, 1, &offset)) ERR;
   if (offset != 4) ERR;
   if (nc_inq_compound_fieldtype(ncid, xtype, 1, &field_xtype)) ERR;
   if (field_xtype != NC_INT) ERR;
~~~~

* * * * *

  ---------------------------------------------------------------------- --------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005fsize "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- --------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.11 Learn the Name of a Compound Type: nc\_inq\_compound\_name {.section}
---------------------------------------------------------------

Get the name of a compound type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound_name(int ncid, nc_type xtype, char *name);
~~~~

 `ncid`
:   The groupid where this compound type exists.

 `xtype`
:   The typeid for this compound type.

 `name`
:   Pointer to an already allocated char array which will get the name,
    as a null terminated string. It will have a maximum length of
    NC\_MAX\_NAME+1.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  ------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005fname "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005fnfields "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.12 Learn the Size of a Compound Type: nc\_inq\_compound\_size {.section}
---------------------------------------------------------------

Get the len of a compound type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound_size(int ncid, nc_type xtype, size_t *sizep);
~~~~

 `ncid`
:   The groupid where this compound type exists.

 `xtype`
:   The typeid for this compound type.

 `size`
:   A pointer, which, if not NULL, get the size of the compound type.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  ------------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005fsize "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005ffield "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.13 Learn the Number of Fields in a Compound Type: nc\_inq\_compound\_nfields {.section}
------------------------------------------------------------------------------

Get the number of fields of a compound type.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_compound_nfields(int ncid, nc_type xtype, size_t *nfieldsp);
~~~~

 `ncid`
:   The groupid where this compound type exists.

 `xtype`
:   The typeid for this compound type.

 `nfieldsp`
:   A pointer, which, if not NULL, get the number of fields of the
    compound type.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  ---------------------------------------------------------------------------------- -------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005fnfields "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005ffieldname "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------- -------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.14 Learn About a Field of a Compound Type: nc\_inq\_compound\_field {.section}
---------------------------------------------------------------------

Get information about one of the fields of a compound type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound_field(int ncid, nc_type xtype, int fieldid, char *name, 
                      size_t *offsetp, nc_type *field_typeidp, int *ndimsp, 
                      int *dim_sizesp);
~~~~

 `ncid`
:   The groupid where this compound type exists.

 `xtype`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `fieldid`
:   A zero-based index number specifying a field in the compound type.

 `name`
:   A pointer which, if non-NULL, will get the name of the field.

 `offsetp`
:   A pointer which, if non-NULL, will get the offset of the field.

 `field_typeid`
:   A pointer which, if non-NULL, will get the typeid of the field.

 `ndimsp`
:   A pointer which, if non-NULL, will get the number of dimensions of
    the field.

 `dim_sizesp`
:   A pointer which, if non-NULL, will get the dimension sizes of the
    field.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  -------------------------------------------------------------------------------- --------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005ffield "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005ffieldindex "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- --------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.15 Find the Name of a Field in a Compound Type: nc\_inq\_compound\_fieldname {.section}
------------------------------------------------------------------------------

Given the typeid and the fieldid, get the name.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound_fieldname(nc_type typeid, int fieldid, char *name);
~~~~

 `typeid`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `fieldid`
:   The id of the field in the compound type. Fields are numbered
    starting with 0 for the first inserted field.

 `name`
:   Pointer to an already allocated char array which will get the name,
    as a null terminated string. It will have a maximum length of
    NC\_MAX\_NAME+1.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EBADFIELDID`
:   Bad field id.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  ------------------------------------------------------------------------------------ ---------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005ffieldname "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005ffieldoffset "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------ ---------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.16 Get the FieldID of a Compound Type Field: nc\_inq\_compound\_fieldindex {.section}
----------------------------------------------------------------------------

Given the typeid and the name, get the fieldid.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound_fieldindex(nc_type typeid, const char *name, int *fieldidp);
~~~~

 `typeid`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `name`
:   The name of the field.

 `fieldidp`
:   A pointer to an int which will get the field id of the named field.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EUNKNAME`
:   Can’t find field of this name.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  ------------------------------------------------------------------------------------- -------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005ffieldindex "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005ffieldtype "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------- -------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.17 Get the Offset of a Field: nc\_inq\_compound\_fieldoffset {.section}
--------------------------------------------------------------

Given the typeid and fieldid, get the offset.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound_fieldoffset(nc_type typeid, int fieldid, 
                                size_t *offsetp); 
~~~~

 `typeid`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `fieldid`
:   The id of the field in the compound type. Fields are numbered
    starting with 0 for the first inserted field.

 `offsetp`
:   A pointer to a size\_t. The offset of the field will be placed
    there.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad typeid.

 `NC_EBADFIELDID`
:   Bad fieldid.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  -------------------------------------------------------------------------------------- --------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005ffieldoffset "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005ffieldndims "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------------- --------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.18 Find the Type of a Field: nc\_inq\_compound\_fieldtype {.section}
-----------------------------------------------------------

Given the typeid and the fieldid, get the type of that field.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_compound_fieldtype(nc_type typeid, int fieldid, nc_type *field_typeidp);
~~~~

 `typeid`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `fieldid`
:   The id of the field in the compound type. Fields are numbered
    starting with 0 for the first inserted field.

 `field_typeidp`
:   Pointer to a nc\_type which will get the typeid of the field.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad typeid.

 `NC_EBADFIELDID`
:   Bad fieldid.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  ------------------------------------------------------------------------------------ ----------------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005ffieldtype "Previous section in reading order")]   [[\>](#nc_005finq_005fcompound_005ffielddim_005fsizes "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------ ----------------------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.19 Find the Number of Dimensions in an Array Field: nc\_inq\_compound\_fieldndims {.section}
-----------------------------------------------------------------------------------

Given the typeid and the fieldid, get the number of dimensions of that
field.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound_fieldndims(int ncid, nc_type xtype, int fieldid, 
                               int *ndimsp);
~~~~

 `ncid`
:   The file or group ID.

 `xtype`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `fieldid`
:   The id of the field in the compound type. Fields are numbered
    starting with 0 for the first inserted field.

 `ndimsp`
:   Pointer to an int which will get the number of dimensions of the
    field. Non-array fields have 0 dimensions.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad typeid.

 `NC_EBADFIELDID`
:   Bad fieldid.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  ------------------------------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005ffieldndims "Previous section in reading order")]   [[\>](#Variable-Length-Array "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.20 Find the Sizes of Dimensions in an Array Field: nc\_inq\_compound\_fielddim\_sizes {.section}
---------------------------------------------------------------------------------------

Given the xtype and the fieldid, get the sizes of dimensions for that
field. User must have allocated storage for the dim\_sizes.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_compound_fielddim_sizes(int ncid, nc_type xtype, int fieldid, 
                                   int *dim_sizes);
~~~~

 `ncid`
:   The file or group ID.

 `xtype`
:   The typeid for this compound type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `fieldid`
:   The id of the field in the compound type. Fields are numbered
    starting with 0 for the first inserted field.

 `dim_sizesp`
:   Pointer to an array of int which will get the sizes of the
    dimensions of the field. Non-array fields have 0 dimensions.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad typeid.

 `NC_EBADFIELDID`
:   Bad fieldid.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

See the example section for [Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound).

* * * * *

  --------------------------------------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fcompound_005ffielddim_005fsizes "Previous section in reading order")]   [[\>](#nc_005fdef_005fvlen "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.21 Variable Length Array Introduction {.section}
---------------------------------------

NetCDF-4 added support for a variable length array type. This is not
supported in classic or 64-bit offset files, or in netCDF-4 files which
were created with the NC\_CLASSIC\_MODEL flag.

A variable length array is represented in C as a structure from HDF5,
the nc\_vlen\_t structure. It contains a len member, which contains the
length of that array, and a pointer to the array.

So an array of VLEN in C is an array of nc\_vlen\_t structures.

VLEN arrays are handled differently with respect to allocation of
memory. Generally, when reading data, it is up to the user to malloc
(and subsequently free) the memory needed to hold the data. It is up to
the user to ensure that enough memory is allocated.

With VLENs, this is impossible. The user cannot know the size of an
array of VLEN until after reading the array. Therefore when reading VLEN
arrays, the netCDF library will allocate the memory for the data within
each VLEN.

It is up to the user, however, to eventually free this memory. This is
not just a matter of one call to free, with the pointer to the array of
VLENs; each VLEN contains a pointer which must be freed.

When dynamically allocating space to hold an array of VLEN, allocate
storage for an array of nc\_vlen\_t.

Compression is permitted but may not be effective for VLEN data, because
the compression is applied to the nc\_vlen\_t structs, rather than the
actual data.

* * * * *

  -------------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Variable-Length-Array "Previous section in reading order")]   [[\>](#nc_005finq_005fvlen "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.22 Define a Variable Length Array (VLEN): nc\_def\_vlen {.section}
---------------------------------------------------------

Use this function to define a variable length array type.

Usage {.heading}
-----

 

~~~~ {.example}
nc_def_vlen(int ncid, const char *name, nc_type base_typeid, nc_type *xtypep);
~~~~

 `ncid`
:   The ncid of the file to create the VLEN type in.

 `name` 
:   A name for the VLEN type.

 `base_typeid`
:   The typeid of the base type of the VLEN. For example, for a VLEN of
    shorts, the base type is NC\_SHORT. This can be a user defined type.

 `xtypep`
:   A pointer to an nc\_type variable. The typeid of the new VLEN type
    will be set here.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EMAXNAME`
:   NC\_MAX\_NAME exceeded.

 `NC_ENAMEINUSE`
:   Name is already in use.

 `NC_EBADNAME`
:   Attribute or variable name contains illegal characters.

 `NC_EBADID`
:   ncid invalid.

 `NC_EBADGRPID`
:   Group ID part of ncid was invalid.

 `NC_EINVAL`
:   Size is invalid.

 `NC_ENOMEM`
:   Out of memory.

Example {.heading}
-------

 

~~~~ {.example}
#define DIM_LEN 3
#define ATT_NAME "att_name"

   nc_vlen_t data[DIM_LEN];
   int *phony;

   /* Create phony data. */
   for (i=0; i<DIM_LEN; i++)
   {
      if (!(phony = malloc(sizeof(int) * i+1)))
         return NC_ENOMEM;
      for (j=0; j<i+1; j++)
         phony[j] = -99;
      data[i].p = phony;
      data[i].len = i+1;
   }

      /* Define a VLEN of NC_INT, and write an attribute of that
      type. */
      if (nc_def_vlen(ncid, "name1", NC_INT, &typeid)) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, typeid, DIM_LEN, data)) ERR;
~~~~

* * * * *

  ------------------------------------------------------------------ --------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fvlen "Previous section in reading order")]   [[\>](#nc_005ffree_005fvlen "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ --------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.23 Learning about a Variable Length Array (VLEN) Type: nc\_inq\_vlen {.section}
----------------------------------------------------------------------

Use this type to learn about a vlen.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_vlen(int ncid, nc_type xtype, char *name, size_t *datum_sizep, 
            nc_type *base_nc_typep);
~~~~

 `ncid`
:   The ncid of the file that contains the VLEN type.

 `xtype`
:   The type of the VLEN to inquire about.

 `name`
:   A pointer for storage for the types name. The name will be
    NC\_MAX\_NAME characters or less.

 `datum_sizep`
:   A pointer to a size\_t, this will get the size of one element of
    this vlen.

 `base_nc_typep`
:   A pointer to an nc\_type, this will get the type of the VLEN base
    type. (In other words, what type is this a VLEN of?)

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPE`
:   Can’t find the typeid.

 `NC_EBADID`
:   ncid invalid.

 `NC_EBADGRPID`
:   Group ID part of ncid was invalid.

Example {.heading}
-------

 

~~~~ {.example}
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_vlen(ncid, "name1", NC_INT, &typeid)) ERR;
      if (nc_inq_vlen(ncid, typeid, name_in, &size_in, &base_nc_type_in)) ERR;
      if (base_nc_type_in != NC_INT || (size_in != sizeof(int) || strcmp(name_in, VLEN_NAME))) ERR;
      if (nc_inq_user_type(ncid, typeid, name_in, &size_in, &base_nc_type_in, NULL, &class_in)) ERR;
      if (base_nc_type_in != NC_INT || (size_in != sizeof(int) || strcmp(name_in, VLEN_NAME))) ERR;
      if (nc_inq_compound(ncid, typeid, name_in, &size_in, NULL) != NC_EBADTYPE) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, typeid, DIM_LEN, data)) ERR;
      if (nc_close(ncid)) ERR;
~~~~

* * * * *

  ------------------------------------------------------------------ ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvlen "Previous section in reading order")]   [[\>](#nc_005ffree_005fvlens "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.24 Releasing Memory for a Variable Length Array (VLEN) Type: nc\_free\_vlen {.section}
-----------------------------------------------------------------------------

When a VLEN is read into user memory from the file, the HDF5 library
performs memory allocations for each of the variable length arrays
contained within the VLEN structure. This memory must be freed by the
user to avoid memory leaks.

This violates the normal netCDF expectation that the user is responsible
for all memory allocation. But, with VLEN arrays, the underlying HDF5
library allocates the memory for the user, and the user is responsible
for deallocating that memory.

To save the user the trouble calling free() on each element of the VLEN
array (i.e. the array of arrays), the nc\_free\_vlen function is
provided.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_free_vlen(nc_vlen_t *vl);
~~~~

 `vl`
:   A pointer to the variable length array structure which is to be
    freed.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPE`
:   Can’t find the typeid.

Example {.heading}
-------

This example is from test program libsrc4/tst\_vl.c.

 

~~~~ {.example}
   /* Free the memory used in our phony data. */
   for (i=0; i<DIM_LEN; i++)
      if (nc_free_vlen(&data[i])) ERR;
~~~~

* * * * *

  ------------------------------------------------------------------- ------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005ffree_005fvlen "Previous section in reading order")]   [[\>](#Opaque-Type "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------- ------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.25 Releasing Memory for an Array of Variable Length Array (VLEN) Type: nc\_free\_vlen {.section}
---------------------------------------------------------------------------------------

When a VLEN is read into user memory from the file, the HDF5 library
performs memory allocations for each of the variable length arrays
contained within the VLEN structure. This memory must be freed by the
user to avoid memory leaks.

This violates the normal netCDF expectation that the user is responsible
for all memory allocation. But, with VLEN arrays, the underlying HDF5
library allocates the memory for the user, and the user is responsible
for deallocating that memory.

To save the user the trouble calling free() on each element of the VLEN
array (i.e. the array of arrays), the nc\_free\_vlens function is
provided. It frees all the vlens in an array.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_free_vlens(size_t len, nc_vlen_t vlens[])
~~~~

 `len`
:   Length of the VLEN array to be freed.

 `vlens`
:   Array of VLENs to be freed.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

* * * * *

  -------------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005ffree_005fvlens "Previous section in reading order")]   [[\>](#nc_005fdef_005fopaque "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.26 Opaque Type Introduction {.section}
-----------------------------

NetCDF-4 added support for the opaque type. This is not supported in
classic or 64-bit offset files.

The opaque type is a type which is a collection of objects of a known
size. (And each object is the same size). Nothing is known to netCDF
about the contents of these blobs of data, except their size in bytes,
and the name of the type.

To use an opaque type, first define it with [Creating Opaque Types:
nc\_def\_opaque](#nc_005fdef_005fopaque). If encountering an enum type
in a new data file, use [Learn About an Opaque Type:
nc\_inq\_opaque](#nc_005finq_005fopaque) to learn its name and size.

* * * * *

  ---------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Opaque-Type "Previous section in reading order")]   [[\>](#nc_005finq_005fopaque "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.27 Creating Opaque Types: nc\_def\_opaque {.section}
-------------------------------------------

Create an opaque type. Provide a size and a name.

Usage {.heading}
-----

 

~~~~ {.example}
nc_def_opaque(int ncid, size_t size, const char *name, nc_type *typeidp);
~~~~

 `ncid`
:   The groupid where the type will be created. The type may be used
    anywhere in the file, no matter what group it is in.

 `size`
:   The size of each opaque object.

 `name`
:   The name for this type. Must be shorter than NC\_MAX\_NAME.

 `typeidp`
:   Pointer where the new typeid for this type is returned. Use this
    typeid when defining variables of this type with [Create a Variable:
    `nc_def_var`](#nc_005fdef_005fvar).

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad typeid.

 `NC_EBADFIELDID`
:   Bad fieldid.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from the test program libsrc4/tst\_opaques.c.

 

~~~~ {.example}
      /* Create a file that has an opaque attribute. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_opaque(ncid, BASE_SIZE, TYPE_NAME, &xtype)) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------- ---------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fopaque "Previous section in reading order")]   [[\>](#Enum-Type "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ---------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.28 Learn About an Opaque Type: nc\_inq\_opaque {.section}
------------------------------------------------

Given a typeid, get the information about an opaque type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_opaque(int ncid, nc_type xtype, char *name, size_t *sizep);
~~~~

 `ncid`
:   The ncid for the group containing the opaque type.

 `xtype`
:   The typeid for this opaque type, as returned by nc\_def\_compound,
    or nc\_inq\_var.

 `name`
:   If non-NULL, the name of the opaque type will be copied here. It
    will be NC\_MAX\_NAME bytes or less.

 `sizep`
:   If non-NULL, the size of the opaque type will be copied here.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad typeid.

 `NC_EBADFIELDID`
:   Bad fieldid.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from test program libsrc4/tst\_opaques.c:

 

~~~~ {.example}
      if (nc_def_opaque(ncid, BASE_SIZE, TYPE_NAME, &xtype)) ERR;

      if (nc_inq_opaque(ncid, xtype, name_in, &base_size_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fopaque "Previous section in reading order")]   [[\>](#nc_005fdef_005fenum "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.29 Enum Type Introduction {.section}
---------------------------

NetCDF-4 added support for the enum type. This is not supported in
classic or 64-bit offset files.

* * * * *

  -------------------------------------------------------- ----------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Enum-Type "Previous section in reading order")]   [[\>](#nc_005finsert_005fenum "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------- ----------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.30 Creating a Enum Type: nc\_def\_enum {.section}
----------------------------------------

Create an enum type. Provide an ncid, a name, and a base integer type.

After calling this function, fill out the type with repeated calls to
nc\_insert\_enum (see section [Inserting a Field into a Enum Type:
nc\_insert\_enum](#nc_005finsert_005fenum)). Call nc\_insert\_enum once
for each value you wish to make part of the enumeration.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_def_enum(int ncid, nc_type base_typeid, const char *name, 
                nc_type *typeidp);
~~~~

 `ncid`
:   The groupid where this compound type will be created.

 `base_typeid`
:   The base integer type for this enum. Must be one of: NC\_BYTE,
    NC\_UBYTE, NC\_SHORT, NC\_USHORT, NC\_INT, NC\_UINT, NC\_INT64,
    NC\_UINT64.

 `name`
:   The name of the new enum type.

 `typeidp`
:   A pointer to an nc\_type. The typeid of the new type will be placed
    there.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENAMEINUSE`
:   That name is in use.

 `NC_EMAXNAME`
:   Name exceeds max length NC\_MAX\_NAME.

 `NC_EBADNAME`
:   Name contains illegal characters.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag NC\_NETCDF4. (see section [Open a NetCDF Dataset
    for Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

 `NC_EPERM`
:   Attempt to write to a read-only file.

 `NC_ENOTINDEFINE`
:   Not in define mode.

The following example, from libsrc4/tst\_enums.c, shows the creation and
use of an enum type, including the use of a fill value.

 

~~~~ {.example}
       int dimid, varid;
       size_t num_members_in;
       int class_in;
       unsigned char value_in;

       enum clouds {           /* a C enumeration */
           CLEAR=0,
           CUMULONIMBUS=1,
           STRATUS=2,
           STRATOCUMULUS=3,
           CUMULUS=4,
           ALTOSTRATUS=5,
           NIMBOSTRATUS=6,
           ALTOCUMULUS=7,
           CIRROSTRATUS=8,
           CIRROCUMULUS=9,
           CIRRUS=10,
           MISSING=255};

       struct {
           char *name;
           unsigned char value;
       } cloud_types[] = {
           {"Clear", CLEAR}, 
           {"Cumulonimbus", CUMULONIMBUS}, 
           {"Stratus", STRATUS}, 
           {"Stratocumulus", STRATOCUMULUS}, 
           {"Cumulus", CUMULUS}, 
           {"Altostratus", ALTOSTRATUS}, 
           {"Nimbostratus", NIMBOSTRATUS}, 
           {"Altocumulus", ALTOCUMULUS}, 
           {"Cirrostratus", CIRROSTRATUS}, 
           {"Cirrocumulus", CIRROCUMULUS}, 
           {"Cirrus", CIRRUS}, 
           {"Missing", MISSING}
       };
       int var_dims[VAR2_RANK];
       unsigned char att_val;
       unsigned char cloud_data[DIM2_LEN] = {
           CLEAR, STRATUS, CLEAR, CUMULONIMBUS, MISSING};
       unsigned char cloud_data_in[DIM2_LEN];

       if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;

       /* Create an enum type. */
       if (nc_def_enum(ncid, NC_UBYTE, TYPE2_NAME, &typeid)) ERR;
       num_members = (sizeof cloud_types) / (sizeof cloud_types[0]);
       for (i = 0; i < num_members; i++)
           if (nc_insert_enum(ncid, typeid, cloud_types[i].name,
                              &cloud_types[i].value)) ERR;

       /* Declare a station dimension */
       if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimid)) ERR;
       /* Declare a variable of the enum type */
       var_dims[0] = dimid;
       if (nc_def_var(ncid, VAR2_NAME, typeid, VAR2_RANK, var_dims, &varid)) ERR;
       /* Create and write a variable attribute of the enum type */
       att_val = MISSING;
       if (nc_put_att(ncid, varid, ATT2_NAME, typeid, ATT2_LEN, &att_val)) ERR;
       if (nc_enddef(ncid)) ERR;
       /* Store some data of the enum type */
       if(nc_put_var(ncid, varid, cloud_data)) ERR;
       /* Write the file. */
       if (nc_close(ncid)) ERR;
~~~~

* * * * *

  ------------------------------------------------------------------ -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fenum "Previous section in reading order")]   [[\>](#nc_005finq_005fenum "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ -------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.31 Inserting a Field into a Enum Type: nc\_insert\_enum {.section}
---------------------------------------------------------

Insert a named member into a enum type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_insert_enum(int ncid, nc_type xtype, const char *identifier, 
                   const void *value);
~~~~

 `ncid`
:   The ncid of the group which contains the type.

 `typeid`
:   The typeid for this enum type, as returned by nc\_def\_enum, or
    nc\_inq\_var.

 `identifier`
:   The identifier of the new member.

 `value`
:   The value that is to be associated with this member.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad group id.

 `NC_ENAMEINUSE`
:   That name is in use. Field names must be unique within a enum type.

 `NC_EMAXNAME`
:   Name exceed max length NC\_MAX\_NAME.

 `NC_EBADNAME`
:   Name contains illegal characters.

 `NC_ENOTNC4`
:   Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4
    operations can only be performed on files defined with a create mode
    which includes flag NC\_NETCDF4. (see section [Open a NetCDF Dataset
    for Access: nc\_open](#nc_005fopen)).

 `NC_ESTRICTNC3`
:   This file was created with the strict netcdf-3 flag, therefore
    netcdf-4 operations are not allowed. (see section [Open a NetCDF
    Dataset for Access: nc\_open](#nc_005fopen)).

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

 `NC_ENOTINDEFINE`
:   Not in define mode.

Example {.heading}
-------

This example is from libsrc4/tst\_enums.c; also see the example in See
section [Creating a Enum Type: nc\_def\_enum](#nc_005fdef_005fenum).

 

~~~~ {.example}
      char brady_name[NUM_BRADYS][NC_MAX_NAME + 1] = {"Mike", "Carol", "Greg", "Marsha",
                                                       "Peter", "Jan", "Bobby", "Whats-her-face",
                                                       "Alice"};
      unsigned char brady_value[NUM_BRADYS] = {0, 1,2,3,4,5,6,7,8};
      unsigned char data[BRADY_DIM_LEN] = {0, 4, 8};
      unsigned char value_in;

      /* Create a file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Create an enum type based on unsigned bytes. */
      if (nc_def_enum(ncid, NC_UBYTE, BRADYS, &typeid)) ERR;
      for (i = 0; i < NUM_BRADYS; i++)
         if (nc_insert_enum(ncid, typeid, brady_name[i], 
                            &brady_value[i])) ERR;
~~~~

* * * * *

  --------------------------------------------------------------------- ------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finsert_005fenum "Previous section in reading order")]   [[\>](#nc_005finq_005fenum_005fmember "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ------------------------------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.32 Learn About a Enum Type: nc\_inq\_enum {.section}
-------------------------------------------

Get information about a user-define enumeration type.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_enum(int ncid, nc_type xtype, char *name, nc_type *base_nc_typep, 
            size_t *base_sizep, size_t *num_membersp);
~~~~

 `ncid`
:   The group ID of the group which holds the enum type.

 `xtype`
:   The typeid for this enum type, as returned by nc\_def\_enum, or
    nc\_inq\_var.

 `name`
:   Pointer to an already allocated char array which will get the name,
    as a null terminated string. It will have a maximum length of
    NC\_MAX\_NAME+1.

 `base_nc_typep`
:   If non-NULL, a pointer to a nc\_type, which will get the base
    integer type of this enum.

 `base_sizep`
:   If non-NULL, a size\_t pointer, which will get the size (in bytes)
    of the base integer type of this enum.

 `num_membersp`
:   If non-NULL, a size\_t pointer which will get the number of members
    defined for this enumeration type.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This example is from libsrc4/tst\_enums.c, and is a continuation of the
example above for nc\_insert\_enum. First an enum type is created, with
one element for each of the nine members of the Brady family on a
popular American television show which occupies far too much memory
space in my brain!

In the example, the enum type is created, then checked using the
nc\_inq\_enum and nc\_inq\_enum\_member functions. See [Learn the Name
of a Enum Type: nc\_inq\_enum\_member](#nc_005finq_005fenum_005fmember).

 

~~~~ {.example}
      char brady_name[NUM_BRADYS][NC_MAX_NAME + 1] = {"Mike", "Carol", "Greg", "Marsha",
                                                       "Peter", "Jan", "Bobby", "Whats-her-face",
                                                       "Alice"};
      unsigned char brady_value[NUM_BRADYS] = {0, 1,2,3,4,5,6,7,8};
      unsigned char data[BRADY_DIM_LEN] = {0, 4, 8};
      unsigned char value_in;

      /* Create a file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Create an enum type based on unsigned bytes. */
      if (nc_def_enum(ncid, NC_UBYTE, BRADYS, &typeid)) ERR;
      for (i = 0; i < NUM_BRADYS; i++)
         if (nc_insert_enum(ncid, typeid, brady_name[i], 
                            &brady_value[i])) ERR;

      /* Check it out. */
      if (nc_inq_enum(ncid, typeid, name_in, &base_nc_type, &base_size_in, &num_members)) ERR;
      if (strcmp(name_in, BRADYS) || base_nc_type != NC_UBYTE || base_size_in != 1 ||
          num_members != NUM_BRADYS) ERR;
      for (i = 0; i < NUM_BRADYS; i++)
      {
         if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
         if (strcmp(name_in, brady_name[i]) || value_in != brady_value[i]) ERR;
         if (nc_inq_enum_ident(ncid, typeid, brady_value[i], name_in)) ERR;
         if (strcmp(name_in, brady_name[i])) ERR;
      }
~~~~

* * * * *

  ------------------------------------------------------------------ ------------------------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fenum "Previous section in reading order")]   [[\>](#nc_005finq_005fenum_005fident "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ ------------------------------------------------------------------------ --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.33 Learn the Name of a Enum Type: nc\_inq\_enum\_member {.section}
---------------------------------------------------------

Get information about a member of an enum type.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_enum_member(int ncid, nc_type xtype, int idx, char *name, 
                       void *value);
~~~~

 `ncid`
:   The groupid where this enum type exists.

 `xtype`
:   The typeid for this enum type.

 `idx`
:   The zero-based index number for the member of interest.

 `name`
:   If non-NULL, a pointer to an already allocated char array which will
    get the name, as a null terminated string. It will have a maximum
    length of NC\_MAX\_NAME+1.

 `value`
:   If non-NULL, a pointer to storage of the correct integer type (i.e.
    the base type of this enum type). It will get the value associated
    with this member.

Errors {.heading}
------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

Example {.heading}
-------

This is the continuation of the example in [Creating a Enum Type:
nc\_def\_enum](#nc_005fdef_005fenum). The file is reopened and the cloud
enum type is examined.

 

~~~~ {.example}
       /* Reopen the file. */
       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

       if (nc_inq_user_type(ncid, typeid, name_in, &base_size_in, &base_nc_type_in,
                            &nfields_in, &class_in)) ERR;
       if (strcmp(name_in, TYPE2_NAME) || 
           base_size_in != sizeof(unsigned char) ||
           base_nc_type_in != NC_UBYTE || 
           nfields_in != num_members || 
           class_in != NC_ENUM) ERR;
       if (nc_inq_enum(ncid, typeid, name_in, 
                       &base_nc_type_in, &base_size_in, &num_members_in)) ERR;
       if (strcmp(name_in, TYPE2_NAME) || 
           base_nc_type_in !=  NC_UBYTE || 
           num_members_in != num_members) ERR;
       for (i = 0; i < num_members; i++)
       {
           if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
           if (strcmp(name_in, cloud_types[i].name) || 
               value_in != cloud_types[i].value) ERR;
           if (nc_inq_enum_ident(ncid, typeid, cloud_types[i].value, 
                                 name_in)) ERR;
           if (strcmp(name_in, cloud_types[i].name)) ERR;
       }
       if (nc_inq_varid(ncid, VAR2_NAME, &varid)) ERR;

       if (nc_get_att(ncid, varid, ATT2_NAME, &value_in)) ERR;
       if (value_in != MISSING) ERR;

       if(nc_get_var(ncid, varid, cloud_data_in)) ERR;
       for (i = 0; i < DIM2_LEN; i++) {
           if (cloud_data_in[i] != cloud_data[i]) ERR;
       }
   
       if (nc_close(ncid)) ERR; 
~~~~

* * * * *

  ----------------------------------------------------------------------------- ---------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fenum_005fmember "Previous section in reading order")]   [[\>](#Variables "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#User-Defined-Data-Types "Up section")]   [[\>\>](#Variables "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------- ---------------------------------------------------- --- ------------------------------------------------------------------------------------ ----------------------------------------------- ------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

5.34 Learn the Name of a Enum Type: nc\_inq\_enum\_ident {.section}
--------------------------------------------------------

Get the name which is associated with an enum member value.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_enum_ident(int ncid, nc_type xtype, long long value, char *identifier);
~~~~

 `ncid`
:   The groupid where this enum type exists.

 `xtype`
:   The typeid for this enum type.

 `value`
:   The value for which an identifier is sought.

 `identifier`
:   If non-NULL, a pointer to an already allocated char array which will
    get the identifier, as a null terminated string. It will have a
    maximum length of NC\_MAX\_NAME+1.

Return Code {.heading}
-----------

 `NC_NOERR`
:   No error.

 `NC_EBADTYPEID`
:   Bad type id, or not an enum type.

 `NC_EHDFERR`
:   An error was reported by the HDF5 layer.

 `NC_EINVAL`
:   The value was not found in the enum.

Example {.heading}
-------

See the example section for [Learn About a Enum Type:
nc\_inq\_enum](#nc_005finq_005fenum) for a full example.

* * * * *

  ---------------------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ --------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fenum_005fident "Previous section in reading order")]   [[\>](#Variable-Introduction "Next section in reading order")]       [[\<\<](#User-Defined-Data-Types "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------------------------ --------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6. Variables {.chapter}
============

  ----------------------------------------------------------------------------------------------------------------------------- ---- ----------------------------------------------
  [6.1 Introduction](#Variable-Introduction)                                                                                         
  [6.2 Language Types Corresponding to netCDF external data types](#Variable-Types)                                                  
  [6.3 NetCDF-3 Classic and 64-Bit Offset Data Types](#NetCDF_002d3-Variable-Types)                                                  
  [6.4 NetCDF-4 Atomic Types](#NetCDF_002d4-Atomic-Types)                                                                            
  [6.5 Create a Variable: `nc_def_var`](#nc_005fdef_005fvar)                                                                         Create a Variable
  [6.6 Define Chunking Parameters for a Variable: `nc_def_var_chunking`](#nc_005fdef_005fvar_005fchunking)                           Set chunking parameters
  [6.7 Learn About Chunking Parameters for a Variable: `nc_inq_var_chunking`](#nc_005finq_005fvar_005fchunking)                      Learn about chunking parameters
  [6.8 Set HDF5 Chunk Cache for a Variable: nc\_set\_var\_chunk\_cache](#nc_005fset_005fvar_005fchunk_005fcache)                     
  [6.9 Get the HDF5 Chunk Cache Settings for a Variable: nc\_get\_var\_chunk\_cache](#nc_005fget_005fvar_005fchunk_005fcache)        
  [6.10 Define Fill Parameters for a Variable: `nc_def_var_fill`](#nc_005fdef_005fvar_005ffill)                                      
  [6.11 Learn About Fill Parameters for a Variable: `nc_inq_var_fill`](#nc_005finq_005fvar_005ffill)                                 
  [6.12 Define Compression Parameters for a Variable: `nc_def_var_deflate`](#nc_005fdef_005fvar_005fdeflate)                         Set compression parameters
  [6.13 Learn About Deflate Parameters for a Variable: `nc_inq_var_deflate`](#nc_005finq_005fvar_005fdeflate)                        Learn about compression parameters
  [6.14 Learn About Szip Parameters for a Variable: `nc_inq_var_szip`](#nc_005finq_005fvar_005fszip)                                 
  [6.15 Define Checksum Parameters for a Variable: `nc_def_var_fletcher32`](#nc_005fdef_005fvar_005ffletcher32)                      Set checksum filter
  [6.16 Learn About Checksum Parameters for a Variable: `nc_inq_var_fletcher32`](#nc_005finq_005fvar_005ffletcher32)                 Learn if checksum filter is set
  [6.17 Define Endianness of a Variable: `nc_def_var_endian`](#nc_005fdef_005fvar_005fendian)                                        
  [6.18 Learn About Endian Parameters for a Variable: `nc_inq_var_endian`](#nc_005finq_005fvar_005fendian)                           
  [6.19 Get a Variable ID from Its Name: nc\_inq\_varid](#nc_005finq_005fvarid)                                                      Get a Variable ID from Its Name
  [6.20 Get Information about a Variable from Its ID: nc\_inq\_var](#nc_005finq_005fvar)                                             Get Information about a Variable from Its ID
  [6.21 Write a Single Data Value: nc\_put\_var1\_ type](#nc_005fput_005fvar1_005f-type)                                             Write a Single Data Value
  [6.22 Write an Entire Variable: nc\_put\_var\_ type](#nc_005fput_005fvar_005f-type)                                                Write an Entire Variable
  [6.23 Write an Array of Values: nc\_put\_vara\_ type](#nc_005fput_005fvara_005f-type)                                              Write an Array of Values
  [6.24 Write a Subsampled Array of Values: nc\_put\_vars\_ type](#nc_005fput_005fvars_005f-type)                                    Write a Subsampled Array of Values
  [6.25 Write a Mapped Array of Values: nc\_put\_varm\_ type](#nc_005fput_005fvarm_005f-type)                                        Write a Mapped Array of Values
  [6.26 Read a Single Data Value: nc\_get\_var1\_ type](#nc_005fget_005fvar1_005f-type)                                              Read a Single Data Value
  [6.27 Read an Entire Variable nc\_get\_var\_ type](#nc_005fget_005fvar_005f-type)                                                  Read an Entire Variable
  [6.28 Read an Array of Values: nc\_get\_vara\_ type](#nc_005fget_005fvara_005f-type)                                               Read an Array of Values
  [6.29 Read a Subsampled Array of Values: nc\_get\_vars\_ type](#nc_005fget_005fvars_005f-type)                                     Read a Subsampled Array of Values
  [6.30 Read a Mapped Array of Values: nc\_get\_varm\_ type](#nc_005fget_005fvarm_005f-type)                                         Read a Mapped Array of Values
  [6.31 Reading and Writing Character String Values](#Strings)                                                                       
  [6.32 Releasing Memory for a NC\_STRING: nc\_free\_string](#nc_005ffree_005fstring)                                                
  [6.33 Fill Values](#Fill-Values)                                                                                                   What’s Written Where there’s No Data?
  [6.34 Rename a Variable: nc\_rename\_var](#nc_005frename_005fvar)                                                                  Rename a Variable
  [6.35 Copy a Variable from One File to Another: nc\_copy\_var](#nc_005fcopy_005fvar)                                               
  [6.36 Change between Collective and Independent Parallel Access: nc\_var\_par\_access](#nc_005fvar_005fpar_005faccess)             
  [6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)                                                              Deprecated variable “\_ubyte” functions
  ----------------------------------------------------------------------------------------------------------------------------- ---- ----------------------------------------------

* * * * *

  -------------------------------------------------------- --------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Variables "Previous section in reading order")]   [[\>](#Variable-Types "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------- --------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.1 Introduction {.section}
----------------

Variables for a netCDF dataset are defined when the dataset is created,
while the netCDF dataset is in define mode. Other variables may be added
later by reentering define mode. A netCDF variable has a name, a type,
and a shape, which are specified when it is defined. A variable may also
have values, which are established later in data mode.

Ordinarily, the name, type, and shape are fixed when the variable is
first defined. The name may be changed, but the type and shape of a
variable cannot be changed. However, a variable defined in terms of the
unlimited dimension can grow without bound in that dimension.

A netCDF variable in an open netCDF dataset is referred to by a small
integer called a variable ID.

Variable IDs reflect the order in which variables were defined within a
netCDF dataset. Variable IDs are 0, 1, 2,..., in the order in which the
variables were defined. A function is available for getting the variable
ID from the variable name and vice-versa.

Attributes (see section [Attributes](#Attributes)) may be associated
with a variable to specify such properties as units.

Operations supported on variables are:

-   Create a variable, given its name, data type, and shape.
-   Get a variable ID from its name.
-   Get a variable’s name, data type, shape, and number of attributes
    from its ID.
-   Put a data value into a variable, given variable ID, indices, and
    value.
-   Put an array of values into a variable, given variable ID, corner
    indices, edge lengths, and a block of values.
-   Put a subsampled or mapped array-section of values into a variable,
    given variable ID, corner indices, edge lengths, stride vector,
    index mapping vector, and a block of values.
-   Get a data value from a variable, given variable ID and indices.
-   Get an array of values from a variable, given variable ID, corner
    indices, and edge lengths.
-   Get a subsampled or mapped array-section of values from a variable,
    given variable ID, corner indices, edge lengths, stride vector, and
    index mapping vector.
-   Rename a variable.

* * * * *

  -------------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Variable-Introduction "Previous section in reading order")]   [[\>](#NetCDF_002d3-Variable-Types "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.2 Language Types Corresponding to netCDF external data types {.section}
--------------------------------------------------------------

NetCDF supported six atomic data types through version 3.6.0 (char,
byte, short, int, float, and double). Starting with version 4.0, many
new atomic and user defined data types are supported (unsigned int
types, strings, compound types, variable length arrays, enums, opaque).

The additional data types are only supported in netCDF-4/HDF5 files. To
create netCDF-4/HDF5 files, use the HDF5 flag in nc\_create. (see
section [Create a NetCDF Dataset: nc\_create](#nc_005fcreate)).

* * * * *

  ------------------------------------------------------------- -------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Variable-Types "Previous section in reading order")]   [[\>](#NetCDF_002d4-Atomic-Types "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- -------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.3 NetCDF-3 Classic and 64-Bit Offset Data Types {.section}
-------------------------------------------------

NetCDF-3 classic and 64-bit offset files support 6 atomic data types,
and none of the user defined datatype introduced in NetCDF-4.

The following table gives the netCDF-3 external data types and the
corresponding type constants for defining variables in the C interface:

  -------- ------------ ------
  Type     C \#define   Bits
  byte     NC\_BYTE     8
  char     NC\_CHAR     8
  short    NC\_SHORT    16
  int      NC\_INT      32
  float    NC\_FLOAT    32
  double   NC\_DOUBLE   64
  -------- ------------ ------

The first column gives the netCDF external data type, which is the same
as the CDL data type. The next column gives the corresponding C
pre-processor macro for use in netCDF functions (the pre-processor
macros are defined in the netCDF C header-file netcdf.h). The last
column gives the number of bits used in the external representation of
values of the corresponding type.

* * * * *

  -------------------------------------------------------------------------- ------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF_002d3-Variable-Types "Previous section in reading order")]   [[\>](#nc_005fdef_005fvar "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.4 NetCDF-4 Atomic Types {.section}
-------------------------

NetCDF-4 files support all of the atomic data types from netCDF-3, plus
additional unsigned integer types, 64-bit integer types, and a string
type.

  -------------------- -------------- -------------------
  Type                 C \#define     Bits
  byte                 NC\_BYTE       8
  unsigned byte        NC\_UBYTE\^    8
  char                 NC\_CHAR       8
  short                NC\_SHORT      16
  unsigned short       NC\_USHORT\^   16
  int                  NC\_INT        32
  unsigned int         NC\_UINT\^     32
  unsigned long long   NC\_UINT64\^   64
  long long            NC\_INT64\^    64
  float                NC\_FLOAT      32
  double               NC\_DOUBLE     64
  char \*\*            NC\_STRING\^   string length + 1
  -------------------- -------------- -------------------

\^This type was introduced in netCDF-4, and is not supported in netCDF
classic or 64-bit offset format files, or in netCDF-4 files if they are
created with the NC\_CLASSIC\_MODEL flags.

* * * * *

  ------------------------------------------------------------------------ -------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF_002d4-Atomic-Types "Previous section in reading order")]   [[\>](#nc_005fdef_005fvar_005fchunking "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ -------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.5 Create a Variable: `nc_def_var` {.section}
-----------------------------------

The function nc\_def\_var adds a new variable to an open netCDF dataset
in define mode. It returns (as an argument) a variable ID, given the
netCDF ID, the variable name, the variable type, the number of
dimensions, and a list of the dimension IDs.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_def_var (int ncid, const char *name, nc_type xtype,
                int ndims, const int dimids[], int *varidp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `name`
:   Variable name.

 `xtype`
:   One of the set of predefined netCDF external data types. The type of
    this parameter, nc\_type, is defined in the netCDF header file. The
    valid netCDF external data types are NC\_BYTE, NC\_CHAR, NC\_SHORT,
    NC\_INT, NC\_FLOAT, and NC\_DOUBLE. If the file is a NetCDF-4/HDF5
    file, the additional types NC\_UBYTE, NC\_USHORT, NC\_UINT,
    NC\_INT64, NC\_UINT64, and NC\_STRING may be used, as well as a user
    defined type ID.

 `ndims`
:   Number of dimensions for the variable. For example, 2 specifies a
    matrix, 1 specifies a vector, and 0 means the variable is a scalar
    with no dimensions. Must not be negative or greater than the
    predefined constant NC\_MAX\_VAR\_DIMS.

 `dimids`
:   Vector of ndims dimension IDs corresponding to the variable
    dimensions. For classic model netCDF files, if the ID of the
    unlimited dimension is included, it must be first. This argument is
    ignored if ndims is 0. For expanded model netCDF4/HDF5 files, there
    may be any number of unlimited dimensions, and they may be used in
    any element of the dimids array.

 `varidp`
:   Pointer to location for the returned variable ID.

Errors {.heading}
------

nc\_def\_var returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTINDEFINE`
:   Not in define mode. This is returned for netCDF classic or 64-bit
    offset files, or for netCDF-4 files, when they were been created
    with NC\_STRICT\_NC3 flag. (see section [Create a NetCDF Dataset:
    nc\_create](#nc_005fcreate)).

 `NC_ESTRICTNC3`
:   Trying to create a var some place other than the root group in a
    netCDF file with NC\_STRICT\_NC3 turned on.

 `NC_MAX_VARS`
:   Max number of variables exceeded in a classic or 64-bit offset file,
    or an netCDF-4 file with NC\_STRICT\_NC3 on.

 `NC_EBADTYPE`
:   Bad type.

 `NC_EINVAL`
:   Number of dimensions to large.

 `NC_ENAMEINUSE`
:   Name already in use.

 `NC_EPERM`
:   Attempt to create object in read-only file.

Example {.heading}
-------

Here is an example using nc\_def\_var to create a variable named rh of
type double with three dimensions, time, lat, and lon in a new netCDF
dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;                       /* error status */
int  ncid;                         /* netCDF ID */
int  lat_dim, lon_dim, time_dim;   /* dimension IDs */
int  rh_id;                        /* variable ID */
int  rh_dimids[3];                 /* variable shape */
   ... 
status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
                                   /* define dimensions */
status = nc_def_dim(ncid, "lat", 5L, &lat_dim);
if (status != NC_NOERR) handle_error(status);
status = nc_def_dim(ncid, "lon", 10L, &lon_dim);
if (status != NC_NOERR) handle_error(status);
status = nc_def_dim(ncid, "time", NC_UNLIMITED, &time_dim);
if (status != NC_NOERR) handle_error(status);
   ... 
                                   /* define variable */
rh_dimids[0] = time_dim;
rh_dimids[1] = lat_dim;
rh_dimids[2] = lon_dim;
status = nc_def_var (ncid, "rh", NC_DOUBLE, 3, rh_dimids, &rh_id);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ----------------------------------------------------------------- -------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fvar "Previous section in reading order")]   [[\>](#nc_005finq_005fvar_005fchunking "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- -------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.6 Define Chunking Parameters for a Variable: `nc_def_var_chunking` {.section}
--------------------------------------------------------------------

The function nc\_def\_var\_chunking sets the chunking parameters for a
variable in a netCDF-4 file. It can set the chunk sizes to get chunked
storage, or it can set the contiguous flag to get contiguous storage.

Variables that make use of one or more unlimited dimensions,
compression, or checksums must use chunking. Such variables are created
with default chunk sizes of 1 for each unlimited dimension and the
dimension length for other dimensions, except that if the resulting
chunks are too large, the default chunk sizes for non-record dimensions
are reduced.

The total size of a chunk must be less than 4 GiB. That is, the product
of all chunksizes and the size of the data (or the size of nc\_vlen\_t
for VLEN types) must be less than 4 GiB.

This function may only be called after the variable is defined, but
before nc\_enddef is called. Once the chunking parameters are set for a
variable, they cannot be changed. This function can be used to change
the default chunking for record, compressed, or checksummed variables
before nc\_enddef is called.

Note that you cannot set chunking for scalar variables. Only non-scalar
variables can have chunking.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_def_var_chunking(int ncid, int varid, int storage, size_t *chunksizesp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `storage`
:   If NC\_CONTIGUOUS, then contiguous storage is used for this
    variable. Variables with chunking, compression, checksums, or one or
    more unlimited dimensions cannot use contiguous storage.

    If NC\_CHUNKED, then chunked storage is used for this variable.
    Chunk sizes may be specified with the chunksizes parameter or
    default sizes will be used if that parameter is NULL.

    By default contiguous storage is used for fix-sized variables when
    conpression, chunking, and checksums are not used.

 `*chunksizes`
:   A pointer to an array list of chunk sizes. The array must have one
    chunksize for each dimension of the variable.

Errors {.heading}
------

nc\_def\_var\_chunking returns the value NC\_NOERR if no errors
occurred. Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_EBADID`
:   Bad ncid.

 `NC_EINVAL`
:   Invalid input. This can occur if contiguous storage is set on a
    variable which uses compression, checksums, or one or more unlimited
    dimensions.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

 `NC_ELATEDEF`
:   This variable has already been the subject of a nc\_enddef call. In
    netCDF-4 files nc\_enddef will be called automatically for any data
    read or write. Once nc\_enddef has been called after the
    nc\_def\_var call for a variable, it is impossible to set the
    chunking for that variable.

 `NC_ENOTINDEFINE`
:   Not in define mode. This is returned for netCDF classic or 64-bit
    offset files, or for netCDF-4 files, when they were been created
    with NC\_STRICT\_NC3 flag. (see section [Create a NetCDF Dataset:
    nc\_create](#nc_005fcreate)).

 `NC_ESTRICTNC3`
:   Trying to create a var some place other than the root group in a
    netCDF file with NC\_STRICT\_NC3 turned on.

 `NC_EPERM`
:   Attempt to create object in read-only file.

Example {.heading}
-------

In this example from libsrc4/tst\_vars2.c, chunksizes are set with
nc\_var\_def\_chunking, and checked with nc\_var\_inq\_chunking.

 

~~~~ {.example}
   printf("**** testing chunking...");
   {
#define NDIMS5 1
#define DIM5_NAME "D5"
#define VAR_NAME5 "V5"
#define DIM5_LEN 1000

      int dimids[NDIMS5], dimids_in[NDIMS5];
      int varid;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data[DIM5_LEN], data_in[DIM5_LEN];
      size_t chunksize[NDIMS5] = {5};
      size_t chunksize_in[NDIMS5];
      int storage_in;
      int i, d;

      for (i = 0; i < DIM5_LEN; i++)
         data[i] = i;

      /* Create a netcdf-4 file with one dim and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR_NAME5, NC_INT, NDIMS5, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunksize)) ERR;
      if (nc_put_var_int(ncid, varid, data)) ERR;

      /* Check stuff. */
      if (nc_inq_var_chunking(ncid, varid, &storage_in, chunksize_in)) ERR;
      for (d = 0; d < NDIMS5; d++)
         if (chunksize[d] != chunksize_in[d]) ERR;
      if (storage_in != NC_CHUNKED) ERR;
~~~~

* * * * *

  ------------------------------------------------------------------------------ --------------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fvar_005fchunking "Previous section in reading order")]   [[\>](#nc_005fset_005fvar_005fchunk_005fcache "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------ --------------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.7 Learn About Chunking Parameters for a Variable: `nc_inq_var_chunking` {.section}
-------------------------------------------------------------------------

The function nc\_inq\_var\_chunking returns the chunking settings for a
variable in a netCDF-4 file.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_var_chunking(int ncid, int varid, int *storagep, size_t *chunksizesp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `storagep`
:   Address of returned storage property, returned as NC\_CONTIGUOUS if
    this variable uses contiguous storage, or NC\_CHUNKEDif it uses
    chunked storage.

 `*chunksizesp`
:   A pointer to an array list of chunk sizes. The array must have one
    chunksize for each dimension in the variable.

Errors {.heading}
------

nc\_inq\_var\_chunking returns the value NC\_NOERR if no errors
occurred. Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

Example {.heading}
-------

This example is from libsrc4/tst\_vars2.c in which a variable with
contiguous storage is created, and then checked with
nc\_inq\_var\_chunking:

 

~~~~ {.example}
   printf("**** testing contiguous storage...");
   {
#define NDIMS6 1
#define DIM6_NAME "D5"
#define VAR_NAME6 "V5"
#define DIM6_LEN 100

      int dimids[NDIMS6], dimids_in[NDIMS6];
      int varid;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data[DIM6_LEN], data_in[DIM6_LEN];
      size_t chunksize_in[NDIMS6];
      int storage_in;
      int i, d;

      for (i = 0; i < DIM6_LEN; i++)
         data[i] = i;

      /* Create a netcdf-4 file with one dim and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM6_NAME, DIM6_LEN, &dimids[0])) ERR;
      if (dimids[0] != 0) ERR;
      if (nc_def_var(ncid, VAR_NAME6, NC_INT, NDIMS6, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CONTIGUOUS, NULL)) ERR;
      if (nc_put_var_int(ncid, varid, data)) ERR;

      /* Check stuff. */
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
      if (storage_in != NC_CONTIGUOUS) ERR;
~~~~

* * * * *

  ------------------------------------------------------------------------------ --------------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvar_005fchunking "Previous section in reading order")]   [[\>](#nc_005fget_005fvar_005fchunk_005fcache "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------ --------------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.8 Set HDF5 Chunk Cache for a Variable: nc\_set\_var\_chunk\_cache {.section}
-------------------------------------------------------------------

This function changes the chunk cache settings for a variable. The
change in cache size happens immediately. This is a property of the open
file - it does not persist the next time you open the file.

For more information, see the documentation for the H5Pset\_cache()
function in the HDF5 library at the HDF5 website:
[http://hdfgroup.org/HDF5/](http://hdfgroup.org/HDF5/).

Usage {.heading}
-----

 

~~~~ {.example}
nc_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, 
               float preemption);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `size`
:   The total size of the raw data chunk cache, in bytes. This should be
    big enough to hold multiple chunks of data.

 `nelems`
:   The number of chunk slots in the raw data chunk cache hash table.
    This should be a prime number larger than the number of chunks that
    will be in the cache.

 `preemption`
:   The preemtion value must be between 0 and 1 inclusive and indicates
    how much chunks that have been fully read are favored for
    preemption. A value of zero means fully read chunks are treated no
    differently than other chunks (the preemption is strictly LRU) while
    a value of one means fully read chunks are always preempted before
    other chunks.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_EINVAL`
:   Preemption must be between zero and one (inclusive).

Example {.heading}
-------

This example is from libsrc4/tst\_vars2.c:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define CACHE_SIZE 32000000
#define CACHE_NELEMS 1009
#define CACHE_PREEMPTION .75
   ... 

      /* Create a netcdf-4 file with one dim and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimids[0])) ERR;
      if (dimids[0] != 0) ERR;
      if (nc_def_var(ncid, VAR_NAME5, NC_INT, NDIMS5, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunksize)) ERR;
      if (nc_set_var_chunk_cache(ncid, varid, CACHE_SIZE, CACHE_NELEMS, CACHE_PREEMPTION)) ERR;
~~~~

* * * * *

  ------------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fset_005fvar_005fchunk_005fcache "Previous section in reading order")]   [[\>](#nc_005fdef_005fvar_005ffill "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.9 Get the HDF5 Chunk Cache Settings for a Variable: nc\_get\_var\_chunk\_cache {.section}
--------------------------------------------------------------------------------

This function gets the current chunk cache settings for a variable in a
netCDF-4/HDF5 file.

For more information, see the documentation for the H5Pget\_cache()
function in the HDF5 library at the HDF5 website:
[http://hdfgroup.org/HDF5/](http://hdfgroup.org/HDF5/).

Usage {.heading}
-----

 

~~~~ {.example}
int nc_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp, 
                           float *preemptionp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `sizep`
:   The total size of the raw data chunk cache, in bytes, will be put
    here. If NULL, will be ignored.

 `nelemsp`
:   The number of chunk slots in the raw data chunk cache hash table
    will be put here. If NULL, will be ignored.

 `preemptionp`
:   The preemption will be put here. The preemtion value is between 0
    and 1 inclusive and indicates how much chunks that have been fully
    read are favored for preemption. A value of zero means fully read
    chunks are treated no differently than other chunks (the preemption
    is strictly LRU) while a value of one means fully read chunks are
    always preempted before other chunks. If NULL, will be ignored.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

Example {.heading}
-------

This example is from libsrc4/tst\_vars2.c:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
      /* Create a netcdf-4 file with one dim and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR_NAME5, NC_INT, NDIMS5, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunksize)) ERR;
      if (nc_set_var_chunk_cache(ncid, varid, CACHE_SIZE, CACHE_NELEMS, 
                                 CACHE_PREEMPTION)) ERR;
   ...
      if (nc_get_var_chunk_cache(ncid, varid, &cache_size_in, &cache_nelems_in, 
                 &cache_preemption_in)) ERR;
      if (cache_size_in != CACHE_SIZE || cache_nelems_in != CACHE_NELEMS ||
      cache_preemption_in != CACHE_PREEMPTION) ERR;

   ... 
~~~~

* * * * *

  ------------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fget_005fvar_005fchunk_005fcache "Previous section in reading order")]   [[\>](#nc_005finq_005fvar_005ffill "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.10 Define Fill Parameters for a Variable: `nc_def_var_fill` {.section}
-------------------------------------------------------------

The function nc\_def\_var\_fill sets the fill parameters for a variable
in a netCDF-4 file.

This function must be called after the variable is defined, but before
nc\_enddef is called.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_def_var_fill(int ncid, int varid, int no_fill, void *fill_value);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `no_fill`
:   Set no\_fill mode on a variable. When this mode is on, fill values
    will not be written for the variable. This is helpful in high
    performance applications. For netCDF-4/HDF5 files (whether classic
    model or not), this may only be changed after the variable is
    defined, but before it is committed to disk (i.e. before the first
    nc\_enddef after the nc\_def\_var.) For classic and 64-bit offset
    file, the no\_fill mode may be turned on and off at any time.

 `*fill_value`
:   A pointer to a value which will be used as the fill value for the
    variable. Must be the same type as the variable. This will be
    written to a \_FillValue attribute, created for this purpose. If
    NULL, this argument will be ignored.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

 `NC_ELATEDEF`
:   This variable has already been the subject of a nc\_enddef call. In
    netCDF-4 files nc\_enddef will be called automatically for any data
    read or write. Once enddef has been called, it is impossible to set
    the fill for a variable.

 `NC_ENOTINDEFINE`
:   Not in define mode. This is returned for netCDF classic or 64-bit
    offset files, or for netCDF-4 files, when they were been created
    with NC\_STRICT\_NC3 flag. (see section [Create a NetCDF Dataset:
    nc\_create](#nc_005fcreate)).

 `NC_EPERM`
:   Attempt to create object in read-only file.

Example {.heading}
-------

This example is from libsrc4/tst\_vars.c

 

~~~~ {.example}
      int dimids[NDIMS];
      size_t index[NDIMS];
      int varid;
      int no_fill;
      unsigned short ushort_data = 42, ushort_data_in, fill_value_in;

      /* Create a netcdf-4 file with one dim and 1 NC_USHORT var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM7_NAME, DIM7_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR7_NAME, NC_USHORT, NDIMS, dimids,
             &varid)) ERR;
      if (nc_def_var_fill(ncid, varid, 1, NULL)) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------------- ------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fvar_005ffill "Previous section in reading order")]   [[\>](#nc_005fdef_005fvar_005fdeflate "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.11 Learn About Fill Parameters for a Variable: `nc_inq_var_fill` {.section}
------------------------------------------------------------------

The function nc\_inq\_var\_fill returns the fill settings for a variable
in a netCDF-4 file.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_var_fill(int ncid, int varid, int *no_fill, void *fill_value);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `*no_fill`
:   Pointer to an integer which will get a 1 if no\_fill mode is set for
    this variable. See section [Define Fill Parameters for a Variable:
    `nc_def_var_fill`](#nc_005fdef_005fvar_005ffill). This parameter
    will be ignored if it is NULL.

 `*fill_value`
:   A pointer which will get the fill value for this variable. This
    parameter will be ignored if it is NULL.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

Example {.heading}
-------

This example is from libsrc4/tst\_vars.c

 

~~~~ {.example}
      int dimids[NDIMS];
      size_t index[NDIMS];
      int varid;
      int no_fill;
      unsigned short ushort_data = 42, ushort_data_in, fill_value_in;

      /* Create a netcdf-4 file with one dim and 1 NC_USHORT var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM7_NAME, DIM7_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR7_NAME, NC_USHORT, NDIMS, dimids,
             &varid)) ERR;
      if (nc_def_var_fill(ncid, varid, 1, NULL)) ERR;

      /* Check stuff. */
      if (nc_inq_var_fill(ncid, varid, &no_fill, &fill_value_in)) ERR;
      if (!no_fill) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------------- ------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvar_005ffill "Previous section in reading order")]   [[\>](#nc_005finq_005fvar_005fdeflate "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.12 Define Compression Parameters for a Variable: `nc_def_var_deflate` {.section}
-----------------------------------------------------------------------

The function nc\_def\_var\_deflate sets the deflate parameters for a
variable in a netCDF-4 file.

This function must be called after the variable is defined, but before
nc\_enddef is called.

This does not work with scalar variables.

Usage {.heading}
-----

 

~~~~ {.example}
nc_def_var_deflate(int ncid, int varid, int shuffle, int deflate, 
                   int deflate_level);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `shuffle`
:   If non-zero, turn on the shuffle filter.

 `deflate`
:   If non-zero, turn on the deflate filter at the level specified by
    the deflate\_level parameter.

 `deflate_level`
:   If the deflate parameter is non-zero, set the deflate level to this
    value. Must be between 0 and 9.

Errors {.heading}
------

nc\_def\_var\_deflate returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

 `NC_ELATEDEF`
:   This variable has already been the subject of a nc\_enddef call. In
    netCDF-4 files nc\_enddef will be called automatically for any data
    read or write. Once enddef has been called, it is impossible to set
    the deflate for a variable.

 `NC_ENOTINDEFINE`
:   Not in define mode. This is returned for netCDF classic or 64-bit
    offset files, or for netCDF-4 files, when they were been created
    with NC\_STRICT\_NC3 flag. (see section [Create a NetCDF Dataset:
    nc\_create](#nc_005fcreate)).

 `NC_EPERM`
:   Attempt to create object in read-only file.

 `NC_EINVAL`
:   Invalid deflate\_level. The deflate level must be between 0 and 9,
    inclusive.

Example {.heading}
-------

* * * * *

  ----------------------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fvar_005fdeflate "Previous section in reading order")]   [[\>](#nc_005finq_005fvar_005fszip "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------- ---------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.13 Learn About Deflate Parameters for a Variable: `nc_inq_var_deflate` {.section}
------------------------------------------------------------------------

The function nc\_inq\_var\_deflate returns the deflate settings for a
variable in a netCDF-4 file.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_var_deflate(int ncid, int varid, int *shufflep, 
                   int *deflatep, int *deflate_levelp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `*shufflep`
:   If this pointer is non-NULL, the nc\_inq\_var\_deflate function will
    write a 1 if the shuffle filter is turned on for this variable, and
    a 0 otherwise.

 `*deflatep`
:   If this pointer is non-NULL, the nc\_inq\_var\_deflate function will
    write a 1 if the deflate filter is turned on for this variable, and
    a 0 otherwise.

 `*deflate_levelp`
:   If this pointer is non-NULL, and the deflate filter is in use for
    this variable, the nc\_inq\_var\_deflate function will write the
    deflate\_level here.

Errors {.heading}
------

nc\_inq\_var\_deflate returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

Example {.heading}
-------

* * * * *

  ----------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvar_005fdeflate "Previous section in reading order")]   [[\>](#nc_005fdef_005fvar_005ffletcher32 "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.14 Learn About Szip Parameters for a Variable: `nc_inq_var_szip` {.section}
------------------------------------------------------------------

The function nc\_inq\_var\_szip returns the szip settings for a variable
in a netCDF-4 file.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_var_szip(int ncid, int varid, int *options_maskp, int *pixels_per_blockp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `*options_maskp`
:   If this pointer is non-NULL, the nc\_inq\_var\_szip function will
    put the options\_mask here.

 `*pixels_per_blockp`
:   If this pointer is non-NULL, the nc\_inq\_var\_szip function will
    write the bits per pixel here.

Errors {.heading}
------

nc\_inq\_var\_szip returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

Example {.heading}
-------

This example is from libsrc4/tst\_vars3.c.

 

~~~~ {.example}
      /* Make sure we have the szip settings we expect. */
      if (nc_inq_var_szip(ncid, small_varid, &options_mask_in, &pixels_per_block_in)) ERR;
      if (options_mask_in != 0 || pixels_per_block_in !=0) ERR;
      if (nc_inq_var_szip(ncid, medium_varid, &options_mask_in, &pixels_per_block_in)) ERR;
      if (!(options_mask_in & NC_SZIP_EC_OPTION_MASK) || pixels_per_block_in != 32) ERR;
      if (nc_inq_var_szip(ncid, large_varid, &options_mask_in, &pixels_per_block_in)) ERR;
      if (!(options_mask_in & NC_SZIP_NN_OPTION_MASK) || pixels_per_block_in != 16) ERR;
~~~~

* * * * *

  -------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvar_005fszip "Previous section in reading order")]   [[\>](#nc_005finq_005fvar_005ffletcher32 "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- ---------------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.15 Define Checksum Parameters for a Variable: `nc_def_var_fletcher32` {.section}
-----------------------------------------------------------------------

The function nc\_def\_var\_fletcher32 sets the checksum parameters for a
variable in a netCDF-4 file.

This function may only be called after the variable is defined, but
before nc\_enddef is called.

Usage {.heading}
-----

 

~~~~ {.example}
nc_def_var_fletcher32(int ncid, int varid, int checksum);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `checksum`
:   If this is NC\_FLETCHER32, fletcher32 checksums will be turned on
    for this variable.

Errors {.heading}
------

nc\_def\_var\_fletcher32 returns the value NC\_NOERR if no errors
occurred. Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

 `NC_ELATEDEF`
:   This variable has already been the subject of a nc\_enddef call. In
    netCDF-4 files nc\_enddef will be called automatically for any data
    read or write. Once enddef has been called, it is impossible to set
    the checksum property for a variable.

 `NC_ENOTINDEFINE`
:   Not in define mode. This is returned for netCDF classic or 64-bit
    offset files, or for netCDF-4 files, when they were been created
    with NC\_STRICT\_NC3 flag. (see section [Create a NetCDF Dataset:
    nc\_create](#nc_005fcreate)).

 `NC_EPERM`
:   Attempt to create object in read-only file.

Example {.heading}
-------

* * * * *

  -------------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fvar_005ffletcher32 "Previous section in reading order")]   [[\>](#nc_005fdef_005fvar_005fendian "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.16 Learn About Checksum Parameters for a Variable: `nc_inq_var_fletcher32` {.section}
----------------------------------------------------------------------------

The function nc\_inq\_var\_fletcher32 returns the checksum settings for
a variable in a netCDF-4 file.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_var_fletcher32(int ncid, int varid, int *checksump);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `*checksump`
:   If not-NULL, the nc\_inq\_var\_fletcher32 function will set the int
    pointed at to NC\_FLETCHER32 if the fletcher32 checksum filter is
    turned on for this variable, and NC\_NOCHECKSUM if it is not.

Errors {.heading}
------

nc\_inq\_var\_fletcher32 returns the value NC\_NOERR if no errors
occurred. Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

Example {.heading}
-------

* * * * *

  -------------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvar_005ffletcher32 "Previous section in reading order")]   [[\>](#nc_005finq_005fvar_005fendian "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.17 Define Endianness of a Variable: `nc_def_var_endian` {.section}
---------------------------------------------------------

The function nc\_def\_var\_endian sets the endianness for a variable in
a netCDF-4 file.

This function must be called after the variable is defined, but before
nc\_enddef is called.

By default, netCDF-4 variables are in native endianness. That is, they
are big-endian on a big-endian machine, and little-endian on a little
endian machine.

In some cases a user might wish to change from native endianness to
either big or little-endianness. This function allows them to do that.

Usage {.heading}
-----

 

~~~~ {.example}
nc_def_var_endian(int ncid, int varid, int endian);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `endian`
:   Set to NC\_ENDIAN\_NATIVE for native endianness. (This is the
    default). Set to NC\_ENDIAN\_LITTLE for little endian, or
    NC\_ENDIAN\_BIG for big endian.

Errors {.heading}
------

nc\_def\_var\_endian returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

 `NC_ELATEDEF`
:   This variable has already been the subject of a nc\_enddef call. In
    netCDF-4 files nc\_enddef will be called automatically for any data
    read or write. Once enddef has been called, it is impossible to set
    the endianness of a variable.

 `NC_ENOTINDEFINE`
:   Not in define mode. This is returned for netCDF classic or 64-bit
    offset files, or for netCDF-4 files, when they were been created
    with NC\_STRICT\_NC3 flag, and the file is not in define mode. (see
    section [Create a NetCDF Dataset: nc\_create](#nc_005fcreate)).

 `NC_EPERM`
:   Attempt to create object in read-only file.

Example {.heading}
-------

* * * * *

  ---------------------------------------------------------------------------- --------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdef_005fvar_005fendian "Previous section in reading order")]   [[\>](#nc_005finq_005fvarid "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- --------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.18 Learn About Endian Parameters for a Variable: `nc_inq_var_endian` {.section}
----------------------------------------------------------------------

The function nc\_inq\_var\_endian returns the endianness settings for a
variable in a netCDF-4 file.

Usage {.heading}
-----

 

~~~~ {.example}
nc_inq_var_endian(int ncid, int varid, int *endianp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `*endianp`
:   If not-NULL, the nc\_inq\_var\_endian function will set the int
    pointed to this to NC\_ENDIAN\_LITTLE if this variable is stored in
    little-endian format, NC\_ENDIAN\_BIG if it is stored in big-endian
    format, and NC\_ENDIAN\_NATIVE if the endianness is not set, and the
    variable is not created yet.

Errors {.heading}
------

nc\_inq\_var\_endian returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error.

Possible return codes include:

 `NC_NOERR`
:   No error.

 `NC_BADID`
:   Bad ncid.

 `NC_ENOTNC4`
:   Not a netCDF-4 file.

 `NC_ENOTVAR`
:   Can’t find this variable.

Example {.heading}
-------

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvar_005fendian "Previous section in reading order")]   [[\>](#nc_005finq_005fvar "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.19 Get a Variable ID from Its Name: nc\_inq\_varid {.section}
----------------------------------------------------

The function nc\_inq\_varid returns the ID of a netCDF variable, given
its name.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_varid (int ncid, const char *name, int *varidp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `name`
:   Variable name for which ID is desired.

 `varidp`
:   Pointer to location for returned variable ID.

Errors {.heading}
------

nc\_inq\_varid returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The specified variable name is not a valid name for a variable in
    the specified netCDF dataset.
-   The specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_inq\_varid to find out the ID of a variable
named rh in an existing netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status, ncid, rh_id;
   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvarid "Previous section in reading order")]   [[\>](#nc_005fput_005fvar1_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.20 Get Information about a Variable from Its ID: nc\_inq\_var {.section}
---------------------------------------------------------------

family &findex nc\_inq\_vardimid

A family of functions that returns information about a netCDF variable,
given its ID. Information about a variable includes its name, type,
number of dimensions, a list of dimension IDs describing the shape of
the variable, and the number of variable attributes that have been
assigned to the variable.

The function nc\_inq\_var returns all the information about a netCDF
variable, given its ID. The other functions each return just one item of
information about a variable.

These other functions include nc\_inq\_varname, nc\_inq\_vartype,
nc\_inq\_varndims, nc\_inq\_vardimid, and nc\_inq\_varnatts.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_var      (int ncid, int varid, char *name, nc_type *xtypep,
                     int *ndimsp, int dimids[], int *nattsp);
int nc_inq_varname  (int ncid, int varid, char *name);
int nc_inq_vartype  (int ncid, int varid, nc_type *xtypep);
int nc_inq_varndims (int ncid, int varid, int *ndimsp);
int nc_inq_vardimid (int ncid, int varid, int dimids[]);
int nc_inq_varnatts (int ncid, int varid, int *nattsp);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `name`
:   Returned variable name. The caller must allocate space for the
    returned name. The maximum possible length, in characters, of a
    variable name is given by the predefined constant NC\_MAX\_NAME.
    (This doesn’t include the null terminator, so declare your array to
    be size NC\_MAX\_NAME+1). The returned character array will be
    null-terminated.

 `xtypep`
:   Pointer to location for returned variable type, one of the set of
    predefined netCDF external data types. The type of this parameter,
    nc\_type, is defined in the netCDF header file. The valid netCDF
    external data types are NC\_BYTE, NC\_CHAR, NC\_SHORT, NC\_INT,
    NC\_FLOAT, and NC\_DOUBLE.

 `ndimsp`
:   Pointer to location for returned number of dimensions the variable
    was defined as using. For example, 2 indicates a matrix, 1 indicates
    a vector, and 0 means the variable is a scalar with no dimensions.

 `dimids`
:   Returned vector of \*ndimsp dimension IDs corresponding to the
    variable dimensions. The caller must allocate enough space for a
    vector of at least \*ndimsp integers to be returned. The maximum
    possible number of dimensions for a variable is given by the
    predefined constant NC\_MAX\_VAR\_DIMS.

 `nattsp`
:   Pointer to location for returned number of variable attributes
    assigned to this variable.

These functions return the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

The variable ID is invalid for the specified netCDF dataset. The
specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_inq\_var to find out about a variable named
rh in an existing netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status                        /* error status */
int  ncid;                         /* netCDF ID */
int  rh_id;                        /* variable ID */
nc_type rh_type;                   /* variable type */
int rh_ndims;                      /* number of dims */
int  rh_dimids[NC_MAX_VAR_DIMS];   /* dimension IDs */
int rh_natts                       /* number of attributes */
   ... 
status = nc_open ("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
/* we don't need name, since we already know it */
status = nc_inq_var (ncid, rh_id, 0, &rh_type, &rh_ndims, rh_dimids,
                     &rh_natts);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ----------------------------------------------------------------- ----------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fvar "Previous section in reading order")]   [[\>](#nc_005fput_005fvar_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- ----------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.21 Write a Single Data Value: nc\_put\_var1\_ type {.section}
----------------------------------------------------

The functions nc\_put\_var1\_ type put a single data value of the
specified type into a variable of an open netCDF dataset that is in data
mode. Inputs are the netCDF ID, the variable ID, an index that specifies
which value to add or alter, and the data value. The value is converted
to the external data type of the variable, if necessary.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_put\_var1() function will write a variable of any type,
including user defined type. For this function, the type of the data in
memory must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_put_var1_text  (int ncid, int varid, const size_t index[],
                       const char *tp);
int nc_put_var1_uchar (int ncid, int varid, const size_t index[],
                       const unsigned char *up);
int nc_put_var1_schar (int ncid, int varid, const size_t index[],
                       const signed char *cp);
int nc_put_var1_short (int ncid, int varid, const size_t index[],
                       const short *sp);
int nc_put_var1_int   (int ncid, int varid, const size_t index[],
                       const int *ip);
int nc_put_var1_long  (int ncid, int varid, const size_t index[],
                       const long *lp);
int nc_put_var1_float (int ncid, int varid, const size_t index[],
                       const float *fp); 
int nc_put_var1_double(int ncid, int varid, const size_t index[],
                       const double *dp);
int nc_put_var1_ushort(int ncid, int varid, const size_t index[],
                       const unsigned short *sp);
int nc_put_var1_uint  (int ncid, int varid, const size_t index[],
                       const unsigned int *ip);
int nc_put_var1_longlong(int ncid, int varid, const size_t index[],
                         const long long *ip);
int nc_put_var1_ulonglong(int ncid, int varid, const size_t index[],
                         const unsigned long long *ip);
int nc_put_var1_string(int ncid, int varid, const size_t index[],
                       const char **ip);
int nc_put_var1(int ncid, int varid, const size_t *indexp,
                const void *op);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `index[]`
:   The index of the data value to be written. The indices are relative
    to 0, so for example, the first data value of a two-dimensional
    variable would have index (0,0). The elements of index must
    correspond to the variable’s dimensions. Hence, if the variable uses
    the unlimited dimension, the first index would correspond to the
    unlimited dimension.

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to the data value to be written. If the type of data values
    differs from the netCDF variable type, type conversion will occur.
    See [(netcdf)Type Conversion](netcdf.html#Type-Conversion) section
    ‘Type Conversion’ in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified indices were out of range for the
    rank of the specified variable. For example, a negative index or an
    index that is larger than the corresponding dimension length will
    cause an error.
-   NC\_ERANGE The specified value is out of the range of values
    representable by the external data type of the variable. (Does not
    apply to nc\_put\_var1() function).
-   NC\_EINDEFINE The specified netCDF is in define mode rather than
    data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.

Example {.heading}
-------

Here is an example using nc\_put\_var1\_double to set the (1,2,3)
element of the variable named rh to 0.5 in an existing netCDF dataset
named foo.nc. For simplicity in this example, we assume that we know
that rh is dimensioned with time, lat, and lon, so we want to set the
value of rh that corresponds to the second time value, the third lat
value, and the fourth lon value:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;                          /* error status */
int  ncid;                            /* netCDF ID */
int  rh_id;                           /* variable ID */
static size_t rh_index[] = {1, 2, 3}; /* where to put value */
static double rh_val = 0.5;           /* value to put */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_put_var1_double(ncid, rh_id, rh_index, &rh_val);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fput_005fvar1_005f-type "Previous section in reading order")]   [[\>](#nc_005fput_005fvara_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.22 Write an Entire Variable: nc\_put\_var\_ type {.section}
--------------------------------------------------

The nc\_put\_var\_ type family of functions write all the values of a
variable into a netCDF variable of an open netCDF dataset. This is the
simplest interface to use for writing a value in a scalar variable or
whenever all the values of a multidimensional variable can all be
written at once. The values to be written are associated with the netCDF
variable by assuming that the last dimension of the netCDF variable
varies fastest in the C interface. The values are converted to the
external data type of the variable, if necessary.

Take care when using the simplest forms of this interface with record
variables (variables that use the NC\_UNLIMITED dimension) when you
don’t specify how many records are to be written. If you try to write
all the values of a record variable into a netCDF file that has no
record data yet (hence has 0 records), nothing will be written.
Similarly, if you try to write all the values of a record variable but
there are more records in the file than you assume, more in-memory data
will be accessed than you supply, which may result in a segmentation
violation. To avoid such problems, it is better to use the nc\_put\_vara
interfaces for variables that use the NC\_UNLIMITED dimension. See
section [Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type).

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_put\_var() function will write a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_put_var_text  (int ncid, int varid, const char *tp);
int nc_put_var_uchar (int ncid, int varid, const unsigned char *up);
int nc_put_var_schar (int ncid, int varid, const signed char *cp);
int nc_put_var_short (int ncid, int varid, const short *sp);
int nc_put_var_int   (int ncid, int varid, const int *ip);
int nc_put_var_long  (int ncid, int varid, const long *lp);
int nc_put_var_float (int ncid, int varid, const float *fp);
int nc_put_var_double(int ncid, int varid, const double *dp);
int nc_put_var_ushort(int ncid, int varid, const unsigned short *op);
int nc_put_var_uint  (int ncid, int varid, const unsigned int *op);
int nc_put_var_longlong (int ncid, int varid, const long long *op);
int nc_put_var_ulonglong(int ncid, int varid, const unsigned long long *op);
int nc_put_var_string(int ncid, int varid, const char **op);
int nc_put_var       (int ncid, int varid,  const void *op);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to a block of contiguous data values to be written. The
    order in which the data will be written to the netCDF variable is
    with the last dimension of the specified variable varying fastest.
    If the type of data values differs from the netCDF variable type,
    type conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ERANGE One or more of the specified values are out of the range
    of values representable by the external data type of the variable.
    (Does not apply to nc\_put\_var() function).
-   NC\_EINDEFINE The specified netCDF dataset is in define mode rather
    than data mode.
-   NC\_BADID The specified netCDF ID does not refer to an open netCDF
    dataset.
-   NC\_ENOTVAR Bad variable ID.

Example {.heading}
-------

Here is an example using nc\_put\_var\_double to add or change all the
values of the variable named rh to 0.5 in an existing netCDF dataset
named foo.nc. For simplicity in this example, we assume that we know
that rh is dimensioned with lat and lon, and that there are five lat
values and ten lon values.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define LATS  5
#define LONS  10
int  status;                       /* error status */
int  ncid;                         /* netCDF ID */
int  rh_id;                        /* variable ID */
double rh_vals[LATS*LONS];         /* array to hold values */
int i;
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
for (i = 0; i < LATS*LONS; i++)
    rh_vals[i] = 0.5;
/* write values into netCDF variable */
status = nc_put_var_double(ncid, rh_id, rh_vals);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  --------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fput_005fvar_005f-type "Previous section in reading order")]   [[\>](#nc_005fput_005fvars_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.23 Write an Array of Values: nc\_put\_vara\_ type {.section}
---------------------------------------------------

The function nc\_put\_vara\_ type writes values into a netCDF variable
of an open netCDF dataset. The part of the netCDF variable to write is
specified by giving a corner and a vector of edge lengths that refer to
an array section of the netCDF variable. The values to be written are
associated with the netCDF variable by assuming that the last dimension
of the netCDF variable varies fastest in the C interface. The netCDF
dataset must be in data mode.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_put\_var() function will write a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_put_vara_ type (int ncid, int varid, const size_t start[],
                       const size_t count[], const type *valuesp);
int nc_put_vara_text  (int ncid, int varid, const size_t start[],
                       const size_t count[], const char *tp);
int nc_put_vara_uchar (int ncid, int varid, const size_t start[],
                       const size_t count[], const unsigned char *up);
int nc_put_vara_schar (int ncid, int varid, const size_t start[],
                       const size_t count[], const signed char *cp);
int nc_put_vara_short (int ncid, int varid, const size_t start[],
                       const size_t count[], const short *sp);
int nc_put_vara_int   (int ncid, int varid, const size_t start[],
                       const size_t count[], const int *ip);
int nc_put_vara_long  (int ncid, int varid, const size_t start[],
                       const size_t count[], const long *lp);
int nc_put_vara_float (int ncid, int varid, const size_t start[],
                       const size_t count[], const float *fp);
int nc_put_vara_double(int ncid, int varid, const size_t start[],
                       const size_t count[], const double *dp);
int nc_put_vara_ushort(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const unsigned short *op);
int nc_put_vara_uint  (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const unsigned int *op);
int nc_put_vara_longlong (int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const long long *op);
int nc_put_vara_ulonglong(int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const unsigned long long *op);
int nc_put_vara_string(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const char **op);
int nc_put_vara       (int ncid, int varid,  const size_t *startp, 
                       const size_t *countp, const void *op);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `start`
:   A vector of size\_t integers specifying the index in the variable
    where the first of the data values will be written. The indices are
    relative to 0, so for example, the first data value of a variable
    would have index (0, 0, ... , 0). The size of start must be the same
    as the number of dimensions of the specified variable. The elements
    of start must correspond to the variable’s dimensions in order.
    Hence, if the variable is a record variable, the first index would
    correspond to the starting record number for writing the data
    values.

 `count`
:   A vector of size\_t integers specifying the edge lengths along each
    dimension of the block of data values to be written. To write a
    single value, for example, specify count as (1, 1, ... , 1). The
    length of count is the number of dimensions of the specified
    variable. The elements of count correspond to the variable’s
    dimensions. Hence, if the variable is a record variable, the first
    element of count corresponds to a count of the number of records to
    write.

    Note: setting any element of the count array to zero causes the
    function to exit without error, and without doing anything.

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to a block of contiguous data values to be written. The
    order in which the data will be written to the netCDF variable is
    with the last dimension of the specified variable varying fastest.
    If the type of data values differs from the netCDF variable type,
    type conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified corner indices were out of range for
    the rank of the specified variable. For example, a negative index,
    or an index that is larger than the corresponding dimension length
    will cause an error.
-   NC\_EEDGE The specified edge lengths added to the specified corner
    would have referenced data out of range for the rank of the
    specified variable. For example, an edge length that is larger than
    the corresponding dimension length minus the corner index will cause
    an error.
-   NC\_ERANGE One or more of the specified values are out of the range
    of values representable by the external data type of the variable.
    (Does not apply to the nc\_put\_vara() function).
-   NC\_EINDEFINE The specified netCDF dataset is in define mode rather
    than data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.
-   NC\_ECHAR Attempt to convert to or from char.
-   NC\_ENOMEM Out of memory.
-   NC\_EBADTYPE Bad type.

Example {.heading}
-------

Here is an example using nc\_put\_vara\_double to add or change all the
values of the variable named rh to 0.5 in an existing netCDF dataset
named foo.nc. For simplicity in this example, we assume that we know
that rh is dimensioned with time, lat, and lon, and that there are three
time values, five lat values, and ten lon values.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define TIMES 3
#define LATS  5
#define LONS  10
int  status;                       /* error status */
int  ncid;                         /* netCDF ID */
int  rh_id;                        /* variable ID */
static size_t start[] = {0, 0, 0}; /* start at first value */
static size_t count[] = {TIMES, LATS, LONS};
double rh_vals[TIMES*LATS*LONS];   /* array to hold values */
int i;
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
for (i = 0; i < TIMES*LATS*LONS; i++)
    rh_vals[i] = 0.5;
/* write values into netCDF variable */
status = nc_put_vara_double(ncid, rh_id, start, count, rh_vals);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fput_005fvara_005f-type "Previous section in reading order")]   [[\>](#nc_005fput_005fvarm_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.24 Write a Subsampled Array of Values: nc\_put\_vars\_ type {.section}
-------------------------------------------------------------

Each member of the family of functions nc\_put\_vars\_ type writes a
subsampled (strided) array section of values into a netCDF variable of
an open netCDF dataset. The subsampled array section is specified by
giving a corner, a vector of counts, and a stride vector. The netCDF
dataset must be in data mode.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_put\_vars() function will write a variable of any type,
including user defined type. For this function, the type of the data in
memory must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_put_vars_text  (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const char *tp);
int nc_put_vars_uchar (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const unsigned char *up);
int nc_put_vars_schar (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const signed char *cp);
int nc_put_vars_short (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const short *sp);
int nc_put_vars_int   (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const int *ip);
int nc_put_vars_long  (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const long *lp);
int nc_put_vars_float (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const float *fp);
int nc_put_vars_double(int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const double *dp);
int nc_put_vars_ushort(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const unsigned short *op);
int nc_put_vars_uint  (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const unsigned int *op);
int nc_put_vars_longlong (int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const ptrdiff_t *stridep, 
                          const long long *op);
int nc_put_vars_ulonglong(int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const ptrdiff_t *stridep, 
                          const unsigned long long *op);
int nc_put_vars_string(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const char **op);
int nc_put_vars       (int ncid, int varid,  const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep,
                       const void *op);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `start`
:   A vector of size\_t integers specifying the index in the variable
    where the first of the data values will be written. The indices are
    relative to 0, so for example, the first data value of a variable
    would have index (0, 0, ... , 0). The elements of start correspond,
    in order, to the variable’s dimensions. Hence, if the variable is a
    record variable, the first index corresponds to the starting record
    number for writing the data values.

 `count`
:   A vector of size\_t integers specifying the number of indices
    selected along each dimension. To write a single value, for example,
    specify count as (1, 1, ... , 1). The elements of count correspond,
    in order, to the variable’s dimensions. Hence, if the variable is a
    record variable, the first element of count corresponds to a count
    of the number of records to write.

    Note: setting any element of the count array to zero causes the
    function to exit without error, and without doing anything.

 `stride`
:   A vector of ptrdiff\_t integers that specifies the sampling interval
    along each dimension of the netCDF variable. The elements of the
    stride vector correspond, in order, to the netCDF variable’s
    dimensions (stride[0] gives the sampling interval along the most
    slowly varying dimension of the netCDF variable). Sampling intervals
    are specified in type-independent units of elements (a value of 1
    selects consecutive elements of the netCDF variable along the
    corresponding dimension, a value of 2 selects every other element,
    etc.). A NULL stride argument is treated as (1, 1, ... , 1).

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to a block of contiguous data values to be written. The
    order in which the data will be written to the netCDF variable is
    with the last dimension of the specified variable varying fastest.
    If the type of data values differs from the netCDF variable type,
    type conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified corner indices were out of range for
    the rank of the specified variable. For example, a negative index,
    or an index that is larger than the corresponding dimension length
    will cause an error.
-   NC\_EEDGE The specified edge lengths added to the specified corner
    would have referenced data out of range for the rank of the
    specified variable. For example, an edge length that is larger than
    the corresponding dimension length minus the corner index will cause
    an error.
-   NC\_ERANGE One or more of the specified values are out of the range
    of values representable by the external data type of the variable.
    (Does not apply to the nc\_put\_vars() function).
-   NC\_EINDEFINE The specified netCDF dataset is in define mode rather
    than data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.
-   NC\_ECHAR Attempt to convert to or from char.
-   NC\_ENOMEM Out of memory.
-   NC\_EBADTYPE Bad type.

Example {.heading}
-------

Here is an example of using nc\_put\_vars\_float to write – from an
internal array – every other point of a netCDF variable named rh which
is described by the C declaration float rh[4][6] (note the size of the
dimensions):

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define NDIM 2                /* rank of netCDF variable */
int ncid;                     /* netCDF ID */
int status;                   /* error status */
int rhid;                     /* variable ID */
static size_t start[NDIM]     /* netCDF variable start point: */
                 = {0, 0};    /* first element */
static size_t count[NDIM]     /* size of internal array: entire */
                   = {2, 3};  /* (subsampled) netCDF variable */
static ptrdiff_t stride[NDIM] /* variable subsampling intervals: */
                 = {2, 2};    /* access every other netCDF element */
float rh[2][3];               /* note subsampled sizes for */
                              /* netCDF variable dimensions */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid(ncid, "rh", &rhid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_put_vars_float(ncid, rhid, start, count, stride, rh);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fput_005fvars_005f-type "Previous section in reading order")]   [[\>](#nc_005fget_005fvar1_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.25 Write a Mapped Array of Values: nc\_put\_varm\_ type {.section}
---------------------------------------------------------

The nc\_put\_varm\_ type family of functions writes a mapped array
section of values into a netCDF variable of an open netCDF dataset. The
mapped array section is specified by giving a corner, a vector of
counts, a stride vector, and an index mapping vector. The index mapping
vector is a vector of integers that specifies the mapping between the
dimensions of a netCDF variable and the in-memory structure of the
internal data array. No assumptions are made about the ordering or
length of the dimensions of the data array. The netCDF dataset must be
in data mode.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_put\_varm() function will write a variable of any type,
including user defined type. For this function, the type of the data in
memory must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_put_varm_text  (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], const char *tp);
int nc_put_varm_uchar (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], const unsigned char *up);
int nc_put_varm_schar (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], const signed char *cp);
int nc_put_varm_short (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], const short *sp);
int nc_put_varm_int   (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], const int *ip);
int nc_put_varm_long  (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], const long *lp);
int nc_put_varm_float (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], const float *fp);
int nc_put_varm_double(int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], const double *dp);
int nc_put_varm_ushort(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imapp, const unsigned short *op);
int nc_put_varm_uint  (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imapp, const unsigned int *op);
int nc_put_varm_longlong (int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const ptrdiff_t *stridep, 
                          const ptrdiff_t * imapp, const long long *op);
int nc_put_varm_ulonglong(int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const ptrdiff_t *stridep, 
                          const ptrdiff_t * imapp, const unsigned long long *op);
int nc_put_varm_string(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imapp, const char **op);
int nc_put_varm       (int ncid, int varid,  const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const void *op);
n
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `start`
:   A vector of size\_t integers specifying the index in the variable
    where the first of the data values will be written. The indices are
    relative to 0, so for example, the first data value of a variable
    would have index (0, 0, ... , 0). The elements of start correspond,
    in order, to the variable’s dimensions. Hence, if the variable is a
    record variable, the first index corresponds to the starting record
    number for writing the data values.

 `count`
:   A vector of size\_t integers specifying the number of indices
    selected along each dimension. To write a single value, for example,
    specify count as (1, 1, ... , 1). The elements of count correspond,
    in order, to the variable’s dimensions. Hence, if the variable is a
    record variable, the first element of count corresponds to a count
    of the number of records to write.

    Note: setting any element of the count array to zero causes the
    function to exit without error, and without doing anything.

 `stride`
:   A vector of ptrdiff\_t integers that specifies the sampling interval
    along each dimension of the netCDF variable. The elements of the
    stride vector correspond, in order, to the netCDF variable’s
    dimensions (stride[0] gives the sampling interval along the most
    slowly varying dimension of the netCDF variable). Sampling intervals
    are specified in type-independent units of elements (a value of 1
    selects consecutive elements of the netCDF variable along the
    corresponding dimension, a value of 2 selects every other element,
    etc.). A NULL stride argument is treated as (1, 1, ... , 1).

 `imap`
:   A vector of ptrdiff\_t integers that specifies the mapping between
    the dimensions of a netCDF variable and the in-memory structure of
    the internal data array. The elements of the index mapping vector
    correspond, in order, to the netCDF variable’s dimensions (imap[0]
    gives the distance between elements of the internal array
    corresponding to the most slowly varying dimension of the netCDF
    variable). Distances between elements are specified in
    type-independent units of elements (the distance between internal
    elements that occupy adjacent memory locations is 1 and not the
    element’s byte-length as in netCDF 2). A NULL argument means the
    memory-resident values have the same structure as the associated
    netCDF variable.

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to the location used for computing where the data values
    will be found; the data should be of the type appropriate for the
    function called. If the type of data values differs from the netCDF
    variable type, type conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified corner indices were out of range for
    the rank of the specified variable. For example, a negative index,
    or an index that is larger than the corresponding dimension length
    will cause an error.
-   NC\_EEDGE The specified edge lengths added to the specified corner
    would have referenced data out of range for the rank of the
    specified variable. For example, an edge length that is larger than
    the corresponding dimension length minus the corner index will cause
    an error.
-   NC\_ERANGE One or more of the specified values are out of the range
    of values representable by the external data type of the variable.
    (Does not apply to the nc\_put\_vars() function).
-   NC\_EINDEFINE The specified netCDF dataset is in define mode rather
    than data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.
-   NC\_ECHAR Attempt to convert to or from char.
-   NC\_ENOMEM Out of memory.

Example {.heading}
-------

The following imap vector maps in the trivial way a 4x3x2 netCDF
variable and an internal array of the same shape:

 

~~~~ {.example}
float a[4][3][2];       /* same shape as netCDF variable */
int   imap[3] = {6, 2, 1};
                        /* netCDF dimension       inter-element distance */
                        /* ----------------       ---------------------- */
                        /* most rapidly varying       1                  */
                        /* intermediate               2 (=imap[2]*2)     */
                        /* most slowly varying        6 (=imap[1]*3)     */
~~~~

Using the imap vector above with nc\_put\_varm\_float obtains the same
result as simply using nc\_put\_var\_float.

Here is an example of using nc\_put\_varm\_float to write – from a
transposed, internal array – a netCDF variable named rh which is
described by the C declaration float rh[6][4] (note the size and order
of the dimensions):

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define NDIM 2               /* rank of netCDF variable */
int ncid;                    /* netCDF ID */
int status;                  /* error status */
int rhid;                    /* variable ID */
static size_t start[NDIM]    /* netCDF variable start point: */
                 = {0, 0};   /* first element */
static size_t count[NDIM]    /* size of internal array: entire netCDF */
                 = {6, 4};   /* variable; order corresponds to netCDF */
                             /* variable -- not internal array */
static ptrdiff_t stride[NDIM]/* variable subsampling intervals: */
                 = {1, 1};   /* sample every netCDF element */
static ptrdiff_t imap[NDIM]  /* internal array inter-element distances; */
                 = {1, 6};   /* would be {4, 1} if not transposing */
float rh[4][6];              /* note transposition of netCDF variable */
                             /* dimensions */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid(ncid, "rh", &rhid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_put_varm_float(ncid, rhid, start, count, stride, imap, rh);
if (status != NC_NOERR) handle_error(status);
~~~~

Here is another example of using nc\_put\_varm\_float to write – from a
transposed, internal array – a subsample of the same netCDF variable, by
writing every other point of the netCDF variable:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define NDIM 2                /* rank of netCDF variable */
int ncid;                     /* netCDF ID */
int status;                   /* error status */
int rhid;                     /* variable ID */
static size_t start[NDIM]     /* netCDF variable start point: */
                 = {0, 0};    /* first element */
static size_t count[NDIM]     /* size of internal array: entire */
                   = {3, 2};  /* (subsampled) netCDF variable; order of */
                              /* dimensions corresponds to netCDF */
                              /* variable -- not internal array */
static ptrdiff_t stride[NDIM] /* variable subsampling intervals: */
                 = {2, 2};    /* sample every other netCDF element */
static ptrdiff_t imap[NDIM]   /* internal array inter-element distances; */
                 = {1, 3};    /* would be {2, 1} if not transposing */
float rh[2][3];               /* note transposition of (subsampled) */
                              /* netCDF variable dimensions */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid(ncid, "rh", &rhid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_put_varm_float(ncid, rhid, start, count, stride, imap, rh);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------------------------- ----------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fput_005fvarm_005f-type "Previous section in reading order")]   [[\>](#nc_005fget_005fvar_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ----------------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.26 Read a Single Data Value: nc\_get\_var1\_ type {.section}
---------------------------------------------------

The functions nc\_get\_var1\_ type get a single data value from a
variable of an open netCDF dataset that is in data mode. Inputs are the
netCDF ID, the variable ID, a multidimensional index that specifies
which value to get, and the address of a location into which the data
value will be read. The value is converted from the external data type
of the variable, if necessary.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_get\_var1() function will read a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_get_var1_text  (int ncid, int varid, const size_t index[],
                       char *tp);
int nc_get_var1_uchar (int ncid, int varid, const size_t index[],
                       unsigned char *up);
int nc_get_var1_schar (int ncid, int varid, const size_t index[],
                       signed char *cp);
int nc_get_var1_short (int ncid, int varid, const size_t index[],
                       short *sp);
int nc_get_var1_int   (int ncid, int varid, const size_t index[],
                       int *ip);
int nc_get_var1_long  (int ncid, int varid, const size_t index[],
                       long *lp);
int nc_get_var1_float (int ncid, int varid, const size_t index[],
                       float *fp); 
int nc_get_var1_double(int ncid, int varid, const size_t index[],
                       double *dp);
int nc_get_var1_ushort(int ncid, int varid, const size_t *indexp, 
                       unsigned short *ip);
int nc_get_var1_uint  (int ncid, int varid, const size_t *indexp, 
                       unsigned int *ip);
int nc_get_var1_longlong (int ncid, int varid, const size_t *indexp, 
                          long long *ip);
int nc_get_var1_ulonglong(int ncid, int varid, const size_t *indexp, 
                          unsigned long long *ip);
int nc_get_var1_string(int ncid, int varid, const size_t *indexp, 
                       char **ip);
int nc_get_var1       (int ncid, int varid,  const size_t *indexp,
                       void *ip);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `index[]`
:   The index of the data value to be read. The indices are relative to
    0, so for example, the first data value of a two-dimensional
    variable would have index (0,0). The elements of index must
    correspond to the variable’s dimensions. Hence, if the variable is a
    record variable, the first index is the record number.

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to the location into which the data value is read. If the
    type of data value differs from the netCDF variable type, type
    conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified corner indices were out of range for
    the rank of the specified variable. For example, a negative index,
    or an index that is larger than the corresponding dimension length
    will cause an error.
-   NC\_EEDGE The specified edge lengths added to the specified corner
    would have referenced data out of range for the rank of the
    specified variable. For example, an edge length that is larger than
    the corresponding dimension length minus the corner index will cause
    an error.
-   NC\_ERANGE One or more of the specified values are out of the range
    of values representable by the external data type of the variable.
    (Does not apply to the nc\_put\_vars() function).
-   NC\_EINDEFINE The specified netCDF is in define mode rather than
    data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.
-   NC\_ECHAR Attempt to convert to or from char.
-   NC\_ENOMEM Out of memory.

Example {.heading}
-------

Here is an example using nc\_get\_var1\_double to get the (1,2,3)
element of the variable named rh in an existing netCDF dataset named
foo.nc. For simplicity in this example, we assume that we know that rh
is dimensioned with time, lat, and lon, so we want to get the value of
rh that corresponds to the second time value, the third lat value, and
the fourth lon value:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;                           /* error status */
int ncid;                              /* netCDF ID */
int rh_id;                             /* variable ID */
static size_t rh_index[] = {1, 2, 3};  /* where to get value from */
double rh_val;                         /* where to put it */
   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_get_var1_double(ncid, rh_id, rh_index, &rh_val);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fget_005fvar1_005f-type "Previous section in reading order")]   [[\>](#nc_005fget_005fvara_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.27 Read an Entire Variable nc\_get\_var\_ type {.section}
------------------------------------------------

The members of the nc\_get\_var\_ type family of functions read all the
values from a netCDF variable of an open netCDF dataset. This is the
simplest interface to use for reading the value of a scalar variable or
when all the values of a multidimensional variable can be read at once.
The values are read into consecutive locations with the last dimension
varying fastest. The netCDF dataset must be in data mode.

Take care when using the simplest forms of this interface with record
variables (variables that use the NC\_UNLIMITED dimension) when you
don’t specify how many records are to be read. If you try to read all
the values of a record variable into an array but there are more records
in the file than you assume, more data will be read than you expect,
which may cause a segmentation violation. To avoid such problems, it is
better to use the nc\_get\_vara interfaces for variables that use the
NC\_UNLIMITED dimension. See section [Read an Array of Values:
nc\_get\_vara\_ type](#nc_005fget_005fvara_005f-type).

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_get\_var() function will read a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_get_var_text  (int ncid, int varid, char *tp);
int nc_get_var_uchar (int ncid, int varid, unsigned char *up);
int nc_get_var_schar (int ncid, int varid, signed char *cp);
int nc_get_var_short (int ncid, int varid, short *sp);
int nc_get_var_int   (int ncid, int varid, int *ip);
int nc_get_var_long  (int ncid, int varid, long *lp);
int nc_get_var_float (int ncid, int varid, float *fp);
int nc_get_var_double(int ncid, int varid, double *dp);
int nc_get_var_ushort(int ncid, int varid, unsigned short *ip);
int nc_get_var_uint  (int ncid, int varid, unsigned int *ip);
int nc_get_var_longlong (int ncid, int varid, long long *ip);
int nc_get_var_ulonglong(int ncid, int varid, unsigned long long *ip);
int nc_get_var_string(int ncid, int varid, char **ip);
int nc_get_var       (int ncid, int varid,  void *ip);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to a block of contiguous locations in memory into which the
    data values are read. If the type of data value differs from the
    netCDF variable type, type conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified corner indices were out of range for
    the rank of the specified variable. For example, a negative index,
    or an index that is larger than the corresponding dimension length
    will cause an error.
-   NC\_EEDGE The specified edge lengths added to the specified corner
    would have referenced data out of range for the rank of the
    specified variable. For example, an edge length that is larger than
    the corresponding dimension length minus the corner index will cause
    an error.
-   NC\_ERANGE One or more of the specified values are out of the range
    of values representable by the external data type of the variable.
    (Does not apply to the nc\_put\_vars() function).
-   NC\_EINDEFINE The specified netCDF is in define mode rather than
    data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.
-   NC\_ECHAR Attempt to convert to or from char.
-   NC\_ENOMEM Out of memory.

Example {.heading}
-------

Here is an example using nc\_get\_var\_double to read all the values of
the variable named rh from an existing netCDF dataset named foo.nc. For
simplicity in this example, we assume that we know that rh is
dimensioned with lat and lon, and that there are five lat values and ten
lon values.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define LATS 5
#define LONS 10
int  status;                       /* error status */
int ncid;                          /* netCDF ID */
int rh_id;                         /* variable ID */
double rh_vals[LATS*LONS];         /* array to hold values */
   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
/* read values from netCDF variable */
status = nc_get_var_double(ncid, rh_id, rh_vals);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  --------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fget_005fvar_005f-type "Previous section in reading order")]   [[\>](#nc_005fget_005fvars_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.28 Read an Array of Values: nc\_get\_vara\_ type {.section}
--------------------------------------------------

The members of the nc\_get\_vara\_ type family of functions read an
array of values from a netCDF variable of an open netCDF dataset. The
array is specified by giving a corner and a vector of edge lengths. The
values are read into consecutive locations with the last dimension
varying fastest. The netCDF dataset must be in data mode.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_get\_vara() function will write a variable of any type,
including user defined type. For this function, the type of the data in
memory must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_get_vara_text  (int ncid, int varid, const size_t start[],
                       const size_t count[], char *tp);
int nc_get_vara_uchar (int ncid, int varid, const size_t start[],
                       const size_t count[], unsigned char *up);
int nc_get_vara_schar (int ncid, int varid, const size_t start[],
                       const size_t count[], signed char *cp);
int nc_get_vara_short (int ncid, int varid, const size_t start[],
                       const size_t count[], short *sp);
int nc_get_vara_int   (int ncid, int varid, const size_t start[],
                       const size_t count[], int *ip);
int nc_get_vara_long  (int ncid, int varid, const size_t start[],
                       const size_t count[], long *lp);
int nc_get_vara_float (int ncid, int varid, const size_t start[],
                       const size_t count[], float *fp);
int nc_get_vara_double(int ncid, int varid, const size_t start[],
                       const size_t count[], double *dp);
int nc_get_vara_ushort(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, unsigned short *ip);
int nc_get_vara_uint  (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, unsigned int *ip);
int nc_get_vara_longlong(int ncid, int varid, const size_t *startp, 
                         const size_t *countp, long long *ip);
int nc_get_vara_ulonglong(int ncid, int varid, const size_t *startp, 
                          const size_t *countp, unsigned long long *ip);
int nc_get_vara_string(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, char **ip);
int nc_get_vara       (int ncid, int varid, const size_t start[], 
                       const size_t count[], void *ip);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `start`
:   A vector of size\_t integers specifying the index in the variable
    where the first of the data values will be read. The indices are
    relative to 0, so for example, the first data value of a variable
    would have index (0, 0, ... , 0). The length of start must be the
    same as the number of dimensions of the specified variable. The
    elements of start correspond, in order, to the variable’s
    dimensions. Hence, if the variable is a record variable, the first
    index would correspond to the starting record number for reading the
    data values.

 `count`
:   A vector of size\_t integers specifying the edge lengths along each
    dimension of the block of data values to be read. To read a single
    value, for example, specify count as (1, 1, ... , 1). The length of
    count is the number of dimensions of the specified variable. The
    elements of count correspond, in order, to the variable’s
    dimensions. Hence, if the variable is a record variable, the first
    element of count corresponds to a count of the number of records to
    read.

    Note: setting any element of the count array to zero causes the
    function to exit without error, and without doing anything.

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to a block of contiguous locations in memory into which the
    data values are read. If the type of data value differs from the
    netCDF variable type, type conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified corner indices were out of range for
    the rank of the specified variable. For example, a negative index or
    an index that is larger than the corresponding dimension length will
    cause an error.
-   NC\_EEDGE The specified edge lengths added to the specified corner
    would have referenced data out of range for the rank of the
    specified variable. For example, an edge length that is larger than
    the corresponding dimension length minus the corner index will cause
    an error.
-   NC\_ERANGE One or more of the values are out of the range of values
    representable by the desired type. (Does not apply to
    nc\_get\_vara() function).
-   NC\_EINDEFINE The specified netCDF is in define mode rather than
    data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.

Example {.heading}
-------

Here is an example using nc\_get\_vara\_double to read all the values of
the variable named rh from an existing netCDF dataset named foo.nc. For
simplicity in this example, we assume that we know that rh is
dimensioned with time, lat, and lon, and that there are three time
values, five lat values, and ten lon values.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define TIMES 3
 #define LATS 5
#define LONS 10
int  status;                       /* error status */
int ncid;                          /* netCDF ID */
int rh_id;                         /* variable ID */
static size_t start[] = {0, 0, 0}; /* start at first value */
static size_t count[] = {TIMES, LATS, LONS};
double rh_vals[TIMES*LATS*LONS];   /* array to hold values */
   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
/* read values from netCDF variable */
status = nc_get_vara_double(ncid, rh_id, start, count, rh_vals);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fget_005fvara_005f-type "Previous section in reading order")]   [[\>](#nc_005fget_005fvarm_005f-type "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.29 Read a Subsampled Array of Values: nc\_get\_vars\_ type {.section}
------------------------------------------------------------

The nc\_get\_vars\_ type family of functions read a subsampled (strided)
array section of values from a netCDF variable of an open netCDF
dataset. The subsampled array section is specified by giving a corner, a
vector of edge lengths, and a stride vector. The values are read with
the last dimension of the netCDF variable varying fastest. The netCDF
dataset must be in data mode.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_get\_vars() function will read a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_get_vars_text  (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       char *tp);
int nc_get_vars_uchar (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       unsigned char *up);
int nc_get_vars_schar (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       signed char *cp);
int nc_get_vars_short (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       short *sp);
int nc_get_vars_int   (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       int *ip);
int nc_get_vars_long  (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       long *lp);
int nc_get_vars_float (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       float *fp);
int nc_get_vars_double(int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       double *dp)
int nc_get_vars_ushort(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       unsigned short *ip);
int nc_get_vars_uint  (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       unsigned int *ip);
int nc_get_vars_longlong (int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const ptrdiff_t *stridep, 
                          long long *ip);
int nc_get_vars_ulonglong(int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const ptrdiff_t *stridep, 
                          unsigned long long *ip);
int nc_get_vars_string(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       char **ip);
int nc_get_vars       (int ncid, int varid,  const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep,
                       void *ip);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `start`
:   A vector of size\_t integers specifying the index in the variable
    where the first of the data values will be read. The indices are
    relative to 0, so for example, the first data value of a variable
    would have index (0, 0, ... , 0). The elements of start correspond,
    in order, to the variable’s dimensions. Hence, if the variable is a
    record variable, the first index corresponds to the starting record
    number for reading the data values.

 `count`
:   A vector of size\_t integers specifying the number of indices
    selected along each dimension. To read a single value, for example,
    specify count as (1, 1, ... , 1). The elements of count correspond,
    in order, to the variable’s dimensions. Hence, if the variable is a
    record variable, the first element of count corresponds to a count
    of the number of records to read.

    Note: setting any element of the count array to zero causes the
    function to exit without error, and without doing anything.

 `stride`
:   A vector of ptrdiff\_t integers specifying, for each dimension, the
    interval between selected indices. The elements of the stride vector
    correspond, in order, to the variable’s dimensions. A value of 1
    accesses adjacent values of the netCDF variable in the corresponding
    dimension; a value of 2 accesses every other value of the netCDF
    variable in the corresponding dimension; and so on. A NULL stride
    argument is treated as (1, 1, ... , 1).

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to a block of contiguous locations in memory into which the
    data values are read. If the type of data value differs from the
    netCDF variable type, type conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified corner indices were out of range for
    the rank of the specified variable. For example, a negative index,
    or an index that is larger than the corresponding dimension length
    will cause an error.
-   NC\_EEDGE The specified edge lengths added to the specified corner
    would have referenced data out of range for the rank of the
    specified variable. For example, an edge length that is larger than
    the corresponding dimension length minus the corner index will cause
    an error.
-   NC\_ERANGE One or more of the specified values are out of the range
    of values representable by the external data type of the variable.
    (Does not apply to the nc\_get\_vars() function).
-   NC\_EINDEFINE The specified netCDF is in define mode rather than
    data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.
-   NC\_ECHAR Attempt to convert to or from char.
-   NC\_ENOMEM Out of memory.

Example {.heading}
-------

Here is an example that uses nc\_get\_vars\_double to read every other
value in each dimension of the variable named rh from an existing netCDF
dataset named foo.nc. For simplicity in this example, we assume that we
know that rh is dimensioned with time, lat, and lon, and that there are
three time values, five lat values, and ten lon values.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define TIMES 3
#define LATS  5
#define LONS 10
int  status;                          /* error status */
int ncid;                             /* netCDF ID */
int rh_id;                            /* variable ID */
static size_t start[] = {0, 0, 0};    /* start at first value */
static size_t count[] = {TIMES, LATS, LONS};
static ptrdiff_t stride[] = {2, 2, 2};/* every other value */
double data[TIMES][LATS][LONS];       /* array to hold values */
 ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
 ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
 ... 
/* read subsampled values from netCDF variable into array */
status = nc_get_vars_double(ncid, rh_id, start, count, stride, 
                            &data[0][0][0]);
if (status != NC_NOERR) handle_error(status);
 ... 
~~~~

* * * * *

  ---------------------------------------------------------------------------- -------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fget_005fvars_005f-type "Previous section in reading order")]   [[\>](#Strings "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- -------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.30 Read a Mapped Array of Values: nc\_get\_varm\_ type {.section}
--------------------------------------------------------

The nc\_get\_varm\_ type family of functions reads a mapped array
section of values from a netCDF variable of an open netCDF dataset. The
mapped array section is specified by giving a corner, a vector of edge
lengths, a stride vector, and an index mapping vector. The index mapping
vector is a vector of integers that specifies the mapping between the
dimensions of a netCDF variable and the in-memory structure of the
internal data array. No assumptions are made about the ordering or
length of the dimensions of the data array. The netCDF dataset must be
in data mode.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc\_get\_varm() function will read a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_get_varm_text  (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], char *tp);
int nc_get_varm_uchar (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], unsigned char *up);
int nc_get_varm_schar (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], signed char *cp);
int nc_get_varm_short (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], short *sp);
int nc_get_varm_int   (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], int *ip);
int nc_get_varm_long  (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], long *lp);
int nc_get_varm_float (int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], float *fp);
int nc_get_varm_double(int ncid, int varid, const size_t start[],
                       const size_t count[], const ptrdiff_t stride[],
                       const ptrdiff_t imap[], double *dp);
int nc_get_varm_ushort(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imapp, unsigned short *ip);
int nc_get_varm_uint  (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imapp, unsigned int *ip);
int nc_get_varm_longlong (int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const ptrdiff_t *stridep, 
                          const ptrdiff_t * imapp, long long *ip);
int nc_get_varm_ulonglong(int ncid, int varid, const size_t *startp, 
                          const size_t *countp, const ptrdiff_t *stridep, 
                          const ptrdiff_t * imapp, unsigned long long *ip);
int nc_get_varm_string(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imapp, char **ip);
int nc_get_varm       (int ncid, int varid,  const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, void *ip);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID.

 `start`
:   A vector of size\_t integers specifying the index in the variable
    where the first of the data values will be read. The indices are
    relative to 0, so for example, the first data value of a variable
    would have index (0, 0, ... , 0). The elements of start correspond,
    in order, to the variable’s dimensions. Hence, if the variable is a
    record variable, the first index corresponds to the starting record
    number for reading the data values.

 `count`
:   A vector of size\_t integers specifying the number of indices
    selected along each dimension. To read a single value, for example,
    specify count as (1, 1, ... , 1). The elements of count correspond,
    in order, to the variable’s dimensions. Hence, if the variable is a
    record variable, the first element of count corresponds to a count
    of the number of records to read.

    Note: setting any element of the count array to zero causes the
    function to exit without error, and without doing anything.

 `stride`
:   A vector of ptrdiff\_t integers specifying, for each dimension, the
    interval between selected indices. The elements of the stride vector
    correspond, in order, to the variable’s dimensions. A value of 1
    accesses adjacent values of the netCDF variable in the corresponding
    dimension; a value of 2 accesses every other value of the netCDF
    variable in the corresponding dimension; and so on. A NULL stride
    argument is treated as (1, 1, ... , 1).

 `imap`
:   A vector of integers that specifies the mapping between the
    dimensions of a netCDF variable and the in-memory structure of the
    internal data array. imap[0] gives the distance between elements of
    the internal array corresponding to the most slowly varying
    dimension of the netCDF variable. imap[n-1] (where n is the rank of
    the netCDF variable) gives the distance between elements of the
    internal array corresponding to the most rapidly varying dimension
    of the netCDF variable. Intervening imap elements correspond to
    other dimensions of the netCDF variable in the obvious way.
    Distances between elements are specified in type-independent units
    of elements (the distance between internal elements that occupy
    adjacent memory locations is 1 and not the element’s byte-length as
    in netCDF 2).

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to the location used for computing where the data values are
    read; the data should be of the type appropriate for the function
    called. If the type of data value differs from the netCDF variable
    type, type conversion will occur. See [(netcdf)Type
    Conversion](netcdf.html#Type-Conversion) section ‘Type Conversion’
    in The NetCDF Users Guide.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EHDFERR Error reported by HDF5 layer.
-   NC\_ENOTVAR The variable ID is invalid for the specified netCDF
    dataset.
-   NC\_EINVALCOORDS The specified corner indices were out of range for
    the rank of the specified variable. For example, a negative index,
    or an index that is larger than the corresponding dimension length
    will cause an error.
-   NC\_EEDGE The specified edge lengths added to the specified corner
    would have referenced data out of range for the rank of the
    specified variable. For example, an edge length that is larger than
    the corresponding dimension length minus the corner index will cause
    an error.
-   NC\_ERANGE One or more of the specified values are out of the range
    of values representable by the external data type of the variable.
    (Does not apply to the nc\_get\_vars() function).
-   NC\_EINDEFINE The specified netCDF is in define mode rather than
    data mode.
-   NC\_EBADID The specified netCDF ID does not refer to an open netCDF
    dataset.
-   NC\_ECHAR Attempt to convert to or from char.
-   NC\_ENOMEM Out of memory.

Example {.heading}
-------

The following imap vector maps in the trivial way a 4x3x2 netCDF
variable and an internal array of the same shape:

 

~~~~ {.example}
float a[4][3][2];       /* same shape as netCDF variable */
size_t imap[3] = {6, 2, 1};
                        /* netCDF dimension       inter-element distance */
                        /* ----------------       ---------------------- */
                        /* most rapidly varying       1                  */
                        /* intermediate               2 (=imap[2]*2)     */
                        /* most slowly varying        6 (=imap[1]*3)     */
~~~~

Using the imap vector above with nc\_get\_varm\_float obtains the same
result as simply using nc\_get\_var\_float.

Here is an example of using nc\_get\_varm\_float to transpose a netCDF
variable named rh which is described by the C declaration float rh[6][4]
(note the size and order of the dimensions):

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define NDIM 2                /* rank of netCDF variable */
int ncid;                     /* netCDF ID */
int status;                   /* error status */
int rhid;                     /* variable ID */
static size_t start[NDIM]     /* netCDF variable start point: */
                 = {0, 0};    /* first element */
static size_t count[NDIM]     /* size of internal array: entire netCDF */
                 = {6, 4};    /* variable; order corresponds to netCDF */
                              /* variable -- not internal array */
static ptrdiff_t stride[NDIM] /* variable subsampling intervals: */
                 = {1, 1};    /* sample every netCDF element */
static ptrdiff_t imap[NDIM]   /* internal array inter-element distances; */
                 = {1, 6};    /* would be {4, 1} if not transposing */
float rh[4][6];               /* note transposition of netCDF variable */
                              /* dimensions */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid(ncid, "rh", &rhid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_get_varm_float(ncid, rhid, start, count, stride, imap, rh);
if (status != NC_NOERR) handle_error(status);
~~~~

Here is another example of using nc\_get\_varm\_float to simultaneously
transpose and subsample the same netCDF variable, by accessing every
other point of the netCDF variable:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
#define NDIM 2               /* rank of netCDF variable */
int ncid;                    /* netCDF ID */
int status;                  /* error status */
int rhid;                    /* variable ID */
static size_t start[NDIM]    /* netCDF variable start point: */
                 = {0, 0};   /* first element */
static size_t count[NDIM]    /* size of internal array: entire */
                   = {3, 2}; /* (subsampled) netCDF variable; order of */
                             /* dimensions corresponds to netCDF */
                             /* variable -- not internal array */
static ptrdiff_t stride[NDIM]/* variable subsampling intervals: */
                 = {2, 2};   /* sample every other netCDF element */
static ptrdiff_t imap[NDIM]  /* internal array inter-element distances; */
                 = {1, 3};   /* would be {2, 1} if not transposing */
float rh[2][3];              /* note transposition of (subsampled) */
                             /* netCDF variable dimensions */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid(ncid, "rh", &rhid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_get_varm_float(ncid, rhid, start, count, stride, imap, rh);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ---------------------------------------------------------------------------- ---------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fget_005fvarm_005f-type "Previous section in reading order")]   [[\>](#Classic-Strings "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ---------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.31 Reading and Writing Character String Values {.section}
------------------------------------------------

Prior to version 4.0, strings could only be stored as simple arrays of
characters. Users may still wish to store strings this way, as it
ensures maximum compatibility with other software.

Starting in netCDF-4.0, the atomic string type allows a new way to store
strings, as a variable length array in the underlying HDF5 layer. This
allows arrays of strings to be stored compactly.

For more information of classic models strings [Reading and Writing
Character String Values in the Classic Model](#Classic-Strings). For
more information on the netCDF-4.0 string type [Reading and Writing
Arrays of Strings](#Arrays-of-Strings).

  --------------------------------------------------------------------------------------------- ---- --
  [6.31.1 Reading and Writing Character String Values in the Classic Model](#Classic-Strings)        
  [6.31.2 Reading and Writing Arrays of Strings](#Arrays-of-Strings)                                 
  --------------------------------------------------------------------------------------------- ---- --

* * * * *

  ------------------------------------------------------ ------------------------------------------------------------ --- ---------------------------------------------------------------------- ------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Strings "Previous section in reading order")]   [[\>](#Arrays-of-Strings "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Strings "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------ ------------------------------------------------------------ --- ---------------------------------------------------------------------- ------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.31.1 Reading and Writing Character String Values in the Classic Model {.subsection}

Character strings are not a primitive netCDF external data type, in part
because FORTRAN does not support the abstraction of variable-length
character strings (the FORTRAN LEN function returns the static length of
a character string, not its dynamic length). As a result, a character
string cannot be written or read as a single object in the netCDF
interface. Instead, a character string must be treated as an array of
characters, and array access must be used to read and write character
strings as variable data in netCDF datasets. Furthermore,
variable-length strings are not supported by the netCDF interface except
by convention; for example, you may treat a zero byte as terminating a
character string, but you must explicitly specify the length of strings
to be read from and written to netCDF variables.

Character strings as attribute values are easier to use, since the
strings are treated as a single unit for access. However, the value of a
character-string attribute is still an array of characters with an
explicit length that must be specified when the attribute is defined.

When you define a variable that will have character-string values, use a
character-position dimension as the most quickly varying dimension for
the variable (the last dimension for the variable in C). The length of
the character-position dimension will be the maximum string length of
any value to be stored in the character-string variable. Space for
maximum-length strings will be allocated in the disk representation of
character-string variables whether you use the space or not. If two or
more variables have the same maximum length, the same character-position
dimension may be used in defining the variable shapes.

To write a character-string value into a character-string variable, use
either entire variable access or array access. The latter requires that
you specify both a corner and a vector of edge lengths. The
character-position dimension at the corner should be zero for C. If the
length of the string to be written is n, then the vector of edge lengths
will specify n in the character-position dimension, and one for all the
other dimensions:(1, 1, ... , 1, n).

In C, fixed-length strings may be written to a netCDF dataset without
the terminating zero byte, to save space. Variable-length strings should
be written with a terminating zero byte so that the intended length of
the string can be determined when it is later read.

Here is an example that defines a record variable, tx, for character
strings and stores a character-string value into the third record using
nc\_put\_vara\_text. In this example, we assume the string variable and
data are to be added to an existing netCDF dataset named foo.nc that
already has an unlimited record dimension time.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  ncid;            /* netCDF ID */
int  chid;            /* dimension ID for char positions */
int  timeid;          /* dimension ID for record dimension */
int  tx_id;           /* variable ID */
#define TDIMS 2       /* rank of tx variable */
int tx_dims[TDIMS];   /* variable shape */
size_t tx_start[TDIMS];
size_t tx_count[TDIMS];
static char tx_val[] =
        "example string"; /* string to be put */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
status = nc_redef(ncid);       /* enter define mode */
if (status != NC_NOERR) handle_error(status);
   ... 
/* define character-position dimension for strings of max length 40 */
status = nc_def_dim(ncid, "chid", 40L, &chid);
if (status != NC_NOERR) handle_error(status);
   ... 
/* define a character-string variable */
tx_dims[0] = timeid;
tx_dims[1] = chid;    /* character-position dimension last */
status = nc_def_var (ncid, "tx", NC_CHAR, TDIMS, tx_dims, &tx_id);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_enddef(ncid);       /* leave define mode */
if (status != NC_NOERR) handle_error(status);
   ... 
/* write tx_val into tx netCDF variable in record 3 */
tx_start[0] = 3;      /* record number to write */
tx_start[1] = 0;      /* start at beginning of variable */
tx_count[0] = 1;      /* only write one record */
tx_count[1] = strlen(tx_val) + 1;  /* number of chars to write */
status = nc_put_vara_text(ncid, tx_id, tx_start, tx_count, tx_val);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  -------------------------------------------------------------- ----------------------------------------------------------------- --- ---------------------------------------------------------------------- ------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Classic-Strings "Previous section in reading order")]   [[\>](#nc_005ffree_005fstring "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Strings "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- ----------------------------------------------------------------- --- ---------------------------------------------------------------------- ------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### 6.31.2 Reading and Writing Arrays of Strings {.subsection}

In netCDF-4, the NC\_STRING type is introduced. It can store arrays of
strings compactly.

By using the NC\_STRING type, arrays of strings (char \*\*) can be read
and written to the file.

This allows attributes to hold more than one string. Since attributes
are one-dimensional, using the classic model, an attribute could only
hold one string, as an array of char. With the NC\_STRING type, an array
of strings can be stored in one attribute.

When reading data of type NC\_STRING, the HDF5 layer will allocate
memory to hold the data. It is up to the user to free this memory with
the nc\_free\_string function. See section [Releasing Memory for a
NC\_STRING: nc\_free\_string](#nc_005ffree_005fstring).

 

~~~~ {.example}
   int ncid, varid, i, dimids[NDIMS];
   char *data[DIM_LEN] = {"Let but your honour know",
                      "Whom I believe to be most strait in virtue", 
                      "That, in the working of your own affections", 
                      "Had time cohered with place or place with wishing", 
                      "Or that the resolute acting of your blood",
                      "Could have attain'd the effect of your own purpose",
                      "Whether you had not sometime in your life",
                      "Err'd in this point which now you censure him", 
                      "And pull'd the law upon you."};
   char *data_in[DIM_LEN];

   printf("*** testing string attribute...");
   {
      size_t att_len;
      int ndims, nvars, natts, unlimdimid;
      nc_type att_type;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, NC_STRING, DIM_LEN, data)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &att_type, &att_len)) ERR;
      if (att_type != NC_STRING || att_len != DIM_LEN) ERR;
      if (nc_close(ncid)) ERR;
      nc_exit();
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &att_type, &att_len)) ERR;
      if (att_type != NC_STRING || att_len != DIM_LEN) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, ATT_NAME, data_in)) ERR; 
      for (i=0; i<att_len; i++)
         if (strcmp(data_in[i], data[i])) ERR;
      if (nc_free_string(att_len, (char **)data_in)) ERR;
      if (nc_close(ncid)) ERR;
      nc_exit();
   }
~~~~

* * * * *

  ---------------------------------------------------------------- ------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Arrays-of-Strings "Previous section in reading order")]   [[\>](#Fill-Values "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- ------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.32 Releasing Memory for a NC\_STRING: nc\_free\_string {.section}
--------------------------------------------------------

When a STRING is read into user memory from the file, the HDF5 library
performs memory allocations for each of the variable length character
arrays contained within the STRING structure. This memory must be freed
by the user to avoid memory leaks.

This violates the normal netCDF expectation that the user is responsible
for all memory allocation. But, with NC\_STRING arrays, the underlying
HDF5 library allocates the memory for the user, and the user is
responsible for deallocating that memory.

To save the user the trouble calling free() on each element of the
NC\_STRING array (i.e. the array of arrays), the nc\_free\_string
function is provided.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_free_string(size_t len, char **data);
~~~~

 `len`
:   The number of character arrays in the array.

 `**data`
:   The pointer to the data array.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

Example {.heading}
-------

 

~~~~ {.example}
      if (nc_get_att(ncid, NC_GLOBAL, ATT_NAME, data_in)) ERR; 
      ...
      if (nc_free_string(att_len, (char **)data_in)) ERR;
~~~~

* * * * *

  --------------------------------------------------------------------- ---------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005ffree_005fstring "Previous section in reading order")]   [[\>](#nc_005frename_005fvar "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ---------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.33 Fill Values {.section}
----------------

What happens when you try to read a value that was never written in an
open netCDF dataset? You might expect that this should always be an
error, and that you should get an error message or an error status
returned. You do get an error if you try to read data from a netCDF
dataset that is not open for reading, if the variable ID is invalid for
the specified netCDF dataset, or if the specified indices are not
properly within the range defined by the dimension lengths of the
specified variable. Otherwise, reading a value that was not written
returns a special fill value used to fill in any undefined values when a
netCDF variable is first written.

You may ignore fill values and use the entire range of a netCDF external
data type, but in this case you should make sure you write all data
values before reading them. If you know you will be writing all the data
before reading it, you can specify that no prefilling of variables with
fill values will occur by calling nc\_set\_fill before writing. This may
provide a significant performance gain for netCDF writes.

The variable attribute \_FillValue may be used to specify the fill value
for a variable. Their are default fill values for each type, defined in
the include file netcdf.h: NC\_FILL\_CHAR, NC\_FILL\_BYTE,
NC\_FILL\_SHORT, NC\_FILL\_INT, NC\_FILL\_FLOAT, and NC\_FILL\_DOUBLE.

The netCDF byte and character types have different default fill values.
The default fill value for characters is the zero byte, a useful value
for detecting the end of variable-length C character strings. If you
need a fill value for a byte variable, it is recommended that you
explicitly define an appropriate \_FillValue attribute, as generic
utilities such as ncdump will not assume a default fill value for byte
variables.

Type conversion for fill values is identical to type conversion for
other values: attempting to convert a value from one type to another
type that can’t represent the value results in a range error. Such
errors may occur on writing or reading values from a larger type (such
as double) to a smaller type (such as float), if the fill value for the
larger type cannot be represented in the smaller type.

* * * * *

  ---------------------------------------------------------- -------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Fill-Values "Previous section in reading order")]   [[\>](#nc_005fcopy_005fvar "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- -------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.34 Rename a Variable: nc\_rename\_var {.section}
---------------------------------------

The function nc\_rename\_var changes the name of a netCDF variable in an
open netCDF dataset. If the new name is longer than the old name, the
netCDF dataset must be in define mode. You cannot rename a variable to
have the name of any existing variable.

Usage {.heading}
-----

int nc\_rename\_var(int ncid, int varid, const char\* name);

ncid NetCDF or group ID, from a previous call to nc\_open, nc\_create,
nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

varid Variable ID.

name New name for the specified variable.

Errors {.heading}
------

nc\_rename\_var returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

The new name is in use as the name of another variable. The variable ID
is invalid for the specified netCDF dataset. The specified netCDF ID
does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_rename\_var to rename the variable rh to
rel\_hum in an existing netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;              /* error status */
int  ncid;                /* netCDF ID */
int  rh_id;               /* variable ID */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_redef(ncid);  /* put in define mode to rename variable */
if (status != NC_NOERR) handle_error(status);
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
status = nc_rename_var (ncid, rh_id, "rel_hum");
if (status != NC_NOERR) handle_error(status);
status = nc_enddef(ncid); /* leave define mode */
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  -------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005frename_005fvar "Previous section in reading order")]   [[\>](#nc_005fvar_005fpar_005faccess "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ------------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.35 Copy a Variable from One File to Another: nc\_copy\_var {.section}
------------------------------------------------------------

This function will copy a variable and its attributes from one file to
another, assuming dimensions in the output file are already defined and
have same dimension IDs and length.

This function works even if the files are different formats, (for
example, one netcdf classic, the other netcdf-4).

If you’re copying into a classic-model file, from a netcdf-4 file, you
must be copying a variable of one of the six classic-model types, and
similarly for the attributes.

For large netCDF-3 files, this can be a very inefficient way to copy
data from one file to another, because adding a new variable to the
target file may require more space in the header and thus result in
moving data for other variables in the target file. This is not a
problem for netCDF-4 files, which support efficient addition of
variables without moving data for other variables.

Usage {.heading}
-----

 

~~~~ {.example}
nc_copy_var(int ncid_in, int varid_in, int ncid_out)
~~~~

-   ncid\_in The file ID for the file that contains the variable to be
    copied.
-   varid\_in The variable ID for the variable to be copied.
-   ncid\_out The file ID for the file where the variable should be
    copied to.

Return Codes {.heading}
------------

-   NC\_NOERR No error.
-   NC\_EBADID Bad ncid.
-   NC\_EBADVAR Bad varid.
-   NC\_EHDFERR HDF5 layer error.
-   NC\_ENOMEM Out of memory.
-   NC\_ERANGE One or more values out of range.

Example {.heading}
-------

* * * * *

  ------------------------------------------------------------------ --------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fcopy_005fvar "Previous section in reading order")]   [[\>](#nc_005fvar_005fubyte "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ --------------------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.36 Change between Collective and Independent Parallel Access: nc\_var\_par\_access {.section}
------------------------------------------------------------------------------------

The function nc\_var\_par\_access changes whether read/write operations
on a parallel file system are performed collectively or independently
(the default) on the variable. This function can only be called if the
file was created with nc\_create\_par (see [Create a NetCDF Dataset With
Performance Options: nc\_create\_par](#nc_005fcreate_005fpar)) or opened
with nc\_open\_par (see [Open a NetCDF Dataset for Parallel
Access](#nc_005fopen_005fpar)).

Calling this function affects only the open file - information about
whether a variable is to be accessed collectively or independently is
not written to the data file. Every time you open a file on a parallel
file system, all variables default to independent operations. The change
a variable to collective I/O lasts only as long as that file is open.

The variable can be changed from collective to independent, and back, as
often as desired.

Note that classic and 64-bit offset files are access using the
parallel-netcdf library, which does not allow per-variable setting of
the parallel access mode. For these files, calling nc\_var\_par\_access
sets the access for all of the variables in the file.

Usage {.heading}
-----

int nc\_var\_par\_access(int ncid, int varid, int access);

 `ncid`
:   NetCDF ID, from a previous call to nc\_open\_par (see [Open a NetCDF
    Dataset for Parallel Access](#nc_005fopen_005fpar)) or
    nc\_create\_par (see [Create a NetCDF Dataset With Performance
    Options: nc\_create\_par](#nc_005fcreate_005fpar)).

 `varid`
:   Variable ID.

 `access`
:   NC\_INDEPENDENT to set this variable to independent operations.
    NC\_COLLECTIVE to set it to collective operations.

Return Values {.heading}
-------------

 `NC_NOERR`
:   No error.

Example {.heading}
-------

Here is an example using nc\_var\_par\_access:

 

~~~~ {.example}
#include <netcdf.h>

    ... 
    int ncid, v1id, dimids[NDIMS];
    int data[DIMSIZE*DIMSIZE], j, i, res;
    ...

    /* Create a parallel netcdf-4 file. */
    if ((res = nc_create_par(FILE, NC_NETCDF4|NC_MPIIO, comm, info, &ncid)))
        BAIL(res);

    /* Create two dimensions. */
    if ((res = nc_def_dim(ncid, "d1", DIMSIZE, dimids)))
       BAIL(res);
    if ((res = nc_def_dim(ncid, "d2", DIMSIZE, &dimids[1])))
       BAIL(res);

    /* Create one var. */
    if ((res = nc_def_var(ncid, "v1", NC_INT, NDIMS, dimids, &v1id)))
       BAIL(res);

    if ((res = nc_enddef(ncid)))
       BAIL(res);

    /* Tell HDF5 to use independent parallel access for this var. */
    if ((res = nc_var_par_access(ncid, v1id, NC_INDEPENDENT)))
       BAIL(res);

    /* Write slabs of phony data. */
    if ((res = nc_put_vara_int(ncid, v1id, start, count, 
                               &data[mpi_rank*QTR_DATA])))
       BAIL(res);
~~~~

* * * * *

  ---------------------------------------------------------------------------- ----------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fvar_005fpar_005faccess "Previous section in reading order")]   [[\>](#Attributes "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Variables "Up section")]   [[\>\>](#Attributes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ----------------------------------------------------- --- ---------------------------------------------------------------------- --------------------------------- -------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

6.37 Deprecated “\_ubyte” variable functions {.section}
--------------------------------------------

Note: the following functions are also defined but deprecated, as they
are identical in arguments and behavior to the corresponding functions
with “uchar” substituted for “ubyte” in the function name.

 

~~~~ {.example}
int nc_put_var1_ubyte (int ncid, int varid, const size_t index[],
                       const unsigned char *up);
int nc_put_var_ubyte (int ncid, int varid, const unsigned char *op);
int nc_put_vara_ubyte (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const unsigned char *op);
int nc_put_vars_ubyte (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const unsigned char *op);
int nc_put_varm_ubyte (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imapp, const unsigned char *op);
int nc_get_var1_ubyte (int ncid, int varid, const size_t *indexp, 
                       unsigned char *ip);
int nc_get_var_ubyte (int ncid, int varid, unsigned char *ip);
int nc_get_vara_ubyte (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, unsigned char *ip);
int nc_get_vars_ubyte (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       unsigned char *ip);
int nc_get_varm_ubyte (int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imapp, unsigned char *ip);
~~~~

* * * * *

  ------------------------------------------------------------------- ------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fvar_005fubyte "Previous section in reading order")]   [[\>](#Attributes-Introduction "Next section in reading order")]       [[\<\<](#Variables "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Summary-of-C-Interface "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------- ------------------------------------------------------------------ --- ---------------------------------------------------------------------- --------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

7. Attributes {.chapter}
=============

  ------------------------------------------------------------------------------------------- ---- ------------------------------------------
  [7.1 Introduction](#Attributes-Introduction)                                                     
  [7.2 Create an Attribute: nc\_put\_att\_ type](#nc_005fput_005fatt_005f-type)                    
  [7.3 Get Information about an Attribute: nc\_inq\_att Family](#nc_005finq_005fatt-Family)        
  [7.4 Get Attribute’s Values:nc\_get\_att\_ type](#nc_005fget_005fatt_005f-type)                  
  [7.5 Copy Attribute from One NetCDF to Another: nc\_copy\_att](#nc_005fcopy_005fatt)             
  [7.6 Rename an Attribute: nc\_rename\_att](#nc_005frename_005fatt)                               
  [7.7 Delete an Attribute: nc\_del\_att](#nc_005fdel_005fatt)                                     
  [• nc\_att\_ubyte](#nc_005fatt_005fubyte)                                                        Deprecated attribute “\_ubyte” functions
  ------------------------------------------------------------------------------------------- ---- ------------------------------------------

* * * * *

  --------------------------------------------------------- ----------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Attributes "Previous section in reading order")]   [[\>](#nc_005fput_005fatt_005f-type "Next section in reading order")]       [[\<\<](#Attributes "Beginning of this chapter or previous chapter")]   [[Up](#Attributes "Up section")]   [[\>\>](#Summary-of-C-Interface "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------- ----------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

7.1 Introduction {.section}
----------------

Attributes may be associated with each netCDF variable to specify such
properties as units, special values, maximum and minimum valid values,
scaling factors, and offsets. Attributes for a netCDF dataset are
defined when the dataset is first created, while the netCDF dataset is
in define mode. Additional attributes may be added later by reentering
define mode. A netCDF attribute has a netCDF variable to which it is
assigned, a name, a type, a length, and a sequence of one or more
values. An attribute is designated by its variable ID and name. When an
attribute name is not known, it may be designated by its variable ID and
number in order to determine its name, using the function
nc\_inq\_attname.

The attributes associated with a variable are typically defined
immediately after the variable is created, while still in define mode.
The data type, length, and value of an attribute may be changed even
when in data mode, as long as the changed attribute requires no more
space than the attribute as originally defined.

It is also possible to have attributes that are not associated with any
variable. These are called global attributes and are identified by using
NC\_GLOBAL as a variable pseudo-ID. Global attributes are usually
related to the netCDF dataset as a whole and may be used for purposes
such as providing a title or processing history for a netCDF dataset.

Operations supported on attributes are:

-   Create an attribute, given its variable ID, name, data type, length,
    and value.
-   Get attribute’s data type and length from its variable ID and name.
-   Get attribute’s value from its variable ID and name.
-   Copy attribute from one netCDF variable to another.
-   Get name of attribute from its number.
-   Rename an attribute.
-   Delete an attribute.

* * * * *

  ---------------------------------------------------------------------- -------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Attributes-Introduction "Previous section in reading order")]   [[\>](#nc_005finq_005fatt-Family "Next section in reading order")]       [[\<\<](#Attributes "Beginning of this chapter or previous chapter")]   [[Up](#Attributes "Up section")]   [[\>\>](#Summary-of-C-Interface "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- -------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

7.2 Create an Attribute: nc\_put\_att\_ type {.section}
--------------------------------------------

The function nc\_put\_att\_ type adds or changes a variable attribute or
global attribute of an open netCDF dataset. If this attribute is new, or
if the space required to store the attribute is greater than before, the
netCDF dataset must be in define mode.

Usage {.heading}
-----

With netCDF-4 files, nc\_put\_att will notice if you are writing a
\_Fill\_Value\_ attribute, and will tell the HDF5 layer to use the
specified fill value for that variable.

Although it’s possible to create attributes of all types, text and
double attributes are adequate for most purposes.

Use the nc\_put\_att function to create attributes of any type,
including user-defined types. We recommend using the type safe versions
of this function whenever possible.

 

~~~~ {.example}
int nc_put_att_text      (int ncid, int varid, const char *name, 
                          size_t len, const char *tp);
int nc_put_att_uchar     (int ncid, int varid, const char *name, nc_type xtype, 
                          size_t len, const unsigned char *up);
int nc_put_att_schar     (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const signed char *cp);
int nc_put_att_short     (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const short *sp);
int nc_put_att_int       (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const int *ip);
int nc_put_att_long      (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const long *lp);
int nc_put_att_float     (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const float *fp);
int nc_put_att_double    (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const double *dp);
int nc_put_att_ushort    (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const unsigned short *op);
int nc_put_att_uint      (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const unsigned int *op);
int nc_put_att_longlong  (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const long long *op);
int nc_put_att_ulonglong (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, 
                          const unsigned long long *op);
int nc_put_att_string    (int ncid, int varid, const char *name, size_t len, 
                          const char **op);
int nc_put_att           (int ncid, int varid, const char *name, nc_type xtype, 
                          size_t len, const void *op);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID of the variable to which the attribute will be assigned
    or NC\_GLOBAL for a global attribute.

 `name`
:   Attribute name. Attribute name conventions are assumed by some
    netCDF generic applications, e.g., ‘units’ as the name for a string
    attribute that gives the units for a netCDF variable. For examples
    of attribute conventions see [(netcdf)Attribute
    Conventions](netcdf.html#Attribute-Conventions) section ‘Attribute
    Conventions’ in The NetCDF Users Guide.

 `xtype`
:   One of the set of predefined netCDF external data types. The type of
    this parameter, nc\_type, is defined in the netCDF header file. The
    valid netCDF external data types are NC\_BYTE, NC\_CHAR, NC\_SHORT,
    NC\_INT, NC\_FLOAT, and NC\_DOUBLE. Although it’s possible to create
    attributes of all types, NC\_CHAR and NC\_DOUBLE attributes are
    adequate for most purposes.

 `len`
:   Number of values provided for the attribute.

 `tp, up, cp, sp, ip, lp, fp, or dp`
:   Pointer to one or more values. If the type of values differs from
    the netCDF attribute type specified as xtype, type conversion will
    occur. See [(netcdf)Type Conversion](netcdf.html#Type-Conversion)
    section ‘Type Conversion’ in The NetCDF Users Guide.

Errors {.heading}
------

nc\_put\_att\_ type returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The variable ID is invalid for the specified netCDF dataset.
-   The specified netCDF type is invalid.
-   The specified length is negative.
-   The specified open netCDF dataset is in data mode and the specified
    attribute would expand.
-   The specified open netCDF dataset is in data mode and the specified
    attribute does not already exist.
-   The specified netCDF ID does not refer to an open netCDF dataset.
-   The number of attributes for this variable exceeds NC\_MAX\_ATTRS.

Return Codes {.heading}
------------

 `NC_NOERR`
:   No error.

 `NC_EINVAL`
:   Trying to set global \_FillValue. (NetCDF-4 files only).

 `NC_ENOTVAR`
:   Couldn’t find varid.

 `NC_EBADTYPE`
:   Fill value must be same type as variable. (NetCDF-4 files only).

 `NC_ENOMEM`
:   Out of memory

 `NC_EFILLVALUE`
:   Fill values must be written while the file is still in initial
    define mode, that is, after the file is created, but before it
    leaves define mode for the first time. NC\_EFILLVALUE is returned
    when the user attempts to set the fill value after it’s too late.

Example {.heading}
-------

Here is an example using nc\_put\_att\_double to add a variable
attribute named valid\_range for a netCDF variable named rh and a global
attribute named title to an existing netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;                            /* error status */
int  ncid;                              /* netCDF ID */
int  rh_id;                             /* variable ID */
static double rh_range[] = {0.0, 100.0};/* attribute vals */
static char title[] = "example netCDF dataset";
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_redef(ncid);                /* enter define mode */
if (status != NC_NOERR) handle_error(status);
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_put_att_double (ncid, rh_id, "valid_range", 
                            NC_DOUBLE, 2, rh_range);
if (status != NC_NOERR) handle_error(status);
status = nc_put_att_text (ncid, NC_GLOBAL, "title",
                          strlen(title), title)
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_enddef(ncid);               /* leave define mode */
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  --------------------------------------------------------------------------- ----------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fput_005fatt_005f-type "Previous section in reading order")]   [[\>](#nc_005fget_005fatt_005f-type "Next section in reading order")]       [[\<\<](#Attributes "Beginning of this chapter or previous chapter")]   [[Up](#Attributes "Up section")]   [[\>\>](#Summary-of-C-Interface "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ----------------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

7.3 Get Information about an Attribute: nc\_inq\_att Family {.section}
-----------------------------------------------------------

This family of functions returns information about a netCDF attribute.
All but one of these functions require the variable ID and attribute
name; the exception is nc\_inq\_attname. Information about an attribute
includes its type, length, name, and number. See the nc\_get\_att family
for getting attribute values.

The function nc\_inq\_attname gets the name of an attribute, given its
variable ID and number. This function is useful in generic applications
that need to get the names of all the attributes associated with a
variable, since attributes are accessed by name rather than number in
all other attribute functions. The number of an attribute is more
volatile than the name, since it can change when other attributes of the
same variable are deleted. This is why an attribute number is not called
an attribute ID.

The function nc\_inq\_att returns the attribute’s type and length. The
other functions each return just one item of information about an
attribute.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_inq_att    (int ncid, int varid, const char *name,
                   nc_type *xtypep, size_t *lenp);
int nc_inq_atttype(int ncid, int varid, const char *name,
                   nc_type *xtypep);
int nc_inq_attlen  (int ncid, int varid, const char *name, size_t *lenp);
int nc_inq_attname(int ncid, int varid, int attnum, char *name);
int nc_inq_attid   (int ncid, int varid, const char *name, int *attnump); 
~~~~

 `ncid`  
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`  
:   Variable ID of the attribute’s variable, or NC\_GLOBAL for a global
    attribute.

 `name`  
:   Attribute name. For nc\_inq\_attname, this is a pointer to the
    location for the returned attribute name.

 `xtypep`  
:   Pointer to location for returned attribute type, one of the set of
    predefined netCDF external data types. The type of this parameter,
    nc\_type, is defined in the netCDF header file. The valid netCDF
    external data types are NC\_BYTE, NC\_CHAR, NC\_SHORT, NC\_INT,
    NC\_FLOAT, and NC\_DOUBLE. If this parameter is given as ’0’ (a null
    pointer), no type will be returned so no variable to hold the type
    needs to be declared.

 `lenp`  
:   Pointer to location for returned number of values currently stored
    in the attribute. For attributes of type NC\_CHAR, you should not
    assume that this includes a trailing zero byte; it doesn’t if the
    attribute was stored without a trailing zero byte, for example from
    a FORTRAN program. Before using the value as a C string, make sure
    it is null-terminated. If this parameter is given as ’0’ (a null
    pointer), no length will be returned so no variable to hold this
    information needs to be declared.

 `attnum`  
:   For nc\_inq\_attname, attribute number. The attributes for each
    variable are numbered from 0 (the first attribute) to natts-1, where
    natts is the number of attributes for the variable, as returned from
    a call to nc\_inq\_varnatts.

 `attnump`  
:   For nc\_inq\_attid, pointer to location for returned attribute
    number that specifies which attribute this is for this variable (or
    which global attribute). If you already know the attribute name,
    knowing its number is not very useful, because accessing information
    about an attribute requires its name.

Errors {.heading}
------

Each function returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The variable ID is invalid for the specified netCDF dataset.
-   The specified attribute does not exist.
-   The specified netCDF ID does not refer to an open netCDF dataset.
-   For nc\_inq\_attname, the specified attribute number is negative or
    more than the number of attributes defined for the specified
    variable.

Example {.heading}
-------

Here is an example using nc\_inq\_att to find out the type and length of
a variable attribute named valid\_range for a netCDF variable named rh
and a global attribute named title in an existing netCDF dataset named
foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;               /* error status */
int  ncid;                 /* netCDF ID */
int  rh_id;                /* variable ID */
nc_type vr_type, t_type;   /* attribute types */
size_t  vr_len, t_len;     /* attribute lengths */

   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_att (ncid, rh_id, "valid_range", &vr_type, &vr_len);
if (status != NC_NOERR) handle_error(status);
status = nc_inq_att (ncid, NC_GLOBAL, "title", &t_type, &t_len);
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ------------------------------------------------------------------------ -------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005finq_005fatt-Family "Previous section in reading order")]   [[\>](#nc_005fcopy_005fatt "Next section in reading order")]       [[\<\<](#Attributes "Beginning of this chapter or previous chapter")]   [[Up](#Attributes "Up section")]   [[\>\>](#Summary-of-C-Interface "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ -------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

7.4 Get Attribute’s Values:nc\_get\_att\_ type {.section}
----------------------------------------------

Members of the nc\_get\_att\_ type family of functions get the value(s)
of a netCDF attribute, given its variable ID and name.

The nc\_get\_att() functions works for any type of attribute, and must
be used to get attributes of user-defined type. We recommend that they
type safe versions of this function be used where possible.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_get_att_text      (int ncid, int varid, const char *name, char *tp);
int nc_get_att_uchar     (int ncid, int varid, const char *name, unsigned char *up);
int nc_get_att_schar     (int ncid, int varid, const char *name, signed char *cp);
int nc_get_att_short     (int ncid, int varid, const char *name, short *sp);
int nc_get_att_int       (int ncid, int varid, const char *name, int *ip);
int nc_get_att_long      (int ncid, int varid, const char *name, long *lp);
int nc_get_att_float     (int ncid, int varid, const char *name, float *fp);
int nc_get_att_double    (int ncid, int varid, const char *name, double *dp);
int nc_get_att_ushort    (int ncid, int varid, const char *name, unsigned short *ip);
int nc_get_att_uint      (int ncid, int varid, const char *name, unsigned int *ip);
int nc_get_att_longlong  (int ncid, int varid, const char *name, long long *ip);
int nc_get_att_ulonglong (int ncid, int varid, const char *name, unsigned long long *ip);
int nc_get_att_string    (int ncid, int varid, const char *name, char **ip);
int nc_get_att           (int ncid, int varid, const char *name, void *ip);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   Variable ID of the attribute’s variable, or NC\_GLOBAL for a global
    attribute.

 `name`
:   Attribute name.

 `tp`\
 `up`\
 `cp`\
 `sp`\
 `ip`\
 `lp`\
 `fp`\
 `dp`
:   Pointer to location for returned attribute value(s). All elements of
    the vector of attribute values are returned, so you must allocate
    enough space to hold them. For attributes of type NC\_CHAR, you
    should not assume that the returned values include a trailing zero
    byte; they won’t if the attribute was stored without a trailing zero
    byte, for example from a FORTRAN program. Before using the value as
    a C string, make sure it is null-terminated. If you don’t know how
    much space to reserve, call nc\_inq\_attlen first to find out the
    length of the attribute.

Errors {.heading}
------

nc\_get\_att\_ type returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The variable ID is invalid for the specified netCDF dataset.
-   The specified attribute does not exist.
-   The specified netCDF ID does not refer to an open netCDF dataset.
-   One or more of the attribute values are out of the range of values
    representable by the desired type.

Example {.heading}
-------

Here is an example using nc\_get\_att\_double to determine the values of
a variable attribute named valid\_range for a netCDF variable named rh
and a global attribute named title in an existing netCDF dataset named
foo.nc. In this example, it is assumed that we don’t know how many
values will be returned, but that we do know the types of the
attributes. Hence, to allocate enough space to store them, we must first
inquire about the length of the attributes.

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;               /* error status */
int  ncid;                 /* netCDF ID */
int  rh_id;                /* variable ID */
int  vr_len, t_len;        /* attribute lengths */
double *vr_val;            /* ptr to attribute values */
char *title;               /* ptr to attribute values */
extern char *malloc();     /* memory allocator */

   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
/* find out how much space is needed for attribute values */
status = nc_inq_attlen (ncid, rh_id, "valid_range", &vr_len);
if (status != NC_NOERR) handle_error(status);
status = nc_inq_attlen (ncid, NC_GLOBAL, "title", &t_len);
if (status != NC_NOERR) handle_error(status);

/* allocate required space before retrieving values */
vr_val = (double *) malloc(vr_len * sizeof(double));
title = (char *) malloc(t_len + 1);  /* + 1 for trailing null */

/* get attribute values */
status = nc_get_att_double(ncid, rh_id, "valid_range", vr_val);
if (status != NC_NOERR) handle_error(status);
status = nc_get_att_text(ncid, NC_GLOBAL, "title", title);
if (status != NC_NOERR) handle_error(status);
title[t_len] = '\0';       /* null terminate */
   ... 
~~~~

* * * * *

  --------------------------------------------------------------------------- ---------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fget_005fatt_005f-type "Previous section in reading order")]   [[\>](#nc_005frename_005fatt "Next section in reading order")]       [[\<\<](#Attributes "Beginning of this chapter or previous chapter")]   [[Up](#Attributes "Up section")]   [[\>\>](#Summary-of-C-Interface "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ---------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

7.5 Copy Attribute from One NetCDF to Another: nc\_copy\_att {.section}
------------------------------------------------------------

The function nc\_copy\_att copies an attribute from one open netCDF
dataset to another. It can also be used to copy an attribute from one
variable to another within the same netCDF.

If used to copy an attribute of user-defined type, then that
user-defined type must already be defined in the target file. In the
case of user-defined attributes, enddef/redef is called for ncid\_in and
ncid\_out if they are in define mode. (This is the ensure that all
user-defined types are committed to the file(s) before the copy is
attempted.)

Usage {.heading}
-----

 

~~~~ {.example}
int nc_copy_att (int ncid_in, int varid_in, const char *name,
                 int ncid_out, int varid_out);
~~~~

 `ncid_in`
:   The netCDF ID of an input netCDF dataset from which the attribute
    will be copied, from a previous call to nc\_open or nc\_create.

 `varid_in`
:   ID of the variable in the input netCDF dataset from which the
    attribute will be copied, or NC\_GLOBAL for a global attribute.

 `name`
:   Name of the attribute in the input netCDF dataset to be copied.

 `ncid_out`
:   The netCDF ID of the output netCDF dataset to which the attribute
    will be copied, from a previous call to nc\_open or nc\_create. It
    is permissible for the input and output netCDF IDs to be the same.
    The output netCDF dataset should be in define mode if the attribute
    to be copied does not already exist for the target variable, or if
    it would cause an existing target attribute to grow.

 `varid_out`
:   ID of the variable in the output netCDF dataset to which the
    attribute will be copied, or NC\_GLOBAL to copy to a global
    attribute.

Errors {.heading}
------

nc\_copy\_att returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The input or output variable ID is invalid for the specified netCDF
    dataset.
-   The specified attribute does not exist.
-   The output netCDF is not in define mode and the attribute is new for
    the output dataset is larger than the existing attribute.
-   The input or output netCDF ID does not refer to an open netCDF
    dataset.

Example {.heading}
-------

Here is an example using nc\_copy\_att to copy the variable attribute
units from the variable rh in an existing netCDF dataset named foo.nc to
the variable avgrh in another existing netCDF dataset named bar.nc,
assuming that the variable avgrh already exists, but does not yet have a
units attribute:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;               /* error status */
int  ncid1, ncid2;         /* netCDF IDs */
int  rh_id, avgrh_id;      /* variable IDs */
   ... 
status = nc_open("foo.nc", NC_NOWRITE, ncid1);
if (status != NC_NOERR) handle_error(status);
status = nc_open("bar.nc", NC_WRITE, ncid2);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid1, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
status = nc_inq_varid (ncid2, "avgrh", &avgrh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_redef(ncid2);  /* enter define mode */
if (status != NC_NOERR) handle_error(status);
/* copy variable attribute from "rh" to "avgrh" */
status = nc_copy_att(ncid1, rh_id, "units", ncid2, avgrh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_enddef(ncid2); /* leave define mode */
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  ------------------------------------------------------------------ ------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fcopy_005fatt "Previous section in reading order")]   [[\>](#nc_005fdel_005fatt "Next section in reading order")]       [[\<\<](#Attributes "Beginning of this chapter or previous chapter")]   [[Up](#Attributes "Up section")]   [[\>\>](#Summary-of-C-Interface "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ ------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

7.6 Rename an Attribute: nc\_rename\_att {.section}
----------------------------------------

The function nc\_rename\_att changes the name of an attribute. If the
new name is longer than the original name, the netCDF dataset must be in
define mode. You cannot rename an attribute to have the same name as
another attribute of the same variable.

Usage {.heading}
-----

 

~~~~ {.example}
int nc_rename_att (int ncid, int varid, const char* name, 
                   const char* newname);
~~~~

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   ID of the attribute’s variable, or NC\_GLOBAL for a global attribute

 `name`
:   The current attribute name.

 `newname`
:   The new name to be assigned to the specified attribute. If the new
    name is longer than the current name, the netCDF dataset must be in
    define mode.

Errors {.heading}
------

nc\_rename\_att returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The specified variable ID is not valid.
-   The new attribute name is already in use for another attribute of
    the specified variable.
-   The specified netCDF dataset is in data mode and the new name is
    longer than the old name.
-   The specified attribute does not exist.
-   The specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_rename\_att to rename the variable
attribute units to Units for a variable rh in an existing netCDF dataset
named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;      /* error status */
int  ncid;        /* netCDF ID */
int  rh_id;       /* variable id */
   ... 
status = nc_open("foo.nc", NC_NOWRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
/* rename attribute */
status = nc_rename_att(ncid, rh_id, "units", "Units");
if (status != NC_NOERR) handle_error(status);
~~~~

* * * * *

  -------------------------------------------------------------------- ----------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005frename_005fatt "Previous section in reading order")]   [[\>](#Summary-of-C-Interface "Next section in reading order")]       [[\<\<](#Attributes "Beginning of this chapter or previous chapter")]   [[Up](#Attributes "Up section")]   [[\>\>](#Summary-of-C-Interface "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ----------------------------------------------------------------- --- ----------------------------------------------------------------------- ---------------------------------- -------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

7.7 Delete an Attribute: nc\_del\_att {.section}
-------------------------------------

The function nc\_del\_att deletes a netCDF attribute from an open netCDF
dataset. The netCDF dataset must be in define mode.

Usage {.heading}
-----

int nc\_del\_att (int ncid, int varid, const char\* name);

 `ncid`
:   NetCDF or group ID, from a previous call to nc\_open, nc\_create,
    nc\_def\_grp, or associated inquiry functions such as nc\_inq\_ncid.

 `varid`
:   ID of the attribute’s variable, or NC\_GLOBAL for a global
    attribute.

 `name`
:   The name of the attribute to be deleted.

Errors {.heading}
------

nc\_del\_att returns the value NC\_NOERR if no errors occurred.
Otherwise, the returned status indicates an error. Possible causes of
errors include:

-   The specified variable ID is not valid.
-   The specified netCDF dataset is in data mode.
-   The specified attribute does not exist.
-   The specified netCDF ID does not refer to an open netCDF dataset.

Example {.heading}
-------

Here is an example using nc\_del\_att to delete the variable attribute
Units for a variable rh in an existing netCDF dataset named foo.nc:

 

~~~~ {.example}
#include <netcdf.h>
   ... 
int  status;      /* error status */
int  ncid;        /* netCDF ID */
int  rh_id;       /* variable ID */
   ... 
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ... 
status = nc_inq_varid (ncid, "rh", &rh_id);
if (status != NC_NOERR) handle_error(status);
   ... 
/* delete attribute */
status = nc_redef(ncid);        /* enter define mode */
if (status != NC_NOERR) handle_error(status);
status = nc_del_att(ncid, rh_id, "Units");
if (status != NC_NOERR) handle_error(status);
status = nc_enddef(ncid);       /* leave define mode */
if (status != NC_NOERR) handle_error(status);
~~~~

Note: the following functions are also defined but deprecated, as they
are identical in arguments and behavior to the corresponding functions
with “uchar” substituted for “ubyte” in the function name.

 

~~~~ {.example}
int nc_put_att_ubyte     (int ncid, int varid, const char *name, nc_type xtype,
                          size_t len, const unsigned char *op);
int nc_get_att_ubyte     (int ncid, int varid, const char *name, unsigned char *ip);
~~~~

* * * * *

  ----------------------------------------------------------------- -------------------------------------------------------------------- --- ----------------------------------------------------------------------- --------------------------- ----------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#nc_005fdel_005fatt "Previous section in reading order")]   [[\>](#NetCDF-3-Transition-Guide "Next section in reading order")]       [[\<\<](#Attributes "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#NetCDF-3-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------- -------------------------------------------------------------------- --- ----------------------------------------------------------------------- --------------------------- ----------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

A. Summary of C Interface {.appendix}
=========================

 

~~~~ {.example}
const char* nc_inq_libvers (void);
const char* nc_strerror    (int ncerr);

int nc_create         (const char *path, int cmode, int *ncidp);
int nc_open           (const char *path, int mode, int *ncidp);
int nc_set_fill       (int ncid, int fillmode, int *old_modep);
int nc_redef          (int ncid);
int nc_enddef         (int ncid);
int nc_sync           (int ncid);
int nc_abort          (int ncid);
int nc_close          (int ncid);
int nc_inq            (int ncid, int *ndimsp, int *nvarsp,
                       int *ngattsp, int *unlimdimidp);
int nc_inq_ndims      (int ncid, int *ndimsp);
int nc_inq_nvars      (int ncid, int *nvarsp);
int nc_inq_natts      (int ncid, int *ngattsp);
int nc_inq_unlimdim   (int ncid, int *unlimdimidp);

int nc_def_dim        (int ncid, const char *name, size_t len,
                       int *idp);
int nc_inq_dimid      (int ncid, const char *name, int *idp);
int nc_inq_dim        (int ncid, int dimid, char *name, size_t *lenp);
int nc_inq_dimname    (int ncid, int dimid, char *name);
int nc_inq_dimlen     (int ncid, int dimid, size_t *lenp);
int nc_rename_dim     (int ncid, int dimid, const char *name);

int nc_def_var        (int ncid, const char *name, nc_type xtype,
                       int ndims, const int *dimidsp, int *varidp);
int nc_inq_var        (int ncid, int varid, char *name, 
                       nc_type *xtypep, int *ndimsp, int *dimidsp,
                       int *nattsp);
int nc_inq_varid      (int ncid, const char *name, int *varidp);
int nc_inq_varname    (int ncid, int varid, char *name);
int nc_inq_vartype    (int ncid, int varid, nc_type *xtypep);
int nc_inq_varndims   (int ncid, int varid, int *ndimsp);
int nc_inq_vardimid   (int ncid, int varid, int *dimidsp);
int nc_inq_varnatts   (int ncid, int varid, int *nattsp);
int nc_rename_var     (int ncid, int varid, const char *name);
int nc_put_var_text   (int ncid, int varid, const char *op);
int nc_get_var_text   (int ncid, int varid,       char *ip);
int nc_put_var_uchar  (int ncid, int varid, const unsigned char *op);
int nc_get_var_uchar  (int ncid, int varid,       unsigned char *ip);
int nc_put_var_schar  (int ncid, int varid, const signed char *op);
int nc_get_var_schar  (int ncid, int varid,       signed char *ip);
int nc_put_var_short  (int ncid, int varid, const short *op);
int nc_get_var_short  (int ncid, int varid,       short *ip);
int nc_put_var_int    (int ncid, int varid, const int *op);
int nc_get_var_int    (int ncid, int varid,       int *ip);
int nc_put_var_long   (int ncid, int varid, const long *op);
int nc_get_var_long   (int ncid, int varid,       long *ip);
int nc_put_var_float  (int ncid, int varid, const float *op); 
int nc_get_var_float  (int ncid, int varid,       float *ip); 
int nc_put_var_double (int ncid, int varid, const double *op);
int nc_get_var_double (int ncid, int varid,       double *ip);
int nc_put_var1_text  (int ncid, int varid, const size_t *indexp,
                       const char *op);
int nc_get_var1_text  (int ncid, int varid, const size_t *indexp,
                       char *ip);
int nc_put_var1_uchar (int ncid, int varid, const size_t *indexp,
                       const unsigned char *op);
int nc_get_var1_uchar (int ncid, int varid, const size_t *indexp,
                       unsigned char *ip);
int nc_put_var1_schar (int ncid, int varid, const size_t *indexp,
                       const signed char *op);
int nc_get_var1_schar (int ncid, int varid, const size_t *indexp,
                       signed char *ip);
int nc_put_var1_short (int ncid, int varid, const size_t *indexp,
                       const short *op);
int nc_get_var1_short (int ncid, int varid, const size_t *indexp,
                       short *ip);
int nc_put_var1_int   (int ncid, int varid, const size_t *indexp,
                       const int *op);
int nc_get_var1_int   (int ncid, int varid, const size_t *indexp,
                       int *ip);
int nc_put_var1_long  (int ncid, int varid, const size_t *indexp,
                       const long *op);
int nc_get_var1_long  (int ncid, int varid, const size_t *indexp,
                       long *ip);
int nc_put_var1_float (int ncid, int varid, const size_t *indexp,
                       const float *op); 
int nc_get_var1_float (int ncid, int varid, const size_t *indexp,
                       float *ip); 
int nc_put_var1_double(int ncid, int varid, const size_t *indexp,
                       const double *op);
int nc_get_var1_double(int ncid, int varid, const size_t *indexp,
                       double *ip);
int nc_put_vara_text  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const char *op);
int nc_get_vara_text  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, char *ip);
int nc_put_vara_uchar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const unsigned char *op);
int nc_get_vara_uchar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, unsigned char *ip);
int nc_put_vara_schar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const signed char *op);
int nc_get_vara_schar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, signed char *ip);
int nc_put_vara_short (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const short *op);
int nc_get_vara_short (int ncid, int varid, const size_t *startp,
                       const size_t *countp, short *ip);
int nc_put_vara_int   (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const int *op);
int nc_get_vara_int   (int ncid, int varid, const size_t *startp,
                       const size_t *countp, int *ip);
int nc_put_vara_long  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const long *op);
int nc_get_vara_long  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, long *ip);
int nc_put_vara_float (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const float *op);
int nc_get_vara_float (int ncid, int varid, const size_t *startp,
                       const size_t *countp, float *ip);
int nc_put_vara_double(int ncid, int varid, const size_t *startp,
                       const size_t *countp, const double *op);
int nc_get_vara_double(int ncid, int varid, const size_t *startp,
                       const size_t *countp, double *ip);
int nc_put_vars_text  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const char *op);
int nc_get_vars_text  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       char *ip);
int nc_put_vars_uchar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const unsigned char *op);
int nc_get_vars_uchar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       unsigned char *ip);
int nc_put_vars_schar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const signed char *op);
int nc_get_vars_schar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       signed char *ip);
int nc_put_vars_short (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const short *op);
int nc_get_vars_short (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       short *ip);
int nc_put_vars_int   (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const int *op);
int nc_get_vars_int   (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       int *ip);
int nc_put_vars_long  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const long *op);
int nc_get_vars_long  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       long *ip);
int nc_put_vars_float (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const float *op);
int nc_get_vars_float (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       float *ip);
int nc_put_vars_double(int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const double *op);
int nc_get_vars_double(int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       double *ip);
int nc_put_varm_text  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const char *op);
int nc_get_varm_text  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, char *ip);
int nc_put_varm_uchar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const unsigned char *op);
int nc_get_varm_uchar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, unsigned char *ip);
int nc_put_varm_schar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const signed char *op);
int nc_get_varm_schar (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, signed char *ip);
int nc_put_varm_short (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const short *op);
int nc_get_varm_short (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, short *ip);
int nc_put_varm_int   (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const int *op);
int nc_get_varm_int   (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, int *ip);
int nc_put_varm_long  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const long *op);
int nc_get_varm_long  (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, long *ip);
int nc_put_varm_float (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const float *op);
int nc_get_varm_float (int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, float *ip);
int nc_put_varm_double(int ncid, int varid, const size_t *startp,
                       const size_t *countp, const ptrdiff_t *stridep,
                       const ptrdiff_t *imapp, const double *op);  
int nc_get_varm_double(int ncid, int varid, const size_t *startp, 
                       const size_t *countp, const ptrdiff_t *stridep, 
                       const ptrdiff_t * imap, double *ip);

int nc_inq_att        (int ncid, int varid, const char *name,
                       nc_type *xtypep, size_t *lenp);
int nc_inq_attid      (int ncid, int varid, const char *name, int *idp);
int nc_inq_atttype    (int ncid, int varid, const char *name,
                       nc_type *xtypep);
int nc_inq_attlen     (int ncid, int varid, const char *name,
                       size_t *lenp);
int nc_inq_attname    (int ncid, int varid, int attnum, char *name);
int nc_copy_att       (int ncid_in, int varid_in, const char *name,
                       int ncid_out, int varid_out);
int nc_rename_att     (int ncid, int varid, const char *name,
                       const char *newname);
int nc_del_att        (int ncid, int varid, const char *name);
int nc_put_att_text   (int ncid, int varid, const char *name, size_t len,
                       const char *op);
int nc_get_att_text   (int ncid, int varid, const char *name, char *ip);
int nc_put_att_uchar  (int ncid, int varid, const char *name,
                       nc_type xtype, size_t len, const unsigned char *op);
int nc_get_att_uchar  (int ncid, int varid, const char *name,
                       unsigned char *ip);
int nc_put_att_schar  (int ncid, int varid, const char *name,
                       nc_type xtype, size_t len, const signed char *op);
int nc_get_att_schar  (int ncid, int varid, const char *name,
                       signed char *ip);
int nc_put_att_short  (int ncid, int varid, const char *name,
                       nc_type xtype, size_t len, const short *op);
int nc_get_att_short  (int ncid, int varid, const char *name, short *ip);
int nc_put_att_int    (int ncid, int varid, const char *name,
                       nc_type xtype,size_t len, const int *op);
int nc_get_att_int    (int ncid, int varid, const char *name, int *ip);
int nc_put_att_long   (int ncid, int varid, const char *name,
                       nc_type xtype, size_t len, const long *op);
int nc_get_att_long   (int ncid, int varid, const char *name, long *ip);
int nc_put_att_float  (int ncid, int varid, const char *name,
                        nc_type xtype, size_t len, const float *op);
int nc_get_att_float  (int ncid, int varid, const char *name, float *ip);
int nc_put_att_double (int ncid, int varid, const char *name,
                       nc_type xtype, size_t len, const double *op);
int nc_get_att_double (int ncid, int varid, const char *name, 
                       double *ip);
~~~~

* * * * *

  --------------------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------------- --------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Summary-of-C-Interface "Previous section in reading order")]   [[\>](#Introduction-1 "Next section in reading order")]       [[\<\<](#Summary-of-C-Interface "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- --------------------------------------------------------- --- ----------------------------------------------------------------------------------- --------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B. NetCDF 3 to NetCDF 4 Transition Guide {.appendix}
========================================

* * * * *

  ------------------------------------------------------------------------ ---------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF-3-Transition-Guide "Previous section in reading order")]   [[\>](#NetCDF_002d4-and-HDF5 "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ ---------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.1 Introduction {.section}
----------------

The release of netCDF-4 represents a substantial increase in the
capabilities of the netCDF C and Fortran APIs.

The netCDF-4.0 release (June, 2008) allows the use of the popular HDF5
data format as a storage layer. The HDF5 format has many features, and
only a subset of them are exposed in the netCDF-4 API. This represents a
deliberate selection process by netCDF-4 developers to choose the most
useful features of the HDF5 model, while retaining the simplicity of the
netCDF APIs.

Despite many new features, full backward compatibility is assured (and
extensively tested). Existing software and data files will continue to
work with netCDF-4.0, just as with previous releases of the netCDF
library.

The use of netCDF-4 files allows the use of the expanded data model,
including user-defined types, groups, the new unsigned, 64-bit, and
string types.

Using netCDF-4 files also allows the use of such features as endianness
control, per-variable data compression, chunking, parallel I/O, and
checksums. These features fit neatly within the classic netCDF data
model.

Although the expanded data model offers many exciting new features, we
expect and encourage users to proceed with care - it also allows the
creation of needlessly, even horribly complex files. This would decrease
interoperability and increase the work of the poor programmers trying to
use the data file.

There are many netCDF-4 features which fit comfortably within the
classic netCDF model. Existing programs can be very quickly converted to
use features such as compression, endianness control, and chunking. This
allows users to gain immediate performance pay off, with minimal
software development effort.

* * * * *

  ------------------------------------------------------------- ----------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Introduction-1 "Previous section in reading order")]   [[\>](#Backward-Compatibility "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ----------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.2 NetCDF-4 and HDF5 {.section}
---------------------

NetCDF-4 depends on HDF5 to deliver the new features of the expanded
data model, as well as the features required to support the classic data
model.

NetCDF-4 users must have at least HDF5 version 1.8.1 (and at least
zlib-1.2.3) to use HDF5 with netCDF-4.0. If these packages are not found
when netCDF is built, then the netCDF library may still be built
(without the –enable-netcdf-4 option), but will not allow users to
create netCDF-4/HDF5 files, or use the expanded data model. Only classic
and 64-bit offset format netCDF files will be created or readable. (see
[(netcdf-install)Configure](netcdf-install.html#Configure) section
‘Configure’ in The NetCDF Installation and Porting Guide).

The HDF5 files created by netCDF-4 will be readable (and writable) by
any HDF5 application. However, netCDF-4.0 cannot read any HDF5 file,
only those created by netCDF-4.

* * * * *

  -------------------------------------------------------------------- ------------------------------------------------------------------------------------------ --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF_002d4-and-HDF5 "Previous section in reading order")]   [[\>](#The-Classic-and-the-Expanded-NetCDF-Data-Models "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- ------------------------------------------------------------------------------------------ --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.3 Backward Compatibility {.section}
--------------------------

In the context of netCDF, backward compatibility has several meanings.

 `Data Compatibility`
:   NetCDF-4 provides backward compatibility for existing data. All
    netCDF data files remain readable and writable to the netCDF
    library. When a file is opened, the library detects the underlying
    format of the file; this is transparent to the programmer and user.

 `Code Compatibility`
:   NetCDF-4 provides backward compatibility for existing software.
    Programs using the 4.0 release can use it as a drop-in replacement
    for netCDF-3.x. Existing programs will continue to create netCDF
    classic or 64-bit offset files.

 `Model Compatibility`
:   NetCDF-4 introduces an expanded model of a netCDF data file (include
    such new elements as groups, user-defined types, multiple-unlimited
    dimensions, etc.) This expanded model is a super-set of the classic
    netCDF model. Everything that works in the classic model works in
    the expanded model as well. (The reverse is not true - code using
    the expanded data model will fail if run on classic model netCDF
    files.)

* * * * *

  --------------------------------------------------------------------- ------------------------------------------------------------------------------------------------------------------ --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Backward-Compatibility "Previous section in reading order")]   [[\>](#Using-NetCDF_002d4_002e0-with-the-Classic-and-64_002dbit-Offset-Formats "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ------------------------------------------------------------------------------------------------------------------ --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.4 The Classic and the Expanded NetCDF Data Models {.section}
---------------------------------------------------

The classic netCDF data model consists of variables, dimensions, and
attributes.

The netCDF-4.0 release introduces an expanded data model, which offers
many new features. These features will only work on files which have
been created with the NC\_NETCDF4 flag, and without the
NC\_CLASSIC\_MODEL flag (see section [Create a NetCDF Dataset:
nc\_create](#nc_005fcreate)).

 `New Types`
:   New data types are introduced: NC\_UBYTE, NC\_USHORT, NC\_UINT,
    NC\_INT64, NC\_UINT64, and NC\_STRING. These types many be used for
    attributes and variables. See section [Create a Variable:
    `nc_def_var`](#nc_005fdef_005fvar).

 `Groups`
:   NetCDF objects may now be organizes into a hierarchical set of
    groups. Groups are organized much line a UNIX file system, with each
    group capable of containing more groups. Within each group a classic
    model netCDF “file” exists, with its own dimensions, variables, and
    attributes. See section [Create a New Group:
    nc\_def\_grp](#nc_005fdef_005fgrp).

 `User Defined Types`
:   NetCDF-4 allows the user to define new data types, including a
    compound type (see section [Creating a Compound Type:
    nc\_def\_compound](#nc_005fdef_005fcompound)), a variable length
    array type (see section [Define a Variable Length Array (VLEN):
    nc\_def\_vlen](#nc_005fdef_005fvlen)), an enumerated type (see
    section [Creating a Enum Type:
    nc\_def\_enum](#nc_005fdef_005fenum)), and an opaque type (see
    section [Creating Opaque Types:
    nc\_def\_opaque](#nc_005fdef_005fopaque)).

 `Multiple Unlimited Dimensions`
:   NetCDF-4/HDF5 data files may use multiple unlimited dimensions with
    a file, and even within a variable.

* * * * *

  ---------------------------------------------------------------------------------------------- -------------------------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#The-Classic-and-the-Expanded-NetCDF-Data-Models "Previous section in reading order")]   [[\>](#Creating-a-NetCDF_002d4_002fHDF5-File "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------------------- -------------------------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.5 Using NetCDF-4.0 with the Classic and 64-bit Offset Formats {.section}
---------------------------------------------------------------

Prior to the 4.0 release, two underlying data formats were available for
the netCDF user, the classic, and the 64-bit offset format. (The 64-bit
offset format was introduced in the 3.6.0 release, and allows the use of
larger variables and files).

Software using netCDF, relinked against the netCDF-4.0 library, will
continue to work exactly as before. Since the default create mode in
nc\_create is to create a classic format file, using unmodified netCDF-3
code with the netCDF-4 library will result in the exact same output - a
classic netCDF file or 64-bit offset file.

When writing or reading classic and 64-bit offset files, the netCDF-4.0
library relies on the core netCDF-3.x code.

* * * * *

  ---------------------------------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------------ --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Using-NetCDF_002d4_002e0-with-the-Classic-and-64_002dbit-Offset-Formats "Previous section in reading order")]   [[\>](#Using-NetCDF_002d4_002e0-with-the-Classic-Model "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------------ --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.6 Creating a NetCDF-4/HDF5 File {.section}
---------------------------------

The extra features of netCDF-4 can only be accessed by adding the
NC\_NETCDF4 flag to the create mode of nc\_create. Files created with
the NC\_NETCDF4 flag can have multiple unlimited dimensions, use the new
atomic types, use compound and opaque types, and take advantage of the
other features of netCDF-4. (see section [Create a NetCDF Dataset:
nc\_create](#nc_005fcreate)).

* * * * *

  ------------------------------------------------------------------------------------ ------------------------------------------------------------------------------------------------ --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Creating-a-NetCDF_002d4_002fHDF5-File "Previous section in reading order")]   [[\>](#Use-of-the-Expanded-Model-Impacts-Fortran-Portability "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------ ------------------------------------------------------------------------------------------------ --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.7 Using NetCDF-4.0 with the Classic Model {.section}
-------------------------------------------

By changing your nc\_create call to create a netCDF-4/HDF5 file you gain
access to many new features - perhaps too many! Using groups or
user-defined types will make the file unreadable to existing netCDF
applications, until they are updated to handle the new netCDF-4 model.

Using the NC\_CLASSIC\_MODEL flag with the NC\_NETCDF4 flag tells the
library to create a netCDF-4/HDF5 file which must abide by the rules of
the classic netCDF data model. Such a file many not contain groups, user
defined types, multiple unlimited dimensions, etc.

But a classic model file is guaranteed to be compatible with existing
netCDF software, once relinked to the netCDF 4.0 library.

Some features of netCDF-4 are transparent to the user when the file is
read. For example, a netCDF-4/HDF5 file may contain compressed data.
When such a file is read, the decompression of the data takes place
transparently. This means that data may use the data compression
feature, and still conform to the classic netCDF data model, and thus
retain compatibility with existing netCDF software (see section [Define
Compression Parameters for a Variable:
`nc_def_var_deflate`](#nc_005fdef_005fvar_005fdeflate)). The same
applies for control of endianness (see section [Define Endianness of a
Variable: `nc_def_var_endian`](#nc_005fdef_005fvar_005fendian)),
chunking (see section [Define Chunking Parameters for a Variable:
`nc_def_var_chunking`](#nc_005fdef_005fvar_005fchunking)), checksums
(see section [Define Checksum Parameters for a Variable:
`nc_def_var_fletcher32`](#nc_005fdef_005fvar_005ffletcher32)), and
parallel I/O, if netCDF-4 was built on a system with the MPI libraries.

To use these feature, change your nc\_create calls to use the
NC\_NETCDF4 and NC\_CLASSIC\_MODEL flags. Then call the appropriate
nc\_dev\_var\_\* function after the variable is defined, but before the
next call to nc\_enddef.

* * * * *

  ---------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Using-NetCDF_002d4_002e0-with-the-Classic-Model "Previous section in reading order")]   [[\>](#The-C_002b_002b-API-Does-Not-Handle-Expanded-Model-in-this-Release "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.8 Use of the Expanded Model Impacts Fortran Portability {.section}
---------------------------------------------------------

Using expanded model features impacts portability for Fortran
programmers.

Fortran compilers do not always agree as to how data should be laid out
in memory. This makes handling compound and variable length array types
compiler and platform dependant.

(This is also true for C, but the clever HDF5 configuration has solved
this problem for C. Alas, not for Fortran.)

Despite this, Fortran programs can take advantage of the new data model.
The portability challenge is no different from that which Fortran
programmers already deal with when doing data I/O.

* * * * *

  ---------------------------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Use-of-the-Expanded-Model-Impacts-Fortran-Portability "Previous section in reading order")]   [[\>](#NetCDF-2-C-Transition-Guide "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-3-Transition-Guide "Up section")]   [[\>\>](#NetCDF-2-C-Transition-Guide "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------------------------- ---------------------------------------------------------------------- --- -------------------------------------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

B.9 The C++ API Does Not Handle Expanded Model in this Release {.section}
--------------------------------------------------------------

Unfortunately, the C++ API does not support the netCDF-4 expanded data
model. A new C++ API is being developed and may be built by adventurous
users using the –enable-cxx4 option to configure (see
[(netcdf-install)Configure](netcdf-install.html#Configure) section
‘Configure’ in The NetCDF Installation and Porting Guide).

* * * * *

  ----------------------------------------------------------------------------------------------------------------- -------------------------------------------------------------------------- --- -------------------------------------------------------------------------------------- --------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#The-C_002b_002b-API-Does-Not-Handle-Expanded-Model-in-this-Release "Previous section in reading order")]   [[\>](#Overview-of-C-interface-changes "Next section in reading order")]       [[\<\<](#NetCDF-3-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------------------------------------- -------------------------------------------------------------------------- --- -------------------------------------------------------------------------------------- --------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C. NetCDF 2 to NetCDF 3 C Transition Guide {.appendix}
==========================================

* * * * *

  -------------------------------------------------------------------------- -------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF-2-C-Transition-Guide "Previous section in reading order")]   [[\>](#The-New-C-Interface "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-2-C-Transition-Guide "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- -------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C.1 Overview of C interface changes {.section}
-----------------------------------

NetCDF version 3 includes a complete rewrite of the netCDF library. It
is about twice as fast as the previous version. The netCDF file format
is unchanged, so files written with version 3 can be read with version 2
code and vice versa.

The core library is now written in ANSI C. For example, prototypes are
used throughout as well as const qualifiers where appropriate. You must
have an ANSI C compiler to compile this version.

Rewriting the library offered an opportunity to implement improved C and
FORTRAN interfaces that provide some significant benefits:

type safety, by eliminating the need to use generic void\* pointers;

automatic type conversions, by eliminating the undesirable coupling
between the language-independent external netCDF types (NC\_BYTE, ...,
NC\_DOUBLE) and language-dependent internal data types (char, ...,
double);

support for future enhancements, by eliminating obstacles to the clean
addition of support for packed data and multithreading;

more standard error behavior, by uniformly communicating an error status
back to the calling program in the return value of each function.

It is not necessary to rewrite programs that use the version 2 C
interface, because the netCDF-3 library includes a backward
compatibility interface that supports all the old functions, globals,
and behavior. We are hoping that the benefits of the new interface will
be an incentive to use it in new netCDF applications. It is possible to
convert old applications to the new interface incrementally, replacing
netCDF-2 calls with the corresponding netCDF-3 calls one at a time. If
you want to check that only netCDF-3 calls are used in an application, a
preprocessor macro (NO\_NETCDF\_2) is available for that purpose.

Other changes in the implementation of netCDF result in improved
portability, maintainability, and performance on most platforms. A clean
separation between I/O and type layers facilitates platform-specific
optimizations. The new library no longer uses a vendor-provided XDR
library, which simplifies linking programs that use netCDF and speeds up
data access significantly in most cases.

* * * * *

  ------------------------------------------------------------------------------ ---------------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Overview-of-C-interface-changes "Previous section in reading order")]   [[\>](#Function-Naming-Conventions "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-2-C-Transition-Guide "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------ ---------------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C.2 The New C Interface {.section}
-----------------------

First, here’s an example of C code that uses the netCDF-2 interface:

 

~~~~ {.example}
void *bufferp;
nc_type xtype;
ncvarinq(ncid, varid, ..., &xtype, ...
...
/* allocate bufferp based on dimensions and type */
...
if (ncvarget(ncid, varid, start, count, bufferp) == -1) {
    fprintf(stderr, "Can't get data, error code = %d\n",ncerr);
    /* deal with it */
    ...
}
switch(xtype) {
    /* deal with the data, according to type */
...
case  NC_FLOAT:
    fanalyze((float *)bufferp);
    break;
case NC_DOUBLE:
    danalyze((double *)bufferp);
    break;
}
~~~~

Here’s how you might handle this with the new netCDF-3 C interface:

 

~~~~ {.example}
/*
 * I want to use doubles for my analysis.
 */
double dbuf[NDOUBLES];
int status;

/* So, I use the function that gets the data as doubles. */
status = nc_get_vara_double(ncid, varid, start, count, dbuf)
if (status != NC_NOERR) {
   fprintf(stderr, "Can't get data: %s\n", nc_strerror(status));
    /* deal with it */
    ...
}
danalyze(dbuf);
~~~~

The example above illustrates changes in function names, data type
conversion, and error handling, discussed in detail in the sections
below.

* * * * *

  ------------------------------------------------------------------ ---------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#The-New-C-Interface "Previous section in reading order")]   [[\>](#Type-Conversion "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-2-C-Transition-Guide "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------ ---------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C.3 Function Naming Conventions {.section}
-------------------------------

The netCDF-3 C library employs a new naming convention, intended to make
netCDF programs more readable. For example, the name of the function to
rename a variable is now nc\_rename\_var instead of the previous
ncvarrename.

All netCDF-3 C function names begin with the nc\_ prefix. The second
part of the name is a verb, like get, put, inq (for inquire), or open.
The third part of the name is typically the object of the verb: for
example dim, var, or att for functions dealing with dimensions,
variables, or attributes. To distinguish the various I/O operations for
variables, a single character modifier is appended to var:

 var
:   entire variable access

 var1
:   single value access

 vara
:   array or array section access

 vars
:   strided access to a subsample of values

 varm
:   mapped access to values not contiguous in memory

At the end of the name for variable and attribute functions, there is a
component indicating the type of the final argument: text, uchar, schar,
short, int, long, float, or double. This part of the function name
indicates the type of the data container you are using in your program:
character string, unsigned char, signed char, and so on.

Also, all macro names in the public C interface begin with the prefix
NC\_. For example, the macro which was formerly MAX\_NC\_NAME is now
NC\_MAX\_NAME, and the former FILL\_FLOAT is now NC\_FILL\_FLOAT.

As previously mentioned, all the old names are still supported for
backward compatibility.

* * * * *

  -------------------------------------------------------------------------- --------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Function-Naming-Conventions "Previous section in reading order")]   [[\>](#Error-handling "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-2-C-Transition-Guide "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------------- --------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C.4 Type Conversion {.section}
-------------------

With the new interface, users need not be aware of the external type of
numeric variables, since automatic conversion to or from any desired
numeric type is now available. You can use this feature to simplify
code, by making it independent of external types. The elimination of
void\* pointers provides detection of type errors at compile time that
could not be detected with the previous interface. Programs may be made
more robust with the new interface, because they need not be changed to
accommodate a change to the external type of a variable.

If conversion to or from an external numeric type is necessary, it is
handled by the library. This automatic conversion and separation of
external data representation from internal data types will become even
more important in netCDF version 4, when new external types will be
added for packed data for which there is no natural corresponding
internal type, for example, arrays of 11-bit values.

Converting from one numeric type to another may result in an error if
the target type is not capable of representing the converted value. (In
netCDF-2, such overflows can only happen in the XDR layer.) For example,
a float may not be able to hold data stored externally as an NC\_DOUBLE
(an IEEE floating-point number). When accessing an array of values, an
NC\_ERANGE error is returned if one or more values are out of the range
of representable values, but other values are converted properly.

Note that mere loss of precision in type conversion does not return an
error. Thus, if you read double precision values into an int, for
example, no error results unless the magnitude of the double precision
value exceeds the representable range of ints on your platform.
Similarly, if you read a large integer into a float incapable of
representing all the bits of the integer in its mantissa, this loss of
precision will not result in an error. If you want to avoid such
precision loss, check the external types of the variables you access to
make sure you use an internal type that has a compatible precision.

The new interface distinguishes arrays of characters intended to
represent text strings from arrays of 8-bit bytes intended to represent
small integers. The interface supports the internal types text, uchar,
and schar, intended for text strings, unsigned byte values, and signed
byte values.

The \_uchar and \_schar functions were introduced in netCDF-3 to
eliminate an ambiguity, and support both signed and unsigned byte data.
In netCDF-2, whether the external NC\_BYTE type represented signed or
unsigned values was left up to the user. In netcdf-3, we treat NC\_BYTE
as signed for the purposes of conversion to short, int, long, float, or
double. (Of course, no conversion takes place when the internal type is
signed char.) In the \_uchar functions, we treat NC\_BYTE as if it were
unsigned. Thus, no NC\_ERANGE error can occur converting between
NC\_BYTE and unsigned char. The \_uchar and \_schar functions will
behave differently when writing data or attribute values to a larger
type, because the type conversion is from unsigned or signed to the
larger type, respectively.

* * * * *

  -------------------------------------------------------------- --------------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Type-Conversion "Previous section in reading order")]   [[\>](#NC_005fLONG-and-NC_005fINT "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-2-C-Transition-Guide "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- --------------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C.5 Error handling {.section}
------------------

The new interface handles errors differently than netCDF-2. In the old
interface, the default behavior when an error was detected was to print
an error message and exit. To get control of error handling, you had to
set flag bits in a global variable, ncopts, and to determine the cause
of an error, you had to test the value of another global variable ncerr.

In the new interface, functions return an integer status that indicates
not only success or failure, but also the cause of the error. The global
variables ncerr and ncopt have been eliminated. The library will never
try to print anything, nor will it call exit (unless you are using the
netCDF version 2 compatibility functions). You will have to check the
function return status and do this yourself. We eliminated these globals
in the interest of supporting parallel (multiprocessor) execution
cleanly, as well as reducing the number of assumptions about the
environment where netCDF is used. The new behavior should provide better
support for using netCDF as a hidden layer in applications that have
their own GUI interface.

* * * * *

  ------------------------------------------------------------- ------------------------------------------------------------------ --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Error-handling "Previous section in reading order")]   [[\>](#What_0027s-Missing_003f "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-2-C-Transition-Guide "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- ------------------------------------------------------------------ --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C.6 NC\_LONG and NC\_INT {.section}
------------------------

Where the netCDF-2 interface used NC\_LONG to identify an external data
type corresponding to 32-bit integers, the new interface uses NC\_INT
instead. NC\_LONG is defined to have the same value as NC\_INT for
backward compatibility, but it should not be used in new code. With new
64-bit platforms using long for 64-bit integers, we would like to reduce
the confusion caused by this name clash. Note that there is still no
netCDF external data type corresponding to 64-bit integers.

* * * * *

  ------------------------------------------------------------------------- -------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NC_005fLONG-and-NC_005fINT "Previous section in reading order")]   [[\>](#Other-Changes "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-2-C-Transition-Guide "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------- -------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C.7 What’s Missing? {.section}
-------------------

The new C interface omits three "record I/O" functions, ncrecput,
ncrecget, and ncrecinq, from the netCDF-2 interface, although these
functions are still supported via the netCDF-2 compatibility interface.

This means you may have to replace one record-oriented call with
multiple type-specific calls, one for each record variable. For example,
a single call to ncrecput can always be replaced by multiple calls to
the appropriate nc\_put\_var functions, one call for each variable
accessed. The record-oriented functions were omitted, because there is
no simple way to provide type-safety and automatic type conversion for
such an interface.

There is no function corresponding to the nctypelen function from the
version 2 interface. The separation of internal and external types and
the new type-conversion interfaces make nctypelen unnecessary. Since
users read into and write out of native types, the sizeof operator is
perfectly adequate to determine how much space to allocate for a value.

In the previous library, there was no checking that the characters used
in the name of a netCDF object were compatible with CDL restrictions.
The ncdump and ncgen utilities now properly escape and handle escaped
special characters in names, so that all valid netCDF names are
representable in CDL.

* * * * *

  ---------------------------------------------------------------------- ------------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#What_0027s-Missing_003f "Previous section in reading order")]   [[\>](#NetCDF_002d3-Error-Codes "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-2-C-Transition-Guide "Up section")]   [[\>\>](#NetCDF_002d3-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- ------------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

C.8 Other Changes {.section}
-----------------

There are two new functions in netCDF-3 that don’t correspond to any
netCDF-2 functions: nc\_inq\_libvers and nc\_strerror. The version of
the netCDF library in use is returned as a string by nc\_inq\_libvers.
An error message corresponding to the status returned by a netCDF
function call is returned as a string by the nc\_strerror function.

A new NC\_SHARE flag is available for use in an nc\_open or nc\_create
call, to suppress the default buffering of accesses. The use of
NC\_SHARE for concurrent access to a netCDF dataset means you don’t have
to call nc\_sync after every access to make sure that disk updates are
synchronous. It is important to note that changes to ancillary data,
such as attribute values, are not propagated automatically by use of the
NC\_SHARE flag. Use of the nc\_sync function is still required for this
purpose.

The version 2 interface had a single inquiry function, ncvarinq for
getting the name, type, and shape of a variable. Similarly, only a
single inquiry function was available for getting information about a
dimension, an attribute, or a netCDF dataset. When you only wanted a
subset of this information, you had to provide NULL arguments as
placeholders for the unneeded information. The new interface includes
additional inquire functions that return each item separately, so errors
are less likely from miscounting arguments.

The previous implementation returned an error when 0-valued count
components were specified in ncvarput and ncvarget calls. This
restriction has been removed, so that now functions in the nc\_put\_var
and nc\_get\_var families may be called with 0-valued count components,
resulting in no data being accessed. Although this may seem useless, it
simplifies some programs to not treat 0-valued counts as a special case.

The previous implementation returned an error when the same dimension
was used more than once in specifying the shape of a variable in
ncvardef. This restriction is relaxed in the netCDF-3 implementation,
because an auto-correlation matrix is a good example where using the
same dimension twice makes sense.

In the new interface, units for the imap argument to the nc\_put\_varm
and nc\_get\_varm families of functions are now in terms of the number
of data elements of the desired internal type, not in terms of bytes as
in the netCDF version-2 mapped access interfaces.

Following is a table of netCDF-2 function names and names of the
corresponding netCDF-3 functions. For parameter lists of netCDF-2
functions, see the netCDF-2 User’s Guide.

 `ncabort`
:   nc\_abort

 `ncattcopy`
:   nc\_copy\_att

 `ncattdel`
:   nc\_del\_att

 `ncattget`
:   nc\_get\_att\_double, nc\_get\_att\_float, nc\_get\_att\_int,
    nc\_get\_att\_long, nc\_get\_att\_schar, nc\_get\_att\_short,
    nc\_get\_att\_text, nc\_get\_att\_uchar

 `ncattinq`
:   nc\_inq\_att, nc\_inq\_attid, nc\_inq\_attlen, nc\_inq\_atttype

 `ncattname`
:   nc\_inq\_attname

 `ncattput`
:   nc\_put\_att\_double, nc\_put\_att\_float, nc\_put\_att\_int,
    nc\_put\_att\_long, nc\_put\_att\_schar, nc\_put\_att\_short,
    nc\_put\_att\_text, nc\_put\_att\_uchar

 `ncattrename`
:   nc\_rename\_att

 `ncclose`
:   nc\_close

 `nccreate`
:   nc\_create

 `ncdimdef`
:   nc\_def\_dim

 `ncdimid`
:   nc\_inq\_dimid

 `ncdiminq`
:   nc\_inq\_dim, nc\_inq\_dimlen, nc\_inq\_dimname

 `ncdimrename`
:   nc\_rename\_dim

 `ncendef`
:   nc\_enddef

 `ncinquire`
:   nc\_inq, nc\_inq\_natts, nc\_inq\_ndims, nc\_inq\_nvars,
    nc\_inq\_unlimdim

 `ncopen`
:   nc\_open

 `ncrecget`
:   (none)

 `ncrecinq`
:   (none)

 `ncrecput`
:   (none)

 `ncredef`
:   nc\_redef

 `ncsetfill`
:   nc\_set\_fill

 `ncsync`
:   nc\_sync

 `nctypelen`
:   (none)

 `ncvardef`
:   nc\_def\_var

 `ncvarget`
:   nc\_get\_vara\_double, nc\_get\_vara\_float, nc\_get\_vara\_int,
    nc\_get\_vara\_long, nc\_get\_vara\_schar, nc\_get\_vara\_short,
    nc\_get\_vara\_text, nc\_get\_vara\_uchar

 `ncvarget1`
:   nc\_get\_var1\_double, nc\_get\_var1\_float, nc\_get\_var1\_int,
    nc\_get\_var1\_long, nc\_get\_var1\_schar, nc\_get\_var1\_short,
    nc\_get\_var1\_text, nc\_get\_var1\_uchar

 `ncvargetg`
:   nc\_get\_varm\_double, nc\_get\_varm\_float, nc\_get\_varm\_int,
    nc\_get\_varm\_long, nc\_get\_varm\_schar, nc\_get\_varm\_short,
    nc\_get\_varm\_text, nc\_get\_varm\_uchar, nc\_get\_vars\_double,
    nc\_get\_vars\_float, nc\_get\_vars\_int, nc\_get\_vars\_long,
    nc\_get\_vars\_schar, nc\_get\_vars\_short, nc\_get\_vars\_text,
    nc\_get\_vars\_uchar

 `ncvarid`
:   nc\_inq\_varid

 `ncvarinq`
:   nc\_inq\_var, nc\_inq\_vardimid, nc\_inq\_varname,
    nc\_inq\_varnatts, nc\_inq\_varndims, nc\_inq\_vartype

 `ncvarput`
:   nc\_put\_vara\_double, nc\_put\_vara\_float, nc\_put\_vara\_int,
    nc\_put\_vara\_long, nc\_put\_vara\_schar, nc\_put\_vara\_short,
    nc\_put\_vara\_text, nc\_put\_vara\_uchar

 `ncvarput1`
:   nc\_put\_var1\_double, nc\_put\_var1\_float, nc\_put\_var1\_int,
    nc\_put\_var1\_long, nc\_put\_var1\_schar, nc\_put\_var1\_short,
    nc\_put\_var1\_text, nc\_put\_var1\_uchar

 `ncvarputg`
:   nc\_put\_varm\_double, nc\_put\_varm\_float, nc\_put\_varm\_int,
    nc\_put\_varm\_long, nc\_put\_varm\_schar, nc\_put\_varm\_short,
    nc\_put\_varm\_text, nc\_put\_varm\_uchar, nc\_put\_vars\_double,
    nc\_put\_vars\_float, nc\_put\_vars\_int, nc\_put\_vars\_long,
    nc\_put\_vars\_schar, nc\_put\_vars\_short, nc\_put\_vars\_text,
    nc\_put\_vars\_uchar

 `ncvarrename`
:   nc\_rename\_var

 `(none)`
:   nc\_inq\_libvers

 `(none)`
:   nc\_strerror

* * * * *

  ------------------------------------------------------------ ------------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Other-Changes "Previous section in reading order")]   [[\>](#NetCDF_002d4-Error-Codes "Next section in reading order")]       [[\<\<](#NetCDF-2-C-Transition-Guide "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#NetCDF_002d4-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ------------------------------------------------------------------- --- ---------------------------------------------------------------------------------------- --------------------------- ---------------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

D. NetCDF-3 Error Codes {.appendix}
=======================

 

~~~~ {.example}
#define NC_NOERR        0       /* No Error */

#define NC_EBADID       (-33)   /* Not a netcdf id */
#define NC_ENFILE       (-34)   /* Too many netcdfs open */
#define NC_EEXIST       (-35)   /* netcdf file exists && NC_NOCLOBBER */
#define NC_EINVAL       (-36)   /* Invalid Argument */
#define NC_EPERM        (-37)   /* Write to read only */
#define NC_ENOTINDEFINE (-38)   /* Operation not allowed in data mode */
#define NC_EINDEFINE    (-39)   /* Operation not allowed in define mode */
#define NC_EINVALCOORDS (-40)   /* Index exceeds dimension bound */
#define NC_EMAXDIMS     (-41)   /* NC_MAX_DIMS exceeded */
#define NC_ENAMEINUSE   (-42)   /* String match to name in use */
#define NC_ENOTATT      (-43)   /* Attribute not found */
#define NC_EMAXATTS     (-44)   /* NC_MAX_ATTRS exceeded */
#define NC_EBADTYPE     (-45)   /* Not a netcdf data type */
#define NC_EBADDIM      (-46)   /* Invalid dimension id or name */
#define NC_EUNLIMPOS    (-47)   /* NC_UNLIMITED in the wrong index */
#define NC_EMAXVARS     (-48)   /* NC_MAX_VARS exceeded */
#define NC_ENOTVAR      (-49)   /* Variable not found */
#define NC_EGLOBAL      (-50)   /* Action prohibited on NC_GLOBAL varid */
#define NC_ENOTNC       (-51)   /* Not a netcdf file */
#define NC_ESTS         (-52)   /* In Fortran, string too short */
#define NC_EMAXNAME     (-53)   /* NC_MAX_NAME exceeded */
#define NC_EUNLIMIT     (-54)   /* NC_UNLIMITED size already in use */
#define NC_ENORECVARS   (-55)   /* nc_rec op when there are no record vars */
#define NC_ECHAR        (-56)   /* Attempt to convert between text & numbers */
#define NC_EEDGE        (-57)   /* Edge+start exceeds dimension bound */
#define NC_ESTRIDE      (-58)   /* Illegal stride */
#define NC_EBADNAME     (-59)   /* Attribute or variable name
                                         contains illegal characters */
/* N.B. following must match value in ncx.h */
#define NC_ERANGE       (-60)   /* Math result not representable */
#define NC_ENOMEM       (-61)   /* Memory allocation (malloc) failure */

#define NC_EVARSIZE     (-62)   /* One or more variable sizes violate
                                   format constraints */ 
#define NC_EDIMSIZE     (-63)   /* Invalid dimension size */
#define NC_ETRUNC       (-64)   /* File likely truncated or possibly corrupted */
~~~~

* * * * *

  ----------------------------------------------------------------------- ---------------------------------------------------------- --- ------------------------------------------------------------------------------------- --------------------------- ------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF_002d3-Error-Codes "Previous section in reading order")]   [[\>](#DAP-Error-Codes "Next section in reading order")]       [[\<\<](#NetCDF_002d3-Error-Codes "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#DAP-Error-Codes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------- ---------------------------------------------------------- --- ------------------------------------------------------------------------------------- --------------------------- ------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

E. NetCDF-4 Error Codes {.appendix}
=======================

NetCDF-4 uses all error codes from NetCDF-3 (see section [NetCDF-3 Error
Codes](#NetCDF_002d3-Error-Codes)). The following additional error codes
were added for new errors unique to netCDF-4.

 

~~~~ {.example}
#define NC_EHDFERR       (-101)
#define NC_ECANTREAD     (-102)
#define NC_ECANTWRITE    (-103)
#define NC_ECANTCREATE   (-104)
#define NC_EFILEMETA     (-105)
#define NC_EDIMMETA      (-106)
#define NC_EATTMETA      (-107)
#define NC_EVARMETA      (-108)
#define NC_ENOCOMPOUND   (-109)
#define NC_EATTEXISTS    (-110)
#define NC_ENOTNC4       (-111) /* Attempting netcdf-4 operation on netcdf-3 file. */  
#define NC_ESTRICTNC3    (-112) /* Attempting netcdf-4 operation on strict nc3 netcdf-4 file. */  
#define NC_EBADGRPID     (-113) /* Bad group id. Bad! */
#define NC_EBADTYPEID    (-114) /* Bad type id. */
#define NC_EBADFIELDID   (-115) /* Bad field id. */
#define NC_EUNKNAME      (-116)
~~~~

* * * * *

  ----------------------------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF_002d4-Error-Codes "Previous section in reading order")]   [[\>](#Combined-Index "Next section in reading order")]       [[\<\<](#NetCDF_002d4-Error-Codes "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

F. DAP Error Codes {.appendix}
==================

If the DAP client is enabled, then the following additional error codes
may occur.

 

~~~~ {.example}
#define NC_EDAP         (-66)   /* Generic DAP error */
#define NC_ECURL        (-67)   /* Generic libcurl error */
#define NC_EIO          (-68)   /* Generic IO error */
#define NC_ENODATA      (-69)   /* Attempt to access variable with no data */
#define NC_EDAPSVC      (-70)   /* DAP Server side error */
#define NC_EDAS         (-71)   /* Malformed or inaccessible DAS */
#define NC_EDDS         (-72)   /* Malformed or inaccessible DDS */
#define NC_EDATADDS     (-73)   /* Malformed or inaccessible DATADDS */
#define NC_EDAPURL      (-74)   /* Malformed DAP URL */
#define NC_EDAPCONSTRAINT (-75) /* Malformed DAP Constraint*/
#define NC_EDAP         (-66)   /* Generic DAP error */
#define NC_ECURL        (-67)   /* Generic libcurl error */
#define NC_EIO          (-68)   /* Generic IO error */
#define NC_ENODATA      (-69)   /* Attempt to access variable with no data */
#define NC_EDAPSVC      (-70)   /* DAP Server side error */
#define NC_EDAS         (-71)   /* Malformed or inaccessible DAS */
#define NC_EDDS         (-72)   /* Malformed or inaccessible DDS */
#define NC_EDATADDS     (-73)   /* Malformed or inaccessible DATADDS */
#define NC_EDAPURL      (-74)   /* Malformed DAP URL */
#define NC_EDAPCONSTRAINT (-75) /* Malformed DAP Constraint*/
~~~~

* * * * *

  -------------------------------------------------------------- -------- --- ---------------------------------------------------------------------------- --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#DAP-Error-Codes "Previous section in reading order")]   [ \> ]       [[\<\<](#DAP-Error-Codes "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [ \>\> ]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- -------- --- ---------------------------------------------------------------------------- --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

G. Index {.chapter}
========

Jump to:  

[**A**](#Index_cp_letter-A)   [**B**](#Index_cp_letter-B)  
[**C**](#Index_cp_letter-C)   [**D**](#Index_cp_letter-D)  
[**E**](#Index_cp_letter-E)   [**F**](#Index_cp_letter-F)  
[**G**](#Index_cp_letter-G)   [**H**](#Index_cp_letter-H)  
[**I**](#Index_cp_letter-I)   [**L**](#Index_cp_letter-L)  
[**M**](#Index_cp_letter-M)   [**N**](#Index_cp_letter-N)  
[**O**](#Index_cp_letter-O)   [**P**](#Index_cp_letter-P)  
[**R**](#Index_cp_letter-R)   [**S**](#Index_cp_letter-S)  
[**T**](#Index_cp_letter-T)   [**U**](#Index_cp_letter-U)  
[**V**](#Index_cp_letter-V)   [**W**](#Index_cp_letter-W)  
[**X**](#Index_cp_letter-X)   [**Z**](#Index_cp_letter-Z)  

Index Entry

Section

* * * * *

A

[abnormal termination](#index-abnormal-termination)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[aborting define mode](#index-aborting-define-mode)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[aborting definitions](#index-aborting-definitions)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[adding attributes](#index-adding-attributes)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[adding attributes using
nc\_redef](#index-adding-attributes-using-nc_005fredef)

[2.11 Put Open NetCDF Dataset into Define Mode:
nc\_redef](#nc_005fredef)

[adding dimensions](#index-adding-dimensions)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[adding dimensions using
nc\_redef](#index-adding-dimensions-using-nc_005fredef)

[2.11 Put Open NetCDF Dataset into Define Mode:
nc\_redef](#nc_005fredef)

[adding variables](#index-adding-variables)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[adding variables using
nc\_redef](#index-adding-variables-using-nc_005fredef)

[2.11 Put Open NetCDF Dataset into Define Mode:
nc\_redef](#nc_005fredef)

[API, C summary](#index-API_002c-C-summary)

[A. Summary of C Interface](#Summary-of-C-Interface)

[appending data to variable](#index-appending-data-to-variable)

[6.1 Introduction](#Variable-Introduction)

[array section, reading
mapped](#index-array-section_002c-reading-mapped)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[array section, reading
subsampled](#index-array-section_002c-reading-subsampled)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[array section, writing](#index-array-section_002c-writing)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[array section, writing
mapped](#index-array-section_002c-writing-mapped)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[array section, writing
subsampled](#index-array-section_002c-writing-subsampled)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[array, writing mapped](#index-array_002c-writing-mapped)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[attnum](#index-attnum)

[Usage](#Usage-20)

[attnump](#index-attnump)

[Usage](#Usage-20)

[attributes, adding](#index-attributes_002c-adding)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[attributes, array of strings](#index-attributes_002c-array-of-strings)

[6.31 Reading and Writing Character String Values](#Strings)

[attributes, character string](#index-attributes_002c-character-string)

[6.31.1 Reading and Writing Character String Values in the Classic
Model](#Classic-Strings)

[attributes, copying](#index-attributes_002c-copying)

[7.5 Copy Attribute from One NetCDF to Another:
nc\_copy\_att](#nc_005fcopy_005fatt)

[attributes, creating](#index-attributes_002c-creating)

[7.2 Create an Attribute: nc\_put\_att\_
type](#nc_005fput_005fatt_005f-type)

[attributes, deleting](#index-attributes_002c-deleting)

[7.7 Delete an Attribute: nc\_del\_att](#nc_005fdel_005fatt)

[attributes, deleting,
introduction](#index-attributes_002c-deleting_002c-introduction)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[attributes, finding length](#index-attributes_002c-finding-length)

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

[attributes, getting information
about](#index-attributes_002c-getting-information-about)

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

[attributes, ID](#index-attributes_002c-ID)

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

[attributes, inquiring about](#index-attributes_002c-inquiring-about)

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

[attributes, introduction](#index-attributes_002c-introduction)

[7.1 Introduction](#Attributes-Introduction)

[attributes, number of](#index-attributes_002c-number-of)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[attributes, operations on](#index-attributes_002c-operations-on)

[7.1 Introduction](#Attributes-Introduction)

[attributes, reading](#index-attributes_002c-reading)

[7.4 Get Attribute’s Values:nc\_get\_att\_
type](#nc_005fget_005fatt_005f-type)

[attributes, renaming](#index-attributes_002c-renaming)

[7.6 Rename an Attribute: nc\_rename\_att](#nc_005frename_005fatt)

[attributes, writing](#index-attributes_002c-writing)

[7.2 Create an Attribute: nc\_put\_att\_
type](#nc_005fput_005fatt_005f-type)

* * * * *

B

[backing out of definitions](#index-backing-out-of-definitions)

[2.17 Back Out of Recent Definitions: nc\_abort](#nc_005fabort)

[backward compatibility with v2
API](#index-backward-compatibility-with-v2-API)

[C. NetCDF 2 to NetCDF 3 C Transition
Guide](#NetCDF-2-C-Transition-Guide)

[big-endian](#index-big_002dendian)

[6.17 Define Endianness of a Variable:
`nc_def_var_endian`](#nc_005fdef_005fvar_005fendian)

[bit lengths of data types](#index-bit-lengths-of-data-types)

[6.2 Language Types Corresponding to netCDF external data
types](#Variable-Types)

[bit lengths of netcdf-3 data
types](#index-bit-lengths-of-netcdf_002d3-data-types)

[6.3 NetCDF-3 Classic and 64-Bit Offset Data
Types](#NetCDF_002d3-Variable-Types)

[bit lengths of netcdf-4 data
types](#index-bit-lengths-of-netcdf_002d4-data-types)

[6.4 NetCDF-4 Atomic Types](#NetCDF_002d4-Atomic-Types)

[byte vs. char fill values](#index-byte-vs_002e-char-fill-values)

[6.33 Fill Values](#Fill-Values)

[byte, zero](#index-byte_002c-zero)

[6.31.1 Reading and Writing Character String Values in the Classic
Model](#Classic-Strings)

* * * * *

C

[C API summary](#index-C-API-summary)

[A. Summary of C Interface](#Summary-of-C-Interface)

[call sequence, typical](#index-call-sequence_002c-typical)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[canceling definitions](#index-canceling-definitions)

[2.17 Back Out of Recent Definitions: nc\_abort](#nc_005fabort)

[character-string data,
writing](#index-character_002dstring-data_002c-writing)

[6.31.1 Reading and Writing Character String Values in the Classic
Model](#Classic-Strings)

[checksum](#index-checksum)

[6.15 Define Checksum Parameters for a Variable:
`nc_def_var_fletcher32`](#nc_005fdef_005fvar_005ffletcher32)

[chunking](#index-chunking)

[6.6 Define Chunking Parameters for a Variable:
`nc_def_var_chunking`](#nc_005fdef_005fvar_005fchunking)

[code templates](#index-code-templates)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[compiling with netCDF library](#index-compiling-with-netCDF-library)

[1.6 Compiling and Linking with the NetCDF Library](#Compiling)

[compound types, overview](#index-compound-types_002c-overview)

[5.6 Compound Types Introduction](#Compound-Types)

[compression, setting
parameters](#index-compression_002c-setting-parameters)

[6.12 Define Compression Parameters for a Variable:
`nc_def_var_deflate`](#nc_005fdef_005fvar_005fdeflate)

[contiguous](#index-contiguous)

[6.6 Define Chunking Parameters for a Variable:
`nc_def_var_chunking`](#nc_005fdef_005fvar_005fchunking)

[copying attributes](#index-copying-attributes)

[7.5 Copy Attribute from One NetCDF to Another:
nc\_copy\_att](#nc_005fcopy_005fatt)

[create flag, setting default](#index-create-flag_002c-setting-default)

[2.19 Set Default Creation Format:
nc\_set\_default\_format](#nc_005fset_005fdefault_005fformat)

[creating a dataset](#index-creating-a-dataset)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[creating variables](#index-creating-variables)

[6.5 Create a Variable: `nc_def_var`](#nc_005fdef_005fvar)

* * * * *

D

[DAP error codes](#index-DAP-error-codes)

[F. DAP Error Codes](#DAP-Error-Codes)

[datasets, overview](#index-datasets_002c-overview)

[2. Datasets](#Datasets)

[deflate](#index-deflate)

[6.12 Define Compression Parameters for a Variable:
`nc_def_var_deflate`](#nc_005fdef_005fvar_005fdeflate)

[deleting attributes](#index-deleting-attributes)

[7.7 Delete an Attribute: nc\_del\_att](#nc_005fdel_005fatt)

[dimensions, adding](#index-dimensions_002c-adding)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[dimensions, number of](#index-dimensions_002c-number-of)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

* * * * *

E

[endianness](#index-endianness)

[6.17 Define Endianness of a Variable:
`nc_def_var_endian`](#nc_005fdef_005fvar_005fendian)

[entire variable, reading](#index-entire-variable_002c-reading)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[entire variable, writing](#index-entire-variable_002c-writing)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[enum type](#index-enum-type)

[5.29 Enum Type Introduction](#Enum-Type)

[error codes](#index-error-codes)

[2.3 Get error message corresponding to error status:
nc\_strerror](#nc_005fstrerror)

[error codes, DAP](#index-error-codes_002c-DAP)

[F. DAP Error Codes](#DAP-Error-Codes)

[error codes, netcdf-3](#index-error-codes_002c-netcdf_002d3)

[D. NetCDF-3 Error Codes](#NetCDF_002d3-Error-Codes)

[error codes, netcdf-4](#index-error-codes_002c-netcdf_002d4)

[E. NetCDF-4 Error Codes](#NetCDF_002d4-Error-Codes)

[error handling](#index-error-handling)

[1.5 Error Handling](#Errors)

* * * * *

F

[file name](#index-file-name)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[file path](#index-file-path)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[fill](#index-fill)

[6.10 Define Fill Parameters for a Variable:
`nc_def_var_fill`](#nc_005fdef_005fvar_005ffill)

[fill values](#index-fill-values)

[6.33 Fill Values](#Fill-Values)

[fletcher32](#index-fletcher32)

[6.15 Define Checksum Parameters for a Variable:
`nc_def_var_fletcher32`](#nc_005fdef_005fvar_005ffletcher32)

[format version](#index-format-version)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

* * * * *

G

[groups, overview](#index-groups_002c-overview)

[3. Groups](#Groups)

* * * * *

H

[handle\_err](#index-handle_005ferr)

[2.3 Get error message corresponding to error status:
nc\_strerror](#nc_005fstrerror)

[HDF5 chunk cache](#index-HDF5-chunk-cache)

[2.20 Set HDF5 Chunk Cache for Future File Opens/Creates:
nc\_set\_chunk\_cache](#nc_005fset_005fchunk_005fcache)

[HDF5 chunk cache](#index-HDF5-chunk-cache-1)

[2.21 Get the HDF5 Chunk Cache Settings for Future File Opens/Creates:
nc\_get\_chunk\_cache](#nc_005fget_005fchunk_005fcache)

[HDF5 chunk cache](#index-HDF5-chunk-cache-2)

[6.9 Get the HDF5 Chunk Cache Settings for a Variable:
nc\_get\_var\_chunk\_cache](#nc_005fget_005fvar_005fchunk_005fcache)

[HDF5 chunk cache,
per-variable](#index-HDF5-chunk-cache_002c-per_002dvariable)

[6.8 Set HDF5 Chunk Cache for a Variable:
nc\_set\_var\_chunk\_cache](#nc_005fset_005fvar_005fchunk_005fcache)

[HDF5 errors, first create](#index-HDF5-errors_002c-first-create)

[2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)

[HDF5 errors, first create](#index-HDF5-errors_002c-first-create-1)

[2.8 Open a NetCDF Dataset for Access: nc\_open](#nc_005fopen)

* * * * *

I

[inquiring about attributes](#index-inquiring-about-attributes)

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

[inquiring about variables](#index-inquiring-about-variables)

[6.19 Get a Variable ID from Its Name:
nc\_inq\_varid](#nc_005finq_005fvarid)

[interface descriptions](#index-interface-descriptions)

[2.1 NetCDF Library Interface Descriptions](#Interface-Descriptions)

* * * * *

L

[length of attributes](#index-length-of-attributes)

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

[lenp](#index-lenp)

[Usage](#Usage-20)

[linking to netCDF library](#index-linking-to-netCDF-library)

[1.6 Compiling and Linking with the NetCDF Library](#Compiling)

[little-endian](#index-little_002dendian)

[6.17 Define Endianness of a Variable:
`nc_def_var_endian`](#nc_005fdef_005fvar_005fendian)

* * * * *

M

[mapped array section,
writing](#index-mapped-array-section_002c-writing)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[mapped array, writing](#index-mapped-array_002c-writing)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

* * * * *

N

[name](#index-name)

[Usage](#Usage-20)

[ncid](#index-ncid)

[Usage](#Usage-20)

[NC\_64BIT\_OFFSET](#index-NC_005f64BIT_005fOFFSET)

[2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)

[NC\_64BIT\_OFFSET](#index-NC_005f64BIT_005fOFFSET-1)

[2.6 Create a NetCDF Dataset With Performance Options:
nc\_\_create](#nc_005f_005fcreate)

[nc\_abort](#index-nc_005fabort)

[2.17 Back Out of Recent Definitions: nc\_abort](#nc_005fabort)

[nc\_abort, example](#index-nc_005fabort_002c-example)

[2.17 Back Out of Recent Definitions: nc\_abort](#nc_005fabort)

[NC\_CLOBBER](#index-NC_005fCLOBBER)

[2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)

[NC\_CLOBBER](#index-NC_005fCLOBBER-1)

[2.6 Create a NetCDF Dataset With Performance Options:
nc\_\_create](#nc_005f_005fcreate)

[NC\_CLOBBER](#index-NC_005fCLOBBER-2)

[2.7 Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)

[nc\_close](#index-nc_005fclose)

[2.14 Close an Open NetCDF Dataset: nc\_close](#nc_005fclose)

[nc\_close, example](#index-nc_005fclose_002c-example)

[2.14 Close an Open NetCDF Dataset: nc\_close](#nc_005fclose)

[nc\_close, root group](#index-nc_005fclose_002c-root-group)

[2.14 Close an Open NetCDF Dataset: nc\_close](#nc_005fclose)

[nc\_close, typical use](#index-nc_005fclose_002c-typical-use)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[nc\_copy\_att](#index-nc_005fcopy_005fatt)

[7.5 Copy Attribute from One NetCDF to Another:
nc\_copy\_att](#nc_005fcopy_005fatt)

[nc\_copy\_att, example](#index-nc_005fcopy_005fatt_002c-example)

[7.5 Copy Attribute from One NetCDF to Another:
nc\_copy\_att](#nc_005fcopy_005fatt)

[nc\_copy\_var](#index-nc_005fcopy_005fvar)

[6.35 Copy a Variable from One File to Another:
nc\_copy\_var](#nc_005fcopy_005fvar)

[nc\_create](#index-nc_005fcreate)

[2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)

[nc\_create, example](#index-nc_005fcreate_002c-example)

[2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)

[nc\_create, flags](#index-nc_005fcreate_002c-flags)

[2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)

[nc\_create, typical use](#index-nc_005fcreate_002c-typical-use)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[nc\_create\_par](#index-nc_005fcreate_005fpar)

[2.7 Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)

[nc\_create\_par, example](#index-nc_005fcreate_005fpar_002c-example)

[2.7 Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)

[nc\_create\_par, flags](#index-nc_005fcreate_005fpar_002c-flags)

[2.7 Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)

[nc\_def\_compound](#index-nc_005fdef_005fcompound)

[5.7 Creating a Compound Type:
nc\_def\_compound](#nc_005fdef_005fcompound)

[nc\_def\_dim](#index-nc_005fdef_005fdim)

[4.2 Create a Dimension: nc\_def\_dim](#nc_005fdef_005fdim)

[nc\_def\_dim, example](#index-nc_005fdef_005fdim_002c-example)

[4.2 Create a Dimension: nc\_def\_dim](#nc_005fdef_005fdim)

[nc\_def\_dim, typical use](#index-nc_005fdef_005fdim_002c-typical-use)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[nc\_def\_dim, typical
use](#index-nc_005fdef_005fdim_002c-typical-use-1)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[nc\_def\_enum](#index-nc_005fdef_005fenum)

[5.30 Creating a Enum Type: nc\_def\_enum](#nc_005fdef_005fenum)

[nc\_def\_grp](#index-nc_005fdef_005fgrp)

[3.11 Create a New Group: nc\_def\_grp](#nc_005fdef_005fgrp)

[nc\_def\_opaque](#index-nc_005fdef_005fopaque)

[5.27 Creating Opaque Types: nc\_def\_opaque](#nc_005fdef_005fopaque)

[nc\_def\_var](#index-nc_005fdef_005fvar)

[6.5 Create a Variable: `nc_def_var`](#nc_005fdef_005fvar)

[nc\_def\_var, example](#index-nc_005fdef_005fvar_002c-example)

[6.5 Create a Variable: `nc_def_var`](#nc_005fdef_005fvar)

[nc\_def\_var, typical use](#index-nc_005fdef_005fvar_002c-typical-use)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[nc\_def\_var\_chunking](#index-nc_005fdef_005fvar_005fchunking)

[6.6 Define Chunking Parameters for a Variable:
`nc_def_var_chunking`](#nc_005fdef_005fvar_005fchunking)

[nc\_def\_var\_deflate](#index-nc_005fdef_005fvar_005fdeflate)

[6.12 Define Compression Parameters for a Variable:
`nc_def_var_deflate`](#nc_005fdef_005fvar_005fdeflate)

[nc\_def\_var\_endian](#index-nc_005fdef_005fvar_005fendian)

[6.17 Define Endianness of a Variable:
`nc_def_var_endian`](#nc_005fdef_005fvar_005fendian)

[nc\_def\_var\_fill](#index-nc_005fdef_005fvar_005ffill)

[6.10 Define Fill Parameters for a Variable:
`nc_def_var_fill`](#nc_005fdef_005fvar_005ffill)

[nc\_def\_var\_fletcher32](#index-nc_005fdef_005fvar_005ffletcher32)

[6.15 Define Checksum Parameters for a Variable:
`nc_def_var_fletcher32`](#nc_005fdef_005fvar_005ffletcher32)

[nc\_def\_vlen](#index-nc_005fdef_005fvlen)

[5.22 Define a Variable Length Array (VLEN):
nc\_def\_vlen](#nc_005fdef_005fvlen)

[nc\_def\_vlen](#index-nc_005fdef_005fvlen-1)

[5.23 Learning about a Variable Length Array (VLEN) Type:
nc\_inq\_vlen](#nc_005finq_005fvlen)

[nc\_del\_att](#index-nc_005fdel_005fatt)

[7.7 Delete an Attribute: nc\_del\_att](#nc_005fdel_005fatt)

[nc\_del\_att, example](#index-nc_005fdel_005fatt_002c-example)

[7.7 Delete an Attribute: nc\_del\_att](#nc_005fdel_005fatt)

[nc\_enddef](#index-nc_005fenddef)

[2.12 Leave Define Mode: nc\_enddef](#nc_005fenddef)

[nc\_enddef, example](#index-nc_005fenddef_002c-example)

[2.12 Leave Define Mode: nc\_enddef](#nc_005fenddef)

[nc\_enddef, typical use](#index-nc_005fenddef_002c-typical-use)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[nc\_free\_string](#index-nc_005ffree_005fstring)

[6.32 Releasing Memory for a NC\_STRING:
nc\_free\_string](#nc_005ffree_005fstring)

[nc\_free\_vlen](#index-nc_005ffree_005fvlen)

[5.24 Releasing Memory for a Variable Length Array (VLEN) Type:
nc\_free\_vlen](#nc_005ffree_005fvlen)

[nc\_free\_vlen](#index-nc_005ffree_005fvlen-1)

[5.25 Releasing Memory for an Array of Variable Length Array (VLEN)
Type: nc\_free\_vlen](#nc_005ffree_005fvlens)

[nc\_get\_att, typical use](#index-nc_005fget_005fatt_002c-typical-use)

[1.2 Reading a NetCDF Dataset with Known Names](#Reading-Known)

[nc\_get\_att, typical
use](#index-nc_005fget_005fatt_002c-typical-use-1)

[1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)

[nc\_get\_att\_ type](#index-nc_005fget_005fatt_005f-type)

[7.4 Get Attribute’s Values:nc\_get\_att\_
type](#nc_005fget_005fatt_005f-type)

[nc\_get\_att\_ type,
example](#index-nc_005fget_005fatt_005f-type_002c-example)

[7.4 Get Attribute’s Values:nc\_get\_att\_
type](#nc_005fget_005fatt_005f-type)

[nc\_get\_att\_ubyte](#index-nc_005fget_005fatt_005fubyte)

[7.7 Delete an Attribute: nc\_del\_att](#nc_005fdel_005fatt)

[nc\_get\_chunk\_cache](#index-nc_005fget_005fchunk_005fcache)

[2.21 Get the HDF5 Chunk Cache Settings for Future File Opens/Creates:
nc\_get\_chunk\_cache](#nc_005fget_005fchunk_005fcache)

[nc\_get\_chunk\_cache](#index-nc_005fget_005fchunk_005fcache-1)

[6.9 Get the HDF5 Chunk Cache Settings for a Variable:
nc\_get\_var\_chunk\_cache](#nc_005fget_005fvar_005fchunk_005fcache)

[nc\_get\_var](#index-nc_005fget_005fvar)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var, typical use](#index-nc_005fget_005fvar_002c-typical-use)

[1.2 Reading a NetCDF Dataset with Known Names](#Reading-Known)

[nc\_get\_var, typical
use](#index-nc_005fget_005fvar_002c-typical-use-1)

[1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)

[nc\_get\_var1](#index-nc_005fget_005fvar1)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_ type](#index-nc_005fget_005fvar1_005f-type)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_ type,
example](#index-nc_005fget_005fvar1_005f-type_002c-example)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_double](#index-nc_005fget_005fvar1_005fdouble)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_float](#index-nc_005fget_005fvar1_005ffloat)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_int](#index-nc_005fget_005fvar1_005fint)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_long](#index-nc_005fget_005fvar1_005flong)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_longlong](#index-nc_005fget_005fvar1_005flonglong)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_schar](#index-nc_005fget_005fvar1_005fschar)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_short](#index-nc_005fget_005fvar1_005fshort)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_string](#index-nc_005fget_005fvar1_005fstring)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_text](#index-nc_005fget_005fvar1_005ftext)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_ubyte](#index-nc_005fget_005fvar1_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_get\_var1\_uchar](#index-nc_005fget_005fvar1_005fuchar)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_uint](#index-nc_005fget_005fvar1_005fuint)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_ulonglong](#index-nc_005fget_005fvar1_005fulonglong)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_var1\_ushort](#index-nc_005fget_005fvar1_005fushort)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[nc\_get\_vara](#index-nc_005fget_005fvara)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_ type](#index-nc_005fget_005fvara_005f-type)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_ type,
example](#index-nc_005fget_005fvara_005f-type_002c-example)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_double](#index-nc_005fget_005fvara_005fdouble)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_float](#index-nc_005fget_005fvara_005ffloat)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_int](#index-nc_005fget_005fvara_005fint)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_long](#index-nc_005fget_005fvara_005flong)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_longlong](#index-nc_005fget_005fvara_005flonglong)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_schar](#index-nc_005fget_005fvara_005fschar)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_short](#index-nc_005fget_005fvara_005fshort)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_string](#index-nc_005fget_005fvara_005fstring)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_text](#index-nc_005fget_005fvara_005ftext)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_ubyte](#index-nc_005fget_005fvara_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_get\_vara\_uchar](#index-nc_005fget_005fvara_005fuchar)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_uint](#index-nc_005fget_005fvara_005fuint)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_ulonglong](#index-nc_005fget_005fvara_005fulonglong)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_vara\_ushort](#index-nc_005fget_005fvara_005fushort)

[6.28 Read an Array of Values: nc\_get\_vara\_
type](#nc_005fget_005fvara_005f-type)

[nc\_get\_varm](#index-nc_005fget_005fvarm)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_ type](#index-nc_005fget_005fvarm_005f-type)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_ type,
example](#index-nc_005fget_005fvarm_005f-type_002c-example)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_double](#index-nc_005fget_005fvarm_005fdouble)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_float](#index-nc_005fget_005fvarm_005ffloat)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_int](#index-nc_005fget_005fvarm_005fint)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_long](#index-nc_005fget_005fvarm_005flong)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_longlong](#index-nc_005fget_005fvarm_005flonglong)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_schar](#index-nc_005fget_005fvarm_005fschar)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_short](#index-nc_005fget_005fvarm_005fshort)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_string](#index-nc_005fget_005fvarm_005fstring)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_text](#index-nc_005fget_005fvarm_005ftext)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_ubyte](#index-nc_005fget_005fvarm_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_get\_varm\_uchar](#index-nc_005fget_005fvarm_005fuchar)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_uint](#index-nc_005fget_005fvarm_005fuint)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_ulonglong](#index-nc_005fget_005fvarm_005fulonglong)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_varm\_ushort](#index-nc_005fget_005fvarm_005fushort)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[nc\_get\_vars](#index-nc_005fget_005fvars)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_ type](#index-nc_005fget_005fvars_005f-type)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_ type,
example](#index-nc_005fget_005fvars_005f-type_002c-example)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_double](#index-nc_005fget_005fvars_005fdouble)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_float](#index-nc_005fget_005fvars_005ffloat)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_int](#index-nc_005fget_005fvars_005fint)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_long](#index-nc_005fget_005fvars_005flong)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_longlong](#index-nc_005fget_005fvars_005flonglong)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_schar](#index-nc_005fget_005fvars_005fschar)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_short](#index-nc_005fget_005fvars_005fshort)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_string](#index-nc_005fget_005fvars_005fstring)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_text](#index-nc_005fget_005fvars_005ftext)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_ubyte](#index-nc_005fget_005fvars_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_get\_vars\_uchar](#index-nc_005fget_005fvars_005fuchar)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_uint](#index-nc_005fget_005fvars_005fuint)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_ulonglong](#index-nc_005fget_005fvars_005fulonglong)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_vars\_ushort](#index-nc_005fget_005fvars_005fushort)

[6.29 Read a Subsampled Array of Values: nc\_get\_vars\_
type](#nc_005fget_005fvars_005f-type)

[nc\_get\_var\_ type](#index-nc_005fget_005fvar_005f-type)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_ type,
example](#index-nc_005fget_005fvar_005f-type_002c-example)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_double](#index-nc_005fget_005fvar_005fdouble)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_float](#index-nc_005fget_005fvar_005ffloat)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_int](#index-nc_005fget_005fvar_005fint)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_long](#index-nc_005fget_005fvar_005flong)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_longlong](#index-nc_005fget_005fvar_005flonglong)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_schar](#index-nc_005fget_005fvar_005fschar)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_short](#index-nc_005fget_005fvar_005fshort)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_string](#index-nc_005fget_005fvar_005fstring)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_text](#index-nc_005fget_005fvar_005ftext)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_ubyte](#index-nc_005fget_005fvar_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_get\_var\_uchar](#index-nc_005fget_005fvar_005fuchar)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_uint](#index-nc_005fget_005fvar_005fuint)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_ulonglong](#index-nc_005fget_005fvar_005fulonglong)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_get\_var\_ushort](#index-nc_005fget_005fvar_005fushort)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[nc\_inq Family](#index-nc_005finq-Family)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[nc\_inq Family, example](#index-nc_005finq-Family_002c-example)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[nc\_inq, typical use](#index-nc_005finq_002c-typical-use)

[1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)

[nc\_inq\_att Family](#index-nc_005finq_005fatt-Family)

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

[nc\_inq\_att Family,
example](#index-nc_005finq_005fatt-Family_002c-example)

[7.3 Get Information about an Attribute: nc\_inq\_att
Family](#nc_005finq_005fatt-Family)

[nc\_inq\_att, typical use](#index-nc_005finq_005fatt_002c-typical-use)

[1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)

[nc\_inq\_compound](#index-nc_005finq_005fcompound)

[5.10 Learn About a Compound Type:
nc\_inq\_compound](#nc_005finq_005fcompound)

[nc\_inq\_compound\_field](#index-nc_005finq_005fcompound_005ffield)

[5.14 Learn About a Field of a Compound Type:
nc\_inq\_compound\_field](#nc_005finq_005fcompound_005ffield)

[nc\_inq\_compound\_fielddim\_sizes](#index-nc_005finq_005fcompound_005ffielddim_005fsizes)

[5.20 Find the Sizes of Dimensions in an Array Field:
nc\_inq\_compound\_fielddim\_sizes](#nc_005finq_005fcompound_005ffielddim_005fsizes)

[nc\_inq\_compound\_fieldindex](#index-nc_005finq_005fcompound_005ffieldindex)

[5.16 Get the FieldID of a Compound Type Field:
nc\_inq\_compound\_fieldindex](#nc_005finq_005fcompound_005ffieldindex)

[nc\_inq\_compound\_fieldname](#index-nc_005finq_005fcompound_005ffieldname)

[5.15 Find the Name of a Field in a Compound Type:
nc\_inq\_compound\_fieldname](#nc_005finq_005fcompound_005ffieldname)

[nc\_inq\_compound\_fieldndims](#index-nc_005finq_005fcompound_005ffieldndims)

[5.19 Find the Number of Dimensions in an Array Field:
nc\_inq\_compound\_fieldndims](#nc_005finq_005fcompound_005ffieldndims)

[nc\_inq\_compound\_fieldoffset](#index-nc_005finq_005fcompound_005ffieldoffset)

[5.17 Get the Offset of a Field:
nc\_inq\_compound\_fieldoffset](#nc_005finq_005fcompound_005ffieldoffset)

[nc\_inq\_compound\_fieldtype](#index-nc_005finq_005fcompound_005ffieldtype)

[5.18 Find the Type of a Field:
nc\_inq\_compound\_fieldtype](#nc_005finq_005fcompound_005ffieldtype)

[nc\_inq\_compound\_name](#index-nc_005finq_005fcompound_005fname)

[5.11 Learn the Name of a Compound Type:
nc\_inq\_compound\_name](#nc_005finq_005fcompound_005fname)

[nc\_inq\_compound\_nfields](#index-nc_005finq_005fcompound_005fnfields)

[5.13 Learn the Number of Fields in a Compound Type:
nc\_inq\_compound\_nfields](#nc_005finq_005fcompound_005fnfields)

[nc\_inq\_compound\_size](#index-nc_005finq_005fcompound_005fsize)

[5.12 Learn the Size of a Compound Type:
nc\_inq\_compound\_size](#nc_005finq_005fcompound_005fsize)

[nc\_inq\_dim](#index-nc_005finq_005fdim)

[4.4 Inquire about a Dimension: nc\_inq\_dim
Family](#nc_005finq_005fdim-Family)

[nc\_inq\_dim Family](#index-nc_005finq_005fdim-Family)

[4.4 Inquire about a Dimension: nc\_inq\_dim
Family](#nc_005finq_005fdim-Family)

[nc\_inq\_dim Family,
example](#index-nc_005finq_005fdim-Family_002c-example)

[4.4 Inquire about a Dimension: nc\_inq\_dim
Family](#nc_005finq_005fdim-Family)

[nc\_inq\_dim, typical use](#index-nc_005finq_005fdim_002c-typical-use)

[1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)

[nc\_inq\_dimid](#index-nc_005finq_005fdimid)

[4.3 Get a Dimension ID from Its Name:
nc\_inq\_dimid](#nc_005finq_005fdimid)

[nc\_inq\_dimid](#index-nc_005finq_005fdimid-1)

[4.4 Inquire about a Dimension: nc\_inq\_dim
Family](#nc_005finq_005fdim-Family)

[nc\_inq\_dimid, example](#index-nc_005finq_005fdimid_002c-example)

[4.3 Get a Dimension ID from Its Name:
nc\_inq\_dimid](#nc_005finq_005fdimid)

[nc\_inq\_dimid, typical
use](#index-nc_005finq_005fdimid_002c-typical-use)

[1.2 Reading a NetCDF Dataset with Known Names](#Reading-Known)

[nc\_inq\_dimids](#index-nc_005finq_005fdimids)

[3.4 Find all Dimensions Visible in a Group:
nc\_inq\_dimids](#nc_005finq_005fdimids)

[nc\_inq\_dimids](#index-nc_005finq_005fdimids-1)

[4.4 Inquire about a Dimension: nc\_inq\_dim
Family](#nc_005finq_005fdim-Family)

[nc\_inq\_dimlen](#index-nc_005finq_005fdimlen)

[4.4 Inquire about a Dimension: nc\_inq\_dim
Family](#nc_005finq_005fdim-Family)

[nc\_inq\_dimname](#index-nc_005finq_005fdimname)

[4.4 Inquire about a Dimension: nc\_inq\_dim
Family](#nc_005finq_005fdim-Family)

[nc\_inq\_enum](#index-nc_005finq_005fenum)

[5.32 Learn About a Enum Type: nc\_inq\_enum](#nc_005finq_005fenum)

[nc\_inq\_enum\_ident](#index-nc_005finq_005fenum_005fident)

[5.34 Learn the Name of a Enum Type:
nc\_inq\_enum\_ident](#nc_005finq_005fenum_005fident)

[nc\_inq\_enum\_member](#index-nc_005finq_005fenum_005fmember)

[5.33 Learn the Name of a Enum Type:
nc\_inq\_enum\_member](#nc_005finq_005fenum_005fmember)

[nc\_inq\_format](#index-nc_005finq_005fformat)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[nc\_inq\_grpname](#index-nc_005finq_005fgrpname)

[3.5 Find a Group’s Name: nc\_inq\_grpname](#nc_005finq_005fgrpname)

[nc\_inq\_grpname\_full](#index-nc_005finq_005fgrpname_005ffull)

[3.6 Find a Group’s Full Name:
nc\_inq\_grpname\_full](#nc_005finq_005fgrpname_005ffull)

[nc\_inq\_grpname\_len](#index-nc_005finq_005fgrpname_005flen)

[3.7 Find the Length of a Group’s Full Name:
nc\_inq\_grpname\_len](#nc_005finq_005fgrpname_005flen)

[nc\_inq\_grps](#index-nc_005finq_005fgrps)

[3.2 Get a List of Groups in a Group:
nc\_inq\_grps](#nc_005finq_005fgrps)

[nc\_inq\_grp\_parent](#index-nc_005finq_005fgrp_005fparent)

[3.8 Find a Group’s Parent:
nc\_inq\_grp\_parent](#nc_005finq_005fgrp_005fparent)

[nc\_inq\_grp\_parent](#index-nc_005finq_005fgrp_005fparent-1)

[3.9 Find a Group by Name:
nc\_inq\_grp\_ncid](#nc_005finq_005fgrp_005fncid)

[nc\_inq\_grp\_parent](#index-nc_005finq_005fgrp_005fparent-2)

[3.10 Find a Group by its Fully-qualified Name:
nc\_inq\_grp\_full\_ncid](#nc_005finq_005fgrp_005ffull_005fncid)

[nc\_inq\_libvers](#index-nc_005finq_005flibvers)

[2.4 Get netCDF library version:
nc\_inq\_libvers](#nc_005finq_005flibvers)

[nc\_inq\_libvers, example](#index-nc_005finq_005flibvers_002c-example)

[2.4 Get netCDF library version:
nc\_inq\_libvers](#nc_005finq_005flibvers)

[nc\_inq\_natts](#index-nc_005finq_005fnatts)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[nc\_inq\_ncid](#index-nc_005finq_005fncid)

[3.1 Find a Group ID: nc\_inq\_ncid](#nc_005finq_005fncid)

[nc\_inq\_ndims](#index-nc_005finq_005fndims)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[nc\_inq\_nvars](#index-nc_005finq_005fnvars)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[nc\_inq\_opaque](#index-nc_005finq_005fopaque)

[5.28 Learn About an Opaque Type:
nc\_inq\_opaque](#nc_005finq_005fopaque)

[nc\_inq\_path](#index-nc_005finq_005fpath)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[nc\_inq\_type](#index-nc_005finq_005ftype)

[5.4 Learn About a User Defined Type:
nc\_inq\_type](#nc_005finq_005ftype)

[nc\_inq\_typeid](#index-nc_005finq_005ftypeid)

[5.3 Find a Typeid from Group and Name:
nc\_inq\_typeid](#nc_005finq_005ftypeid)

[nc\_inq\_typeids](#index-nc_005finq_005ftypeids)

[5.2 Learn the IDs of All Types in Group:
nc\_inq\_typeids](#nc_005finq_005ftypeids)

[nc\_inq\_unlimdim](#index-nc_005finq_005funlimdim)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[nc\_inq\_unlimdims](#index-nc_005finq_005funlimdims)

[4.6 Find All Unlimited Dimension IDs:
nc\_inq\_unlimdims](#nc_005finq_005funlimdims)

[nc\_inq\_user\_type](#index-nc_005finq_005fuser_005ftype)

[5.5 Learn About a User Defined Type:
nc\_inq\_user\_type](#nc_005finq_005fuser_005ftype)

[nc\_inq\_var](#index-nc_005finq_005fvar)

[6.20 Get Information about a Variable from Its ID:
nc\_inq\_var](#nc_005finq_005fvar)

[nc\_inq\_var, example](#index-nc_005finq_005fvar_002c-example)

[6.20 Get Information about a Variable from Its ID:
nc\_inq\_var](#nc_005finq_005fvar)

[nc\_inq\_var, typical use](#index-nc_005finq_005fvar_002c-typical-use)

[1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)

[nc\_inq\_varid](#index-nc_005finq_005fvarid)

[6.19 Get a Variable ID from Its Name:
nc\_inq\_varid](#nc_005finq_005fvarid)

[nc\_inq\_varid, example](#index-nc_005finq_005fvarid_002c-example)

[6.19 Get a Variable ID from Its Name:
nc\_inq\_varid](#nc_005finq_005fvarid)

[nc\_inq\_varid, typical
use](#index-nc_005finq_005fvarid_002c-typical-use)

[1.2 Reading a NetCDF Dataset with Known Names](#Reading-Known)

[nc\_inq\_varids](#index-nc_005finq_005fvarids)

[3.3 Find all the Variables in a Group:
nc\_inq\_varids](#nc_005finq_005fvarids)

[nc\_inq\_varname](#index-nc_005finq_005fvarname)

[6.20 Get Information about a Variable from Its ID:
nc\_inq\_var](#nc_005finq_005fvar)

[nc\_inq\_varnatts](#index-nc_005finq_005fvarnatts)

[6.20 Get Information about a Variable from Its ID:
nc\_inq\_var](#nc_005finq_005fvar)

[nc\_inq\_varndims](#index-nc_005finq_005fvarndims)

[6.20 Get Information about a Variable from Its ID:
nc\_inq\_var](#nc_005finq_005fvar)

[nc\_inq\_vartype](#index-nc_005finq_005fvartype)

[6.20 Get Information about a Variable from Its ID:
nc\_inq\_var](#nc_005finq_005fvar)

[nc\_inq\_var\_chunking](#index-nc_005finq_005fvar_005fchunking)

[6.7 Learn About Chunking Parameters for a Variable:
`nc_inq_var_chunking`](#nc_005finq_005fvar_005fchunking)

[nc\_inq\_var\_deflate](#index-nc_005finq_005fvar_005fdeflate)

[6.13 Learn About Deflate Parameters for a Variable:
`nc_inq_var_deflate`](#nc_005finq_005fvar_005fdeflate)

[nc\_inq\_var\_endian](#index-nc_005finq_005fvar_005fendian)

[6.18 Learn About Endian Parameters for a Variable:
`nc_inq_var_endian`](#nc_005finq_005fvar_005fendian)

[nc\_inq\_var\_fill](#index-nc_005finq_005fvar_005ffill)

[6.11 Learn About Fill Parameters for a Variable:
`nc_inq_var_fill`](#nc_005finq_005fvar_005ffill)

[nc\_inq\_var\_fletcher32](#index-nc_005finq_005fvar_005ffletcher32)

[6.16 Learn About Checksum Parameters for a Variable:
`nc_inq_var_fletcher32`](#nc_005finq_005fvar_005ffletcher32)

[nc\_inq\_var\_szip](#index-nc_005finq_005fvar_005fszip)

[6.14 Learn About Szip Parameters for a Variable:
`nc_inq_var_szip`](#nc_005finq_005fvar_005fszip)

[nc\_insert\_array\_compound](#index-nc_005finsert_005farray_005fcompound)

[5.9 Inserting an Array Field into a Compound Type:
nc\_insert\_array\_compound](#nc_005finsert_005farray_005fcompound)

[nc\_insert\_compound](#index-nc_005finsert_005fcompound)

[5.8 Inserting a Field into a Compound Type:
nc\_insert\_compound](#nc_005finsert_005fcompound)

[nc\_insert\_enum](#index-nc_005finsert_005fenum)

[5.31 Inserting a Field into a Enum Type:
nc\_insert\_enum](#nc_005finsert_005fenum)

[NC\_MPIIO](#index-NC_005fMPIIO)

[2.7 Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)

[NC\_MPIPOSIX](#index-NC_005fMPIPOSIX)

[2.7 Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)

[NC\_NETCDF4](#index-NC_005fNETCDF4)

[2.10 Open a NetCDF Dataset for Parallel Access](#nc_005fopen_005fpar)

[NC\_NOCLOBBER](#index-NC_005fNOCLOBBER)

[2.5 Create a NetCDF Dataset: nc\_create](#nc_005fcreate)

[NC\_NOCLOBBER](#index-NC_005fNOCLOBBER-1)

[2.6 Create a NetCDF Dataset With Performance Options:
nc\_\_create](#nc_005f_005fcreate)

[NC\_NOCLOBBER](#index-NC_005fNOCLOBBER-2)

[2.7 Create a NetCDF Dataset With Performance Options:
nc\_create\_par](#nc_005fcreate_005fpar)

[NC\_NOWRITE](#index-NC_005fNOWRITE)

[2.8 Open a NetCDF Dataset for Access: nc\_open](#nc_005fopen)

[NC\_NOWRITE](#index-NC_005fNOWRITE-1)

[2.9 Open a NetCDF Dataset for Access with Performance Tuning:
nc\_\_open](#nc_005f_005fopen)

[NC\_NOWRITE](#index-NC_005fNOWRITE-2)

[2.10 Open a NetCDF Dataset for Parallel Access](#nc_005fopen_005fpar)

[nc\_open](#index-nc_005fopen)

[2.8 Open a NetCDF Dataset for Access: nc\_open](#nc_005fopen)

[nc\_open, example](#index-nc_005fopen_002c-example)

[2.8 Open a NetCDF Dataset for Access: nc\_open](#nc_005fopen)

[nc\_open\_par](#index-nc_005fopen_005fpar)

[2.10 Open a NetCDF Dataset for Parallel Access](#nc_005fopen_005fpar)

[nc\_put\_att, typical use](#index-nc_005fput_005fatt_002c-typical-use)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[nc\_put\_att, typical
use](#index-nc_005fput_005fatt_002c-typical-use-1)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[nc\_put\_att\_ type](#index-nc_005fput_005fatt_005f-type)

[7.2 Create an Attribute: nc\_put\_att\_
type](#nc_005fput_005fatt_005f-type)

[nc\_put\_att\_ type,
example](#index-nc_005fput_005fatt_005f-type_002c-example)

[7.2 Create an Attribute: nc\_put\_att\_
type](#nc_005fput_005fatt_005f-type)

[nc\_put\_att\_ubyte](#index-nc_005fput_005fatt_005fubyte)

[7.7 Delete an Attribute: nc\_del\_att](#nc_005fdel_005fatt)

[nc\_put\_var](#index-nc_005fput_005fvar)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var, typical use](#index-nc_005fput_005fvar_002c-typical-use)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[nc\_put\_var1](#index-nc_005fput_005fvar1)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_ type](#index-nc_005fput_005fvar1_005f-type)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_ type,
example](#index-nc_005fput_005fvar1_005f-type_002c-example)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_double](#index-nc_005fput_005fvar1_005fdouble)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_float](#index-nc_005fput_005fvar1_005ffloat)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_int](#index-nc_005fput_005fvar1_005fint)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_long](#index-nc_005fput_005fvar1_005flong)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_longlong](#index-nc_005fput_005fvar1_005flonglong)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_schar](#index-nc_005fput_005fvar1_005fschar)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_short](#index-nc_005fput_005fvar1_005fshort)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_string](#index-nc_005fput_005fvar1_005fstring)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_text](#index-nc_005fput_005fvar1_005ftext)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_ubyte](#index-nc_005fput_005fvar1_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_put\_var1\_uchar](#index-nc_005fput_005fvar1_005fuchar)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_uint](#index-nc_005fput_005fvar1_005fuint)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_ulonglong](#index-nc_005fput_005fvar1_005fulonglong)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_var1\_ushort](#index-nc_005fput_005fvar1_005fushort)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[nc\_put\_vara](#index-nc_005fput_005fvara)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_ type](#index-nc_005fput_005fvara_005f-type)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_ type](#index-nc_005fput_005fvara_005f-type-1)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_ type,
example](#index-nc_005fput_005fvara_005f-type_002c-example)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_double](#index-nc_005fput_005fvara_005fdouble)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_float](#index-nc_005fput_005fvara_005ffloat)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_int](#index-nc_005fput_005fvara_005fint)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_long](#index-nc_005fput_005fvara_005flong)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_longlong](#index-nc_005fput_005fvara_005flonglong)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_schar](#index-nc_005fput_005fvara_005fschar)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_short](#index-nc_005fput_005fvara_005fshort)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_string](#index-nc_005fput_005fvara_005fstring)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_text](#index-nc_005fput_005fvara_005ftext)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_ubyte](#index-nc_005fput_005fvara_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_put\_vara\_uchar](#index-nc_005fput_005fvara_005fuchar)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_uint](#index-nc_005fput_005fvara_005fuint)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_ulonglong](#index-nc_005fput_005fvara_005fulonglong)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_vara\_ushort](#index-nc_005fput_005fvara_005fushort)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[nc\_put\_varm](#index-nc_005fput_005fvarm)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_ type](#index-nc_005fput_005fvarm_005f-type)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_ type,
example](#index-nc_005fput_005fvarm_005f-type_002c-example)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_double](#index-nc_005fput_005fvarm_005fdouble)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_float](#index-nc_005fput_005fvarm_005ffloat)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_int](#index-nc_005fput_005fvarm_005fint)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_long](#index-nc_005fput_005fvarm_005flong)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_longlong](#index-nc_005fput_005fvarm_005flonglong)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_schar](#index-nc_005fput_005fvarm_005fschar)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_short](#index-nc_005fput_005fvarm_005fshort)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_string](#index-nc_005fput_005fvarm_005fstring)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_text](#index-nc_005fput_005fvarm_005ftext)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_ubyte](#index-nc_005fput_005fvarm_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_put\_varm\_uchar](#index-nc_005fput_005fvarm_005fuchar)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_uint](#index-nc_005fput_005fvarm_005fuint)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_ulonglong](#index-nc_005fput_005fvarm_005fulonglong)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_varm\_ushort](#index-nc_005fput_005fvarm_005fushort)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[nc\_put\_vars](#index-nc_005fput_005fvars)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_ type](#index-nc_005fput_005fvars_005f-type)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_ type,
example](#index-nc_005fput_005fvars_005f-type_002c-example)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_double](#index-nc_005fput_005fvars_005fdouble)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_float](#index-nc_005fput_005fvars_005ffloat)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_int](#index-nc_005fput_005fvars_005fint)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_long](#index-nc_005fput_005fvars_005flong)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_longlong](#index-nc_005fput_005fvars_005flonglong)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_schar](#index-nc_005fput_005fvars_005fschar)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_short](#index-nc_005fput_005fvars_005fshort)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_string](#index-nc_005fput_005fvars_005fstring)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_text](#index-nc_005fput_005fvars_005ftext)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_ubyte](#index-nc_005fput_005fvars_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_put\_vars\_uchar](#index-nc_005fput_005fvars_005fuchar)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_uint](#index-nc_005fput_005fvars_005fuint)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_ulonglong](#index-nc_005fput_005fvars_005fulonglong)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_vars\_ushort](#index-nc_005fput_005fvars_005fushort)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

[nc\_put\_var\_ type](#index-nc_005fput_005fvar_005f-type)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_ type,
example](#index-nc_005fput_005fvar_005f-type_002c-example)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_double](#index-nc_005fput_005fvar_005fdouble)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_float](#index-nc_005fput_005fvar_005ffloat)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_int](#index-nc_005fput_005fvar_005fint)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_long](#index-nc_005fput_005fvar_005flong)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_longlong](#index-nc_005fput_005fvar_005flonglong)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_schar](#index-nc_005fput_005fvar_005fschar)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_short](#index-nc_005fput_005fvar_005fshort)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_string](#index-nc_005fput_005fvar_005fstring)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_text](#index-nc_005fput_005fvar_005ftext)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_ubyte](#index-nc_005fput_005fvar_005fubyte)

[6.37 Deprecated “\_ubyte” variable functions](#nc_005fvar_005fubyte)

[nc\_put\_var\_uchar](#index-nc_005fput_005fvar_005fuchar)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_uint](#index-nc_005fput_005fvar_005fuint)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_ulonglong](#index-nc_005fput_005fvar_005fulonglong)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_put\_var\_ushort](#index-nc_005fput_005fvar_005fushort)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[nc\_redef](#index-nc_005fredef)

[2.11 Put Open NetCDF Dataset into Define Mode:
nc\_redef](#nc_005fredef)

[nc\_redef, example](#index-nc_005fredef_002c-example)

[2.11 Put Open NetCDF Dataset into Define Mode:
nc\_redef](#nc_005fredef)

[nc\_redef, typical use](#index-nc_005fredef_002c-typical-use)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[nc\_rename\_att](#index-nc_005frename_005fatt)

[7.6 Rename an Attribute: nc\_rename\_att](#nc_005frename_005fatt)

[nc\_rename\_att, example](#index-nc_005frename_005fatt_002c-example)

[7.6 Rename an Attribute: nc\_rename\_att](#nc_005frename_005fatt)

[nc\_rename\_dim](#index-nc_005frename_005fdim)

[4.5 Rename a Dimension: nc\_rename\_dim](#nc_005frename_005fdim)

[nc\_rename\_dim, example](#index-nc_005frename_005fdim_002c-example)

[4.5 Rename a Dimension: nc\_rename\_dim](#nc_005frename_005fdim)

[nc\_rename\_var](#index-nc_005frename_005fvar)

[6.34 Rename a Variable: nc\_rename\_var](#nc_005frename_005fvar)

[nc\_rename\_var, example](#index-nc_005frename_005fvar_002c-example)

[6.34 Rename a Variable: nc\_rename\_var](#nc_005frename_005fvar)

[nc\_set\_chunk\_cache](#index-nc_005fset_005fchunk_005fcache)

[2.20 Set HDF5 Chunk Cache for Future File Opens/Creates:
nc\_set\_chunk\_cache](#nc_005fset_005fchunk_005fcache)

[nc\_set\_default\_format](#index-nc_005fset_005fdefault_005fformat)

[2.19 Set Default Creation Format:
nc\_set\_default\_format](#nc_005fset_005fdefault_005fformat)

[nc\_set\_default\_format,
example](#index-nc_005fset_005fdefault_005fformat_002c-example)

[2.19 Set Default Creation Format:
nc\_set\_default\_format](#nc_005fset_005fdefault_005fformat)

[nc\_set\_fill](#index-nc_005fset_005ffill)

[2.18 Set Fill Mode for Writes: nc\_set\_fill](#nc_005fset_005ffill)

[nc\_set\_fill, example](#index-nc_005fset_005ffill_002c-example)

[2.18 Set Fill Mode for Writes: nc\_set\_fill](#nc_005fset_005ffill)

[nc\_set\_var\_chunk\_cache](#index-nc_005fset_005fvar_005fchunk_005fcache)

[6.8 Set HDF5 Chunk Cache for a Variable:
nc\_set\_var\_chunk\_cache](#nc_005fset_005fvar_005fchunk_005fcache)

[NC\_SHARE](#index-NC_005fSHARE)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[NC\_SHARE](#index-NC_005fSHARE-2)

[2.6 Create a NetCDF Dataset With Performance Options:
nc\_\_create](#nc_005f_005fcreate)

[NC\_SHARE, and buffering](#index-NC_005fSHARE_002c-and-buffering)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[NC\_SHARE, in nc\_open](#index-NC_005fSHARE_002c-in-nc_005fopen)

[2.8 Open a NetCDF Dataset for Access: nc\_open](#nc_005fopen)

[NC\_SHARE, in nc\_\_open](#index-NC_005fSHARE_002c-in-nc_005f_005fopen)

[2.9 Open a NetCDF Dataset for Access with Performance Tuning:
nc\_\_open](#nc_005f_005fopen)

[nc\_strerror](#index-nc_005fstrerror)

[2.3 Get error message corresponding to error status:
nc\_strerror](#nc_005fstrerror)

[nc\_strerror, example](#index-nc_005fstrerror_002c-example)

[2.3 Get error message corresponding to error status:
nc\_strerror](#nc_005fstrerror)

[nc\_strerror, introduction](#index-nc_005fstrerror_002c-introduction)

[1.5 Error Handling](#Errors)

[NC\_STRING, freeing](#index-NC_005fSTRING_002c-freeing)

[6.32 Releasing Memory for a NC\_STRING:
nc\_free\_string](#nc_005ffree_005fstring)

[NC\_STRING, using](#index-NC_005fSTRING_002c-using)

[6.31.2 Reading and Writing Arrays of Strings](#Arrays-of-Strings)

[nc\_sync](#index-nc_005fsync)

[2.16 Synchronize an Open NetCDF Dataset to Disk:
nc\_sync](#nc_005fsync)

[nc\_sync, example](#index-nc_005fsync_002c-example)

[2.16 Synchronize an Open NetCDF Dataset to Disk:
nc\_sync](#nc_005fsync)

[nc\_var\_par\_access](#index-nc_005fvar_005fpar_005faccess)

[6.36 Change between Collective and Independent Parallel Access:
nc\_var\_par\_access](#nc_005fvar_005fpar_005faccess)

[nc\_var\_par\_access,
example](#index-nc_005fvar_005fpar_005faccess_002c-example)

[6.36 Change between Collective and Independent Parallel Access:
nc\_var\_par\_access](#nc_005fvar_005fpar_005faccess)

[NC\_WRITE](#index-NC_005fWRITE)

[2.8 Open a NetCDF Dataset for Access: nc\_open](#nc_005fopen)

[NC\_WRITE](#index-NC_005fWRITE-1)

[2.9 Open a NetCDF Dataset for Access with Performance Tuning:
nc\_\_open](#nc_005f_005fopen)

[NC\_WRITE](#index-NC_005fWRITE-2)

[2.10 Open a NetCDF Dataset for Parallel Access](#nc_005fopen_005fpar)

[nc\_\_create](#index-nc_005f_005fcreate)

[2.6 Create a NetCDF Dataset With Performance Options:
nc\_\_create](#nc_005f_005fcreate)

[nc\_\_create, example](#index-nc_005f_005fcreate_002c-example)

[2.6 Create a NetCDF Dataset With Performance Options:
nc\_\_create](#nc_005f_005fcreate)

[nc\_\_create, flags](#index-nc_005f_005fcreate_002c-flags)

[2.6 Create a NetCDF Dataset With Performance Options:
nc\_\_create](#nc_005f_005fcreate)

[nc\_\_enddef](#index-nc_005f_005fenddef)

[2.13 Leave Define Mode with Performance Tuning:
nc\_\_enddef](#nc_005f_005fenddef)

[nc\_\_enddef, example](#index-nc_005f_005fenddef_002c-example)

[2.13 Leave Define Mode with Performance Tuning:
nc\_\_enddef](#nc_005f_005fenddef)

[nc\_\_open](#index-nc_005f_005fopen)

[2.9 Open a NetCDF Dataset for Access with Performance Tuning:
nc\_\_open](#nc_005f_005fopen)

[nc\_\_open, example](#index-nc_005f_005fopen_002c-example)

[2.9 Open a NetCDF Dataset for Access with Performance Tuning:
nc\_\_open](#nc_005f_005fopen)

[netCDF 2 transition guide](#index-netCDF-2-transition-guide)

[C. NetCDF 2 to NetCDF 3 C Transition
Guide](#NetCDF-2-C-Transition-Guide)

[netCDF library version](#index-netCDF-library-version)

[2.4 Get netCDF library version:
nc\_inq\_libvers](#nc_005finq_005flibvers)

[netcdf-3 error codes](#index-netcdf_002d3-error-codes)

[D. NetCDF-3 Error Codes](#NetCDF_002d3-Error-Codes)

[netcdf-4 error codes](#index-netcdf_002d4-error-codes)

[E. NetCDF-4 Error Codes](#NetCDF_002d4-Error-Codes)

* * * * *

O

[opaque type](#index-opaque-type)

[5.26 Opaque Type Introduction](#Opaque-Type)

* * * * *

P

[parallel access](#index-parallel-access)

[2.2 Parallel Access for NetCDF Files](#parallel-access)

[parallel example](#index-parallel-example)

[2.2 Parallel Access for NetCDF Files](#parallel-access)

[path](#index-path)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

* * * * *

R

[reading attributes](#index-reading-attributes)

[7.4 Get Attribute’s Values:nc\_get\_att\_
type](#nc_005fget_005fatt_005f-type)

[reading entire variable](#index-reading-entire-variable)

[6.27 Read an Entire Variable nc\_get\_var\_
type](#nc_005fget_005fvar_005f-type)

[reading netCDF dataset with known
names](#index-reading-netCDF-dataset-with-known-names)

[1.2 Reading a NetCDF Dataset with Known Names](#Reading-Known)

[reading netCDF dataset with unknown
names](#index-reading-netCDF-dataset-with-unknown-names)

[1.3 Reading a netCDF Dataset with Unknown Names](#Reading-Unknown)

[reading single value](#index-reading-single-value)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[renaming attributes](#index-renaming-attributes)

[7.6 Rename an Attribute: nc\_rename\_att](#nc_005frename_005fatt)

[renaming variable](#index-renaming-variable)

[6.34 Rename a Variable: nc\_rename\_var](#nc_005frename_005fvar)

* * * * *

S

[single value, reading](#index-single-value_002c-reading)

[6.26 Read a Single Data Value: nc\_get\_var1\_
type](#nc_005fget_005fvar1_005f-type)

[string arrays](#index-string-arrays)

[6.31 Reading and Writing Character String Values](#Strings)

[strings in classic model](#index-strings-in-classic-model)

[6.31.1 Reading and Writing Character String Values in the Classic
Model](#Classic-Strings)

[subsampled array, writing](#index-subsampled-array_002c-writing)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

* * * * *

T

[templates, code](#index-templates_002c-code)

[1. Use of the NetCDF Library](#Use-of-the-NetCDF-Library)

[transition guide, netCDF 2](#index-transition-guide_002c-netCDF-2)

[C. NetCDF 2 to NetCDF 3 C Transition
Guide](#NetCDF-2-C-Transition-Guide)

* * * * *

U

[URL](#index-URL)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[user defined types](#index-user-defined-types)

[5. User Defined Data Types](#User-Defined-Data-Types)

[user defined types, overview](#index-user-defined-types_002c-overview)

[5.1 User Defined Types Introduction](#User-Defined-Types)

* * * * *

V

[variable length array type,
overview](#index-variable-length-array-type_002c-overview)

[5. User Defined Data Types](#User-Defined-Data-Types)

[variable length arrays](#index-variable-length-arrays)

[5.21 Variable Length Array Introduction](#Variable-Length-Array)

[variable, copying](#index-variable_002c-copying)

[6.35 Copy a Variable from One File to Another:
nc\_copy\_var](#nc_005fcopy_005fvar)

[variable, renaming](#index-variable_002c-renaming)

[6.34 Rename a Variable: nc\_rename\_var](#nc_005frename_005fvar)

[variable, writing entire](#index-variable_002c-writing-entire)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[variables, adding](#index-variables_002c-adding)

[1.4 Adding New Dimensions, Variables, Attributes](#Adding)

[variables, checksum](#index-variables_002c-checksum)

[6.15 Define Checksum Parameters for a Variable:
`nc_def_var_fletcher32`](#nc_005fdef_005fvar_005ffletcher32)

[variables, chunking](#index-variables_002c-chunking)

[6.6 Define Chunking Parameters for a Variable:
`nc_def_var_chunking`](#nc_005fdef_005fvar_005fchunking)

[variables, contiguous](#index-variables_002c-contiguous)

[6.6 Define Chunking Parameters for a Variable:
`nc_def_var_chunking`](#nc_005fdef_005fvar_005fchunking)

[variables, creating](#index-variables_002c-creating)

[6.5 Create a Variable: `nc_def_var`](#nc_005fdef_005fvar)

[variables, endian](#index-variables_002c-endian)

[6.17 Define Endianness of a Variable:
`nc_def_var_endian`](#nc_005fdef_005fvar_005fendian)

[variables, fill](#index-variables_002c-fill)

[6.10 Define Fill Parameters for a Variable:
`nc_def_var_fill`](#nc_005fdef_005fvar_005ffill)

[variables, fletcher32](#index-variables_002c-fletcher32)

[6.15 Define Checksum Parameters for a Variable:
`nc_def_var_fletcher32`](#nc_005fdef_005fvar_005ffletcher32)

[variables, getting name](#index-variables_002c-getting-name)

[6.20 Get Information about a Variable from Its ID:
nc\_inq\_var](#nc_005finq_005fvar)

[variables, inquiring about](#index-variables_002c-inquiring-about)

[6.19 Get a Variable ID from Its Name:
nc\_inq\_varid](#nc_005finq_005fvarid)

[variables, number of](#index-variables_002c-number-of)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[variables, setting deflate](#index-variables_002c-setting-deflate)

[6.12 Define Compression Parameters for a Variable:
`nc_def_var_deflate`](#nc_005fdef_005fvar_005fdeflate)

[varid](#index-varid)

[Usage](#Usage-20)

[version of netCDF,
discovering](#index-version-of-netCDF_002c-discovering)

[2.4 Get netCDF library version:
nc\_inq\_libvers](#nc_005finq_005flibvers)

[version, format](#index-version_002c-format)

[2.15 Inquire about an Open NetCDF Dataset: nc\_inq
Family](#nc_005finq-Family)

[VLEN](#index-VLEN)

[5.21 Variable Length Array Introduction](#Variable-Length-Array)

[VLEN, defining](#index-VLEN_002c-defining)

[5.22 Define a Variable Length Array (VLEN):
nc\_def\_vlen](#nc_005fdef_005fvlen)

[VLEN, defining](#index-VLEN_002c-defining-1)

[5.23 Learning about a Variable Length Array (VLEN) Type:
nc\_inq\_vlen](#nc_005finq_005fvlen)

[VLEN, defining](#index-VLEN_002c-defining-2)

[5.24 Releasing Memory for a Variable Length Array (VLEN) Type:
nc\_free\_vlen](#nc_005ffree_005fvlen)

[VLEN, defining](#index-VLEN_002c-defining-3)

[5.25 Releasing Memory for an Array of Variable Length Array (VLEN)
Type: nc\_free\_vlen](#nc_005ffree_005fvlens)

* * * * *

W

[write errors](#index-write-errors)

[1.5 Error Handling](#Errors)

[write fill mode, setting](#index-write-fill-mode_002c-setting)

[2.18 Set Fill Mode for Writes: nc\_set\_fill](#nc_005fset_005ffill)

[writing array section](#index-writing-array-section)

[6.23 Write an Array of Values: nc\_put\_vara\_
type](#nc_005fput_005fvara_005f-type)

[writing attributes](#index-writing-attributes)

[7.2 Create an Attribute: nc\_put\_att\_
type](#nc_005fput_005fatt_005f-type)

[writing character-string
data](#index-writing-character_002dstring-data)

[6.31.1 Reading and Writing Character String Values in the Classic
Model](#Classic-Strings)

[writing entire variable](#index-writing-entire-variable)

[6.22 Write an Entire Variable: nc\_put\_var\_
type](#nc_005fput_005fvar_005f-type)

[writing mapped array](#index-writing-mapped-array)

[6.25 Write a Mapped Array of Values: nc\_put\_varm\_
type](#nc_005fput_005fvarm_005f-type)

[writing mapped array section](#index-writing-mapped-array-section)

[6.30 Read a Mapped Array of Values: nc\_get\_varm\_
type](#nc_005fget_005fvarm_005f-type)

[writing single value](#index-writing-single-value)

[6.21 Write a Single Data Value: nc\_put\_var1\_
type](#nc_005fput_005fvar1_005f-type)

[writing subsampled array](#index-writing-subsampled-array)

[6.24 Write a Subsampled Array of Values: nc\_put\_vars\_
type](#nc_005fput_005fvars_005f-type)

* * * * *

X

[XDR library](#index-XDR-library)

[C. NetCDF 2 to NetCDF 3 C Transition
Guide](#NetCDF-2-C-Transition-Guide)

[xtypep](#index-xtypep)

[Usage](#Usage-20)

* * * * *

Z

[zero byte](#index-zero-byte)

[6.31.1 Reading and Writing Character String Values in the Classic
Model](#Classic-Strings)

[zero length edge](#index-zero-length-edge)

[C. NetCDF 2 to NetCDF 3 C Transition
Guide](#NetCDF-2-C-Transition-Guide)

[zero valued count vector](#index-zero-valued-count-vector)

[C. NetCDF 2 to NetCDF 3 C Transition
Guide](#NetCDF-2-C-Transition-Guide)

* * * * *

Jump to:  

[**A**](#Index_cp_letter-A)   [**B**](#Index_cp_letter-B)  
[**C**](#Index_cp_letter-C)   [**D**](#Index_cp_letter-D)  
[**E**](#Index_cp_letter-E)   [**F**](#Index_cp_letter-F)  
[**G**](#Index_cp_letter-G)   [**H**](#Index_cp_letter-H)  
[**I**](#Index_cp_letter-I)   [**L**](#Index_cp_letter-L)  
[**M**](#Index_cp_letter-M)   [**N**](#Index_cp_letter-N)  
[**O**](#Index_cp_letter-O)   [**P**](#Index_cp_letter-P)  
[**R**](#Index_cp_letter-R)   [**S**](#Index_cp_letter-S)  
[**T**](#Index_cp_letter-T)   [**U**](#Index_cp_letter-U)  
[**V**](#Index_cp_letter-V)   [**W**](#Index_cp_letter-W)  
[**X**](#Index_cp_letter-X)   [**Z**](#Index_cp_letter-Z)  

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

