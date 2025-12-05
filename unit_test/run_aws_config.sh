#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

#CMD="valgrind --leak-check=full"

isolate "testdir_aws_config"

THISDIR=`pwd`
cd $ISOPATH

mkdir -p $THISDIR/.aws/

test_cleanup() {
    rm -rfv $THISDIR/.aws/
}
trap test_cleanup EXIT

test_cleanup

mkdir -p $THISDIR/.aws

cat << 'EOF' > $THISDIR/.aws/config
[uni]
region = somewhere-1
endpoint_url = https://example.com/bucket/prefix/1
key = value
extrakey = willbepropagated

[profile unidata]
region = us-east-1
endpoint_url = https://s3.example.domain/
dummy_key = dummy_value

[profile play]
region = us-east-1
endpoint_url = https://endpoint.example.com/
EOF

cat << 'EOF' > $THISDIR/.aws/credentials
[uni]
region = somewhere-2
endpoint_url = https://example.com/bucket/prefix/2
key = value-overwritten

[play] 
aws_access_key_id = DummyKeys
aws_secret_access_key = DummySecret
EOF

echo -e "Testing loading AWS configuration in ${THISDIR}/.aws/config"
export NC_TEST_AWS_DIR=${THISDIR}
export AWS_PROFILE=unidata
${CMD} ${execdir}/aws_config  endpoint_url region dummy_key
echo "Status: $?"
export AWS_PROFILE=play
${CMD} ${execdir}/aws_config  endpoint_url region 
echo "Status: $?"

export AWS_PROFILE=uni
${CMD} ${execdir}/aws_config  endpoint_url region key 
echo "Status: $?"

export AWS_PROFILE=uni
${CMD} ${execdir}/aws_config key=value-overwritten region=somewhere-2 endpoint_url=https://example.com/bucket/prefix/2 extrakey=willbepropagated
echo "Status: $?"

# Will use profile=no
unset AWS_PROFILE
${CMD} ${execdir}/aws_config 2>&1 | grep -q 'Active profile:no'
echo "Status: $?"
echo -e "Finished"

exit

