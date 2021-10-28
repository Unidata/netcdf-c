#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

export NC_TEST_AWS_DIR=`pwd`

rm -fr ./.aws
mkdir .aws
cat >.aws/config <<EOF
[default]
    aws_access_key_id=ACCESSKEYDEFAULTXXXX
    aws_secret_access_key=DEFAULT/ef0ghijklmnopqr/defaultxxxxxxxxx
[ncar]
    aws_access_key_id=ACCESSKEYNCARXXXXXXX
    aws_secret_access_key=NCAR/ef0ghijklmnopqr/ncarxxxxxxxxxxxxxxx
[unidata]
    aws_access_key_id=ACCESSKEYUNIDATAXXXX

    aws_secret_access_key=UNIDATA/ef0ghijklmnopqr/unidataxxxxxxxxx
; comment1
    aws_region=us-west-1
;comment2
EOF

${execdir}/test_aws

rm -fr .aws

