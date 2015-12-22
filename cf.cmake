rm -fr build
mkdir build
cd build

export CC=mpicc

for p in /usr/local/lib /usr/lib ; do
if test -z "$ZP" -a -f $p/libz.so ; then ZP=$p; fi
if test -z "$HP" -a -f $p/libhdf5.so ; then HP=$p; fi
if test -z "$CP" -a -f $p/libcurl.so ; then CP=$p; fi
done


ZLIB="-DZLIB_LIBRARY=${ZP}/libz.so -DZLIB_INCLUDE_DIR=${ZP}/include -DZLIB_INCLUDE_DIRS=${ZP}/include"
HDF5="-DHDF5_LIB=${HP}/libhdf5.so -DHDF5_HL_LIB=${HP}/libhdf5_hl.so -DHDF5_INCLUDE_DIR=${HP}/include"
CURL="-DCURL_LIBRARY=${CP}/libcurl.so -DCURL_INCLUDE_DIR=${CP}/include -DCURL_INCLUDE_DIRS=${CP}/include"
FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=/usr/local"
#FLAGS="-DCMAKE_PREFIX_PATH=$PPATH"
#FLAGS="$FLAGS -DCMAKE_PREFIX_PATH=$PPATH"
FLAGS="$FLAGS -DENABLE_DAP_REMOTE_TESTS=true"
FLAGS="$FLAGS -DENABLE_DAP_AUTH_TESTS=true"

cmake $FLAGS ${ZLIB} ${HDF5} ${CURL} ..
cmake --build .
cmake --build . --target test
