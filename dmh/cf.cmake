# Visual Studio

rm -f ev0 ev1
printenv >ev0

#NCC="c:/tools/hdf5-1.10.6"
#HDF5_DIR="$NCC/cmake/hdf5"
#PTHREADS4W_DIR="$NCC/lib/cmake/"
CMAKEPREFIXPATH="c:/tools/hdf5-1.14.0"

# Is netcdf-4 and/or DAP enabled?
HDF5=1
NCZARR=1
#DAP=1
#S3=1
#S3I=1
#S3TEST=1
#CDF5=1
#HDF4=1
FILTERS=1
#XML2=1
#BYTES=1
#THREADSAFE=1
#NONETWORK=1

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

echo "NOBUILD=${NOBUILD}"
echo "NOTEST=${NOTEST}"

#export NCPATHDEBUG=1

PREFIX="d:/tmp/netcdf"
#PREFIX="c:/tools/netCDF4.8.2"

#export HDF5_PLUGIN_PATH="d:\\git\\netcdf.fork\\build\\plugins"
#export HDF5_PLUGIN_PATH="c:\\ProgramData\\hdf5\\plugin;c:\\tmp\\plugins"

PLUGINDIR="d:/tmp/plugins"

mkdir -p ${PREFIX}
rm -fr ${PLUGINDIR}
mkdir -p ${PLUGINDIR} 

#FLAGS="-DCMAKE_PREFIX_PATH=${CMAKEPREFIXPATH}"
FLAGS="$FLAGS -DBUILD_SHARED_LIBS=ON"
FLAGS="$FLAGS -DNC_FIND_SHARED_LIBS=ON"
#FLAGS="$FLAGS -DCMAKE_IMPORT_LIBRARY_PREFIX=${NCC}"

if test "x$NCZARR" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_NCZARR=ON"
else
FLAGS="$FLAGS -DNETCDF_ENABLE_NCZARR=OFF"
fi

if test "x$THREADSAFE" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_THREADSAFE=ON"
FLAGS="$FLAGS -DPTHREADS4W_DIR=$PTHREADS4W_DIR"
else
FLAGS="$FLAGS -DNETCDF_ENABLE_THREADSAFE=OFF"
fi

if test "x$DAP" = x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_DAP=OFF"
else
FLAGS="$FLAGS -DNETCDF_ENABLE_DAP_REMOTE_TESTS=ON"
FLAGS="$FLAGS -DNETCDF_ENABLE_EXTERNAL_SERVER_TESTS=ON"
FLAGS="$FLAGS -DNETCDF_ENABLE_DAP_AUTH_TESTS=OFF"
fi

if test "x$HDF5" = x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_HDF5=OFF"
else
FLAGS="$FLAGS -DNETCDF_ENABLE_HDF5=ON"
#FLAGS="$FLAGS -DDEFAULT_API_VERSION:STRING=v110"
#FLAGS="$FLAGS -DHDF5_ROOT=c:/tools/hdf5"
#FLAGS="$FLAGS -DHDF5_ROOT_DIR_HINT=c:/tools/hdf5/cmake/hdf5/hdf5-config.cmake"
#FLAGS="$FLAGS -DHDF5_DIR=$HDF5_DIR"
#hdf5-config.cmake
#FLAGS="$FLAGS -DHDF5_LIBRARIES=${NCC}/lib/hdf5 -DHDF5_HL_LIBRARY=${NCC}/lib/hdf5_hl -DHDF5_INCLUDE_DIR=${NCC}/include"
fi
if test "x$CDF5" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_CDF5=ON"
fi
if test "x$HDF4" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_HDF4=ON"
fi
if test "x$FILTERS" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_PLUGINS=on"
FLAGS="$FLAGS -DNETCDF_ENABLE_FILTER_TESTING=on"
else
FLAGS="$FLAGS -DNETCDF_ENABLE_PLUGINS=off"
FLAGS="$FLAGS -DNETCDF_ENABLE_FILTER_TESTING=off"
fi

if test "x$XML2" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_LIBXML2=on"
else
FLAGS="$FLAGS -DNETCDF_ENABLE_LIBXML2=off"
fi

if test "x$TESTSERVERS" != x ; then
FLAGS="$FLAGS -DREMOTETESTSERVERS=${TESTSERVERS}"
fi

