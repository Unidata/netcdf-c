rm -fr build
mkdir build
cd build
UL=/usr/local
PPATH="$UL"
HDF5="-DHDF5_LIB=${UL}/lib/libhdf5.so -DHDF5_HL_LIB=${UL}/lib/libhdf5_hl.so -DHDF5_INCLUDE_DIR=${UL}/include"
cmake -DCMAKE_INSTALL_PREFIX=${UL} -DCMAKE_PREFIX_PATH="$PPATH" ${HDF5} ..
cmake --build .
#cmake --build . --target RUN_TESTS
