\page netcdf-cxx-texi NetCDF-CXX Documentation

  ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------- ------------ ------------------------------------ ----------------------------------

The NetCDF C++ Interface Guide {.settitle}
==============================

This document describes the netCDF C++ API. It applies to netCDF version
No value for VERSION release of the software, but the C++ interface
still only supports the “classic” data model from the netCDF-3 release.
This document was last updated in No value for UPDATED.

The netCDF library provides an application- and machine-independent
interface to self-describing, array-oriented data. It supports an
abstract view of such data as a collection of named variables and their
attributes, and provides high-level access to data that is faithful to
the abstraction. This on-line document describes the C++ interface to
netCDF. This C++ interface is still based on the classic data model
supported by netCDF-3 software; it has not yet been updated to the
enhanced data model supported by netCDF-4.

The first part of this master menu lists the major nodes in this Info
document. The rest of the menu lists all the lower level nodes in the
document.

For a complete description of the netCDF format and utilities see
[(netcdf)Top](netcdf.html#Top) section ‘Top’ in The NetCDF Users Guide.

[Introduction](#Introduction)

  

[NetCDF Classes](#NetCDF-Classes)

  

[Auxiliary Classes](#Auxiliary-Classes)

  

[Index](#Combined-Index)

  

~~~~ {.menu-comment}
 — The Detailed Node Listing —

Introduction
~~~~

[Class Hierarchy](#Class-Hierarchy)

  

[Auxiliary Types and Constants](#Auxiliary-Types-and-Constants)

  

~~~~ {.menu-comment}
NetCDF Classes
~~~~

[Class NcFile](#Class-NcFile)

  

Files

[Class NcDim](#Class-NcDim)

  

Dimensions

[Class NcTypedComponent](#Class-NcTypedComponent)

  

Operations common to variables and attributes

[Class NcVar](#Class-NcVar)

  

Variables

[Class NcAtt](#Class-NcAtt)

  

Attributes

~~~~ {.menu-comment}
Auxiliary Classes
~~~~

[Class NcValues](#Class-NcValues)

  

[Class NcError](#Class-NcError)

  

~~~~ {.menu-comment}
~~~~

* * * * *

  -------------------------------------------------- ---------------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Top "Previous section in reading order")]   [[\>](#Class-Hierarchy "Next section in reading order")]       [[\<\<](#Top "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#NetCDF-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------- ---------------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Introduction {.unnumbered}
============

The main requirements for the design of the C++ interface are:

-   to provide the functionality of the C interface;
-   to provide type safety by eliminating all use of `void*` pointers;
    and
-   to provide an interface that is simpler to use than the C interface.

Some of the features of the C++ interface are:

-   No IDs needed for netCDF’s variables, dimensions, or attributes.
-   No explicit open or close calls needed for netCDF files; a
    constructor opens and a destructor closes a file.
-   No need to specify types for creating attributes; they will have the
    type of the value provided.
-   No use of `void*`: values are type-checked.
-   Less indirection is needed for dimensions and dimension sizes than
    with the C interface. A variable’s dimensions can be provided as
    arguments when defining a variable.
-   Code for data types is isolated to make the addition of new types
    easier.
-   No explicit `ncredef` or `ncendef` calls are needed for switching
    between define and data modes. Whenever a mode switch is required,
    it happens implicitly.

The header file ‘`netcdfcpp.h`’ must be included in source code files
using this interface.

This release provides some of the functionality of netCDF version 4, but
not for the enhanced data model introduced with netCDF-4.

This manual assumes familiarity with the netCDF User’s Guide, where the
concepts of netCDF dimensions, variables, and attributes are discussed.

  ----------------------------------------------------------------- ---- --
  [Class Hierarchy](#Class-Hierarchy)                                    
  [Auxiliary Types and Constants](#Auxiliary-Types-and-Constants)        
  ----------------------------------------------------------------- ---- --

* * * * *

  ----------------------------------------------------------- ------------------------------------------------------------------------ --- ------------------------------------------------------------------------- ------------------------------------ ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Introduction "Previous section in reading order")]   [[\>](#Auxiliary-Types-and-Constants "Next section in reading order")]       [[\<\<](#Introduction "Beginning of this chapter or previous chapter")]   [[Up](#Introduction "Up section")]   [[\>\>](#NetCDF-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- ------------------------------------------------------------------------ --- ------------------------------------------------------------------------- ------------------------------------ ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Class Hierarchy {.unnumberedsec}
---------------

The class for netCDF file objects is `NcFile`.

The components of a netCDF file are dimensions, variables, and
attributes. There is a class for each of these kinds of objects;
`NcDim`, `NcVar`, and `NcAtt`. Variables and attributes share some
common characteristics that are factored out in the abstract base class
`NcTypedComponent`.

An auxiliary class, `NcValues`, provides a type for arrays of values
that are read from or written to netCDF files. Another auxiliary class,
`NcError`, provides facilities for handling errors.

 

~~~~ {.example}
NcFile                     netCDF file

NcDim                      dimension

NcTypedComponent           abstract base class
        NcVar                variable
        NcAtt                attribute

NcValues                   abstract base class for array
        NcValues_ncbyte      array of bytes
        NcValues_char        array of characters
        NcValues_short       array of shorts
        NcValues_int         array of ints
        NcValues_long        array of longs
        NcValues_float       array of floats
        NcValues_double      array of doubles

NcError                    for error handling
~~~~

* * * * *

  -------------------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------------- ------------------------------------ ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Class-Hierarchy "Previous section in reading order")]   [[\>](#NetCDF-Classes "Next section in reading order")]       [[\<\<](#Introduction "Beginning of this chapter or previous chapter")]   [[Up](#Introduction "Up section")]   [[\>\>](#NetCDF-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------------- ------------------------------------ ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Auxiliary Types and Constants {.unnumberedsec}
-----------------------------

The netCDF classes use several auxiliary types for arguments and return
types from member functions: `NcToken`, `NcType`, `NcBool`, and
`ncbyte`.

`NcToken`

Used for names for netCDF objects, in particular variable names,
dimension names, and attribute names. Currently this is just a typedef
for `const char*`.

`NcType`

Used for specifying netCDF external value types. Currently this is an
enumerated type with the following legitimate values: `ncByte`,
`ncChar`, `ncShort`, `ncInt`, `ncLong` (deprecated), `ncFloat`, and
`ncDouble`.

`NcBool`

Used for the return type of some member functions. If the member
function fails, 0 is returned, otherwise some non-zero value. Currently
this is just a typedef for `unsigned int`. It will be changed to `bool`
when all C++ compilers support the new `bool` type.

`ncbyte`

Used to declare values of type `ncByte`, for 8-bit integer data. (This
is currently a typedef for `unsigned char`, but it may be changed to a
typedef for `signed char`, so don’t depend on the underlying
representation.)

* * * * *

  ---------------------------------------------------------------------------- ------------------------------------------------------- --- ------------------------------------------------------------------------- --------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Auxiliary-Types-and-Constants "Previous section in reading order")]   [[\>](#Class-NcFile "Next section in reading order")]       [[\<\<](#Introduction "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------- ------------------------------------------------------- --- ------------------------------------------------------------------------- --------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

NetCDF Classes {.unnumbered}
==============

  --------------------------------------------------- ---- -----------------------------------------------
  [Class NcFile](#Class-NcFile)                            Files
  [Class NcDim](#Class-NcDim)                              Dimensions
  [Class NcTypedComponent](#Class-NcTypedComponent)        Operations common to variables and attributes
  [Class NcVar](#Class-NcVar)                              Variables
  [Class NcAtt](#Class-NcAtt)                              Attributes
  --------------------------------------------------- ---- -----------------------------------------------

* * * * *

  ------------------------------------------------------------- -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#NetCDF-Classes "Previous section in reading order")]   [[\>](#Public-Member-Functions-1 "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-Classes "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Class NcFile {.unnumberedsec}
------------

`NcFile` is the class for netCDF files, providing methods for netCDF
file operations.

Some member functions return pointers to dimensions (`NcDim`) or
variables (`NcVar`). These objects are owned by the `NcFile` they are
associated with, and will be deleted automatically by the `NcFile`
destructor (or by the `close` member function, if this is called earlier
than the destructor), so users should not delete these. Member functions
that return pointers to attributes (`NcAtt`) pass ownership to the
calling function; users should delete attributes when they are finished
with them.

Member functions that return `NcBool` yield `TRUE` on success and
`FALSE` on failure. Member functions that return a pointer value return
a `NULL` pointer on failure.

This class interface hides the distinction in the C and Fortran
interfaces between *define mode* (when dimensions, variables, or
attributes are being defined or renamed), and *data mode* (when data
values are being accessed), by automatically switching between the modes
when necessary. Be aware that switching from accessing data to adding or
renaming dimensions, variables and attributes can be expensive, since it
may entail a copy of the data.

* * * * *

  ----------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------------- ------------------------------------ --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Class-NcFile "Previous section in reading order")]   [[\>](#Class-NcDim "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Class-NcFile "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------------- ------------------------------------ --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### Public Member Functions {.unnumberedsubsec}

`NcFile( const char * path, FileMode = ReadOnly, size_t *chunksizeptr = NULL, size_t initialsize = 0, FileFormat = Classic)`

The constructor creates a new netCDF file or opens an existing netCDF
file. The path argument may be an OPenDAP DAP URL if DAP support is
enabled.

The `FileMode` argument can be any of `ReadOnly` (the default) to open
an existing file for reading, `Write` to open an existing file for
reading or writing, `Replace` to create a new empty file even if the
named file already exists, or `New` to create a new file only if the
named file does not already exist.

The optional `FileFormat` argument can be any of `Classic` (the
default), `Offset64Bits`, `Netcdf4`, or `Netcdf4Classic`.

The optional `chunksizeptr` and `initialsize` tuning parameters are as
described in the corresponding `nc__create()` function in the C
interface.

The constructor will not fail, but in the case of a bad path name,
improper permissions, or if the file already exists and you have
specified `FileMode` as `New`, no netCDF file will be created or opened.
If the constructor fails to create or open a netCDF file, a subsequent
call to the `is_valid()` member function will return `False`.

`~NcFile( void )`

Destructor. The file is closed and all resources associated with it are
released, including the associated `NcVar` and `NcDim` objects. If you
wish to close the file earlier, you may explicitly call the `close`
member function; a subsequent destructor call will work properly.

`NcBool close( void )`

Close netCDF file earlier than it would be closed by the `NcFile`
destructor.

`NcBool is_valid( void ) const`

Returns `TRUE` if valid netCDF file, `FALSE` otherwise (e.g. if
constructor could not open file).

`int num_dims( void ) const`

Returns the number of dimensions in the netCDF file.

`int num_vars( void ) const`

Returns the number of variables in the netCDF file.

`int num_atts( void ) const`

Returns the number of global attributes in the netCDF file.

`NcDim* get_dim(NcToken name) const`

Get a dimension by name.

`NcVar* get_var(NcToken name) const`

Get a variable by name.

`NcAtt* get_att(NcToken name) const`

Get a global attribute by name.

`NcDim* get_dim(int n) const`

Get the nth dimension (beginning with the 0th).

`NcVar* get_var(int n) const`

Get the nth variable (beginning with the 0th).

`NcAtt* get_att(int n) const`

Get the nth global attribute (beginning with the 0th).

`NcDim* rec_dim( void ) const`

Get the unlimited dimension, if any.

The following `add_` member functions put the file in *define mode*, so
could be expensive. To avoid copying of data, invoke these before
writing data to variables.

`NcDim* add_dim(NcToken dimname)`

Add an unlimited dimension named `dimname` to the netCDF file.

`NcDim* add_dim(NcToken dimname, long dimsize)`

Add a dimension named `dimname` of size `dimsize`.

`NcVar* add_var(NcToken varname, NcType type, const NcDim*, …)`

Add a variable named `varname` of the specified type (`ncByte`,
`ncChar`, `ncShort`, `ncInt`, `ncFloat`, `ncDouble`) to the open netCDF
file. The variable is defined with a shape that depends on how many
dimension arguments are provided. A scalar variable would have 0
dimensions, a vector would have 1 dimension, and so on. Supply as many
dimensions as needed, up to 5. If more than 5 dimensions are required,
use the n-dimensional version of this member function instead.

`NcVar* add_var(NcToken varname, NcType type, int ndims, const NcDim** dims)`

Add a variable named `varname` of `ndims` dimensions and of the
specified type. This method must be used when dealing with variables of
more than 5 dimensions.

`NcBool add_att(NcToken name, ncbyte val)`

`NcBool add_att(NcToken name, char val)`

`NcBool add_att(NcToken name, short val)`

`NcBool add_att(NcToken name, int val)`

`NcBool add_att(NcToken name, float val)`

`NcBool add_att(NcToken name, double val)`

Add global scalar attributes of the specified name and with the supplied
value.

`NcBool add_att(NcToken name, const char* val)`

Add global string-valued attribute with the specified name and C string
value (terminated with a `\0` character).

`NcBool add_att(NcToken name, int n, const ncbyte* val)`

`NcBool add_att(NcToken name, int n, const char* val)`

`NcBool add_att(NcToken name, int n, const short* val)`

`NcBool add_att(NcToken name, int n, const int* val)`

`NcBool add_att(NcToken name, int n, const float* val)`

`NcBool add_att(NcToken name, int n, const double* val)`

Add global vector attributes with the specified name, length, and
values.

`NcBool set_fill(FillMode mode = Fill)`

Sets fill-mode to either `NcFile::Fill` or `NcFile::NoFill`. Default is
`Fill`, in which case unwritten values are pre-written with appropriate
type-specific or variable-specific fill values.

`enum NcFile::FillMode get_fill( void ) const`

Returns fill mode of the file, either `NcFile::Fill` or
`NcFile::NoFill`.

`enum NcFile::FileFormat get_format( void ) const`

Returns format version of the file, either `NcFile::Classic`,
`NcFile:Offset64Bits`, `NcFile:Netcdf4`, or `NcFile::Netcdf4Classic`.

`NcBool sync( void )`

Synchronizes file to disk. This flushes buffers so that readers of the
file will see recent changes.

`NcBool abort( void )`

Either just closes file (if recently it has been in data mode as the
result of accessing data), or backs out of the most recent sequence of
changes to the file schema (dimensions, variables, and attributes).

* * * * *

  ------------------------------------------------------------------------ -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Public-Member-Functions-1 "Previous section in reading order")]   [[\>](#Public-Member-Functions-4 "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-Classes "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Class NcDim {.unnumberedsec}
-----------

A netCDF dimension has a name and a size. Dimensions are only created
and destroyed by NcFile member functions, because they cannot exist
independently of an open netCDF file. Hence there are no public
constructors or destructors.

* * * * *

  ---------------------------------------------------------- ----------------------------------------------------------------- --- --------------------------------------------------------------------------- ----------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Class-NcDim "Previous section in reading order")]   [[\>](#Class-NcTypedComponent "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Class-NcDim "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- ----------------------------------------------------------------- --- --------------------------------------------------------------------------- ----------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### Public Member Functions {.unnumberedsubsec}

`NcToken name( void ) const`

Returns the name of the dimension if it exists, 0 otherwise.

`long size( void ) const`

Returns the dimension size.

`NcBool is_valid( void ) const`

Returns `TRUE` if file and dimension are both valid, `FALSE` otherwise.

`NcBool is_unlimited( void ) const`

Returns `TRUE` if the dimension is the unlimited dimension, `FALSE` if
either not a valid netCDF file, or if the dimension is not the unlimited
dimension.

`NcBool rename( NcToken newname )`

Renames the dimension to `newname`.

`NcBool sync( void )`

If the dimension may have been renamed, make sure its name is updated.

* * * * *

  ------------------------------------------------------------------------ -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Public-Member-Functions-4 "Previous section in reading order")]   [[\>](#Public-Member-Functions-6 "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-Classes "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Class NcTypedComponent {.unnumberedsec}
----------------------

`NcTypedComponent` is an abstract base class for `NcVar` and `NcAtt`
that captures the similarities between netCDF variables and attributes.
We list here the member functions that variables and attributes inherit
from `NcTypedComponent`, but these member functions are also documented
under the `NcVar` and `NcAtt` classes for convenience.

* * * * *

  --------------------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------------- ---------------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Class-NcTypedComponent "Previous section in reading order")]   [[\>](#Class-NcVar "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Class-NcTypedComponent "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------------- ---------------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### Public Member Functions {.unnumberedsubsec}

`NcToken name( void ) const`

Returns the name of the variable or attribute.

`NcType type( void ) const`

Returns the type of the variable or attribute. The type will be one of
`ncByte`, `ncChar`, `ncShort`, `ncInt`, `ncFloat`, or `ncDouble`.

`NcBool is_valid( void ) const`

Returns `TRUE` if the component is valid, `FALSE` otherwise.

`long num_vals( void ) const`

Returns the number of values for an attribute or variable. For an
attribute, this is just 1 for a scalar attribute, the number of values
for a vector-valued attribute, and the number of characters for a
string-valued attribute. For a variable, this is the product of the
dimension sizes for all the variable’s dimensions.

`NcBool rename( NcToken newname )`

Renames the variable or attribute.

`NcValues* values( void ) const`

Returns a pointer to the block of all values for the variable or
attribute. The caller is responsible for deleting this block of values
when no longer needed, as well as the pointer returned by the `values`
method. Note that this is not a good way to read selected values of a
variable; use the `get` member function instead, to get single values or
selected cross-sections of values.

`ncbyte as_ncbyte( int n ) const`

`char as_char( int n ) const`

`short as_short( int n ) const`

`int as_int( int n ) const`

`nclong as_nclong( int n ) const // deprecated`

`long as_long( int n ) const`

`float as_float( int n ) const`

`double as_double( int n ) const`

`char* as_string( int n ) const`

Get the n-th value of the attribute or variable. These member functions
provide conversions from the value type of the variable or attribute to
the specified type. If the value is out-of-range, the fill-value of the
appropriate type is returned.

* * * * *

  ------------------------------------------------------------------------ -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Public-Member-Functions-6 "Previous section in reading order")]   [[\>](#Public-Member-Functions-2 "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-Classes "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Class NcVar {.unnumberedsec}
-----------

`NcVar` is derived from NcTypedComponent, and represents a netCDF
variable. A netCDF variable has a name, a type, a shape, zero or more
attributes, and a block of values associated with it. Because variables
are only associated with open netCDF files, there are no public
constructors for this class. Use member functions of `NcFile` to get
variables or add new variables.

* * * * *

  ---------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------------- ----------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Class-NcVar "Previous section in reading order")]   [[\>](#Class-NcAtt "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Class-NcVar "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- ------------------------------------------------------ --- --------------------------------------------------------------------------- ----------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### Public Member Functions {.unnumberedsubsec}

`NcToken name( void ) const`

Returns the name of the variable.

`NcType type( void ) const`

Returns the type of the variable. The type will be one of `ncByte`,
`ncChar`, `ncShort`, `ncInt`, `ncFloat`, or `ncDouble`.

`int num_dims( void ) const`

Returns number of dimensions for this variable.

`NcDim* get_dim( int n ) const`

Returns a pointer to the n-th dimension (starting at 0). Returns a
NULL-pointer if an invalid dimension is requested.

`long* edges( void ) const`

Returns the shape of the variable, in the form of a vector containing
the sizes of the dimensions of the variable. The caller is responsible
for deleting the returned edge vector when no longer needed.

`int num_atts( void ) const`

Returns the number of attributes attached to the variable.

`NcAtt* get_att( NcToken attname ) const`

`NcAtt* get_att( int n ) const`

The first member function returns a variable attribute by name. The
second returns the n-th (starting at 0) attribute of the variable. In
either case, if no such attribute has been attached to the variable,
zero is returned. Attributes returned in this way belong to the caller,
and hence should eventually be deleted by the caller to avoid memory
leaks.

`NcBool is_valid( void ) const`

Returns `TRUE` if the variable is valid, `FALSE` otherwise.

`long num_vals( void ) const`

Returns the number of values for a variable. This is just 1 for a scalar
variable, or the product of the dimension sizes for all the variable’s
dimensions.

`NcValues* values( void ) const`

Returns a pointer to the block of all values for the variable. The
caller is responsible for deleting this block of values when no longer
needed. Note that this is not a good way to read selected values of a
variable; use the `get` member function instead, to get single values or
selected cross-sections of values.

`NcBool put(const ncbyte* vals, long c0, long c1, long c2, long c3, long c4)`

`NcBool put(const char*   vals, long c0, long c1, long c2, long c3, long c4)`

`NcBool put(const short*  vals, long c0, long c1, long c2, long c3, long c4)`

`NcBool put(const int* vals, long c0, long c1, long c2, long c3, long c4)`

`NcBool put(const long* vals, long c0, long c1, long c2, long c3, long c4)`

`NcBool put(const float*  vals, long c0, long c1, long c2, long c3, long c4)`

`NcBool put(const double* vals, long c0, long c1, long c2, long c3, long c4)`

Write scalar or 1 to 5-dimensional arrays by providing enough arguments.
The `vals` argument points to a contiguous block of values in memory to
be written. This means if you allocate each row of an array with a “new”
call for example, you must not write more than one row at a time,
because the rows may not be contiguous in memory. Other arguments are
edge lengths, and their number must not exceed variable’s
dimensionality. Start corner is `[0,0,..., 0]` by default, but may be
reset using the `set_cur()` member function for this variable. `FALSE`
is returned if type of values does not match type for variable. For more
than 5 dimensions, use the overloaded n-dimensional form of the `put`
member function.

`NcBool put(const ncbyte* vals, const long* counts)`

`NcBool put(const char*   vals, const long* counts)`

`NcBool put(const short*  vals, const long* counts)`

`NcBool put(const int* vals, const long* counts)`

`NcBool put(const long* vals, const long* counts)`

`NcBool put(const float*  vals, const long* counts)`

`NcBool put(const double* vals, const long* counts)`

Write n-dimensional arrays, starting at `[0, 0, ..., 0]` by default, may
be reset with `set_cur()`. `FALSE` is returned if type of values does
not match type for variable. The `vals` argument points to a contiguous
block of values in memory to be written. This means if you allocate each
row of an array with a “new” call for example, you must not write more
than one row at a time, because the rows may not be contiguous in
memory.

`NcBool get(ncbyte* vals, long c0, long c1, long c2, long c3, long c4) const`

`NcBool get(char*   vals, long c0, long c1, long c2, long c3, long c4) const`

`NcBool get(short*  vals, long c0, long c1, long c2, long c3, long c4) const`

`NcBool get(int* vals, long c0, long c1, long c2, long c3, long c4) const`

`NcBool get(long* vals, long c0, long c1, long c2, long c3, long c4) const`

`NcBool get(float*  vals, long c0, long c1, long c2, long c3, long c4) const`

`NcBool get(double* vals, long c0, long c1, long c2, long c3, long c4) const`

Get scalar or 1 to 5 dimensional arrays by providing enough arguments.
The `vals` argument points to a contiguous block of values in memory
into which values will be read. This means if you allocate each row of
an array with a “new” call for example, you must not read more than one
row at a time, because the rows may not be contiguous in memory. Other
arguments are edge lengths, and their number must not exceed variable’s
dimensionality. Start corner is `[0,0,..., 0]` by default, but may be
reset using the `set_cur()` member function. `FALSE` is returned if type
of values does not match type for variable.

`NcBool get(ncbyte* vals, const long* counts) const`

`NcBool get(char*   vals, const long* counts) const`

`NcBool get(short*  vals, const long* counts) const`

`NcBool get(int* vals, const long* counts) const`

`NcBool get(long* vals, const long* counts) const`

`NcBool get(float*  vals, const long* counts) const`

`NcBool get(double* vals, const long* counts) const`

Get n-dimensional arrays, starting at `[0, 0, ..., 0]` by default, may
be reset with `set_cur()` member function. `FALSE` is returned if type
of values does not match type for variable. Get scalar or 1 to 5
dimensional arrays by providing enough arguments. The `vals` argument
points to a contiguous block of values in memory into which values will
be read. This means if you allocate each row of an array with a “new”
call for example, you must not read more than one row at a time, because
the rows may not be contiguous in memory.

`NcBool set_cur(long c0=-1, long c1=-1, long c2=-1, long c3=-1, long c4=-1)`

`NcBool set_cur(long* cur)`

Resets the starting corner to the values supplied. The first form works
for a variable of dimensionality from scalar to 5 dimensions. For more
than five dimensions, use the second form, in which the number of longs
supplied must match the rank of the variable. The method returns FALSE
if any argument is greater than the size of the corresponding dimension.

`NcBool add_att( NcToken, char )`

`NcBool add_att( NcToken, ncbyte )`

`NcBool add_att( NcToken, short )`

`NcBool add_att( NcToken, int )`

`NcBool add_att( NcToken, long )`

`NcBool add_att( NcToken, float )`

`NcBool add_att( NcToken, double )`

`NcBool add_att( NcToken, const char* )`

`NcBool add_att( NcToken, int, const char* )`

`NcBool add_att( NcToken, int, const ncbyte* )`

`NcBool add_att( NcToken, int, const short* )`

`NcBool add_att( NcToken, int, const int* )`

`NcBool add_att( NcToken, int, const long* )`

`NcBool add_att( NcToken, int, const float* )`

`NcBool add_att( NcToken, int, const double* )`

Add scalar or vector attribute of any type to a variable, given the
name, number of values, and the vector of values. These put file in
define mode, so could be expensive. To avoid the expense of copying
data, add attributes to variables before writing data.

`NcBool rename( NcToken newname )`

Renames the variable. If variable is renamed to a longer name, this puts
file in define mode, so could be expensive.

`ncbyte as_ncbyte( int n ) const`

`char as_char( int n ) const`

`short as_short( int n ) const`

`int as_int( int n ) const`

`nclong as_nclong( int n ) const // deprecated`

`long as_long( int n ) const`

`float as_float( int n ) const`

`double as_double( int n ) const`

`char* as_string( int n ) const`

Get the n-th value of the variable, ignoring its shape. These member
functions provide conversions from the value type of the variable to the
specified type. If the requested value is out-of-range, the fill-value
of the appropriate type is returned.

`int id( void ) const`

Return the variable number. This is not needed in the C++ interface, but
might be needed in calling a C-function that requires that a variable be
identified by number instead of name.

`NcBool sync( void )`

If the variable may have been renamed, make sure its name is updated.

`~NcVar( void )`

Destructor.

The following member functions are intended for record variables. They
will also work for non-record variables, if the first dimension is
interpreted as the record dimension.

`long rec_size( void )`

`long rec_size( NcDim* )`

Return the number of values per record or the number of values per
dimension slice for the specified dimension.

`NcValues* get_rec( void )`

`NcValues* get_rec( long n )`

Get the data for this variable for the current record or for the nth
record.

`NcValues* get_rec( NcDim* )`

`NcValues* get_rec( NcDim*, long n )`

Get the data for this variable for the current dimension slice or for
the nth dimension slice.

`NcBool put_rec( const ncbyte* vals )`

`NcBool put_rec( const char* vals )`

`NcBool put_rec( const short* vals )`

`NcBool put_rec( const int* vals )`

`NcBool put_rec( const long* vals )`

`NcBool put_rec( const float* vals )`

`NcBool put_rec( const double* vals )`

Put a record’s worth of data for this variable in the current record.

`NcBool put_rec( NcDim*, const ncbyte* vals )`

`NcBool put_rec( NcDim*, const char* vals )`

`NcBool put_rec( NcDim*, const short* vals )`

`NcBool put_rec( NcDim*, const int* vals )`

`NcBool put_rec( NcDim*, const long* vals )`

`NcBool put_rec( NcDim*, const float* vals )`

`NcBool put_rec( NcDim*, const double* vals )`

Put a dimension slice worth of data for this variable in the current
dimension slice.

`NcBool put_rec( const ncbyte* vals, long rec )`

`NcBool put_rec( const char* vals, long rec )`

`NcBool put_rec( const short* vals, long rec )`

`NcBool put_rec( const int* vals, long rec )`

`NcBool put_rec( const long* vals, long rec )`

`NcBool put_rec( const float* vals, long rec )`

`NcBool put_rec( const double* vals, long rec )`

Put a record’s worth of data for this variable in the specified record.

`NcBool put_rec( NcDim*, const ncbyte* vals, long slice )`

`NcBool put_rec( NcDim*, const char* vals, long slice )`

`NcBool put_rec( NcDim*, const short* vals, long slice )`

`NcBool put_rec( NcDim*, const int* vals, long slice )`

`NcBool put_rec( NcDim*, const long* vals, long slice )`

`NcBool put_rec( NcDim*, const float* vals, long slice )`

`NcBool put_rec( NcDim*, const double* vals, long slice )`

Put a dimension slice worth of data for this variable in the specified
dimension slice.

`long get_index( const ncbyte* vals )`

`long get_index( const char* vals )`

`long get_index( const short* vals )`

`long get_index( const int* vals )`

`long get_index( const long* vals )`

`long get_index( const float* vals )`

`long get_index( const double* vals )`

Get first record index for this variable corresponding to the specified
key value(s).

`long get_index( NcDim*, const ncbyte* vals )`

`long get_index( NcDim*, const char* vals )`

`long get_index( NcDim*, const short* vals )`

`long get_index( NcDim*, const int* vals )`

`long get_index( NcDim*, const long* vals )`

`long get_index( NcDim*, const float* vals )`

`long get_index( NcDim*, const double* vals )`

Get first index of specified dimension for this variable corresponding
to the specified key value(s).

`void set_rec ( long rec )`

Set the current record for this variable.

`void set_rec ( NcDim*, long rec )`

Set the current dimension slice for the specified dimension for this
variable.

* * * * *

  ------------------------------------------------------------------------ -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Public-Member-Functions-2 "Previous section in reading order")]   [[\>](#Public-Member-Functions-3 "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#NetCDF-Classes "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ -------------------------------------------------------------------- --- --------------------------------------------------------------------------- -------------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Class NcAtt {.unnumberedsec}
-----------

`NcAtt` is derived from `NcTypedComponent`, and represents a netCDF
attribute. A netCDF attribute has a name and a type, and may be either a
scalar attribute or a vector attribute. Scalar attributes have one value
and vector attributes have multiple values. In addition, each attribute
is attached to a specific netCDF variable or is global to an entire
netCDF file. Because attributes are only associated with open netCDF
files, there are no public constructors for this class. Use member
functions of `NcFile` and `NcVar` to get netCDF attributes or add new
attributes. Most of the useful member functions for `NcAtt` are
inherited from class `NcTypedComponent`.

* * * * *

  ---------------------------------------------------------- ------------------------------------------------------------ --- --------------------------------------------------------------------------- ----------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Class-NcAtt "Previous section in reading order")]   [[\>](#Auxiliary-Classes "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Class-NcAtt "Up section")]   [[\>\>](#Auxiliary-Classes "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- ------------------------------------------------------------ --- --------------------------------------------------------------------------- ----------------------------------- --------------------------------------------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### Public Member Functions {.unnumberedsubsec}

`NcToken name( void ) const`

Returns the name of the attribute.

`NcType type( void ) const`

Returns the type of the attribute. The type will be one of `ncByte`,
`ncChar`, `ncShort`, `ncInt`, `ncFloat`, or `ncDouble`.

`NcBool is_valid( void ) const`

Returns `TRUE` if the attribute is valid, `FALSE` otherwise.

`long num_vals( void ) const`

Returns the number of values for an attribute. This is just 1 for a
scalar attribute, the number of values for a vector-valued attribute,
and the number of characters for a string-valued attribute.

`NcBool rename( NcToken newname )`

Renames the attribute.

`NcValues* values( void ) const`

Returns a pointer to the block of all values for the attribute. The
caller is responsible for deleting this block of values when no longer
needed.

`ncbyte as_ncbyte( int n ) const`

`char as_char( int n ) const`

`short as_short( int n ) const`

`int as_int( int n ) const`

`nclong as_nclong( int n ) const // deprecated`

`long as_long( int n ) const`

`float as_float( int n ) const`

`double as_double( int n ) const`

`char* as_string( int n ) const`

Get the n-th value of the attribute. These member functions provide
conversions from the value type of the attribute to the specified type.
If the value is out-of-range, the fill-value of the appropriate type is
returned.

`NcBool remove( void )`

Deletes the attribute from the file and detaches it from the variable.
Does not call the destructor. Subsequent calls to `is_valid()` will
return `FALSE`.

`~NcAtt( void )`

Destructor.

* * * * *

  ------------------------------------------------------------------------ --------------------------------------------------------- --- --------------------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Public-Member-Functions-3 "Previous section in reading order")]   [[\>](#Class-NcValues "Next section in reading order")]       [[\<\<](#NetCDF-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ --------------------------------------------------------- --- --------------------------------------------------------------------------- --------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Auxiliary Classes {.unnumbered}
=================

Auxiliary classes include the abstract base class `NcValues`, its
type-specific derived subclasses, and the error-handling class
`NcError`.

  ----------------------------------- ---- --
  [Class NcValues](#Class-NcValues)        
  [Class NcError](#Class-NcError)          
  ----------------------------------- ---- --

* * * * *

  ---------------------------------------------------------------- ------------------------------------------------------------------ --- ------------------------------------------------------------------------------ ----------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Auxiliary-Classes "Previous section in reading order")]   [[\>](#Public-Member-Functions "Next section in reading order")]       [[\<\<](#Auxiliary-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Auxiliary-Classes "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- ------------------------------------------------------------------ --- ------------------------------------------------------------------------------ ----------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Class NcValues {.unnumberedsec}
--------------

Class `NcValues` is an abstract base class for a block of typed values.
The derived classes are `NcValues_ncbyte`, `NcValues_char`,
`NcValues_short`, `NcValues_int`, `NcValues_nclong` (deprecated), and
`NcValues_long`, `NcValues_float`, `NcValues_double`. These classes are
used as the return type of the `NcTypedComponent::values()` member
function, for typed-value arrays associated with variables and
attributes.

* * * * *

  ------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------------------ -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Class-NcValues "Previous section in reading order")]   [[\>](#Class-NcError "Next section in reading order")]       [[\<\<](#Auxiliary-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Class-NcValues "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------------------ -------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### Public Member Functions {.unnumberedsubsec}

`NcValues( void )`

Default constructor.

`NcValues(NcType, long)`

Constructor for a value block of the specified type and length.

`~NcValues( void )`

Destructor.

`long num( void )`

Returns the number of values in the value block.

`ostream& print(ostream&) const`

Used to print the comma-delimited sequence of values of the value block.

`void* base( void ) const`

Returns a bland pointer to the beginning of the value block.

`int bytes_for_one( void ) const`

Returns the number of bytes required for one value.

`ncbyte as_ncbyte( int n ) const`

`char as_char( int n ) const`

`short as_short( int n ) const`

`int as_int( int n ) const`

`nclong as_nclong( int n ) const // deprecated`

`long as_long( int n ) const`

`float as_float( int n ) const`

`double as_double( int n ) const`

`char* as_string( int n ) const`

Provide conversions for the nth value from the value type to a desired
basic type. If the value is out of range, the default "fill-value" for
the appropriate type is returned.

* * * * *

  ---------------------------------------------------------------------- -------------------------------------------------------------------- --- ------------------------------------------------------------------------------ ----------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Public-Member-Functions "Previous section in reading order")]   [[\>](#Public-Member-Functions-5 "Next section in reading order")]       [[\<\<](#Auxiliary-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Auxiliary-Classes "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------- -------------------------------------------------------------------- --- ------------------------------------------------------------------------------ ----------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Class NcError {.unnumberedsec}
-------------

This class provides control for netCDF error handling. Declaring an
`NcError` object temporarily changes the error-handling behavior for all
netCDF classes until the `NcError` object is destroyed (typically by
going out of scope), at which time the previous error-handling behavior
is restored.

* * * * *

  ------------------------------------------------------------ --------------------------------------------------------- --- ------------------------------------------------------------------------------ ------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Class-NcError "Previous section in reading order")]   [[\>](#Combined-Index "Next section in reading order")]       [[\<\<](#Auxiliary-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Class-NcError "Up section")]   [[\>\>](#Combined-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ --------------------------------------------------------- --- ------------------------------------------------------------------------------ ------------------------------------- ------------------------------------------ --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

### Public Member Functions {.unnumberedsubsec}

`NcError( Behavior b = verbose_fatal )`

The constructor saves the previous error state for restoration when the
destructor is invoked, and sets a new specified state. Valid error
states are `NcError::silent_nonfatal`, `NcError::verbose_nonfatal`,
`NcError::silent_fatal`, or `NcError::verbose_fatal`, to control whether
error messages are output from the underlying library and whether such
messages are fatal or nonfatal.

`~NcError( void )`

Destructor, restores previous error state.

`int get_err( void )`

Returns most recent error, as enumerated in ‘`netcdf.h`’.

* * * * *

  ------------------------------------------------------------------------ -------- --- ------------------------------------------------------------------------------ --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------
  [[\<](#Public-Member-Functions-5 "Previous section in reading order")]   [ \> ]       [[\<\<](#Auxiliary-Classes "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [ \>\> ]                   [[Top](#Top "Cover (top) of document")]   [Contents]   [[Index](#Combined-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------ -------- --- ------------------------------------------------------------------------------ --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------ ------------------------------------ ----------------------------------

Index {.unnumbered}
=====

Jump to:  

[**\~**](#Index_cp_symbol-1)   \
 [**A**](#Index_cp_letter-A)   [**B**](#Index_cp_letter-B)  
[**C**](#Index_cp_letter-C)   [**E**](#Index_cp_letter-E)  
[**G**](#Index_cp_letter-G)   [**I**](#Index_cp_letter-I)  
[**N**](#Index_cp_letter-N)   [**P**](#Index_cp_letter-P)  
[**R**](#Index_cp_letter-R)   [**S**](#Index_cp_letter-S)  
[**T**](#Index_cp_letter-T)   [**V**](#Index_cp_letter-V)  

Index Entry

Section

* * * * *

\~

[\~NcAtt](#index-_007eNcAtt)

[Public Member Functions](#Public-Member-Functions-3)

[\~NcError](#index-_007eNcError)

[Public Member Functions](#Public-Member-Functions-5)

[\~NcFile](#index-_007eNcFile)

[Public Member Functions](#Public-Member-Functions-1)

[\~NcValues](#index-_007eNcValues)

[Public Member Functions](#Public-Member-Functions)

[\~NcVar](#index-_007eNcVar)

[Public Member Functions](#Public-Member-Functions-2)

* * * * *

A

[abort](#index-abort)

[Public Member Functions](#Public-Member-Functions-1)

[add\_att](#index-add_005fatt)

[Public Member Functions](#Public-Member-Functions-1)

[add\_att](#index-add_005fatt-1)

[Public Member Functions](#Public-Member-Functions-1)

[add\_att](#index-add_005fatt-2)

[Public Member Functions](#Public-Member-Functions-2)

[add\_dim](#index-add_005fdim)

[Public Member Functions](#Public-Member-Functions-1)

[add\_dim](#index-add_005fdim-1)

[Public Member Functions](#Public-Member-Functions-1)

[add\_var](#index-add_005fvar)

[Public Member Functions](#Public-Member-Functions-1)

[add\_var](#index-add_005fvar-1)

[Public Member Functions](#Public-Member-Functions-1)

[as\_char](#index-as_005fchar)

[Public Member Functions](#Public-Member-Functions-6)

[as\_char](#index-as_005fchar-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_char](#index-as_005fchar-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_char](#index-as_005fchar-3)

[Public Member Functions](#Public-Member-Functions)

[as\_double](#index-as_005fdouble)

[Public Member Functions](#Public-Member-Functions-6)

[as\_double](#index-as_005fdouble-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_double](#index-as_005fdouble-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_double](#index-as_005fdouble-3)

[Public Member Functions](#Public-Member-Functions)

[as\_float](#index-as_005ffloat)

[Public Member Functions](#Public-Member-Functions-6)

[as\_float](#index-as_005ffloat-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_float](#index-as_005ffloat-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_float](#index-as_005ffloat-3)

[Public Member Functions](#Public-Member-Functions)

[as\_int](#index-as_005fint)

[Public Member Functions](#Public-Member-Functions-6)

[as\_int](#index-as_005fint-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_int](#index-as_005fint-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_int](#index-as_005fint-3)

[Public Member Functions](#Public-Member-Functions)

[as\_long](#index-as_005flong)

[Public Member Functions](#Public-Member-Functions-6)

[as\_long](#index-as_005flong-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_long](#index-as_005flong-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_long](#index-as_005flong-3)

[Public Member Functions](#Public-Member-Functions)

[as\_ncbyte](#index-as_005fncbyte)

[Public Member Functions](#Public-Member-Functions-6)

[as\_ncbyte](#index-as_005fncbyte-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_ncbyte](#index-as_005fncbyte-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_ncbyte](#index-as_005fncbyte-3)

[Public Member Functions](#Public-Member-Functions)

[as\_nclong](#index-as_005fnclong)

[Public Member Functions](#Public-Member-Functions-6)

[as\_nclong](#index-as_005fnclong-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_nclong](#index-as_005fnclong-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_nclong](#index-as_005fnclong-3)

[Public Member Functions](#Public-Member-Functions)

[as\_short](#index-as_005fshort)

[Public Member Functions](#Public-Member-Functions-6)

[as\_short](#index-as_005fshort-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_short](#index-as_005fshort-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_short](#index-as_005fshort-3)

[Public Member Functions](#Public-Member-Functions)

[as\_string](#index-as_005fstring)

[Public Member Functions](#Public-Member-Functions-6)

[as\_string](#index-as_005fstring-1)

[Public Member Functions](#Public-Member-Functions-2)

[as\_string](#index-as_005fstring-2)

[Public Member Functions](#Public-Member-Functions-3)

[as\_string](#index-as_005fstring-3)

[Public Member Functions](#Public-Member-Functions)

[auxiliary types and constants](#index-auxiliary-types-and-constants)

[Auxiliary Types and Constants](#Auxiliary-Types-and-Constants)

* * * * *

B

[base](#index-base)

[Public Member Functions](#Public-Member-Functions)

[bytes\_for\_one](#index-bytes_005ffor_005fone)

[Public Member Functions](#Public-Member-Functions)

* * * * *

C

[class hierarchy](#index-class-hierarchy)

[Class Hierarchy](#Class-Hierarchy)

[close](#index-close)

[Public Member Functions](#Public-Member-Functions-1)

* * * * *

E

[edges](#index-edges)

[Public Member Functions](#Public-Member-Functions-2)

* * * * *

G

[get](#index-get)

[Public Member Functions](#Public-Member-Functions-2)

[get](#index-get-1)

[Public Member Functions](#Public-Member-Functions-2)

[get\_att](#index-get_005fatt)

[Public Member Functions](#Public-Member-Functions-1)

[get\_att](#index-get_005fatt-1)

[Public Member Functions](#Public-Member-Functions-1)

[get\_att](#index-get_005fatt-2)

[Public Member Functions](#Public-Member-Functions-2)

[get\_dim](#index-get_005fdim)

[Public Member Functions](#Public-Member-Functions-1)

[get\_dim](#index-get_005fdim-1)

[Public Member Functions](#Public-Member-Functions-1)

[get\_dim](#index-get_005fdim-2)

[Public Member Functions](#Public-Member-Functions-2)

[get\_err](#index-get_005ferr)

[Public Member Functions](#Public-Member-Functions-5)

[get\_fill](#index-get_005ffill)

[Public Member Functions](#Public-Member-Functions-1)

[get\_format](#index-get_005fformat)

[Public Member Functions](#Public-Member-Functions-1)

[get\_index](#index-get_005findex)

[Public Member Functions](#Public-Member-Functions-2)

[get\_rec](#index-get_005frec)

[Public Member Functions](#Public-Member-Functions-2)

[get\_var](#index-get_005fvar)

[Public Member Functions](#Public-Member-Functions-1)

[get\_var](#index-get_005fvar-1)

[Public Member Functions](#Public-Member-Functions-1)

* * * * *

I

[id](#index-id)

[Public Member Functions](#Public-Member-Functions-2)

[is\_unlimited](#index-is_005funlimited)

[Public Member Functions](#Public-Member-Functions-4)

[is\_valid](#index-is_005fvalid)

[Public Member Functions](#Public-Member-Functions-1)

[is\_valid](#index-is_005fvalid-1)

[Public Member Functions](#Public-Member-Functions-4)

[is\_valid](#index-is_005fvalid-2)

[Public Member Functions](#Public-Member-Functions-6)

[is\_valid](#index-is_005fvalid-3)

[Public Member Functions](#Public-Member-Functions-2)

[is\_valid](#index-is_005fvalid-4)

[Public Member Functions](#Public-Member-Functions-3)

* * * * *

N

[name](#index-name)

[Public Member Functions](#Public-Member-Functions-4)

[name](#index-name-1)

[Public Member Functions](#Public-Member-Functions-6)

[name](#index-name-2)

[Public Member Functions](#Public-Member-Functions-2)

[name](#index-name-3)

[Public Member Functions](#Public-Member-Functions-3)

[NcAtt](#index-NcAtt)

[Public Member Functions](#Public-Member-Functions-2)

[NcAtt::remove](#index-NcAtt_003a_003aremove)

[Public Member Functions](#Public-Member-Functions-3)

[NcAtt::\~NcAtt](#index-NcAtt_003a_003a_007eNcAtt)

[Public Member Functions](#Public-Member-Functions-3)

[NcBool](#index-NcBool)

[Auxiliary Types and Constants](#Auxiliary-Types-and-Constants)

[ncbyte](#index-ncbyte)

[Auxiliary Types and Constants](#Auxiliary-Types-and-Constants)

[NcDim](#index-NcDim)

[Public Member Functions](#Public-Member-Functions-1)

[NcDim::is\_unlimited](#index-NcDim_003a_003ais_005funlimited)

[Public Member Functions](#Public-Member-Functions-4)

[NcDim::is\_valid](#index-NcDim_003a_003ais_005fvalid)

[Public Member Functions](#Public-Member-Functions-4)

[NcDim::name](#index-NcDim_003a_003aname)

[Public Member Functions](#Public-Member-Functions-4)

[NcDim::rename](#index-NcDim_003a_003arename)

[Public Member Functions](#Public-Member-Functions-4)

[NcDim::size](#index-NcDim_003a_003asize)

[Public Member Functions](#Public-Member-Functions-4)

[NcDim::sync](#index-NcDim_003a_003async)

[Public Member Functions](#Public-Member-Functions-4)

[NcError](#index-NcError-1)

[Public Member Functions](#Public-Member-Functions-5)

[NcError::get\_err](#index-NcError_003a_003aget_005ferr)

[Public Member Functions](#Public-Member-Functions-5)

[NcError::\~NcError](#index-NcError_003a_003a_007eNcError)

[Public Member Functions](#Public-Member-Functions-5)

[NcFile](#index-NcFile-1)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::abort](#index-NcFile_003a_003aabort)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::add\_att](#index-NcFile_003a_003aadd_005fatt)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::add\_att](#index-NcFile_003a_003aadd_005fatt-1)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::add\_dim](#index-NcFile_003a_003aadd_005fdim)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::add\_dim](#index-NcFile_003a_003aadd_005fdim-1)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::add\_var](#index-NcFile_003a_003aadd_005fvar)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::add\_var](#index-NcFile_003a_003aadd_005fvar-1)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::close](#index-NcFile_003a_003aclose)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::get\_att](#index-NcFile_003a_003aget_005fatt)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::get\_att](#index-NcFile_003a_003aget_005fatt-1)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::get\_dim](#index-NcFile_003a_003aget_005fdim)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::get\_dim](#index-NcFile_003a_003aget_005fdim-1)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::get\_fill](#index-NcFile_003a_003aget_005ffill)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::get\_format](#index-NcFile_003a_003aget_005fformat)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::get\_var](#index-NcFile_003a_003aget_005fvar)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::get\_var](#index-NcFile_003a_003aget_005fvar-1)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::is\_valid](#index-NcFile_003a_003ais_005fvalid)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::NcFile](#index-NcFile_003a_003aNcFile)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::num\_atts](#index-NcFile_003a_003anum_005fatts)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::num\_dims](#index-NcFile_003a_003anum_005fdims)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::num\_vars](#index-NcFile_003a_003anum_005fvars)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::rec\_dim](#index-NcFile_003a_003arec_005fdim)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::set\_fill](#index-NcFile_003a_003aset_005ffill)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::sync](#index-NcFile_003a_003async)

[Public Member Functions](#Public-Member-Functions-1)

[NcFile::\~NcFile](#index-NcFile_003a_003a_007eNcFile)

[Public Member Functions](#Public-Member-Functions-1)

[NcToken](#index-NcToken)

[Auxiliary Types and Constants](#Auxiliary-Types-and-Constants)

[NcType](#index-NcType)

[Auxiliary Types and Constants](#Auxiliary-Types-and-Constants)

[NcTypedComponent](#index-NcTypedComponent)

[Public Member Functions](#Public-Member-Functions-4)

[NcTypedComponent::as\_char](#index-NcTypedComponent_003a_003aas_005fchar)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_char](#index-NcTypedComponent_003a_003aas_005fchar-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_char](#index-NcTypedComponent_003a_003aas_005fchar-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::as\_double](#index-NcTypedComponent_003a_003aas_005fdouble)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_double](#index-NcTypedComponent_003a_003aas_005fdouble-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_double](#index-NcTypedComponent_003a_003aas_005fdouble-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::as\_float](#index-NcTypedComponent_003a_003aas_005ffloat)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_float](#index-NcTypedComponent_003a_003aas_005ffloat-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_float](#index-NcTypedComponent_003a_003aas_005ffloat-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::as\_int](#index-NcTypedComponent_003a_003aas_005fint)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_int](#index-NcTypedComponent_003a_003aas_005fint-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_int](#index-NcTypedComponent_003a_003aas_005fint-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::as\_long](#index-NcTypedComponent_003a_003aas_005flong)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_long](#index-NcTypedComponent_003a_003aas_005flong-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_long](#index-NcTypedComponent_003a_003aas_005flong-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::as\_ncbyte](#index-NcTypedComponent_003a_003aas_005fncbyte)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_ncbyte](#index-NcTypedComponent_003a_003aas_005fncbyte-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_ncbyte](#index-NcTypedComponent_003a_003aas_005fncbyte-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::as\_nclong](#index-NcTypedComponent_003a_003aas_005fnclong)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_nclong](#index-NcTypedComponent_003a_003aas_005fnclong-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_nclong](#index-NcTypedComponent_003a_003aas_005fnclong-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::as\_short](#index-NcTypedComponent_003a_003aas_005fshort)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_short](#index-NcTypedComponent_003a_003aas_005fshort-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_short](#index-NcTypedComponent_003a_003aas_005fshort-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::as\_string](#index-NcTypedComponent_003a_003aas_005fstring)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::as\_string](#index-NcTypedComponent_003a_003aas_005fstring-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::as\_string](#index-NcTypedComponent_003a_003aas_005fstring-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::is\_valid](#index-NcTypedComponent_003a_003ais_005fvalid)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::is\_valid](#index-NcTypedComponent_003a_003ais_005fvalid-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::is\_valid](#index-NcTypedComponent_003a_003ais_005fvalid-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::name](#index-NcTypedComponent_003a_003aname)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::name](#index-NcTypedComponent_003a_003aname-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::name](#index-NcTypedComponent_003a_003aname-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::num\_vals](#index-NcTypedComponent_003a_003anum_005fvals)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::num\_vals](#index-NcTypedComponent_003a_003anum_005fvals-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::num\_vals](#index-NcTypedComponent_003a_003anum_005fvals-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::rename](#index-NcTypedComponent_003a_003arename)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::rename](#index-NcTypedComponent_003a_003arename-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::rename](#index-NcTypedComponent_003a_003arename-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::type](#index-NcTypedComponent_003a_003atype)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::type](#index-NcTypedComponent_003a_003atype-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::type](#index-NcTypedComponent_003a_003atype-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcTypedComponent::values](#index-NcTypedComponent_003a_003avalues)

[Public Member Functions](#Public-Member-Functions-6)

[NcTypedComponent::values](#index-NcTypedComponent_003a_003avalues-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcTypedComponent::values](#index-NcTypedComponent_003a_003avalues-2)

[Public Member Functions](#Public-Member-Functions-3)

[NcValues](#index-NcValues-1)

[Public Member Functions](#Public-Member-Functions)

[NcValues](#index-NcValues-2)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_char](#index-NcValues_003a_003aas_005fchar)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_double](#index-NcValues_003a_003aas_005fdouble)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_float](#index-NcValues_003a_003aas_005ffloat)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_int](#index-NcValues_003a_003aas_005fint)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_long](#index-NcValues_003a_003aas_005flong)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_ncbyte](#index-NcValues_003a_003aas_005fncbyte)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_nclong](#index-NcValues_003a_003aas_005fnclong)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_short](#index-NcValues_003a_003aas_005fshort)

[Public Member Functions](#Public-Member-Functions)

[NcValues::as\_string](#index-NcValues_003a_003aas_005fstring)

[Public Member Functions](#Public-Member-Functions)

[NcValues::base](#index-NcValues_003a_003abase)

[Public Member Functions](#Public-Member-Functions)

[NcValues::bytes\_for\_one](#index-NcValues_003a_003abytes_005ffor_005fone)

[Public Member Functions](#Public-Member-Functions)

[NcValues::NcValues](#index-NcValues_003a_003aNcValues)

[Public Member Functions](#Public-Member-Functions)

[NcValues::NcValues](#index-NcValues_003a_003aNcValues-1)

[Public Member Functions](#Public-Member-Functions)

[NcValues::num](#index-NcValues_003a_003anum)

[Public Member Functions](#Public-Member-Functions)

[NcValues::print](#index-NcValues_003a_003aprint)

[Public Member Functions](#Public-Member-Functions)

[NcValues::\~NcValues](#index-NcValues_003a_003a_007eNcValues)

[Public Member Functions](#Public-Member-Functions)

[NcValues\_char](#index-NcValues_005fchar)

[Class NcValues](#Class-NcValues)

[NcValues\_double](#index-NcValues_005fdouble)

[Class NcValues](#Class-NcValues)

[NcValues\_float](#index-NcValues_005ffloat)

[Class NcValues](#Class-NcValues)

[NcValues\_int](#index-NcValues_005fint)

[Class NcValues](#Class-NcValues)

[NcValues\_long](#index-NcValues_005flong)

[Class NcValues](#Class-NcValues)

[NcValues\_ncbyte](#index-NcValues_005fncbyte)

[Class NcValues](#Class-NcValues)

[NcValues\_nclong](#index-NcValues_005fnclong)

[Class NcValues](#Class-NcValues)

[NcValues\_short](#index-NcValues_005fshort)

[Class NcValues](#Class-NcValues)

[NcVar](#index-NcVar)

[Public Member Functions](#Public-Member-Functions-6)

[NcVar::add\_att](#index-NcVar_003a_003aadd_005fatt)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::edges](#index-NcVar_003a_003aedges)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::get](#index-NcVar_003a_003aget)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::get](#index-NcVar_003a_003aget-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::get\_att](#index-NcVar_003a_003aget_005fatt)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::get\_dim](#index-NcVar_003a_003aget_005fdim)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::get\_index](#index-NcVar_003a_003aget_005findex)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::get\_rec](#index-NcVar_003a_003aget_005frec)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::id](#index-NcVar_003a_003aid)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::num\_atts](#index-NcVar_003a_003anum_005fatts)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::num\_dims](#index-NcVar_003a_003anum_005fdims)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::put](#index-NcVar_003a_003aput)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::put](#index-NcVar_003a_003aput-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::put\_rec](#index-NcVar_003a_003aput_005frec)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::put\_rec](#index-NcVar_003a_003aput_005frec-1)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::rec\_size](#index-NcVar_003a_003arec_005fsize)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::set\_cur](#index-NcVar_003a_003aset_005fcur)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::set\_rec](#index-NcVar_003a_003aset_005frec)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::sync](#index-NcVar_003a_003async)

[Public Member Functions](#Public-Member-Functions-2)

[NcVar::\~NcVar](#index-NcVar_003a_003a_007eNcVar)

[Public Member Functions](#Public-Member-Functions-2)

[num](#index-num)

[Public Member Functions](#Public-Member-Functions)

[num\_atts](#index-num_005fatts)

[Public Member Functions](#Public-Member-Functions-1)

[num\_atts](#index-num_005fatts-1)

[Public Member Functions](#Public-Member-Functions-2)

[num\_dims](#index-num_005fdims)

[Public Member Functions](#Public-Member-Functions-1)

[num\_dims](#index-num_005fdims-1)

[Public Member Functions](#Public-Member-Functions-2)

[num\_vals](#index-num_005fvals)

[Public Member Functions](#Public-Member-Functions-6)

[num\_vals](#index-num_005fvals-1)

[Public Member Functions](#Public-Member-Functions-2)

[num\_vals](#index-num_005fvals-2)

[Public Member Functions](#Public-Member-Functions-3)

[num\_vars](#index-num_005fvars)

[Public Member Functions](#Public-Member-Functions-1)

* * * * *

P

[print](#index-print)

[Public Member Functions](#Public-Member-Functions)

[put](#index-put)

[Public Member Functions](#Public-Member-Functions-2)

[put](#index-put-1)

[Public Member Functions](#Public-Member-Functions-2)

[put\_rec](#index-put_005frec)

[Public Member Functions](#Public-Member-Functions-2)

[put\_rec](#index-put_005frec-1)

[Public Member Functions](#Public-Member-Functions-2)

* * * * *

R

[rec\_dim](#index-rec_005fdim)

[Public Member Functions](#Public-Member-Functions-1)

[rec\_size](#index-rec_005fsize)

[Public Member Functions](#Public-Member-Functions-2)

[remove](#index-remove)

[Public Member Functions](#Public-Member-Functions-3)

[rename](#index-rename)

[Public Member Functions](#Public-Member-Functions-4)

[rename](#index-rename-1)

[Public Member Functions](#Public-Member-Functions-6)

[rename](#index-rename-2)

[Public Member Functions](#Public-Member-Functions-2)

[rename](#index-rename-3)

[Public Member Functions](#Public-Member-Functions-3)

[requirements of C++
interface](#index-requirements-of-C_002b_002b-interface)

[Introduction](#Introduction)

* * * * *

S

[set\_cur](#index-set_005fcur)

[Public Member Functions](#Public-Member-Functions-2)

[set\_fill](#index-set_005ffill)

[Public Member Functions](#Public-Member-Functions-1)

[set\_rec](#index-set_005frec)

[Public Member Functions](#Public-Member-Functions-2)

[size](#index-size)

[Public Member Functions](#Public-Member-Functions-4)

[sync](#index-sync)

[Public Member Functions](#Public-Member-Functions-1)

[sync](#index-sync-1)

[Public Member Functions](#Public-Member-Functions-4)

[sync](#index-sync-2)

[Public Member Functions](#Public-Member-Functions-2)

* * * * *

T

[type](#index-type)

[Public Member Functions](#Public-Member-Functions-6)

[type](#index-type-1)

[Public Member Functions](#Public-Member-Functions-2)

[type](#index-type-2)

[Public Member Functions](#Public-Member-Functions-3)

* * * * *

V

[values](#index-values)

[Public Member Functions](#Public-Member-Functions-6)

[values](#index-values-1)

[Public Member Functions](#Public-Member-Functions-2)

[values](#index-values-2)

[Public Member Functions](#Public-Member-Functions-3)

* * * * *

Jump to:  

[**\~**](#Index_cp_symbol-1)   \
 [**A**](#Index_cp_letter-A)   [**B**](#Index_cp_letter-B)  
[**C**](#Index_cp_letter-C)   [**E**](#Index_cp_letter-E)  
[**G**](#Index_cp_letter-G)   [**I**](#Index_cp_letter-I)  
[**N**](#Index_cp_letter-N)   [**P**](#Index_cp_letter-P)  
[**R**](#Index_cp_letter-R)   [**S**](#Index_cp_letter-S)  
[**T**](#Index_cp_letter-T)   [**V**](#Index_cp_letter-V)  

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

