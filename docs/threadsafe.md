Thread-Safe Support in NetCDF-C
============================
<!-- double header is needed to workaround doxygen bug -->

# Thread-Safe Support in NetCDF-C

\tableofcontents

# Introduction {#threadsafe_introduction}

To date, the netcdf-c library has not supported
thread-safe use. However, this is one of the most
requested extensions to the library.

As of version 4.x.x support for thread-safe access is now
provided.  The support is **HIGHLY EXPERIMENTAL** since it has
been tested to only a very limited extent.

# Implementation {#threadsafe_impl}

The implementation is patterned after the HDF5 thread-safety support.
For those platforms that support it, *pthreads* is used,
and specifically the *pthread_mutex* functionality.
Note that for Windows, the *pthreads4w* <a href="#ref_voll">[2]</a>
implementation is used.

There is a single global mutex object that controls access to
the netcdf-c API functions. This means that all netcdf-c
operations are effectively serialized. As with HDF5, which also uses
a global mutex, this limits performance in a parallel environment.

The body of each netcdf-c API function in *netcdf.h* is
wrapped in calls to lock the global mutex on entrance and then
unlock it on exit. This required some modest code modifications
to ensure that unlocking always occurs at exit. These changes are
generally simple, but there are many places to change.

A critical feature of the netcdf thread-safe implementation is
that it uses recursive mutex objects.
Suppose a wrapped API function internally calls another wrapped API
function. In the simplest case, this would lead to a deadlock
since the outer function would have locked the mutex and
locking the second call would block waiting for the mutext to be released,
which would never happen. Note that this kind of nested/recursive
call actually occurs frequently in the netcdf-c code.

When a recursive mutex attempts to lock an already locked
mutex, it examines the owner of the lock and if it is the same
thread as the current one -- the one attempting the lock --
then it just bumps a reference count and continues on.
Similarly for unlocking. The reference count is decremented
and if it is zero then the mutex is actually released.

# Known Issues

* This implementation only supports *libpthread* (including *pthreads4w*).
* Filters are implicitly locked because the nc_get/put_varX functions are locked.
* It is unknown how this interacts with MPI.
* There are some difficult memory leaks.

# Initialization and Finalization {#threadsafe_init}

The global mutex is initialized when the *nc_initialize* API
function is invoked. Usually explicit calls to this function
are unnecessary since it is invoked implicitly when calling
e.g. *nc_open* or *nc_create*.

The situation with finalization is a bit more complex.  The
global mutex is finalized when the *nc_finalize* API function is
invoked. However, the user needs to call this function
explicitly because there is no way for the library to know when
the user is finished with the library. But fortunately calling
*nc_finalize* is usually not necessary. 

# Lock and Unlock API {#threadsafe_lockapi}

When adding a new netcdf-c API function, you will
need to wrap the body of the function int a lock
and unlock pair.

There are two relevant functions.
````
void NC_lock(void);
void NC_unlock(void);
````
These functions are wrapped in macros to allow adding
additional debug information.
````
#define NCLOCK NC_lock()
#define NCUNLOCK NC_unlock()
````
# API Example {#threadsafe_apiexample}

When adding new netcdf-c API functions, you will need to follow
this template example taken from *nc_sync*.
````
int nc_sync(int ncid)
{
    NC* ncp;
    int stat = NC_NOERR;
    NCLOCK;
    stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) goto done;
    stat = ncp->dispatch->sync(ncid);
done:
    NCUNLOCK;
    return stat;
}
````
Note that the code needs to be set up so that NCLOCK is the evaluated
before any other function. Similarly NCUNLOCK needs to be the
last significant action before returning from a function.
A common approach is to define a single exit via the label *done*.
This means that the body of the function never does a *return*,
but rather sets the status and does a *goto done*. 
For those familiar with Java, this code template is more-or-less
equivalent to using the ````Try {lock} ... Finally {Unlock}````
construct.

Some optimizations are possible. If there are preliminary tests that
only operate on the arguments to the function, then those tests
can be performed before *NCLOCK* is invoked. This can avoid
some of the locking overhead.

# Extensions {#threadsafe_extend}

* Ideally, it would be better to not use recursive mutex objects
as it has some performance costs. But fixing this would require
a significant refactoring of the netcdf-c library code. So this
change may be fixed, but only piece-meal.

* Eventually, it should be possible to convert
total API serialization into a finer grain of
locking so that a mutex is locked only when accessing
a shared object such as the open files vector.
This should significantly improve parallel performance.
Unfortunately, implementing this level of locking
is quite complex, so it is unlikely to be realized
any time soon.

* In support of fine grain locking, it may be useful to,
in some cases, move locking down into the dispatcher code.
This means, for example, that the HDF5 dispatcher and the NCZarr
dispatcher might have their on locking regime, while other dispatchers
continue to use the global locking regime.

# Testing

Testing of this feature is currently very limited.
Currently there are two tests in the nc_test directory.

