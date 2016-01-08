# Is visual studio being used?
#VS=yes
CYGWIN=yes

if test "x$VS" = x ; then
#CC=mpicc
CC=gcc
fi

export CC

if test "x$VS" != x -a "x$CYGWIN" != x ; then
  ZLIB=cygz.dll; H5LIB=cyghdf5.dll; H5LIB_HL=cyghdf5_hl.dll; CURLLIB=cygcurl.dll
elif test "x$VS" = x -a "x$CYGWIN" != x ; then
  ZLIB=libz.dll.a; H5LIB=libhdf5.dll.a; H5LIB_HL=libhdf5_hl.dll.a; CURLLIB=libcurl.dll.a
elif test "x$VS" = x -a "x$CYGWIN" == x ; then
  ZLIB=libz.so; H5LIB=libhdf5.so; H5LIB_HL=libhdf5_hl.so; CURLLIB=libcurl.so
else
  echo "cannot determine library names"
  exit 1
fi

for p in /usr/bin /usr/local/bin /usr/local/lib /usr/lib ; do
if test -f $p/$ZLIB ; then ZP=$p; fi
if test -f $p/$H5LIB ; then HP=$p; fi
if test -f $p/$CURLLIB ; then CP=$p; fi
done

if test "x$ZP" = x ; then echo "Cannot find z lib" ; exit 1; fi
if test "x$HP" = x ; then echo "Cannot find hdf5 lib" ; exit 1; fi
if test "x$CP" = x ; then echo "Cannot find curl lib" ; exit 1; fi

if test "x$CYGWIN" != x -a "x$VS" != x; then
ZP=`cygpath -w "$ZP"`
HP=`cygpath -w "$HP"`
CP=`cygpath -w "$CP"`
fi

#if test "x$VS" != x ; then USR=c:/cygwin/usr; else USR=/usr; fi

ZLIB="-DZLIB_LIBRARY=${ZP}/$ZLIB -DZLIB_INCLUDE_DIR=${ZP}/include -DZLIB_INCLUDE_DIRS=${ZP}/include"
HDF5="-DHDF5_LIB=${HP}/$H5LIB -DHDF5_HL_LIB=${HP}/$H5LIB_HL -DHDF5_INCLUDE_DIR=${HP}/include"
CURL="-DCURL_LIBRARY=${CP}/$CURLLIB -DCURL_INCLUDE_DIR=${CP}/include -DCURL_INCLUDE_DIRS=${CP}/include"

FLAGS="$FLAGS -DCMAKE_C_FLAGS='-Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-parameter'"x2

#FLAGS="$FLAGS -DENABLE_DAP=false"
#FLAGS="$FLAGS -DENABLE_NETCDF_4=false"

FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=$USR/local"
#FLAGS="-DCMAKE_PREFIX_PATH=$PPATH"
#FLAGS="$FLAGS -DCMAKE_PREFIX_PATH=$PPATH"
FLAGS="$FLAGS -DENABLE_DAP_REMOTE_TESTS=true"
#FLAGS="$FLAGS -DENABLE_DAP_AUTH_TESTS=true"

rm -fr build
mkdir build
cd build

cmake $FLAGS ${ZLIB} ${HDF5} ${CURL} ..
cmake --build .
CTEST_OUTPUT_ON_FAILURE=1 cmake --build . --target test
