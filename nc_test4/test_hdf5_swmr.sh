#!/usr/bin/env bash

# Test ability to use HDF5's SWMR functionality to simultaneously
# write to and read from a file in different processes.
#
# 1. Create file, enabling SWMR mode; create all variables
# 2. Close and reopen file for writing in SWMR mode
# 3. Then open file in a separate process for SWMR reading
#
# Note that these *must* happen sequentially -- the reader process
# *must not* start until the writer process in step 2 has started.
#
# At this point we can perform some tests on the file.

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Clean up any test files
rm -f ./test_hdf5_swmr_file.nc

echo "  *** Testing SWMR mode"

# Launch the writer process in the background
${execdir}/test_hdf5_swmr_writer &

# Pause briefly here to ensure writer process has both created file
# and then *reopened it*
sleep 0.5

# Now we're safe to launch the reader process
${execdir}/test_hdf5_swmr_reader

echo "  *** Pass: SWMR mode"
