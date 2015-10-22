#!/bin/bash

rm -f build
mkdir build
pushd build
cmake .. -DENABLE_NETCDF_4=OFF -DENABLE_DAP=OFF -DENABLE_DISKLESS=OFF
make -j 4
cd ncdump
./tst_nccopy3.sh
RES=$?
popd

exit $RES
