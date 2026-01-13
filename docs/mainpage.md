# Introduction and Overview {#introduction-and-overview-mainpage}


## What is NetCDF? {#what_is_netcdf}

NetCDF is a set of software libraries and self-describing, machine-independent data formats that support the creation, access, and sharing of array-oriented scientific data. NetCDF was developed and is maintained at [Unidata](https://www.unidata.ucar.edu). Unidata provides data and software tools for use in geoscience education and research. Unidata is part of the University Corporation for Atmospheric Research ([UCAR](http://www2.ucar.edu)) Community Programs ([UCP](http://www.ucp.ucar.edu)). Unidata is funded primarily by the National Science Foundation.

The NetCDF homepage may be found at [https://www.unidata.ucar.edu/software/netcdf/](https://www.unidata.ucar.edu/software/netcdf/). The NetCDF source code is hosted at [GitHub](http://github.com), and may be found directly at [http://github.com/Unidata/netcdf-c](http://github.com/Unidata/netcdf-c).

## Looking for NetCDF-Fortran Documentation? 

You can find the documentation for netCDF-Fortran here:

- [The NetCDF-Fortran Developer's Guide](https://docs.unidata.ucar.edu/netcdf-fortran/current/)

## Learn more about the current NetCDF-C Release

- @subpage RELEASE_NOTES
- [Getting and Building NetCDF](https://docs.unidata.ucar.edu/nug/current/getting_and_building_netcdf.html)

## Learn more about using NetCDF-C

- @ref sec_tut
- [The netCDF User's Guide](https://docs.unidata.ucar.edu/nug/)
- [Working with NetCDF Files](#netcdf_working_with_netcdf_files)

## NetCDF CMake and Windows support

- [NetCDF-CMake](#netCDF-CMake)
- [Windows Binaries](#winbin)

## About NetCDF

**Authors:** Russ Rew, Glenn Davis, Steve Emmerson, Harvey Davies, Ed Hartnett, Dennis Heimbigner, and Ward Fisher

NetCDF was developed and is maintained at [Unidata](https://www.unidata.ucar.edu), part of the [University Corporation for Atmospheric Research](http://www2.ucar.edu) (UCAR) [Community Programs](http://www.ucp.ucar.edu) (UCP). Unidata is funded primarily by the National Science Foundation.

A complete list of credits and contributors to NetCDF may be found at the @ref credits page.

[Unidata](https://www.unidata.ucar.edu) provides data and software tools for use in geoscience education and research.

---

## Internal Documentation

The dispatch interface within the netCDF library allows the netCDF API to be used for different types of back-ends, for example, the HDF5 library, or a DAP client accessing remote data.

