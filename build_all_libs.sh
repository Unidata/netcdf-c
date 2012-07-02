#!/bin/bash
# Build all (or some subset) of the libraries.

####
#Define Target locations, files.
####
HOMEDIR=`pwd`
TARGBASEDIR="$HOMEDIR/netcdf-windows/"
mkdir -p $TARGBASEDIR
LOGFILE="$HOMEDIR/buildall_log.txt"

####
# 32-bit directories
####
CLASSIC32STATDIR="$HOMEDIR/nc-32-classic-static"
NC432STATDIR="$HOMEDIR/nc-32-nc4-static"
NC4DAP32STATDIR="$HOMEDIR/nc-32-dap-static"
CLASSIC32DLLDIR="$HOMEDIR/nc-32-classic-dll"
NC432DLLDIR="$HOMEDIR/nc-32-nc4-dll"
NC4DAP32DLLDIR="$HOMEDIR/nc-32-dap-dll"

####
# 64-bit directories
####
CLASSIC64STATDIR="$HOMEDIR/nc-64-classic-static"
NC464STATDIR="$HOMEDIR/nc-64-nc4-static"
NC4DAP64STATDIR="$HOMEDIR/nc-64-dap-static"
CLASSIC64DLLDIR="$HOMEDIR/nc-64-classic-dll"
NC464DLLDIR="$HOMEDIR/nc-64-nc4-dll"
NC4DAP64DLLDIR="$HOMEDIR/nc-64-dap-dll"

####
#Define CFLAGS, LDFLAGS related to dependency locations.
####
CMNFLAGS="--disable-doxygen"

STATICFLAGS="--enable-static --disable-shared"
SHAREDFLAGS="--disable-static --enable-shared --enable-dll"

MINFLAGS="--disable-dap --disable-netcdf-4"

STATICCFLAGS="-static -fno-stack-check -fno-stack-protector -mno-stack-arg-probe"
STATICLDFLAGS="-static"
FLAGS64="--host=x86_64-w64-mingw32"

#HDF5
HDF5DIR="/c/Users/wfisher/Desktop/hdf5-1.8.9/"
HDF5INCDIR="$HDF5DIR/src"
HDF532DLLDIR="$HDF5DIR/build_win32_dll/bin"
HDF532LIBDIR="$HDF5DIR/build_win32_static/bin"
HDF564DLLDIR="$HDF5DIR/build_win64_dll/bin"
HDF564LIBDIR="$HDF5DIR/build_win64_static/bin"

#SZIP
SZIPDIR="/c/Users/wfisher/Desktop/szip-2.1/"
SZIP32LIBS="/c/Users/wfisher/Desktop/szip-2.1/build_win32/bin"
SZIP64LIBS="/c/Users/wfisher/Desktop/szip-2.1/build_win64/bin"

#ZLIB
ZLIBDIR="/c/Users/wfisher/Desktop/zlib125dll"
ZLIB32LIB="$ZLIBDIR/static32"
ZLIB32DLL="$ZLIBDIR/dll32"
ZLIB64LIB="$ZLIBDIR/staticx64"
ZLIB64DLL="$ZLIBDIR/dllx64"

#CURL
CURLDIR="/c/Users/wfisher/Desktop/curl-7.25.0"
CURL32INCDIR="$CURLDIR/build_win32/include/"
CURL32LIBS="$CURLDIR/build_win32/lib/.libs"
CURL64INCDIR="$CURLDIR/build_win64/include/"
CURL64LIBS="$CURLDIR/build_win64/lib/.libs"

