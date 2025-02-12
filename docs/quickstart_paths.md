Appendix F. Specifying Paths for NetCDF-C QuickStart {#nc_paths_quickstart}
==============================

[TOC]

A key concept in netcdf-c is the notion of a "path".
A path specifies some dataset that is of interest to a user.
It is the primary argument to the *nc_open* and *nc_create*
functions in the NetCDF-C API, as defined by the file netcdf.h.
It is also the primary option for the NetCDF-C utilities:
*ncdump*, *nccopy*, and *ncgen*.
Hence understanding what kind of paths are acceptable is important
for using the NetCDF-C library.

## Classification of Paths {#nc_paths_kinds}

Basically, there are two kinds of paths:
1. <a href="#qpaths_filesystem">File system paths</a>, and
2. <a href="#qpaths_url">Uniform Resource Locator (URL) paths</a>.

### File System Paths {#qpaths_filesystem}

The most common form of path accepted by the NetCDF-C library is a file system path.
Every user of some computer operating system is familiar with the idea of a file system path.

Each operating system has some special quirks when specifying file system paths.
Here are some example paths for various version of Linux/Unix.
* /
* /a/b/c/d

As a rule, Linux/Unix has a single root path typically indicated by "/",
and "/" is also used as the file separator.

For Windows, some example paths would be as follows:
* c:\\
* d:\\a\\b\\c

Windows has a notion of a drive ("d:") and each drive serves as the root
of its own file system. Windows uses "\\" as its file separator, although
many programs also accept "/".

## Uniform Resource Locator (URL) Paths {#qpaths_url}

The NetCDF-C library can access datasets that reside on remote computers,
Hence NetCDF-C now also accepts URLs to specify those remote datasets.

The general form of a URL is as follows:
````
<protocol>://<user>:<pwd>@<host>:<port>/<path>?<query>#<fragment>
````
* \<protocol\> - specifies the format of the messages between the local and remote computers. Some examples used by NetCDF-C are *http*, *https*, *s3*, *dap*, *dap4*, or *file*.
* \<user:pwd\> - Pass authorization credentials to the remote computer.
* \<host\>:\<port\> - The specific remote computer to access, such as *thredds.ucar.edu:8080*.
* \<path\> - A specification of file-path-like string that indicates some specific resource on the remote computer.
* \<query\> - A sequence of (key=value) pairs, separated by "&", and providing special information to the remote computer.
* \<fragment\> - A sequence of (key=value) pairs, separated by "&", and providing special information to the local computer.

Most parts of a URL are optional. As a rule, the protocol is always required,
and either the host is required or, if the protocol is "file", then the path is required.
The query and fragment are optional.

### Examples of URL Paths for NetCDF-C
* https://thredds.ucar.edu/catalog

## Addendum A. Known Fragment Keys {#nc_paths_frag_keys}

The fragment part of a URL is used to pass information deep into
the netcdf-c library to control its actions.
This appendix list known keys, although it may be somewhat out-of-date.

The current set of keys used in the netcdf-c library is as follows.
* _mode_ -- A special key that is used to provide single values for controlling the netcdf-c library. It consists of a comma separated sequence of values
primarily used to control the file format.
The *mode* key currently supports the following values
    - _dap2_ -- Specifies that the URL accesses a resource using the DAP2 protocol (default if no mode is specified)
    - _dap4_ -- Specifies that the URL accesses a resource using the DAP4 protocol
    - _netcdf-3_ -- Specifies that a file is a netcdf-classic file
    - _classic_ -- Alias for _netcdf-3_
    - _netcdf-4_ -- Specifies that a file is a netcdf-enhanced file
    - _enhanced_ -- Alias for _netcdf-4_
    - _udf0_ -- Specifies that the file format is defined by a User Defined format
    - _udf1_ -- Specifies that the file format is defined by a User Defined format
    - _nczarr_ -- Specifies that the file is in NCZarr format
    - _zarr_ -- Specifies that the file is in Zarr format
    - _xarray_ --Specifies that the file is in Zarr format and uses the XARRAY convention
    - _noxarray_ --Specifies that the file is in Zarr format and does not use the XARRAY convention
    - _s3_ --Specifies that the file is remote and is stored on the Amazon S3 cloud
    - _file_ --Specifies that the file is an NCZarr/Zarr file stored as a file tree
    - _zip_ --Specifies that the file is an NCZarr/Zarr file stored as a zip file
    - _bytes_ -- Specifies that the file is remote and is to be read using byte-range support
    - _zoh_ --Specifies that the file is remote and supports the [GWDG ZoH](https://pad.gwdg.de/DtHGRP38Sw2YQDAAjPuP2Q) (Zarr-over-HTTP) protocol
 in NCZarr format
* _dap2_ -- equivalent to "mode=dap2"
* _dap4_ -- equivalent to "mode=dap4"
* _bytes_ -- equivalent to "mode=bytes"
* _log_ -- turn on logging for the duration of the data request
* _show=fetch_ -- log curl fetch commands

## Addendum B. Amazon S3 Specific URLS {#nc_paths_s3_urls}
A URL path is required for accessing datasets on the Amazon S3 storage cloud.
Unfortunately S3 URLs are complicated.
They can have the following forms:
* _Virtual_: the protocol is "http:" or "https:", the mode specifies "s3", and the host starts with the bucket name; e.g. __bucket.s3.&lt;region&gt;.amazonaws.com__  or  __bucket.s3.amazonaws.com__
* _Path_: the protocol is "http:" or "https:", the mode specifies "s3", and the host does not include the bucket name, but rather the bucket name is the first segment of the path. For example __s3.&lt;region&gt;.amazonaws.com/bucket__ or __s3.amazonaws.com/bucket__
* _Protocol_: the protocol is "s3:" and if the host is a single name, then it is interpreted as the bucket. The region is determined using an algorithm defined in the nczarr documentation.

For all of the above URL forms, there are two additional pieces.
* Query: currently not used.
* Fragment: the fragment is of the form _key=value&key=value&..._. Depending on the key, the _value_ part may be left out and some default value will be used. The exact set of possible keys is defined in the nczarr documentation.

## Addendum C. Google Storage Specific URLS {#nc_paths_google_urls}
Google provides an interface to its storage that is compatible with the Amazon S3 REST API.
A URL path is required for accessing datasets on the Google storage cloud.
Note that the Google host is always "storage.googleapis.com" and has no concept of region.
It has the following forms.
* _Path_: the bucket name is the first segment of the path.
For example __storage.googleapis.com/bucket__.
* _Protocol_: the protocol is "gs3:" and if the host is a single name, then it is interpreted as the bucket. The _gs3_ scheme is equivalent to "https" plus setting various tags in the query and/or fragment part of the URL.
For example __gs3://bucket/__.

For all of the above URL forms, there are two additional pieces.
* Query: currently not used.
* Fragment: the fragment is of the form _key=value&key=value&..._. Depending on the key, the _value_ part may be left out and some default value will be used. The exact set of possible keys is defined in the nczarr documentation.

## Addendum D. Zarr-Over-HTTP (ZoH) Protocol Specific URLS {#nc_paths_zoh_urls}
The [GWDG ZoH](https://pad.gwdg.de/DtHGRP38Sw2YQDAAjPuP2Q) (Zarr-over-HTTP) protocol provides an interface to any server supporting the ZoH REST API.
The URLs for this API are very similar to the S3 or Google URLs.
Note the _virtual_ URL format is not currently supported.
A ZoH URL has one of the following forms.
* _Path_: the protocol is "http:" or "https:", the host is any standard host (including an optional port number), and the bucket name is the first segment of the path.
For example __http://zoh.gwdg.de/&lt;bucket&gt;/&lt;key&gt;__.
* _Protocol_: the protocol is "zoh:" and a complete host must be specified.
The URL path part is the key to be interpreted by the ZoH server
as it wishes.

For all of the above URL forms, there are two additional pieces.
* Query: currently not used.
* Fragment: the fragment is of the form _key=value&key=value&..._. Depending on the key, the _value_ part may be left out and some default value will be used. The exact set of possible keys is defined in the nczarr documentation.

## Point of Contact {#nc_paths_poc}

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 4/10/2020<br>
__Last Revised__: 1/16/2025

