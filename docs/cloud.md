
Cloud Storage Access Using The NetCDF-C Library 
============================
<!-- double header is needed to workaround doxygen bug -->

# Cloud Storage Access Using The NetCDF-C Library {#nccloud_head}

\tableofcontents

# Introduction {#nccloud_introduction}

The NetCDF-C library supports limited access to cloud storage.
Currently, that access is restricted to the Amazon S3 cloud storage,
so this document is S3-centric.
It is expected that over time, access to additional cloud stores will be added,
and this document will be expanded to cover those additional cases.

# S3 Use-Cases in NetCDF-C

At the moment, the NetCDF-C library provides access to S3 for the following purposes:
* Byte-Range access to Netcdf-4/HDF5 datasets stored as single object in S3.
* Zarr dataset access as represented as a "tree" of objects in S3.

# Amazon S3 Storage Access Mechanisms {#nccloud_s3_sdks}

Three S3 storage drivers are available for accessing Amazon S3.
1. *H5FDros3* -- This is an HDF5 Virtual File Driver provided as part of the HDF5 library. It is specifically used by NetCDF-C to perform byte-range access to netcdf-4 files. In order for this functionality to be available to NetCDF-C,
it is necessary to build the HDF5 library with the *--enable-ros3-vfd* option.
2. *aws-sdk-cpp* -- This is the standard Amazon AWS S3 SDK. It is written in C++. It is used specifically for NCZarr access to Zarr formatted datasets on S3.
3. *nch5s3comms* -- This is an experimental SDK, written in C, that provides the minimum functionality necessary to access S3. As with *aws-sdk-cpp*, it is used specifically for NCZarr access to Zarr formatted datasets on S3.

All three S3 drivers use the AWS profile mechanism to provide configuration information, and especially to provide authorization information.
Specifically, the ''~/.aws/credentials'' file should contain something like this.

```
[default]
output = json
aws_access_key_id=XXXX...
aws_secret_access_key=YYYY...
```

## Byte-Range Access {#nccloud_byterange}

The NetCDF-C library contains a mechanism for accessing traditional netcdf-4 files stored on remote computers.
The idea is to treat the remote data as if it was one big file,
and to use the HTTP protocol to read a contiguous sequence of bytes
from that remote "file".
This is performed using the "byte-range" header in an HTTP request.

In the Amazon S3 context, a copy of a dataset, a netcdf-3 or netdf-4 file, is uploaded into a single object in some bucket.
Then using the key to this object, it is possible to tell the netcdf-c library to treat the object as a remote file and to use the HTTP Byte-Range protocol to access the contents of the object.
The dataset object is referenced using a URL with the trailing fragment containing the string ````#mode=bytes````.

An examination of the test program _nc_test/test_byterange.sh_ shows simple examples using the _ncdump_ program.
One such test is specified as follows:
````
https://s3.us-east-1.amazonaws.com/noaa-goes16/ABI-L1b-RadC/2017/059/03/OR_ABI-L1b-RadC-M3C13_G16_s20170590337505_e20170590340289_c20170590340316.nc#mode=bytes
````
Note that for S3 access, it is expected that the URL is in what is called "path" format where the bucket, _noaa-goes16_ in this case, is part of the URL path instead of the host.

The _#mode=bytes_ mechanism generalizes to work with most servers that support byte-range access.
 
Specifically, Thredds servers support such access using the HttpServer access method as can be seen from this URL taken from the above test program.
````
https://thredds-test.unidata.ucar.edu/thredds/fileServer/irma/metar/files/METAR_20170910_0000.nc#bytes
````

# References {#nccloud_bib}