if [ $# -lt 1 ]; then
    echo "Usage: $0 [clean/all/32/64] [deps]"
    exit 0
fi

if [ $1 == "clean" ]; then
    echo "Cleaning up."
    set -x
    rm -rf $LOGFILE
    rm -rf $CLASSIC32STATDIR \
	$NC432STATDIR $NC4DAP32STATDIR $CLASSIC32DLLDIR \
	$NC432DLLDIR $NC4DAP32DLLDIR $CLASSIC64STATDIR \
	$NC464STATDIR $NC4DAP64STATDIR $CLASSIC64DLLDIR \
	$NC464DLLDIR $NC4DAP64DLLDIR
    rm -rf netcdf-windows

    exit 0
fi

INCDEPS="false"
if [ $# -gt 1 ]; then
if [ $2 == "deps" ]; then
    INCDEPS="true"
fi
fi
####
# 32-bit libraries
####
echo "Target Home Directory: $TARGBASEDIR"
echo "Writing build output to: $LOGFILE"
echo "Include Dependency libs: $INCDEPS"
echo ""

if [ $1 == "all" -o $1 == "32" ]; then

echo "Building 32-bit libraries."
echo ""
####
# 32, static, classic only
####

# Build
BUILDDIR="$CLASSIC32STATDIR"
TARGDIR="$TARGBASEDIR/nc3_32_static"
echo "o Building 32-bit NetCDF Classic [static] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $STATICFLAGS $MINFLAGS CFLAGS="$STATICCFLAGS" LDFLAGS="$STATICLDFLAGS -L$ZLIB32LIB" > $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
DEPDIR=$TARGDIR/deps
mkdir -p $DEPDIR
if [ $INCDEPS == "true" ]; then
    echo "Copying dependencies to $DEPDIR"
    cp $ZLIB32LIB/* $DEPDIR
fi

echo ""
echo "---------------"
echo "" 

####
# 32, static, nc4
####
BUILDDIR="$NC432STATDIR"
TARGDIR="$TARGBASEDIR/nc4_32_static"
echo "o Building 32-bit NetCDF-4 [static] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $STATICFLAGS --enable-netcdf-4 --disable-dap CFLAGS="$STATICCFLAGS -I$HDF5INCDIR" LDFLAGS="$STATICLDFLAGS -L$HDF532LIBDIR -L$SZIP32LIBS -L$ZLIB32LIB" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $HDF532LIBDIR/*.a $DEPDIR
    cp $SZIP32LIBS/*.a $SZIP32LIBS/*.lib $DEPDIR
    cp $ZLIB32LIB/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""

####
# 32, static, nc4, dap
####
BUILDDIR="$NC4DAP32STATDIR"
TARGDIR="$TARGBASEDIR/nc4_dap_32_static"
echo "o Building 32-bit NetCDF-4 with DAP [static] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $STATICFLAGS --enable-netcdf-4 --enable-dap CFLAGS="$STATICCFLAGS -I$HDF5INCDIR -I$CURL32INCDIR" LDFLAGS="$STATICLDFLAGS -L$HDF532LIBDIR -L$SZIP32LIBS -L$CURL32LIBS -DCURL_STATICLIB -L$ZLIB32LIB" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $HDF532LIBDIR/*.a $DEPDIR
    cp $SZIP32LIBS/*.a $SZIP32LIBS/*.lib $DEPDIR
    cp $CURL32LIBS/libcurl.a $DEPDIR
    cp $ZLIB32LIB/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""

####
# 32, dll, classic only
####
BUILDDIR="$CLASSIC32DLLDIR"
TARGDIR="$TARGBASEDIR/nc3_32_dll"
echo "o Building 32-bit NetCDF Classic [dll] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $SHAREDFLAGS $MINFLAGS LDFLAGS="-L$ZLIB32DLL" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $ZLIB32DLL/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""

####
# 32, dll, nc4
####
BUILDDIR="$NC432DLLDIR"
TARGDIR="$TARGBASEDIR/nc4_32_dll"
echo "o Building 32-bit NetCDF-4 [dll] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $SHAREDFLAGS --enable-netcdf-4 --disable-dap CFLAGS="-I$HDF5INCDIR" LDFLAGS="-L$HDF532DLLDIR -L$SZIP32LIBS -L$ZLIB32DLL" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $HDF532DLLDIR/*.dll $DEPDIR
    cp $SZIP32LIBS/*.dll $SZIP32LIBS/*.lib $DEPDIR
    cp $ZLIB32DLL/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""




####
# 32, dll, nc4, dap
####
BUILDDIR="$NC4DAP32DLLDIR"
TARGDIR="$TARGBASEDIR/nc4_dap_32_dll"
echo "o Building 32-bit NetCDF-4 with DAP [dll] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $SHAREDFLAGS --enable-netcdf-4 --enable-dap CFLAGS="-I$HDF5INCDIR -I$CURL32INCDIR" LDFLAGS="-L$HDF532DLLDIR -L$SZIP32LIBS -L$CURL32LIBS -L$ZLIB32DLL" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $HDF532LIBDIR/*.a $DEPDIR
    cp $SZIP32LIBS/*.a $SZIP32LIBS/*.lib $DEPDIR
    cp $CURL32LIBS/libcurl.a $DEPDIR
    cp $ZLIB32LIB/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""

fi



if [ $1 == "all" -o $1 == "64" ]; then


####
# 64-bit libraries
#### 
echo "Building 64-bit libraries."
echo ""

####
# 64, static, classic only
####

# Build
BUILDDIR="$CLASSIC64STATDIR"
TARGDIR="$TARGBASEDIR/nc3_64_static"
echo "o Building 64-bit NetCDF Classic [static] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $STATICFLAGS $MINFLAGS $FLAGS64 CFLAGS="$STATICCFLAGS" LDFLAGS="$STATICLDFLAGS -L$ZLIB64LIB" > $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
DEPDIR=$TARGDIR/deps
mkdir -p $DEPDIR
if [ $INCDEPS == "true" ]; then
    echo "Copying dependencies to $DEPDIR"
    cp $ZLIB64LIB/* $DEPDIR
fi

echo ""
echo "---------------"
echo "" 


####
# 64, static, nc4
####

BUILDDIR="$NC464STATDIR"
TARGDIR="$TARGBASEDIR/nc4_64_static"
echo "o Building 64-bit NetCDF-4 [static] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $STATICFLAGS --enable-netcdf-4 --disable-dap $FLAGS64 CFLAGS="$STATICCFLAGS -I$HDF5INCDIR -I`dirname $HDF564LIBDIR`" LDFLAGS="$STATICLDFLAGS -L$HDF564LIBDIR -L$SZIP64LIBS -L$ZLIB64LIB" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $HDF564LIBDIR/*.a $DEPDIR
    cp $SZIP64LIBS/*.a $SZIP64LIBS/*.lib $DEPDIR
    cp $ZLIB64LIB/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""


####
# 64, static, nc4, dap
####
BUILDDIR="$NC4DAP64STATDIR"
TARGDIR="$TARGBASEDIR/nc4_dap_64_static"
echo "o Building 64-bit NetCDF-4 with DAP [static] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $STATICFLAGS --enable-netcdf-4 --enable-dap $FLAGS64 CFLAGS="$STATICCFLAGS -I$HDF5INCDIR -I$CURL64INCDIR -I`dirname $HDF564LIBDIR`" LDFLAGS="$STATICLDFLAGS -L$HDF564LIBDIR -L$SZIP64LIBS -L$CURL64LIBS -DCURL_STATICLIB -L$ZLIB64LIB" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $HDF564LIBDIR/*.a $DEPDIR
    cp $SZIP64LIBS/*.a $SZIP64LIBS/*.lib $DEPDIR
    cp $CURL64LIBS/libcurl.a $DEPDIR
    cp $ZLIB64LIB/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""


####
# 64, dll, classic only
####
BUILDDIR="$CLASSIC64DLLDIR"
TARGDIR="$TARGBASEDIR/nc3_64_dll"
echo "o Building 64-bit NetCDF Classic [dll] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $SHAREDFLAGS $MINFLAGS LDFLAGS="-L$ZLIB64DLL" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $ZLIB64DLL/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""

####
# 64, dll, nc4
####
BUILDDIR="$NC464DLLDIR"
TARGDIR="$TARGBASEDIR/nc4_64_dll"
echo "o Building 64-bit NetCDF-4 [dll] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $CMNFLAGS $SHAREDFLAGS --enable-netcdf-4 --disable-dap $FLAGS64 CFLAGS="-I$HDF5INCDIR -I`dirname $HDF564DLLDIR`" LDFLAGS="-L$HDF564DLLDIR -L$SZIP64LIBS -L$ZLIB64DLL" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $HDF564DLLDIR/*.dll $DEPDIR
    cp $SZIP64LIBS/*.dll $SZIP64LIBS/*.lib $DEPDIR
    cp $ZLIB64DLL/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""


####
# 64, dll, nc4, dap
####
BUILDDIR="$NC4DAP64DLLDIR"
TARGDIR="$TARGBASEDIR/nc4_dap_64_dll"
echo "o Building 64-bit NetCDF-4 with DAP [dll] ($BUILDDIR)"
mkdir -p $BUILDDIR
cd $BUILDDIR
../configure --prefix=$TARGDIR $SHAREDFLAGS --enable-netcdf-4 --enable-dap $FLAGS64 CFLAGS="-I$HDF5INCDIR -I$CURL64INCDIR -I`dirname $HDF564DLLDIR`" LDFLAGS="-L$HDF564DLLDIR -L$SZIP64LIBS -L$CURL64LIBS -L$ZLIB64DLL" >> $LOGFILE 2>&1
make -j 10 >> $LOGFILE 2>&1
make install >> $LOGFILE 2>&1

# Dependencies if needed
if [ $INCDEPS == "true" ]; then
    DEPDIR=$TARGDIR/deps
    mkdir -p $DEPDIR
    echo "Copying dependencies to $DEPDIR"
    cp $HDF564LIBDIR/*.a $DEPDIR
    cp $SZIP64LIBS/*.a $SZIP64LIBS/*.lib $DEPDIR
    cp $CURL64LIBS/libcurl.a $DEPDIR
    cp $ZLIB64LIB/* $DEPDIR
fi
echo ""
echo "---------------"
echo ""

## End 'if 64, etc'
fi

echo "Finished"



