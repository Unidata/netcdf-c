#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

if ! command -v h5dump >/dev/null 2>&1; then
  echo "h5dump not found, skipping."
  exit 0
fi

n=0

while [ "$n" -le 4 ]; do
  echo ''

  if [ $n -eq 0 ]; then
    value=""
  else
    value=$(echo XXXXXXXXXXXXXXXXXXXXXXXXXXXX | cut -c 1-$n)
  fi

  # Create new netcdf file with character attribute of length N.
  sed -e "s/STR1/$value/" << EOF | ${NCGEN} -4 -o zero-len-attribute-test.nc
netcdf zero-len-attribute-test {
variables:
  int var1 ;

// global attributes:
    :comment = "STR1" ;
}
EOF

  echo Before:
    h5dump -a comment zero-len-attribute-test.nc \
      | grep -e STRSIZE -e DATASPACE -e \:  \
      | tr -s '\n ' ' '

    # Overwrite the character attribute with length 0.
    ./tst_zero_len_att

  echo After : 
  h5dump -a comment zero-len-attribute-test.nc \
    | grep -e STRSIZE -e DATASPACE -e \:  \
    | tr -s '\n ' ' '
  echo ''
  
  CHECKVAL=$(h5dump -a comment zero-len-attribute-test.nc \
    | grep -e STRSIZE -e DATASPACE -e \:  \
    | tr -s '\n ' ' ' | cut -d ' ' -f 4,5 | xargs)

    if [ "${CHECKVAL}" != "DATASPACE NULL" ]; then
      echo ""
      echo "ERROR: N size ${n}, expected DATASPACE NULL, received ${CHECKVAL}"
      echo "Exiting"
      echo ""
      exit -1
    fi


  n=$((n + 1))
done
