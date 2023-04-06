Logging
=======
# Logging

\tableofcontents

# Introduction {#logging_intro}

The netCDF C/Fortran libraries offer a diagonstic logging capability
for advanced users. This logging capability works best with NC_NETCDF4
files. Logging must be enabled at build time in the C library.

# HDF5 Error Messages

HDF5 has an error message facility, usually turned off by the netCDF
library. In normal operation, we don't want error messages to be sent
to stdout by the library. Instead, all error communication occurs with
return codes. It's left up to the user application to decide if an
error message should be printed, or what other action should occur.

But when logging is turned on to any level, the HDF5 error logging
will be turned on. Not all HDF5 error messages are a problem, it
depends on how the calling application handles errors. But HDF5 error
messages can be useful in debugging, so they are turned on if logging
is turned on.

# Building with Logging {#logging_build}

Logging is turned off by default in netcdf-c builds. When the library
is built with logging, and when logging is turned on, a significant
performance penalty is paid. This is expected, as logging causes I/O
to stdout, and this is slow.

Logging should be used for debugging difficult issues. Production code
should run on an optimized build of netCDF, which does not have
logging enabled.

## Building netcdf-c with Logging using the Autotools Build

To build netcdf-c with logging using autotools, add the
--enable-logging option to configure:

`CPPFLAGS=-I/usr/local/hdf5-1.14.0/include LDFLAGS=-L/usr/local/hdf5-1.14.0/lib ./configure --enable-logging`


## Building netcdf-c with Logging using the CMake Build

To build netcdf-c with logging using CMake, set the ENABLE_LOGGING option to ON:

`cmake -DENABLE_LOGGING=ON -DCMAKE_PREFIX_PATH=/usr/local/hdf5-1.14.0 ..`

## Checking that Logging was Enabled

After configure or CMake is run, and configure summary is printed to
stdout, and the include file netcdf_meta.h is created in the include
directory.

In the configure summary, you will see this line, indicating that
logging has been turned on:

`Logging:     		yes`

In the include/netcdf-meta.h file:

`#define NC_HAS_LOGGING       1 /*!< Logging support. */`

## Building netcdf-fortran with Logging

The netcdf-fortran build will detect whether the netcdf-c build
includes logging, and will automatically enable logging in
netcdf-fortran if it has been enabled for netcdf-c.

As with the netcdf-c build, a configuration summary is printed to
stdout after the configure/CMake step in the build process. If the
netcdf-fortran build found that the netcdf-c build enabled logging,
the following line will appear in the netcdf-fortran configuration
summary:

`Logging Support:                yes`

# Implementation

In netcdf-c, the function nc_set_log_level() is used to turn logging
on and off. In netcdf-fortran, the equivalent nf_set_log_level() is
provided.

If the netcdf-c build does not enable logging, then calls to
nc_set_log_level() and nf_set_log_level() do nothing.

The nc_set_log_level() function is defined in libsrc4/nc4internal.c,
and has the following documentation and signature:

```
/**
 * Use this to set the global log level. 
 * 
 * Set it to NC_TURN_OFF_LOGGING (-1) to turn off all logging. Set it
 * to 0 to show only errors, and to higher numbers to show more and
 * more logging details. If logging is not enabled when building
 * netCDF, this function will do nothing.
 *
 * @param new_level The new logging level.
 *
 * @return ::NC_NOERR No error.
 * @author Ed Hartnett
 */
int
nc_set_log_level(int new_level)
```

# Turing Logging On/Off {#logging_use}

Turn logging on in your code, just before the netCDF code that is
under investigation. After that code runs, turn off logging by setting
the log level to -1.

## Setting Log Level in C Programs

Call the nc_set_log_level() to set the log level.  For example, in the
test program nc_test4/tst_interops5.c, we have:

```
       /* Open the file with netCDF. */
       nc_set_log_level(3);
       if (nc_open(filename, NC_WRITE, &ncid) != NC_ECANTWRITE) ERR;
```

This turns the log level to 3 just before the call to nc_open(). The
output of this is (lines that start with '***' are regurlar test
output. All other output is produced by the logger):

```
./tst_interops5 

*** Testing HDF5/NetCDF-4 interoperability...
*** testing HDF5 compatibility...ok.
	log_level changed to 3
	NC4_open: path tst_interops5.h5 mode 4097 params 0
	HDF5 error messages turned on.
			nc4_open_file: path tst_interops5.h5 mode 4097
			nc4_grp_list_add: name / 
			rec_read_metadata: grp->hdr.name /
ERROR: file hdf5open.c, line 2891.
NetCDF: Can't write file
ERROR: file hdf5open.c, line 953.
NetCDF: Can't write file
			nc4_close_hdf5_file: h5->path tst_interops5.h5 abort 1
			nc4_rec_grp_HDF5_del: grp->name /
			nc4_close_netcdf4_file: h5->path tst_interops5.h5 abort 1
			nc4_rec_grp_del_att_data: grp->name /
			nc4_rec_grp_del: grp->name /
*** testing error when opening HDF5 file without creating ordering...ok.
*** Tests successful!
```

## Setting Log Level in F77 Programs

Use the nf_set_log_level() function to change the logging level in
Fortran. This function is defined in netcdf.inc:

```
!     This is to turn on netCDF internal logging.
      integer nf_set_log_level
      external nf_set_log_level
```

For example, in the netcdf-fortran code we have
nf_test4/ftst_var_compact.F, which contains this code:

