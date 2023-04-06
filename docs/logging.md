Logging
=======
# Logging

\tableofcontents

# Introduction {#logging_intro}

The netCDF C/Fortran libraries offer a diagonstic logging capability
for advanced users. This logging capability works best with NC_NETCDF4
files. Logging must be enabled at build time in the C library.

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

# Turing Logging On/Off {#logging_use}

## Log Levels

# Logging with Parallel I/O {#logging_parallel}