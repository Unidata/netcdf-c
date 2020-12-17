#!/bin/sh
if test -f .github/workflows/run_tests.yml ; then
rm -f .github/workflows/tmp0.yml .github/workflows/tmp1.yml
cp .github/workflows/run_tests.yml .github/workflows/tmp0.yml
cat .github/workflows/tmp0.yml \
| sed -e 's/on: \[[ ]*pull_request[ ]*\]/on: [pull_request,push]/' \
| cat > .github/workflows/tmp1.yml 
rm -f .github/workflows/run_tests.yml
mv .github/workflows/tmp1.yml .github/workflows/run_tests.yml
fi
