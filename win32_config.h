/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* default file chunk cache nelems. */
#define CHUNK_CACHE_NELEMS 1009

/* default file chunk cache preemption policy. */
#define CHUNK_CACHE_PREEMPTION 0.75

/* default file chunk cache size in bytes. */
#define CHUNK_CACHE_SIZE 4194304

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* num chunks in default per-var chunk cache. */
#define DEFAULT_CHUNKS_IN_CACHE 10

/* default chunk size in bytes */
#define DEFAULT_CHUNK_SIZE 4194304

/* set this only when building a DLL under MinGW */
/* #undef DLL_NETCDF */

/* if true, build DAP Client */
/*#define ENABLE_DAP 1 */

/* if true, do remote tests */
#define ENABLE_DAP_REMOTE_TESTS 1

/* if true, run extra tests which may not work yet */
/* #undef EXTRA_TESTS */

/* use HDF5 1.6 API */
/* #undef H5_USE_16_API */

/* Define to 1 if you have `alloca', as a function or macro. */
#if !defined(_WIN32)
#define HAVE_ALLOCA 1
#endif

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#define HAVE_ALLOCA_H 1

/* Define to 1 if you have the <ctype.h> header file. */
#define HAVE_CTYPE_H 1

/* Is CURLOPT_KEYPASSWD defined */
#define HAVE_CURLOPT_KEYPASSWD 1

/* Define to 1 if you have the declaration of `isfinite', and to 0 if you
   don't. */
#define HAVE_DECL_ISFINITE 0

/* Define to 1 if you have the declaration of `isinf', and to 0 if you don't.
   */
#define HAVE_DECL_ISINF 1

/* Define to 1 if you have the declaration of `isnan', and to 0 if you don't.
   */
#define HAVE_DECL_ISNAN 1

/* Define to 1 if you have the declaration of `signbit', and to 0 if you
   don't. */
#define HAVE_DECL_SIGNBIT 0

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `fsync' function. */
#define HAVE_FSYNC 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define to 1 if you have the `getrlimit' function. */
#define HAVE_GETRLIMIT 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `H5Pget_fapl_mpio' function. */
/* #undef HAVE_H5PGET_FAPL_MPIO */

/* Define to 1 if you have the `H5Pget_fapl_mpiposix' function. */
/* #undef HAVE_H5PGET_FAPL_MPIPOSIX */

/* Define to 1 if you have the `H5Pset_deflate' function. */
/* #undef HAVE_H5PSET_DEFLATE */

/* Define to 1 if you have the `H5Z_SZIP' function. */
/* #undef HAVE_H5Z_SZIP */

/* Define to 1 if you have the <hdf5.h> header file. */
/* #undef HAVE_HDF5_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#if !defined(vxWorks) && !defined(_WIN32)
#define HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `curl' library (-lcurl). */
#define HAVE_LIBCURL 1

/* Define to 1 if you have the `df' library (-ldf). */
/* #undef HAVE_LIBDF */

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `mfhdf' library (-lmfhdf). */
/* #undef HAVE_LIBMFHDF */

/* Define to 1 if you have the `pnetcdf' library (-lpnetcdf). */
/* #undef HAVE_LIBPNETCDF */

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if the system has the type `longlong'. */
/* #undef HAVE_LONGLONG */

/* Define to 1 if the system has the type `long long int'. */
/* #undef HAVE_LONG_LONG_INT */

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <mfhdf.h> header file. */
/* #undef HAVE_MFHDF_H */

/* Define to 1 if you have the `mktemp' function. */
#define HAVE_MKTEMP 1

/* Define to 1 if you have the `MPI_Comm_f2c' function. */
/* #undef HAVE_MPI_COMM_F2C */

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if the system has the type `ptrdiff_t'. */
#define HAVE_PTRDIFF_T 1

/* no rpc/types.h */
#define HAVE_RPC_TYPES_H /**/

/* no rpc/xdr.h */
#define HAVE_RPC_XDR_H /**/

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if the system has the type `ssize_t'. */
#if !defined(_WIN32)
#define HAVE_SSIZE_T 1
#endif

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if stdbool.h conforms to C99. */
#if !defined(vxWorks) && !defined(_WIN32)
#define HAVE_STDBOOL_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#if !defined(vxWorks) && !defined(_WIN32)
#define HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <sys/param.h> header file. */
#if !defined(vxWorks) && !defined(_WIN32)
#define HAVE_SYS_PARAM_H 1
#endif

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strcat' function. */
#define HAVE_STRCAT 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strcpy' function. */
#define HAVE_STRCPY 1

