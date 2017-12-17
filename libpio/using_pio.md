# NetCDF Advanced HPC Features

This document describes the High Performance Computing (HPC) features
that are available to netCDF users, thanks to the integration of the
ParallelIO (PIO) library from NCAR.

# Building with PIO

PIO features only make sense in the context of a build on a
supercomputing system. There is never a need to build with PIO for a
single-processor system without a Message Passing Interface (MPI)
library.

PIO features are only available under the following conditions:
* NetCDF has been built with MPI-enabled compiler (i.e. mpicc).
* NetCDF-4 is enabled, and the underlying HDF5 library was built with --enable-parallel.
* --enable-pio is provided as a configure option.
* C90 features must be supported by the compiler.

In a future release, PIO will also be able to use the pnetcdf library,
if that is available at build time.

# The IO System

Using PIO starts with initializing an IO System. The IO System
describes the entire HPC environment. No PIO features can be used
without an defining an IO System.

Two different types of IO system are available. In the simple case (an
Intracomm IO System), a subset of processors can be used for I/O,
while all processors are used for computation. In the more advanced
case (the Async IO System), some processors are dedicated to I/O, and
others to one or more computation groups.

When the IO System is created, an iosysid is returned. The most
recently created IO System will be used by default, but the
nc_set_iosysid() function can also be used to change the current
iosysid.

To free the resources associated with an IO System, the
nc_free_iosystem() function should be called.

## Intracomm IO System

In an intracomm IO system, all processors are used for computation,
and a subset is used for I/O. For example, if 10000 processors are
available, the user may chose 100 to handle all I/O. NetCDF calls on
other (non-I/O) processors will result in data being sent to the I/O
processors with MPI. The I/O processors will then do the actual I/O.

Create an Intracomm IO System with nc_init_intracomm().

## Async IO System

With an async I/O system, some processors do exclusively I/O, and
others do exclusively computation. This I/O System supports
multi-level parallelism. Multiple computation components, each
consisting of many processors, can share the same group of I/O
processors.

For example an Earth system model, which included atmospheric, ocean,
space, and other models, could be run with each model as a computation
component, all sharing the same I/O component. Whenever the
computation components make netCDF calls, the parameters are sent via
MPI to the I/O component.

Create an async IO System with nc_init_async().

# Opening/Creating Files with PIO

Use the NC_PIO mode flag with nc_create()/nc_open() to engage PIO.

# Distributed Arrays

Distributed arrays allow users to distribute the contents of a netCDF
variable or record across many processors. The netCDF library will do
the book-keeping to merge the elements of the distributed array
together as needded for writing.

## Decompositions

To describe how an array is to be distributed, the user must create a
decomposition with nc_init_decomp(). A decomposition ID is returned,
which must be provided when doing distributed reads and writes. The
descomposition resources can be freed with nc_free_decomp().

## Writing a Distributed Array

The function nc_write_darray() is used to write one record of a
variable from a distributed array. Each computation task will call
nc_write_darray() with their local portion of the global array.