```
      retval = nf_set_log_level(3)

C     Create the netCDF file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) stop 1

C     Create a dimension.
      retval = nf_def_dim(ncid, dim_name, DIM_LEN, dimids(1))
      if (retval .ne. nf_noerr) stop 1

      retval = nf_set_log_level(-1)
```

The output of this test is:

```
./ftst_var_compact 
 
 *** Testing compact vars.
	log_level changed to 3
	HDF5 error messages have been turned off.
	NC4_create: path ftst_var_compact.nc cmode 0x1000 parameters (nil)
	HDF5 error messages turned on.
			nc4_create_file: path ftst_var_compact.nc mode 0x1000
			nc4_grp_list_add: name / 
		HDF5_def_dim: ncid 0x10000 name dim1 len 22
```                

## Setting Log Level in F90 Programs

Use nf_set_log_level() in F90 programs to change the log level. For
F90, the function needs to be defined as integer type.

For example, this F90 code turns on logging for some of the metadata
definition code:

```
  integer :: nf_set_log_level, ierr

  ierr = nf_set_log_level(3)
  ! Create the netCDF file. 
  call handle_err(nf90_create(FILE_NAME, nf90_netcdf4, ncid))

  ! Define the dimensions.
  call handle_err(nf90_def_dim(ncid, "x", NF90_UNLIMITED, x_dimid))
  call handle_err(nf90_def_dim(ncid, "y", NY, y_dimid))
  dimids =  (/ y_dimid, x_dimid /)

  ! Define the variables. 
  do v = 1, NUM_VARS
     call handle_err(nf90_def_var(ncid, var_name(v), var_type(v), dimids, varid(v)))
  end do
  ierr = nf_set_log_level(-1)
```

This produces the output:

```
 *** Testing netCDF-4 fill values with unlimited dimension.
	log_level changed to 3
	HDF5 error messages have been turned off.
	NC4_create: path f90tst_fill2.nc cmode 0x1000 parameters (nil)
	HDF5 error messages turned on.
			nc4_create_file: path f90tst_fill2.nc mode 0x1000
			nc4_grp_list_add: name / 
		HDF5_def_dim: ncid 0x10000 name x len 0
		HDF5_def_dim: ncid 0x10000 name y len 16
		NC4_def_var: name byte type 1 ndims 2
		nc_inq_atomic_type: typeid 1
		NC4_def_var: name short type 3 ndims 2
		nc_inq_atomic_type: typeid 3
		NC4_def_var: name int type 4 ndims 2
		nc_inq_atomic_type: typeid 4
		NC4_def_var: name float type 5 ndims 2
		nc_inq_atomic_type: typeid 5
		NC4_def_var: name double type 6 ndims 2
		nc_inq_atomic_type: typeid 6
		NC4_def_var: name ubyte type 7 ndims 2
		nc_inq_atomic_type: typeid 7
		NC4_def_var: name ushort type 8 ndims 2
		nc_inq_atomic_type: typeid 8
		NC4_def_var: name uint type 9 ndims 2
		nc_inq_atomic_type: typeid 9
 *** SUCCESS!
```

## Log Levels

The nc_set_log_level() function accepts one parameter, the new log
level. Set the log level to turn logging on and off, and control the
verbosity of the logging. The log level can be set to:
* -1 to turn off all logging (there's a predefined constant in the
netcdf.h include file ::NC_TURN_OFF_LOGGING).
* 0 will log only internal errors.
* 1-5 will log internal operations with increasing verbosity. Setting
higher log levels is the same as log level 5.

Setting the log level to 1 will show netCDF interal function calls and
their parameters. Setting log level to 2 or 3 will show important
internal varaibles. Setting the log level to 4 or 5 will output a
large amount of text.

# Logging with Parallel I/O {#logging_parallel}

Logging with parallel I/O presents challenges. As different processors
run their code, they encounter logging commands and send output to
stdout. Since there is only one stdout and multiple processors, the
results get jumbled together and the result is unreadable.

To surmount this problem, in parallel I/O runs, logging generates a
file for each processor, with the logging for that processor. The
logging output is not sent to stdout.

The logging files are named nc4_log_N.log, where N is the number of
the processor.

For example, in nc_test4/tst_parallel3.c we have:

```
   /* Create a parallel netcdf-4 file. */
   nc_set_log_level(4);
   if (nc_create_par(file_name, facc_type, comm, info, &ncid)) ERR;
   nc_set_log_level(NC_TURN_OFF_LOGGING);
```

When run with 4 processors, this causes 4 files to be created,
nc4_log_0.log, nc4_log_1.log, nc4_log_2.log, and nc4_log_3.log.

Here's the contents of nc4_log_0.log:

```
	log_level changed to 4
	NC4_create: path ./tst_parallel3.nc cmode 0x1000 parameters 0x7ffd7a021ea0
	HDF5 error messages turned on.
			nc4_create_file: path ./tst_parallel3.nc mode 0x1000
			nc4_grp_list_add: name / 
				creating parallel file with MPI/IO
```                                

Note that that with parallel I/O logging, users must call
nc_set_log_level(NC_TURN_OFF_LOGGING). Without the call to turn off
logging, the logging output to the files is not flushed from buffer
and may not be complete. Explicitly turning off the logging causes the
library to fluch buffers and write the log files to disk.

# Caution

The logging facilities of netCDF are for advanced users and
developers, and do not offer a very user-friendly view of library
operations. However, sometimes a detailed view of internal variables
is helpful when debugging problems, especially in complex parallel I/O
situations.

Logging output is not considered part of the public API of netCDF, and
may change at any time without notice. Do not write code which depends
on particular logging output from the netCDF libary.