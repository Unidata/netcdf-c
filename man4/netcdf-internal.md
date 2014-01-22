\page netcdf-internal-texi NetCDF-Internal Documentation (netCDF 3.6.0)

  ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

NetCDF Library Internals 
========================

The most recent update of this documentation was released with version
3.6.0 of netCDF.

The netCDF data model is described in The NetCDF Users’ Guide (see [The
NetCDF Users Guide: (netcdf)Top](netcdf.html#Top) section ‘Top’ in The
NetCDF Users’ Guide).

Reference guides are available for the C (see [The NetCDF Users Guide
for C: (netcdf-c)Top](netcdf-c.html#Top) section ‘Top’ in The NetCDF
Users’ Guide for C), C++ (see [The NetCDF Users’ Guide for C++:
(netcdf-cxx)Top](netcdf-cxx.html#Top) section ‘Top’ in The NetCDF Users
Guide for C++), FORTRAN 77 (see [The NetCDF Users’ Guide for FORTRAN 77:
(netcdf-f77)Top](netcdf-f77.html#Top) section ‘Top’ in The NetCDF Users
Guide for FORTRAN 77), and FORTRAN (see [The NetCDF Users’ Guide for
FORTRAN 90: (netcdf-f90)Top](netcdf-f90.html#Top) section ‘Top’ in The
NetCDF Users’ Guide for FORTRAN 90).

[1. C Code](#C-Code)

  

[2. Derivitive Works](#Derivitive-Works)

  

[3. Concept Index](#Concept-Index)

  

~~~~ 
 — The Detailed Node Listing —

C Code
~~~~

[1.1 libsrc directory](#libsrc-directory)

  

[1.2 nc\_test directory](#nc_005ftest-directory)

  

[1.3 nctest directory](#nctest-directory)

  

[1.4 cxx directory](#cxx-directory)

  

[1.5 man directory](#man-directory)

  

[1.6 ncgen and ncgen4 directories](#ncgen-and-ncgen4-directories)

  

[1.7 ncdump directory](#ncdump-directory)

  

[1.8 fortran directory](#fortran-directory)

  

~~~~ 
~~~~

~~~~ 
~~~~

* * * * *

  -------------------------------------------------- ----------------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#Top "Previous section in reading order")]   [[\>](#libsrc-directory "Next section in reading order")]       [[\<\<](#Top "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------- ----------------------------------------------------------- --- ---------------------------------------------------------------- --------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1. C Code 
=========

The netCDF library is implemented in C in a bunch of directories under
netcdf-3.

  ------------------------------------------------------------------- ---- --
  [1.1 libsrc directory](#libsrc-directory)                                
  [1.2 nc\_test directory](#nc_005ftest-directory)                         
  [1.3 nctest directory](#nctest-directory)                                
  [1.4 cxx directory](#cxx-directory)                                      
  [1.5 man directory](#man-directory)                                      
  [1.6 ncgen and ncgen4 directories](#ncgen-and-ncgen4-directories)        
  [1.7 ncdump directory](#ncdump-directory)                                
  [1.8 fortran directory](#fortran-directory)                              
  ------------------------------------------------------------------- ---- --

* * * * *

  ----------------------------------------------------- --------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#C-Code "Previous section in reading order")]   [[\>](#m4-Files "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#C-Code "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------- --------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1.1 libsrc directory 
--------------------

The libsrc directory holds the core library C code.

* * * * *

  --------------------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#libsrc-directory "Previous section in reading order")]   [[\>](#C-Header-Files "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#libsrc-directory "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- --------------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

### 1.1.1 m4 Files 

The m4 macro processor is used as a pre-pre-processor for attr.m4,
putget.m4, ncx.m4, t\_ncxx.m4. The m4 macros are used to deal with the 6
different netcdf data types.

 `attr.m4`
:   Attribute functions.

 `putget.m4`
:   Contains putNCvx\_type\_type, putNCv\_type\_type,
    getNCvx\_type\_type, getNCv\_type\_type, plus a bunch of other
    important internal functions dealing with reading and writing data.
    External functions nc\_put\_varX\_type and nc\_get\_varX\_type, are
    implemented.

 `ncx.m4`
:   Contains netCDF implementation of XDR. Bit-fiddling on VAXes and
    other fun stuff.

 `t_ncxx.m4`
:   Test program for netCDF XDR library.

* * * * *

  ------------------------------------------------------- ------------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#m4-Files "Previous section in reading order")]   [[\>](#C-Code-Files "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#libsrc-directory "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ------------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

### 1.1.2 C Header Files 

 `netcdf.h` 
:   The formal definition of the netCDF API.

 `nc.h`
:   Private data structures, objects and interfaces.

 `ncio.h`
:   I/O abstraction interface, including struct ncio.

 `ncx.h`
:   External data representation interface.

 `fbits.h`
:   Preprocessor macros for dealing with flags: fSet, fClr, fIsSet,
    fMask, pIf, pIff.

 `ncconfig.h`
:   Generated automatically by configure.

 `onstack.h`
:   This file provides definitions which allow us to "allocate" arrays
    on the stack where possible. (Where not possible, malloc and free
    are used.)

 `rnd.h`
:   Some rounding macros.

* * * * *

  ------------------------------------------------------------- --------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#C-Header-Files "Previous section in reading order")]   [[\>](#Makefile "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#libsrc-directory "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------- --------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

### 1.1.3 C Code Files 

 `nc.c` 
:   Holds nc\_open, nc\_create, nc\_enddef, nc\_close, nc\_delete,
    nc\_abort, nc\_redef, nc\_inq, nc\_sync,, nc\_set\_fill. It also
    holds a lot if internal functions.

 `attr.c`
:   Generated from attr.m4, contains attribute functions.

 `dim.c`
:   Dimension functions.

 `var.c`
:   Variable function

 `v1hpg.c`
:   This module defines the external representation of the "header" of a
    netcdf version one file. For each of the components of the NC
    structure, There are (static) ncx\_len\_XXX(), ncx\_put\_XXX() and
    v1h\_get\_XXX() functions. These define the external representation
    of the components. The exported entry points for the whole NC
    structure are built up from these.

    Although the name of this file implies that it should only apply to
    data version 1, it was modified by the 64-bit offset people, so that
    it actually handles version 2 data as well.

 `v2i.c`
:   Version 2 API implemented in terms of version 3 API.

 `error.c`
:   Contains nc\_strerror.

 `ncio.c`
:   Just decides whether to use ffio.c (for Crays) or posixio.c (for
    everyone else). (See below).

 `posixio.c`\
 `ffio.c`
:   Some file I/O, and a Cray-specific implementation. These two files
    have some functions with the same name and signatures, for example
    ncio\_open. If building on a Cray, ffio.c is used. If building on a
    posix system, posixio.c is used.

    One of the really complex functions in posixio.c is px\_get, which
    reads data from a netCDF file in perhaps a super-efficient manner?

    There are functions for the rel, get, move, sync, free operations,
    one set of functions (with \_spx\_) if NC\_SHARE is in use, and
    another set (with \_px\_) when NC\_SHARE is not in use.

    See also the section I/O Layering below.

 `putget.c`
:   Generated from putget.m4.

 `string.c`
:   NC\_string structures are manipulated here. Also contains
    NC\_check\_name.

 `imap.c`
:   Check map functionality?

 `libvers.c`
:   Implements nc\_inq\_libvers.

 `ncx.c`\
 `ncx_cray.c`
:   Created from ncx.m4, this file contains implementation of netCDF
    XDR, and a Cray-specific implementation.

 `t_nc.c`\
 `t_ncio.c`\
 `t_ncx.c`
:   There are extra tests, activated with make full\_test, in . They
    didn’t compile on my cygwin system, but worked fine on linux. See
    the extra tests section below.

* * * * *

  ----------------------------------------------------------- ----------------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#C-Code-Files "Previous section in reading order")]   [[\>](#I_002fO-Layering "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#libsrc-directory "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------- ----------------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

### 1.1.4 Makefile 

Let us not neglect the Makefile, hand-crafted by Glenn and Steve to
stand the test of many different installation platforms.

* * * * *

  ------------------------------------------------------- ------------------------------------------------------ --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#Makefile "Previous section in reading order")]   [[\>](#Extra-Tests "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#libsrc-directory "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------- ------------------------------------------------------ --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

### 1.1.5 I/O Layering 

Here’s some discussion from Glenn (July, 1997) in the support archive:

 

~~~~ {.example}
Given any platform specific I/O system which is capable of
random access, it is straightforward to write an ncio
implementation which uses that I/O system. A competent C
programmer can do it in less than a day. The interface is
defined in ncio.h. There are two implementations (buffered and
unbuffered) in posixio.c, another in ffio.c, and another contributed
mmapio.c (in pub/netcdf/contrib at our ftp site)
which can be used as examples. (The buffered version in posixio.c has
gotten unreadable at this point, I'm sorry to say.)

A brief outline of the ncio interface follows.

There are  2 public 'constructors':
    ncio_create()
and
    ncio_open().
The first creates a new file and the second opens an existing one.

There is a public 'destructor',
    ncio_close()
which closes the descriptor and calls internal function ncio.free()
to free any allocated resources.

The 'constructors' return a data structure which includes
4 other 'member functions'

    ncio.get() - converts a file region specified by an offset
        and extent to a memory pointer. The region may be
        locked until the corresponding call to rel().

    ncio.rel() - releases the region specified by offset.

    ncio.move() - Copy one region to another without making anything
        available to higher layers. May be just implemented in
        terms of get() and rel(), or may be tricky to be efficient.
        Only used in by nc_enddef() after redefinition.
and
    ncio.sync() - Flush any buffers to disk. May be a no-op on
        if I/O is unbuffered.

The interactions between layers and error semantics are more clearly defined
for ncio than they were for the older xdr stream based system. The functions
all return the system error indication (errno.h) or 0 for no error.

The sizehint parameter to ncio_open() and ncio_create() is a contract between
the upper layers and ncio. It a negotiated value passed. A suggested value
is passed in by reference, and may be modified upon return. The upper layers
use the returned value as a maximum extent for calls to ncio.get().

In the netcdf distribution, there is test program 't_ncio.c',
which can be used to unit test an ncio implementation. The program
is script driven, so a variety of access patterns can be tested
by feeding it different scripts.
~~~~

And some more:

 

~~~~ {.example}
For netcdf I/O on the CRAY, we can identify a couple of levels of buffering.

There is a "contract" between the higher layers of
netcdf and the "ncio" layer which says "we won't request more than
'chunksize' bytes per request. This controls the maximum size of
the buffer used internal to the ncio implementation for system
read and write.

On the Cray, the ncio implementation is usually 'ffio.c', which uses the
cray specific ffio library. This is in contrast to the implementation
used on most other systems, 'posixio.c', which used POSIX read(), write(),
lseek() calls. The ffio library has all sorts of controls (which I don't
pretend
to understand), including the bufa directive you cite above.
If you are using some sort of assign statement external to netcdf, it
is probably being overridden by the following code sequence from
ffio.c:

        ControlString = getenv("NETCDF_FFIOSPEC");
        if(ControlString == NULL)
        {
                 ControlString="bufa:336:2";
        }
    fd = ffopens(path, oflags, 0, 0, &stat, ControlString);

EG, if the NETCDF_FFIOSPEC environment variable is not set, use the
default shown. Otherwise, use the environment variable.

Currently for ffio, the "chunksize" contract  is set to the st_oblksize
member of struct ffc_stat_s obtained from a fffcntl(). You can check
this by putting a breakpoint in ffio.c:blksize().

As you may be able to tell from ffio.c, we have stubbed out interlayer
communication to allow this to be set as the *sizehintp argument to
ncio_open() and ncio_create(). In netcdf-3.4, we intend to fully implement this
and expose this parameter to the user. 
~~~~

In 2004, while on an unrelated research expedition to find the
Brazillian brown-toed tree frog, Russ Rew found a sealed tomb. Within -
a bronze statue of the terrible Monkey God. While scratching the netCDF
logo in the Monkey God’s forehead, Russ discovered a secret compartment,
containing an old, decaying scroll.

Naturally he brought it back to Unidata’s History and Antiquities
Division, on the 103rd floor of UCAR Tower \#2. Within the scroll,
written in blood, Unidata archeologists found the following:

 

~~~~ {.example}
Internal to the netcdf implementation, there is a parameterized
contract between the upper layers and the i/o layer regarding the
maximum amount of data to be transferred between the layers in a
single call. Call this the "chunksize." If the file is opened for
synchronous operations (NC_SHARE), this is to the amount of data
transferred from the file in a single i/o call, and, typically, the
size of the allocated buffer in the i/o layer. In the more usual
buffered case (NC_NOSHARE), accesses to the underlying file system are
alighned on chunksize boundaries and the size of the allocated buffer
is twice this number.

So, the chunksize controls a space versus time tradeoff, memory
allocated in the netcdf library versus number of system calls. It also
controls the relationship between outer and inner loop counters for
large data accesses. A large chunksize would have a small (1?) outer
look counter and a larger inner loop counter, reducing function call
overhead. (Generally this effect is much less significant than the
memory versus system call effect.)

May the curse of the Monkey God devour any who desecrate the chunksize
parameter.
~~~~

* * * * *

  --------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#I_002fO-Layering "Previous section in reading order")]   [[\>](#nc_005ftest-directory "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#libsrc-directory "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- ---------------------------------------------------------------- --- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

### 1.1.6 Extra Tests 

According to Russ make full\_test runs three tests:

 `test`
:   the library blunder test which is a quick test of the library
    implementation

 `nctest`
:   the test of the netCDF-2 interface, which is still used in a lot of
    third-party netCDF software.

 `test_ncx`
:   a test of the XDR-replacement library. netCDF-2 used the
    industry-standard XDR library, but netCDF-3 uses our own replacement
    for it that Glenn wrote as ncx.c and ncxx.c.

The latter test seems to work if you run something like

 

~~~~ {.example}
c89 t_ncxx.c ncx.o -o t_ncx
~~~~

first to create the "t\_ncx" executable, then run "make test\_ncx" and
it will run the two tests "t\_ncx" and "t\_ncxx":

 

~~~~ {.example}
$ make test_ncx
c89 -o t_ncxx -g  t_ncxx.o ncx.o 
./t_ncx
./t_ncxx
~~~~

which produce no output if they succeed.

* * * * *

  ---------------------------------------------------------- ----------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#Extra-Tests "Previous section in reading order")]   [[\>](#nctest-directory "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#C-Code "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------- ----------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1.2 nc\_test directory 
----------------------

This runs the version 3 tests suite.

The main program, nc\_test, can be run with a command line option to
create a farly rich test file. It’s then called again without the option
to read the file and also engage in a bunch of test writes to
scratch.nc.

* * * * *

  -------------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#nc_005ftest-directory "Previous section in reading order")]   [[\>](#cxx-directory "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#C-Code "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  -------------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1.3 nctest directory 
--------------------

This runs the version 2 test suite.

* * * * *

  --------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#nctest-directory "Previous section in reading order")]   [[\>](#man-directory "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#C-Code "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- -------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1.4 cxx directory 
-----------------

This directory contains the C++ interface to netCDF.

* * * * *

  ------------------------------------------------------------ ----------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#cxx-directory "Previous section in reading order")]   [[\>](#ncgen-and-ncgen4-directories "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#C-Code "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ----------------------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1.5 man directory 
-----------------

This directory holds the .m4 file that is used to generate both the C
and fortran man pages. I wish I had known about this directory before I
introducted the doc directory! I have moved all the docs into the man
directory, and tried to delete the doc directory. But cvs won’t let me.
It intends that I always remember my sins. Thanks cvs, you’re like a
conscience.

* * * * *

  ------------------------------------------------------------ ----------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#man-directory "Previous section in reading order")]   [[\>](#ncdump-directory "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#C-Code "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------ ----------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1.6 ncgen and ncgen4 directories 
--------------------------------

The ncgen directory is the home of ncgen, of course. This program uses
lex and yacc to parse the CDL input file. Note that the ncgen program
used to called ncgen4, so this version of ncgen can in fact handle the
full netCDF-4 enhanced data model as CDL input. The old ncgen is still
available, but is now called ncgen3.

lex and yacc files:

 `ncgen.l`
:   Input for flex, the output of which is renamed ncgenyy.c and
    \#included by ncgentab.c.

 `ncgen.y`
:   Input for yacc, the output of which is reanmed ncgentab.c.

Header files:

 `ncgen.h`
:   Defines a bunch of extern variables, like ncid, ndims, nvars, ...
    \*dims, \*vars, \*atts.

 `ncgentab.h`
:   Long list of defines generared by yacc?

 `genlib.h`
:   Prototypes for all the ncgen functions.

 `generic.h`
:   Defines union generic, which can hold any type of value (used for
    handling fill values).

Code files:

 `main.c`
:   Main entry point. Handles command line options and then calls
    yyparse.

 `ncgentab.c`
:   This is generated from ncgen.y by yacc.

 `ncgenyy.c`
:   This is included in ncgentab.c, and not, therefore, on the compile
    list in the makefile, since it’s compiled as part of ncgentab.c.

 `genlib.c`
:   Bunch of functions for ncgen, including ones to write appropriate
    fortran or C code for a netcdf file. Neat! Also the gen\_netcdf
    function, which actually writes out the netcdf file being generated
    by ncgen. Also has emalloc function.

 `getfill.c`
:   A few functions dealing with fill values and their defaults.

 `init.c`\
 `load.c`\
 `escapes.c`
:   Contains one functions, expand\_escapes, expands escape characters,
    like \\t, in input.

* * * * *

  --------------------------------------------------------------------------- ------------------------------------------------------------ --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#ncgen-and-ncgen4-directories "Previous section in reading order")]   [[\>](#fortran-directory "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#C-Code "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------------------- ------------------------------------------------------------ --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1.7 ncdump directory 
--------------------

This directory holds ncdump, of course! No m4 or any of that stuff here
- just plain old C.

* * * * *

  --------------------------------------------------------------- ----------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#ncdump-directory "Previous section in reading order")]   [[\>](#Derivitive-Works "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#C-Code "Up section")]   [[\>\>](#Derivitive-Works "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- ----------------------------------------------------------- --- ------------------------------------------------------------------- ------------------------------ -------------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

1.8 fortran directory 
---------------------

Amazingly, the fortran interface is actually C code! Steve gets some
package involving cfortran.h, which defines a C function of the exact
signature which will be produced by a fortran program calling a C
function. So \_nf\_open will map to nc\_open.

* * * * *

  ---------------------------------------------------------------- ------------------------------------------------------------------------------------------------------------------------- --- ------------------------------------------------------------------- --------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#fortran-directory "Previous section in reading order")]   [[\>](#From-_0060_0060A-Tale-of-Two-Data-Formats_002c_0027_0027-the-bestselling-novel "Next section in reading order")]       [[\<\<](#C-Code "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [[\>\>](#Concept-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------- ------------------------------------------------------------------------------------------------------------------------- --- ------------------------------------------------------------------- --------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

2. Derivitive Works 
===================

At Unidata, the creative energies are simply enourmous. NetCDF has
spawned a host of derivitive works, some samples of which are listed
below.

* * * * *

  --------------------------------------------------------------- -------------------------------------------------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#Derivitive-Works "Previous section in reading order")]   [[\>](#g_t_0060_0060The-Marriage-of-NetCDF_002c_0027_0027-an-Opera-in-Three-Acts "Next section in reading order")]       [[\<\<](#Derivitive-Works "Beginning of this chapter or previous chapter")]   [[Up](#Derivitive-Works "Up section")]   [[\>\>](#Concept-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  --------------------------------------------------------------- -------------------------------------------------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

2.1 From “A Tale of Two Data Formats,” the bestselling novel 
------------------------------------------------------------

 

~~~~ {.example}
It was the best of times, it was the worst of times,
it was the age of webpages, it was the age of ftp,
it was the epoch of free software, it was the epoch of Microsoft,
it was the season of C, it was the season of Java,
it was the spring of hope, it was the winter of despair,
we had everything before us, we had nothing before us,
we were all going direct to HDF5, we were all going direct
the other way--
~~~~

* * * * *

  ----------------------------------------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- ----------------------------------------------------------------------------- ---------------------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#From-_0060_0060A-Tale-of-Two-Data-Formats_002c_0027_0027-the-bestselling-novel "Previous section in reading order")]   [[\>](#Transcript-from-Jerry-Springer-Show "Next section in reading order")]       [[\<\<](#Derivitive-Works "Beginning of this chapter or previous chapter")]   [[Up](#Derivitive-Works "Up section")]   [[\>\>](#Concept-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------ --- ----------------------------------------------------------------------------- ---------------------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

2.2 “The Marriage of NetCDF,” an Opera in Three Acts 
----------------------------------------------------

The massive response to this opera has been called the “Marriage
Phenomenon” by New York Times Sunday Arts and Leisure Section Editor
Albert Winklepops. The multi-billion dollar marketing empire that sprang
from the opera after it’s first season at the Cambridge Theatre in
London’s West End is headquartered in little-known Boulder, Colorado
(most famous resident: Mork from Ork).

 

~~~~ {.example}
Dr. Rew:
[enters stage left and moves downstage center for aria]
I've developed a format,
a great data format,
and it must take over the world!

But it might be surpassed,
by that pain in the ass,
that data format
from Illinois!

Mike Folk:
[enters stage right and moves downstage center]
I've a nice data format,
a real data format,
with plenty of features,
yoh-hoooo

But scientists don't use it,
they say it's confusing,
A simpler API
would dooooo...
~~~~

* * * * *

  ------------------------------------------------------------------------------------------------------------------------ -------------------------------------------------------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#g_t_0060_0060The-Marriage-of-NetCDF_002c_0027_0027-an-Opera-in-Three-Acts "Previous section in reading order")]   [[\>](#show-title_003a_0060_0060I_0027ve-Beed-Dumped-for-a-Newer-Data-Format_0027_0027 "Next section in reading order")]       [[\<\<](#Derivitive-Works "Beginning of this chapter or previous chapter")]   [[Up](#Derivitive-Works "Up section")]   [[\>\>](#Concept-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------------------------------------------ -------------------------------------------------------------------------------------------------------------------------- --- ----------------------------------------------------------------------------- ---------------------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

2.3 Transcript from Jerry Springer Show, aired 12/12/03, 
--------------------------------------------------------

* * * * *

  ---------------------------------------------------------------------------------- -------------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#Transcript-from-Jerry-Springer-Show "Previous section in reading order")]   [[\>](#Concept-Index "Next section in reading order")]       [[\<\<](#Derivitive-Works "Beginning of this chapter or previous chapter")]   [[Up](#Transcript-from-Jerry-Springer-Show "Up section")]   [[\>\>](#Concept-Index "Next chapter")]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ---------------------------------------------------------------------------------- -------------------------------------------------------- --- ----------------------------------------------------------------------------- ----------------------------------------------------------- ----------------------------------------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

### 2.3.1 show title:“I’ve Beed Dumped for a Newer Data Format” 

 

~~~~ {.example}
[Announcer]: ...live today from Boulder, Colorado, the Jerry Springer Show!

[Audience]: (chanting) Jer-ry! Jer-ry!

[Jerry]: Hello and welcome to the Jerry Springer show - today we have
some very special guests, starting with netCDF, a data format from
right here in Boulder.

[Audience]: (applause)

[netCDF]: Hi Jerry!

[Jerry]: Hello netCDF, and welcome to the show. You're here to tell us
something, right?

[netCDF]: (sobbing) I think I'm going to be abandoned for a newer,
younger data format.

[Audience]: (gasps)

[Jerry]: Well, netCDF, we've brought in WRF, a professional model,
also from right here in Boulder.

[Audience]: (applause)

[Jerry]: Now WRF, you've been involved in an intimate relationship
with netCDF for how long now?

[WRF]: Oh yea, like, a looong time. But, you know, I got "intimate
relationships" with lots of data formats, if you know what I mean!
(winking)

[Audience]: (laughter)

[Jerry]: Hmmm, well I suppose that as a professional model, you must
be quite attractive to data formats. I'm sure lots of data formats are
interested in you. Is that correct?

[WRF]: Oh yea Jerry. Like you sez, I'm, you know, like, "attractive."
Oh yea! (flexes beefy arm muscles)

[Audience]: (booing from men, wooo-hooing from women)

[Jerry]: (to audience) Settle down now. Let's hear what WRF has to say
to netCDF. (to WRF) WRF, how do you feel about the fact that netCDF is
worried about younder data formats? Do you think she's right to be
worried?

[WRF]: Hey baby, what can I say? I'm a love machine! (gyrates flabby
hips)

[Audience]: (more booing, shouts of "sit down lard-ass!")

[WRF]: (appologetically, kneels in front of netCDF) But I'll always
love you baby! Just 'cause there are other formats in my life, like,
it doesn't mean that I can't love you, you know?

[Audience]: (sighing) Ahhhhhh...

[Jerry]: Now we have a suprise guest, flow all the way here from
Champagne-Urbana, Illinois, to appear as a guest on this show. Her
name is HDF5, and she's a professional data format.

[HDF5]: (flouning in, twirling a red handbag, and winking to WRF) Hi-ya, hon.

[Audience]: (booing, shouts of "dumb slut!")

[netCDF]: (screaming to HDF5) I can't believe you have the nerve to
come here, you hoe! (WRF jumps up in alarm and backs away)

[Jerry]: (to security) Steve, you better get between these two before
someone gets hurt...
~~~~

* * * * *

  ------------------------------------------------------------------------------------------------------------------------------ -------- --- ----------------------------------------------------------------------------- --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[\<](#show-title_003a_0060_0060I_0027ve-Beed-Dumped-for-a-Newer-Data-Format_0027_0027 "Previous section in reading order")]   [ \> ]       [[\<\<](#Derivitive-Works "Beginning of this chapter or previous chapter")]   [[Up](#Top "Up section")]   [ \>\> ]                   [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ------------------------------------------------------------------------------------------------------------------------------ -------- --- ----------------------------------------------------------------------------- --------------------------- ---------- --- --- --- --- ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

3. Concept Index 
================

* * * * *

  ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

Table of Contents
=================

-   [1. C Code](#C-Code)
    -   [1.1 libsrc directory](#libsrc-directory)
        -   [1.1.1 m4 Files](#m4-Files)
        -   [1.1.2 C Header Files](#C-Header-Files)
        -   [1.1.3 C Code Files](#C-Code-Files)
        -   [1.1.4 Makefile](#Makefile)
        -   [1.1.5 I/O Layering](#I_002fO-Layering)
        -   [1.1.6 Extra Tests](#Extra-Tests)

    -   [1.2 nc\_test directory](#nc_005ftest-directory)
    -   [1.3 nctest directory](#nctest-directory)
    -   [1.4 cxx directory](#cxx-directory)
    -   [1.5 man directory](#man-directory)
    -   [1.6 ncgen and ncgen4
        directories](#ncgen-and-ncgen4-directories)
    -   [1.7 ncdump directory](#ncdump-directory)
    -   [1.8 fortran directory](#fortran-directory)

-   [2. Derivitive Works](#Derivitive-Works)
    -   [2.1 From “A Tale of Two Data Formats,” the bestselling
        novel](#From-_0060_0060A-Tale-of-Two-Data-Formats_002c_0027_0027-the-bestselling-novel)
    -   [2.2 “The Marriage of NetCDF,” an Opera in Three
        Acts](#g_t_0060_0060The-Marriage-of-NetCDF_002c_0027_0027-an-Opera-in-Three-Acts)
    -   [2.3 Transcript from Jerry Springer Show, aired
        12/12/03,](#Transcript-from-Jerry-Springer-Show)
        -   [2.3.1 show title:“I’ve Beed Dumped for a Newer Data
            Format”](#show-title_003a_0060_0060I_0027ve-Beed-Dumped-for-a-Newer-Data-Format_0027_0027)

-   [3. Concept Index](#Concept-Index)

* * * * *

  ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------
  [[Top](#Top "Cover (top) of document")]   [[Contents](#SEC_Contents "Table of contents")]   [[Index](#Concept-Index "Index")]   [[?](#SEC_About "About (help)")]
  ----------------------------------------- ------------------------------------------------- ----------------------------------- ----------------------------------

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