<a name="ref_aws">[1]</a> [Amazon Simple Storage Service Documentation](https://docs.aws.amazon.com/s3/index.html)<br>
<a name="ref_awssdk">[2]</a> [Amazon Simple Storage Service Library](https://github.com/aws/aws-sdk-cpp)<br>
<a name="ref_awssdk_conda">[11]</a> [Conda-forge / packages / aws-sdk-cpp](https://anaconda.org/conda-forge/aws-sdk-cpp)<br>

# Appendix A. S3 Build Support {#nccloud_s3build}

Currently the following build cases are known to work.

<table>
<tr><td><u>Operating System</u><td><u>Build System</u><td><u>SDK</u><td><u>S3 Support</u>
<tr><td>Linux		<td>Automake	<td>aws-s3-sdk	<td>yes
<tr><td>Linux		<td>Automake	<td>nch5s3comms	<td>yes
<tr><td>Linux		<td>CMake	<td>aws-s3-sdk	<td>yes
<tr><td>Linux		<td>CMake	<td>nch5s3comms	<td>yes
<tr><td>OSX		<td>Automake	<td>aws-s3-sdk	<td>unknown
<tr><td>OSX		<td>Automake	<td>nch5s3comms	<td>unknown
<tr><td>OSX		<td>CMake	<td>aws-s3-sdk	<td>unknown
<tr><td>OSX		<td>CMake	<td>nch5s3comms	<td>unknown
<tr><td>Visual Studio	<td>CMake	<td>aws-s3-sdk	<td>no (tests-fail)
<tr><td>Visual Studio	<td>CMake	<td>nch5s3comms	<td>yes
<tr><td>Cygwin		<td>Automake	<td>aws-s3-sdk	<td>unknown
<tr><td>Cygwin		<td>Automake	<td>nch5s3comms	<td>yes
<tr><td>Cygwin		<td>CMake	<td>aws-s3-sdk	<td>unknown
<tr><td>Cygwin		<td>CMake	<td>nch5s3comms	<td>unknown
<tr><td>Mingw		<td>Automake	<td>aws-s3-sdk	<td>unknown
<tr><td>Mingw		<td>Automake	<td>nch5s3comms	<td>unknown
<tr><td>Mingw		<td>CMake	<td>aws-s3-sdk	<td>unknown
<tr><td>Mingw		<td>CMake	<td>nch5s3comms	<td>unknown
</table>

## Automake

There are several options relevant to Amazon S3 support.
These are as follows.

1. _--enable-s3_ -- Enable S3 support.
2. _--enable-s3-internal_ -- Force use of the *nch5s3comms* SDK instead of the *aws-cpp-sdk* (assuming it is available).
3. _--with-s3-testing_=yes|no|public -- "yes" means do all S3 tests, "no" means do no S3 testing, "public" means do only those tests that involve publically accessible S3 data.

__A note about using S3 with Automake.__
If S3 support is desired, and using the Amazon "aws-sdk-cpp" SDK, and using Automake, then LDFLAGS must be properly set, namely to this.
````
LDFLAGS="$LDFLAGS -L/usr/local/lib -laws-cpp-sdk-s3"
````
The above assumes that these libraries were installed in '/usr/local/lib', so the above requires modification if they were installed elsewhere.

Note also that if S3 support is enabled, then you need to have a C++ compiler installed because the "aws-sdk-cpp" S3 support code is written in C++.

## CMake {#nccloud_cmake}

The necessary CMake flags are as follows (with defaults)

1. *-DENABLE_S3* -- Controll S3 support
2. *-DENABLE_S3_INTERNAL* -- Force use of the *nch5s3comms* SDK instead of the *aws-cpp-sdk*.
3. *-DWITH-S3-TESTING_=ON|OFF|PUBLIC -- "ON" means do all S3 tests, "OFF" means do no S3 testing, "PUBLIC" means do only those tests that involve publically accessible S3 data.

Note that unlike Automake, CMake can properly locate C++ libraries, so it should not be necessary to specify _-laws-cpp-sdk-s3_ assuming that the aws s3 libraries are installed in the default location.
For CMake with Visual Studio, the default location is here:

````
C:/Program Files (x86)/aws-cpp-sdk-all
````

It is possible to install the sdk library in another location.
In this case, one must add the following flag to the cmake command.
````
cmake ... -DAWSSDK_DIR=\<awssdkdir\>
````
where "awssdkdir" is the path to the sdk installation.
For example, this might be as follows.
````
cmake ... -DAWSSDK_DIR="c:\tools\aws-cpp-sdk-all"
````
This can be useful if blanks in path names cause problems in your build environment.

# Appendix B. Building the S3 SDKs {#nccloud_s3sdk}

As mentioned, three S3 storage drivers are available for accessing Amazon S3.
1. *H5FDros3*
2. *aws-sdk-cpp*
2. *nch5s3comms*

## Building *H5FDros3*

This driver is part of the HDF5 library codebase.
It must be enabled at the time that the HDF5 library is built
by using the *--enable-ros3=vfd* option.
If built, then the NetCDF-C build process should detect it and make use of it.

## Building *aws-sdk-cpp*

Amazon provides (thru AWS-labs) an SDK for accessing the Amazon S3 cloud.
This library, [aws-sdk-cpp library](https://github.com/aws/aws-sdk-cpp.git),
has a number of properties of interest:
* It is written in C++
* It is available on [GitHub](https://github.com/aws/aws-sdk-cpp.git),
* It uses CMake + ninja as its primary build system.

### *\*nix\** Build

For linux, the following context works. Of course your mileage may vary.
* OS: ubuntu 21
* aws-sdk-cpp version 1.9.96 (or later)
* Dependencies: openssl, libcurl, cmake, ninja (ninja-build using *apt-get*)

#### AWS-SDK-CPP CMake Build Recipe
````
git clone --recurse-submodules https://www.github.com/aws/aws-sdk-cpp
pushd aws-sdk-cpp
mkdir build
cd build
PREFIX=/usr/local
FLAGS="-DCMAKE_INSTALL_PREFIX=${PREFIX} \
       -DCMAKE_INSTALL_LIBDIR=lib \
       -DCMAKE_MODULE_PATH=${PREFIX}/lib/cmake \
       -DCMAKE_POLICY_DEFAULT_CMP0075=NEW \
       -DBUILD_ONLY=s3 \
       -DENABLE_UNITY_BUILD=ON \
       -DENABLE_TESTING=OFF \
       -DCMAKE_BUILD_TYPE=$CFG \
       -DSIMPLE_INSTALL=ON"
cmake -GNinja $FLAGS ..
ninja all
ninja install
cd ..
popd
````

### Windows build
It is possible to build and install aws-sdk-cpp on Windows using CMake.
Unfortunately, testing currently fails.

For Windows, the following context work. Of course your mileage may vary.
* OS: Windows 10 64-bit with Visual Studio community edition 2019.
* aws-sdk-cpp version 1.9.96 (or later)
* Dependencies: openssl, libcurl, cmake

#### AWS-SDK-CPP Build Recipe

This command-line build assumes one is using Cygwin or Mingw to provide
tools such as bash.

````
git clone --recurse-submodules https://www.github.com/aws/aws-sdk-cpp
pushd aws-sdk-cpp
mkdir build
cd build
CFG="Release"
PREFIX="c:/tools/aws-sdk-cpp"

FLAGS="-DCMAKE_INSTALL_PREFIX=${PREFIX} \
       -DCMAKE_INSTALL_LIBDIR=lib" \
       -DCMAKE_MODULE_PATH=${PREFIX}/cmake \
       -DCMAKE_POLICY_DEFAULT_CMP0075=NEW \
       -DBUILD_ONLY=s3 \
       -DENABLE_UNITY_BUILD=ON \
       -DCMAKE_BUILD_TYPE=$CFG \
       -DSIMPLE_INSTALL=ON"

rm -fr build
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=${CFG} $FLAGS ..
cmake --build . --config ${CFG}
cmake --install . --config ${CFG}
cd ..
popd
````
Notice that the sdk is being installed in the directory "c:\tools\aws-sdk-cpp"
rather than the default location "c:\Program Files (x86)/aws-sdk-cpp-all"
This is because when using a command line, an install path that contains
blanks may not work.

In order for CMake to find the aws sdk libraries,
the following environment variables must be set:
````
AWSSDK_ROOT_DIR="c:/tools/aws-sdk-cpp"
AWSSDKBIN="/cygdrive/c/tools/aws-sdk-cpp/bin"
PATH="$PATH:${AWSSDKBIN}"
````
Then the following options must be specified for cmake.
````
-DAWSSDK_ROOT_DIR=${AWSSDK_ROOT_DIR}
-DAWSSDK_DIR=${AWSSDK_ROOT_DIR}/lib/cmake/AWSSDK"
````
## Building ``nch5s3comms''

This is an experimental SDK provided internally in the netcdf-c library.

* It is written in C
* It provides the minimum functionality necessary to read/write/search an Amazon S3 bucket.
* It was constructed by heavily modifying the HDF5 *H5Fs3commons.c* file and combining it with crypto code wrappers provided by libcurl. The resulting file was then modified to fit into the netcdf coding style.
* The resulting code is rather ugly, but appears to work under at least Linux and under Windows (using Visual C++).

### Dependencies

* *\*nix\**: the following packages need to be installed: openssl, libcurl, (optionally) libxml2.
* *Windows (Visual C++)*: the following packages need to be installed: libcurl, (optionally) libxml2.

### Build Options

In order to enable this SDK, the Automake option *--enable-s3-internal* or the CMake option *-DENABLE_S3_INTERNAL=ON* must be specified.

### Testing S3 Support {#nccloud_testing_S3_support}

The pure S3 test(s) are in the _unit_tests_ directory.
Currently, by default, testing of S3 is supported only for Unidata members of the NetCDF Development Group.
This is because it uses a Unidata-specific bucket is inaccessible to the general user.

# Appendix C. AWS Selection Algorithms. {#nccloud_awsselect}

If byterange support is enabled, the netcdf-c library will parse the files
````
${HOME}/.aws/config
and
${HOME}/.aws/credentials
````
to extract profile names plus a list of key=value pairs.
In case of duplicates, *credentials* takes precedence over *config*.

This example is typical of the contents of these files.
````
[default]
    aws_access_key_id=XXXXXXXXXXXXXXXXXXXX
    aws_secret_access_key=YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
    aws_region=ZZZZZZZZZ
````
The keys in the profile will be used to set various parameters in the library

## Profile Selection

The algorithm for choosing the active profile to use is as follows:

1. If the "aws.profile" fragment flag is defined in the URL, then it is used. For example, see this URL.
````
https://...#mode=nczarr,s3&aws.profile=xxx
````
2. If the "AWS.PROFILE" entry in the .rc file (i.e. .netrc or .dodsrc) is set, then it is used.
3. If defined, then profile "default" is used.
4. Otherwise the profile "no" is used.

The profile named "no" is a special profile that the netcdf-c library automatically defines.
It should not be defined anywhere else. It signals to the library that no credentialas are to used.
It is equivalent to the "--no-sign-request" option in the AWS CLI.

## Region Selection

If the specified URL is of the form
````
s3://<bucket>/key
````
Then this is rebuilt to this form:
````
s3://s2.&lt;region&gt.amazonaws.com>/key
````
However this requires figuring out the region to use.
The algorithm for picking an region is as follows.

1. If the "aws.region" fragment flag is defined in the URL, then it is used.
2. The active profile is searched for the "aws_region" key.
3. If the "AWS.REGION" entry in the .rc file (i.e. .netrc or .dodsrc) is set, then it is used.
4. Otherwise use "us-east-1" region.

## Authorization Selection

Picking an access-key/secret-key pair is always determined
by the current active profile. To choose to not use keys
requires that the active profile must be "no".

# Change Log {#nccloud_changelog}
[Note: minor text changes are not included.]

## 3/8/2023
1. Add an internal, minimal Amazon S3 SDK to support S3 access especially for Windows.

# Point of Contact {#nccloud_poc}

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 3/8/2023<br>
__Last Revised__: 3/8/2023
