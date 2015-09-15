rm -fr build
mkdir build
cd build
UL=/usr/local
PPATH="$UL"
ZLIB="-DZLIB_LIBRARY=${UL}/lib/libz.so  -DZLIB_INCLUDE_DIR=${UL}/include"
HDF5="-DHDF5_LIB=${UL}/lib/libhdf5.so -DHDF5_HL_LIB=${UL}/lib/libhdf5_hl.so -DHDF5_INCLUDE_DIR=${UL}/include"
CURL="-DCURL_LIBRARY=${UL}/lib/libcurl.so  -DCURL_INCLUDE_DIR=${UL}/include"
FLAGS="-DCMAKE_PREFIX_PATH=$PPATH"
FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=${UL}"
FLAGS="$FLAGS -DCMAKE_PREFIX_PATH=$PPATH"
FLAGS="$FLAGS -DENABLE_DAP_REMOTE_TESTS=true"
FLAGS="$FLAGS -DENABLE_DAP_AUTH_TESTS=true"
cmake $FLAGS ${ZLIB} ${HDF5} ${CURL} ..
#cmake --build .
#cmake --build . --target test
