#!/bin/sh

result=0

csum_file()
{
hash=`openssl sha1 $1`

echo "test_file $1 \\" >> test_examples.sh
echo \ \"$hash\" >> test_examples.sh
echo \ >> test_examples.sh
}

echo ''
echo Adding new SHA1 values to end of test_examples.sh, do not forget to edit it!
echo ''
csum_file ../examples/huc/acd/ac_test.iso
csum_file ../examples/huc/overlay/overlay.iso
csum_file ../examples/huc/promotion/promotion.pce
csum_file ../examples/huc/scroll/scroll.pce
csum_file ../examples/huc/sgx/sgx_test.iso
csum_file ../examples/huc/sgx/sgx_test.sgx
csum_file ../examples/huc/shmup/shmup.iso
csum_file ../examples/huc/shmup/shmup.pce

exit $result
