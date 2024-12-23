## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005-2018, see the COPYRIGHT file for more information.

# This is the main automake file for netCDF. It builds the different
# netcdf directories. Not all directories are built, depending on the
# options selected during configure.

# Ed Hartnett, Ward Fisher

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# This directory stores libtool macros, put there by aclocal.
ACLOCAL_AMFLAGS = -I m4

TESTS_ENVIRONMENT =
TEST_EXTENSIONS = .sh
#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT += export SETX=1;

##
# Turn off plugin directory during distcheck, see
# comment thread at https://github.com/Unidata/netcdf-c/pull/2348
# for more information.
##
AM_DISTCHECK_CONFIGURE_FLAGS = --without-plugin-dir

# These files get added to the distribution.
EXTRA_DIST = README.md COPYRIGHT INSTALL.md test_prog.c lib_flags.am	\
cmake CMakeLists.txt COMPILE.cmake.txt config.h.cmake.in		\
cmake_uninstall.cmake.in FixBundle.cmake.in nc-config.cmake.in		\
RELEASE_NOTES.md CTestConfig.cmake.in CTestCustom.cmake.in		\
libnetcdf.settings.in netCDFConfig.cmake.in CMakeInstallation.cmake	\
test-driver-verbose test_common.in fuzz CITATION.cff

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = netcdf.pc

# Does the user want to build and run unit tests?
if BUILD_UNIT_TESTS
UNIT_TEST = unit_test
endif # BUILD_UNIT_TESTS

# Does the user want to build the V2 API?
if BUILD_V2
V2_TEST = nctest
endif

# Does the user want to build ncgen/ncdump?
if NETCDF_BUILD_UTILITIES
NCGEN3 = ncgen3
NCGEN = ncgen
NCDUMP = ncdump
endif

# Does the user want to build the examples?
if BUILD_EXAMPLES
EXAMPLES = examples
endif

# Is the user building netCDF-4?
if USE_NETCDF4
LIBSRC4_DIR = libsrc4
endif

# Is the user building with HDF5?
# Note that USE_HDF5 does not imply USE_NETCDF4
if USE_HDF5
H5_TEST_DIR = h5_test
LIBHDF5 = libhdf5
if USE_HDF5
NC_TEST4 = nc_test4
endif
endif

# Build the dap2 client
if NETCDF_ENABLE_DAP
OCLIB = oc2
DAP2 = libdap2
NCDAP2TESTDIR = ncdap_test
endif

if NETCDF_ENABLE_DAP4
DAP4 = libdap4
NCDAP4TESTDIR = dap4_test
XML = libncxml
endif #DAP4

if NETCDF_ENABLE_S3_INTERNAL
XML = libncxml # Internal S3 requires XML
endif #NETCDF_ENABLE_S3_INTERNAL

# Build PnetCDF
if USE_PNETCDF
LIBSRCP = libsrcp
endif

# Build HDF4 if desired.
if USE_HDF4
HDF4_TEST_DIR = hdf4_test
LIBHDF4 = libhdf4
endif

# Build Plugin support
if NETCDF_ENABLE_PLUGINS
NCPOCO = libncpoco
endif

# Build Cloud Storage if desired.
if NETCDF_ENABLE_NCZARR
ZARR_TEST_DIRS = nczarr_test
ZARR_TEST_DIRS += v3_nczarr_test
ZARR = libnczarr
endif

# Optionally build test plugins
if NETCDF_ENABLE_PLUGINS
PLUGIN_DIR = plugins
endif

# If benchmarks were turned on, build and run a bunch more tests.
if BUILD_BENCHMARKS
BENCHMARKS_DIR = nc_perf
endif # BUILD_BENCHMARKS

# Define Test directories
if BUILD_TESTSETS
TESTDIRS = $(H5_TEST_DIR)
TESTDIRS += $(UNIT_TEST) $(V2_TEST) nc_test $(NC_TEST4)
TESTDIRS += $(BENCHMARKS_DIR) $(HDF4_TEST_DIR) $(NCDAP2TESTDIR) $(NCDAP4TESTDIR)
TESTDIRS += ${ZARR_TEST_DIRS}
endif