## run_threads.sh + tst_threads.c
The shell script invokes tst_threads.c with some different options.
Usage of tst_threads is as follows:
````
tst_threads [-h][-3|-4][-F <filenameformatstring>][-C <ncycles>][-T <nthreads>]"
````
where 
* -h &mdash; print usage.
* -3 &mdash; create a netcdf-3 (classic) file.
* -4 &mdash; create a netcdf-4 (enhanced) file.
* -F &mdash; use this template to specify the name of the created file.
* -C &mdash; execute the body of the test ncycles times.
* -T &mdash; execute the body of the test using n threads.

One cycle creates a file, manipulates it, and then closes the file.

## run_threads_nc_test.sh + threads_nc_test.c
The shell script invokes threads_nc_test.c with some different options.
Usage of nc_test_threads is as follows:
````
threads_nc_test [-h] [-v] [-M <max-messages>] [-T <nthreads>]\n");
````
where 
* -h &mdash; print usage.
* -v &mdash; provide verbose output (default off).
* -M &mdash; provide at most max-messages for each test.
* -T &mdash; execute the body of the test using n threads.

This test performs the same set of tests as the nc_test program,
but using multiple threads.

# Debug Aids {#threadsafe_debug}

Debugging multi-threaded applications are notoriously difficult.
Fortunately, the GNU debugger (*gdb*) provides pretty good thread
support capabilities.

# Configuration {#threadsafe_config}

Enabling thread-safety is controlled at build time
using options to *configure* (Automake) and
*cmake* (Cmake). These options are disabled by default.

The options are as follows.
* [Automake] ````--enable-threadsafe````
* [CMake] ````-DENABLE_THREADSAFE=on````

## Platform Support for libpthread
This implementation is based on the pthread library.
The situation for various platforms is as follows.
* For \*nix\* platforms (including MINGW), pthread support is usually built-in.
* For OS/X platforms, pthread support is usually built-in. However, the *pthread_barrier_t* type and associated operations are not implemented, so a built-in implementation is added to libdispatch/dthreaded.c and the API is defined in include/netcdf_threadsafe.h.
* For Windows (using Visual Studio), it is necessary to use an external implementation of libpthread. See Appendix A for details.

## Build Status

The current status of thread-safe operation is as follows.
<table>
<tr><td><u>Operating System</u><td><u>Build System</u><td><u>Local Build</u><td><u>Github Actions</u>
<tr><td>Linux         <td> Automake <td> yes		<td>yes
<tr><td>Linux         <td> CMake    <td> yes		<td>yes
<tr><td>Visual Studio <td> CMake    <td> yes            <td>N.A.
<tr><td>OS/X           <td> Automake <td> unknown        <td>yes
<tr><td>OS/X           <td> CMake    <td> unknown        <td>yes
<tr><td>MinGW/MSYS2   <td> Automake <td> unknown        <td>yes
<tr><td>Cygwin        <td> Automake <td> unknown	<td>N.A.
</table>

# References {#threadsafe_bib}
<a name="ref_fwb">[1]</a> [Github/FWBuilder](https://github.com/fwbuilder/pthreads4w.git)<br>
<a name="ref_voll">[2]</a> [Github/Vollstrecker](https://github.com/Vollstrecker/pthreads4w.git)

# Appendix A. Providing libpthread Support for Windows Visual Studio

The capability for thread safe operation is based soley on using
*libpthread*, even for Windows using Visual Studio.
Microsoft does not support (yet) the *pthread* API for Visual Studio,
so in order to enable thread safe operation on Windows, it is necessary
to install a libpthread implementation.

At the moment the only port supported by Unidata is derived from
[PTHREADS4W](https://sourceforge.net/projects/pthreads4w/),
version 3.0.0 (2017-01-01) or later.
The derived port used here is the Vollstrecker Github repository
<a href="#ref_voll">[2]</a>.
This is in preference to the other major (and related) 
FWBuilder implementation <a href="#ref_voll">[1]</a>.

The Vollstrecker port is used because it provides a cmake build,
which allows for building for Visual Studio. In addition, it
appears to still be under active development. Both code bases
are released using the Apache V2 license.

The script used to build and install this port is below.
It is probably best to set INSTALLDIR to an absolute path
that contains no blanks.
In order to get the netcdf-c library cmake build to recognize
the pthread library, one needs to add the follow path(s)
to the *PATH* environment variable.
* *${INSTALLDIR}/lib*
* *${INSTALLDIR}/bin*

## Cmake Build Script
````
# Visual Studio
CFG="Release"
INSTALLDIR="c:/tools/hdf5-1.10.6"
FLAGS=
FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=${INSTALLDIR}"
FLAGS="$FLAGS -DCMAKE_INSTALL_LIBDIR=lib"
FLAGS="$FLAGS -DCMAKE_MODULE_PATH=${INSTALLDIR}/cmake"
rm -fr build
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=${CFG} $FLAGS ..
cmake --build . --config ${CFG}
cmake --build . --config ${CFG} --target RUN_TESTS
cmake --install . --config ${CFG}
cd ..
````

# Point of Contact
__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 9/9/2022<br>
__Last Revised__: 11/4/2022<br>
