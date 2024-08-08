#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

#CMD="valgrind --leak-check=full"

isolate "testdir_ut_aws_config"

THISDIR=`pwd`
cd $ISOPATH

echo -e "Testing loading AWS configuration in ${THISDIR}/.aws/config"
env NC_TEST_AWS_DIR=${THISDIR} AWS_PROFILE=unidata ${CMD} ${execdir}/aws_config  endpoint_url region dummy_key
echo "Status: $?"

env NC_TEST_AWS_DIR=${THISDIR} AWS_PROFILE=play ${CMD} ${execdir}/aws_config  endpoint_url region 
echo "Status: $?"

env NC_TEST_AWS_DIR=${THISDIR} AWS_PROFILE=uni ${CMD} ${execdir}/aws_config  endpoint_url region key 
echo "Status: $?"

env NC_TEST_AWS_DIR=${THISDIR} AWS_PROFILE=uni ${CMD} ${execdir}/aws_config region=somewhere-2 endpoint_url=https://example.com/bucket/prefix/2 key=value2 extrakey=willbepropagated
echo "Status: $?"

echo ${CMD} ${execdir}/aws_config
${CMD} ${execdir}/aws_config
echo "Status: $? [this is expected]"
echo -e "Finished"

exit