# This is the list of subdirs for which Makefiles will be constructed
# and run. ncgen must come before ncdump, because their tests
# depend on it.
SUBDIRS = include ${XML} libdispatch libsrc $(LIBSRC4_DIR)	\
$(LIBSRCP) $(LIBHDF4) $(LIBHDF5) $(OCLIB) $(DAP2) ${DAP4} \
${NCPOCO} ${ZARR} liblib \
$(NCGEN3) $(NCGEN) $(NCDUMP) ${PLUGIN_DIR} $(TESTDIRS) docs	\
$(EXAMPLES)

# Remove these generated files, for a distclean.
DISTCLEANFILES = VERSION comps.txt test_prog libnetcdf.settings	\
test_common.sh

# The nc-config script helps the user build programs with netCDF.
bin_SCRIPTS = nc-config

# install libnetcdf.settings in lib directory.
settingsdir = $(libdir)
settings_DATA = libnetcdf.settings

####
# Provide an entry to rebuild all the m4 generated files
# List of files to create: WARNING leave off the extension
####

MM4= ./nc_test/test_put ./nc_test/test_get ./nc_test/test_write		\
     ./nc_test/test_read ./libsrc/netcdf ./libsrc/putget ./libsrc/ncx	\
     ./libsrc/t_ncxx ./libsrc/attr

mm4::
	for m in ${MM4} ; do \
	  b=`basename $$m` ; d=`dirname $$m`;\
	  pushd $$d; m4 -s $${b}.m4 > $${b}.c ; popd; done

check_nc_config:
	$(CC) `./nc-config --cflags` test_prog.c -o test_prog `./nc-config --libs`
	./test_prog

install-data-hook:

	@echo ''
	@echo '+-------------------------------------------------------------+'
	@echo '| Congratulations! You have successfully installed netCDF!    |'
	@echo '|                                                             |'
	@echo '| You can use script "nc-config" to find out the relevant     |'
	@echo '| compiler options to build your application. Enter           |'
	@echo '|                                                             |'
	@echo '|     nc-config --help                                        |'
	@echo '|                                                             |'
	@echo '| for additional information.                                 |'
	@echo '|                                                             |'
	@echo '| CAUTION:                                                    |'
	@echo '|                                                             |'
	@echo '| If you have not already run "make check", then we strongly  |'
	@echo '| recommend you do so. It does not take very long.            |'
	@echo '|                                                             |'
	@echo '| Before using netCDF to store important data, test your      |'
	@echo '| build with "make check".                                    |'
	@echo '|                                                             |'
	@echo '| NetCDF is tested nightly on many platforms at Unidata       |'
	@echo '| but your platform is probably different in some ways.       |'
	@echo '|                                                             |'
	@echo '| If any tests fail, please see the netCDF web site:          |'
	@echo '| https://www.unidata.ucar.edu/software/netcdf/                |'
	@echo '|                                                             |'
	@echo '| NetCDF is developed and maintained at the Unidata Program   |'
	@echo '| Center. Unidata provides a broad array of data and software |'
	@echo '| tools for use in geoscience education and research.         |'
	@echo '| https://www.unidata.ucar.edu                                 |'
	@echo '+-------------------------------------------------------------+'
	@echo ''


# Create the VERSION file after the build
# in case it is being used by packagers
# Also track the S3 cleanup id
all-local: liblib/libnetcdf.la
	echo ${PACKAGE_VERSION} > VERSION
if NETCDF_ENABLE_S3_TESTALL
	rm -f ${abs_top_builddir}/tmp_@PLATFORMUID@.uids
	echo "@TESTUID@" >> ${abs_top_builddir}/s3cleanup_@PLATFORMUID@.uids
	cat ${abs_top_builddir}/s3cleanup_@PLATFORMUID@.uids | sort | uniq > ${abs_top_builddir}/tmp_@PLATFORMUID@.uids
	rm -f ${abs_top_builddir}/s3cleanup_@PLATFORMUID@.uids
	mv ${abs_top_builddir}/tmp_@PLATFORMUID@.uids ${abs_top_builddir}/s3cleanup_@PLATFORMUID@.uids
endif

clean-local:
	rm -fr alltests_*

if NETCDF_ENABLE_S3_TESTALL
distclean-local:
	rm -f ${abs_top_builddir}/s3cleanup_@PLATFORMUID@.uids
endif

# Remove the VERSION file
CLEANFILES = VERSION
