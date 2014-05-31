#!/bin/sh
# Run test_chunk_hdf4 passing ${src_dir}
echo ""
echo "*** Testing hdf4 chunking..."
if ./tst_chunk_hdf4 ${src_dir} ; then
  echo "***SUCCESS!! tst_chunk_hdf4"
else
  echo "***FAIL: tst_chunk_hdf4"
fi

exit 0
