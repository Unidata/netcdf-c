#!/bin/sh
# This shell script tests ncdump -g option for specifying groups for
# which data is to be output.
# $Id$

set -e
echo ""
echo "*** Testing ncdump -g output for specifying group subsets"
echo "*** creating netcdf file tst_grp_spec.nc from tst_grp_spec.cdl..."
../ncgen/ncgen -k3 -b tst_grp_spec.cdl
echo "*** creating tmpgs.cdl from tst_grp_spec.nc with ncdump -g ..."
./ncdump -g g1,g4 tst_grp_spec.nc > tmpgs.cdl
echo "*** comparing tmp.cdl with ref_tst_grp_spec.cdl..."
diff tmpgs.cdl ref_tst_grp_spec.cdl

echo
echo "*** All ncdump test output for -g option passed!"
exit 0
