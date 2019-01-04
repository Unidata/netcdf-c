NetCDF ZARR Architecture
====================================

# Abstract

*This document describes the internal architecture of the
implementation of NCZarr support in the netcdf-c library.*
*Distribution of this document is currently restricted to Unidata.*

[TOC]

# Introduction {#nczarch_intro}

This document provides an overview of how NCZarr support
is provided within the netcdf-c library.

The netcdf-c library uses a dispatch table mechanism to provide
format-specific implementations of the standard netCDF-4 interface
functions defined in *include/netcdf.h*.

The general processing flow for reading a dataset is as follows.

1. The *nc_open()* function is invoked with three primary pieces
   of information: (1) a path specifying the dataset, (2) a set of
   *mode* flags specifying properties to be applied to reading
   the dataset, and (3) the actual dataset itself.
2. Using the path and the mode flags, the netcdf-c library infers the
   format of the dataset.
3. Using the inferred format, a dispatch table object is chosen
   where the methods of the dispatch table can be used to read
   a dataset of the inferred format.
4. Subsequent netcdf API calls then use the table to choose the
   proper implementation of that API with respect to the format.

The operation of *nc_create()* is similar except that there is no
pre-existing dataset to use in inferring the dataset format.

Following this model, reading or writing an NCZarr dataset
requires a path that is in a specific format, namely a URL,
plus some mode specific format-specifying flags. Inference rules
are added to the netcdf-c library (specifically *libdispatch/dfile.c*)
to decide that a specific path plus mode flags should be interpreted
as a reference to a dataset in NCZarr (or in some cases, pure Zarr)
format.

As with all other dispatch tables, the netcdf-c source code
will have a directory called, for example, libnczarr. This directory
will contain the dispatch table for NCZarr plus the code
to implement the methods of the dispatch table.

Internally, the libnczarr code achitecture is driven by the dispatch
table plus the functions implementing the methods of the dispatch table.
Since the goal here is to provide an implementation of netCDF-4,
it is useful to use the libsrc4 code base that provides an
inmemory representation of a netcdf-4 dataset. This approach
is used for the HDF5 dispatch table and the DAP4 dispatch table.
In each case, the internal netcdf-4 representation is annotated
with information specific to HDF5 or DAP4 respectively.

