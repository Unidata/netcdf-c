# Visual Studio

NCC="c:/tools/hdf5-1.10.6"
HDF5_DIR="$NCC/cmake/hdf5"
AWSSDK_DIR="c:/tools/aws-cpp-sdk-all"

# Is netcdf-4 and/or DAP enabled?
NCZARR=1
HDF5=1
DAP=1
#S3=1
#S3TEST=1
#CDF5=1
#HDF4=1

#TR=--trace

export SETX=1

for arg in "$@" ; do
case "$arg" in
vs|VS) VS=1 ;;
linux|nix|l|x) unset VS ;;
nobuild|nb) NOBUILD=1 ;;
notest|nt) NOTEST=1 ;;
*) echo "Must specify env: vs|linux"; exit 1; ;;
esac
done

#export NCPATHDEBUG=1

FLAGS=
FLAGS="$FLAGS -DNC_FIND_SHARED_LIBS=ON"

if test "x$VS" != x -a "x$INSTALL" != x ; then
FLAGS="$FLAGS -DCMAKE_PREFIX_PATH=${NCC}"
fi
FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=/tmp/netcdf"
mkdir -p /tmp/netcdf

if test "x$NCZARR" != x ; then
FLAGS="$FLAGS -DENABLE_NCZARR=true"
else
FLAGS="$FLAGS -DENABLE_NCZARR=false"
fi

if test "x$DAP" = x ; then
FLAGS="$FLAGS -DENABLE_DAP=false"
#FLAGS="$FLAGS -DENABLE_DAP_REMOTE_TESTS=true"
fi

if test "x$HDF5" = x ; then
FLAGS="$FLAGS -DENABLE_HDF5=false"
else
ignore=1
#FLAGS="$FLAGS -DDEFAULT_API_VERSION:STRING=v110"
#FLAGS="$FLAGS -DHDF5_ROOT=c:/tools/hdf5"
#FLAGS="$FLAGS -DHDF5_ROOT_DIR_HINT=c:/tools/hdf5/cmake/hdf5/hdf5-config.cmake"
FLAGS="$FLAGS -DHDF5_DIR=$HDF5_DIR"
#hdf5-config.cmake
#FLAGS="-DHDF5_LIBRARIES=${NCC}/lib/hdf5 -DHDF5_HL_LIBRARY=${NCC}/lib/hdf5_hl -DHDF5_INCLUDE_DIR=${NCC}/include"
fi
if test "x$CDF5" != x ; then
FLAGS="$FLAGS -DENABLE_CDF5=true"
fi
if test "x$HDF4" != x ; then
FLAGS="$FLAGS -DENABLE_HDF4=true"
fi

if test "x$TESTSERVERS" != x ; then
FLAGS="$FLAGS -DREMOTETESTSERVERS=${TESTSERVERS}"
fi

if test "x$S3" != x ; then
FLAGS="$FLAGS -DENABLE_S3_SDK=true"
else
FLAGS="$FLAGS -DENABLE_S3_SDK=false"
fi
if test "x$S3TEST" != x ; then
FLAGS="$FLAGS -DENABLE_NCZARR_S3_TESTS=true"
FLAGS="$FLAGS -DAWSSDK_DIR=${AWSSDK_DIR}"
fi

# Enables
FLAGS="$FLAGS -DENABLE_LOGGING=true"
FLAGS="$FLAGS -DENABLE_BYTERANGE=true"
#FLAGS="$FLAGS -DENABLE_DOXYGEN=true -DENABLE_INTERNAL_DOCS=true"
#FLAGS="$FLAGS -DENABLE_LARGE_FILE_TESTS=true"
FLAGS="$FLAGS -DENABLE_FILTER_TESTING=true"
FLAGS="$FLAGS -DENABLE_EXAMPLES=false"
#FLAGS="$FLAGS -DENABLE_CONVERSION_WARNINGS=false"
#FLAGS="$FLAGS -DENABLE_TESTS=false"
#FLAGS="$FLAGS -DENABLE_DISKLESS=false"
#FLAGS="$FLAGS -DBUILD_UTILITIES=false"

FLAGS="$FLAGS -DCURL_NO_CURL_CMAKE=TRUE"

# Withs
FLAGS="$FLAGS -DNCPROPERTIES_EXTRA=\"key1=value1|key2=value2\""

rm -fr build
mkdir build
cd build

NCWD=`pwd`

if test "x$VS" != x ; then

NCLIB=`cygpath -w ${NCWD}/liblib |tr -d ''`
NCCYGLIB=`cygpath -u ${NCLIB} |tr -d ''`
NCCBIN=`cygpath -u "${NCC}/bin" |tr -d ''`
AWSSDKBIN="/cygdrive/c/tools/aws-cpp-sdk-all/bin"

# Visual Studio
PATH="${NCCBIN}:$PATH:$NCCYGLIB"
if test "x$S3" != x ; then
PATH="$PATH:${AWSSDKBIN}"
fi
export PATH

export LD_LIBRARY_PATH="${NCCBIN}:$LD_LIBRARY_PATH:${AWSSDKBIN}:${NCCYGLIB}"

CFG="Release"

#G=
#TR=--trace
cmake ${TR} "$G" -DCMAKE_BUILD_TYPE=${CFG} $FLAGS ..
if test "x$NOBUILD" = x ; then
cmake ${TR} --build . -v --config ${CFG}
#cmake ${TR} --build . --config ${CFG} --target ZERO_CHECK
#cmake ${TR} --build . --config ${CFG} --target ALL_BUILD
if test "x$NOTEST" = x ; then
cmake ${TR} --build . --config ${CFG} --target RUN_TESTS
fi
fi
else
# GCC
NCLIB="${NCWD}/build/liblib"
#G="-GUnix Makefiles"
#T="--trace-expand"
cmake "${G}" $FLAGS ..
if test "x$NOBUILD" = x ; then
make all
fi
if test "x$NOTEST" = x ; then
make test
fi
fi
exit
