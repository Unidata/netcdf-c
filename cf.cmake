# Is netcdf-4 and/or DAP enabled?
NC4=1
DAP=1

# Is visual studio being used?
VS=yes

if test "x$VS" != x ; then
CYGWIN=
else
CYGWIN=yes
fi

if test "x$VS" = x ; then
#export CC=mpicc
export CC=gcc
else
export CC=
fi

if test "x$VS" != x ; then
CMAKE_PREFIX_PATH="c:/tools/hdf5/cmake"
FLAGS="${FLAGS} -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
fi

if test "x$CYGWIN" != x; then
ZLIB=`cygpath -w "$ZLIB"`
H5LIB=`cygpath -w "$H5LIB"`
CURLLIB=`cygpath -w "$CURLLIB"`
fi

if test "x$DAP" = x ; then
FLAGS="$FLAGS -DENABLE_DAP=false"
fi
if test "x$NC4" = x ; then
FLAGS="$FLAGS -DENABLE_NETCDF_4=false"
fi
FLAGS="$FLAGS -DENABLE_CONVERSION_WARNINGS=false"

FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=/usr/local"
#FLAGS="$FLAGS -DENABLE_DAP_AUTH_TESTS=true"
#FLAGS="$FLAGS -DENABLE_DAP_REMOTE_TESTS=true"

rm -fr build
mkdir build
cd build

#cmake $FLAGS ${ZLIB} ${HDF5} ${CURL} ..
cmake $FLAGS ..
cmake --build .
CTEST_OUTPUT_ON_FAILURE=1 cmake --build . --target test