/* Define to 1 if you have the `strdup' function. */
#if !defined(vxWorks)
#define HAVE_STRDUP 1
#endif

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strrchr' function. */
#define HAVE_STRRCHR 1

/* Define to 1 if you have the `strtod' function. */
#define HAVE_STRTOD 1

/* Define to 1 if you have the `strtoll' function. */
#define HAVE_STRTOLL 1

/* Define to 1 if you have the `strtoull' function. */
#define HAVE_STRTOULL 1

/* Define to 1 if `st_blksize' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_BLKSIZE 1

/* Define to 1 if your `struct stat' has `st_blksize'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLKSIZE' instead. */
#if !defined(_WIN32)
#define HAVE_ST_BLKSIZE 1
#endif

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if the system has the type `uchar'. */
/* #undef HAVE_UCHAR */

/* Define to 1 if you have the <unistd.h> header file. */
#if !defined(_WIN32)
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if the system has the type `unsigned long long int'. */
/* #undef HAVE_UNSIGNED_LONG_LONG_INT */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if the system has the type `_Bool'. */
#if !defined(vxWorks) && !defined(_WIN32)
#define HAVE__BOOL 1
#endif

/* do large file tests */
/* #undef LARGE_FILE_TESTS */

/* If true, turn on logging. */
/* #undef LOGGING */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* max size of the default per-var chunk cache. */
#define MAX_DEFAULT_CACHE_SIZE 67108864

/* type definition */
#define NCBYTE_T byte

/* min blocksize for posixio. */
#define NCIO_MINBLOCKSIZE 256

/* type definition */
#define NCSHORT_T integer*2

/* default */
#define NF_DOUBLEPRECISION_IS_C_DOUBLE 1

/* default */
#define NF_INT1_IS_C_SIGNED_CHAR 1

/* type thing */
#define NF_INT1_T byte

/* default */
#define NF_INT2_IS_C_SHORT 1

/* type thing */
#define NF_INT2_T integer*2

/* default */
#define NF_INT_IS_C_INT 1

/* default */
#define NF_REAL_IS_C_FLOAT 1

/* no IEEE float on this platform */
/* #undef NO_IEEE_FLOAT */

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* do not build the netCDF version 2 API */
/* #undef NO_NETCDF_2 */

/* no stdlib.h */
/* #undef NO_STDLIB_H */

/* no sys_types.h */
/* #undef NO_SYS_TYPES_H */

/* Name of package */
#define PACKAGE "netcdf"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "support-netcdf@unidata.ucar.edu"

/* Define to the full name of this package. */
#define PACKAGE_NAME "netCDF"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "netCDF 4.1.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "netcdf"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.1.3"

/* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `off_t', as computed by sizeof. */
#if defined(vxWorks)
  #define SIZEOF_OFF_T 4
#elif defined(_WIN32)
  #include <io.h>
  #include <process.h>
  #define SIZEOF_OFF_T 8
  #define lseek _lseeki64
  #define off_t __int64
  #define _off_t __int64
  #define _OFF_T_DEFINED
#else
  #define SIZEOF_OFF_T 8
#endif

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 4

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Place to put very large netCDF test files. */
#define TEMP_LARGE "."

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* if true, build CDMREMOTE Client */
/* #undef USE_CDMREMOTE */

/* if true, build DAP Client */
/*#define USE_DAP 1 */

/* set this to use extreme numbers in tests */
#define USE_EXTREME_NUMBERS 1

/* if true, include experimental fsync code */
#if !defined(vxWorks) && !defined(_WIN32)
#define USE_FSYNC 1
#endif

/* if true, use HDF4 too */
/* #undef USE_HDF4 */

/* If true, use use wget to fetch some sample HDF4 data, and then test against
   it. */
/* #undef USE_HDF4_FILE_TESTS */

/* if true, build netCDF-4 */
/* #undef USE_NETCDF4 */

/* if true, parallel netCDF-4 is in use */
/* #undef USE_PARALLEL */

/* if true, compile in parallel netCDF-4 based on MPI/IO */
/* #undef USE_PARALLEL_MPIO */

/* if true, compile in parallel netCDF-4 based on MPI/POSIX */
/* #undef USE_PARALLEL_POSIX */

/* if true, parallel netCDF is used */
/* #undef USE_PNETCDF */

/* if true, compile in szip compression in netCDF-4 variables */
/* #undef USE_SZIP */

/* if true, compile in zlib compression in netCDF-4 variables */
/* #undef USE_ZLIB */

/* Version number of package */
#define VERSION "4.1.3"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Turned on by netCDF configure. */
/* #undef f2cFortran */

/* Turned on by netCDF configure. */
/* #undef gFortran */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Turned on by netCDF configure. */
#define pgiFortran 1

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