if test "x$S3" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_S3=ON"
if test "x$S3I" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_S3_INTERNAL=ON"
#else
#AWSSDK_DIR="/cygdrive/c/tools/aws-sdk-cpp/lib/cmake"
#AWSSDK_ROOT_DIR="c:/tools/aws-sdk-cpp"
#FLAGS="$FLAGS -DAWSSDK_ROOT_DIR=${AWSSDK_ROOT_DIR}"
#FLAGS="$FLAGS -DAWSSDK_DIR=${AWSSDK_ROOT_DIR}/lib/cmake/AWSSDK"
#PREFIXPATH="$PREFIXPATH;${AWSSDK_ROOT_DIR}"
fi
if test "x$S3TEST" != x ; then
FLAGS="$FLAGS -DNETCDF_ENABLE_NCZARR_S3_TESTS=ON"
fi
else
FLAGS="$FLAGS -DNETCDF_ENABLE_S3=OFF"
fi

if test "x$BYTES" != x ; then 
FLAGS="$FLAGS -DNETCDF_ENABLE_BYTERANGE=ON"
else
FLAGS="$FLAGS -DNETCDF_ENABLE_BYTERANGE=OFF"
fi

if test "x$NONETWORK" != x ; then 
FLAGS="$FLAGS -DNETCDF_ENABLE_NETWORK_ACCESS=OFF"
else
FLAGS="$FLAGS -DNETCDF_ENABLE_NETWORK_ACCESS=ON"
fi

# Enables
FLAGS="$FLAGS -DNETCDF_ENABLE_LOGGING=ON"
#FLAGS="$FLAGS -DNETCDF_ENABLE_DOXYGEN=ON -DNETCDF_ENABLE_INTERNAL_DOCS=ON"
#FLAGS="$FLAGS -DNETCDF_ENABLE_LARGE_FILE_TESTS=ON"
#FLAGS="$FLAGS -DNETCDF_ENABLE_EXAMPLES=ON"
#FLAGS="$FLAGS -DNETCDF_ENABLE_CONVERSION_WARNINGS=OFF"
#FLAGS="$FLAGS -DNETCDF_ENABLE_TESTS=OFF"
#FLAGS="$FLAGS -DNETCDF_ENABLE_DISKLESS=OFF"
#FLAGS="$FLAGS -DBUILD_UTILITIES=OFF"

FLAGS="$FLAGS -DCURL_NO_CURL_CMAKE=ON"
FLAGS="$FLAGS -DNETCDF_ENABLE_UNIT_TESTS=ON"

# Withs
#FLAGS="$FLAGS -DPLUGIN_INSTALL_DIR=${PLUGINDIR}"
#FLAGS="$FLAGS -DPLUGIN_INSTALL_DIR=on"
FLAGS="$FLAGS -DNCPROPERTIES_EXTRA=\"key1=value1|key2=value2\""

FLAGS="$FLAGS -DOPENSSL_USE_STATIC_LIBS=TRUE"

FLAGS="$FLAGS -DCMAKE_INSTALL_PREFIX=${PREFIX}"

rm -fr build
mkdir build
cd build
BPATH=`pwd`

NCWD=`pwd`

if test "x$VS" != x ; then

NCLIB=`cygpath -w ${NCWD}/liblib |tr -d ''`
NCCYGLIB=`cygpath -u ${NCLIB} |tr -d ''`
NCCBIN=`cygpath -u "${NCC}/bin" |tr -d ''`
NCCLIB=`cygpath -u "${NCC}/lib" |tr -d ''`
# If using cygwin then must use /cygdrive
AWSSDKBIN="/cygdrive/c/tools/aws-sdk-cpp/bin"

# Visual Studio
PATH="${NCCBIN}:$PATH:$NCCYGLIB"
if test "x$S3" != x && test "x$S3I" == x ; then
PATH="$PATH:${AWSSDKBIN}"
fi

#LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${NCCYGLIB}:${NCCLIB}:${NCCBIN}:${AWSSDKBIN}"
#PATH="${PATH}:${LD_LIBRARY_PATH}"

export PATH
export LD_LIBRARY_PATH

CFG="Release"
#CFG="Debug"
CCFG="--config ${CFG}"
#mkdir -p ${CFG}

#G=
#TR=--trace-expand
#VERB="-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"
#VERB="${VERB} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
#VERB="--verbose"

BBPATH="-B${BPATH}"

set -x
cmake ${TR} ${VERB} ${FLAGS} .. 

if test "x$NOBUILD" = x ; then
echo '>>> Building=YES'
cmake --build . --config Release --target ALL_BUILD
if test "x$NOTEST" = x ; then
echo '>>> Testing=YES'
ctest
else
echo '>>> Testing=NO'
fi
else
echo '>>> Building=NO'
fi
printenv >ev1

else

# GCC
NCLIB="${NCWD}/build/liblib"
G="-GUnix Makefiles"
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
