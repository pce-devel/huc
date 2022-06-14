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
echo Checking compiled example projects against known-good SHA1 values ...
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


echo ''
echo Checking compiled example projects against known-good SHA1 values ...
echo ''
openssl sha1 ../examples/huc/acd/ac_test.iso >> test_examples.sh
openssl sha1 ../examples/huc/overlay/overlay.iso >> test_examples.sh
openssl sha1 ../examples/huc/promotion/promotion.pce >> test_examples.sh
openssl sha1 ../examples/huc/scroll/scroll.pce >> test_examples.sh
openssl sha1 ../examples/huc/sgx/sgx_test.iso >> test_examples.sh
openssl sha1 ../examples/huc/sgx/sgx_test.sgx >> test_examples.sh
openssl sha1 ../examples/huc/shmup/shmup.iso >> test_examples.sh
openssl sha1 ../examples/huc/shmup/shmup.pce >> test_examples.sh

exit $result
