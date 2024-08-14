#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

#CMD="valgrind --leak-check=full"

isolate "testdir_ut_aws_config"

THISDIR=`pwd`
cd $ISOPATH

mkdir -p $THISDIR/.aws/
cat << 'EOF' > $THISDIR/.aws/config
[uni]
region = somewhere-1
endpoint_url = https://example.com/bucket/prefix/1
key = value
extrakey = willbepropagated

[profile unidata]
region = us-east-1
endpoint_url = https://play.min.io/
dummy_key = dummy_value

[profile play]
region = us-east-1
endpoint_url = https://play.min.io/
EOF

cat << 'EOF' > $THISDIR/.aws/credentials
[play] 
aws_access_key_id = DummyKeys
aws_secret_access_key = DummySecret

[uni]
region = somewhere-2
endpoint_url = https://example.com/bucket/prefix/2
key = value2
EOF

echo -e "Testing loading AWS configuration in ${THISDIR}/.aws/config"
NC_TEST_AWS_DIR=${THISDIR} AWS_PROFILE=unidata ${CMD} ${execdir}/aws_config  endpoint_url region dummy_key
echo "Status: $?"

NC_TEST_AWS_DIR=${THISDIR} AWS_PROFILE=play ${CMD} ${execdir}/aws_config  endpoint_url region 
echo "Status: $?"

NC_TEST_AWS_DIR=${THISDIR} AWS_PROFILE=uni ${CMD} ${execdir}/aws_config  endpoint_url region key 
echo "Status: $?"

NC_TEST_AWS_DIR=${THISDIR} AWS_PROFILE=uni ${CMD} ${execdir}/aws_config region=somewhere-2 endpoint_url=https://example.com/bucket/prefix/2 key=value2 extrakey=willbepropagated
echo "Status: $?"

echo ${CMD} ${execdir}/aws_config
${CMD} ${execdir}/aws_config
echo "Status: $? [this is expected]"
echo -e "Finished"

exit

