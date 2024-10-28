#!/bin/sh

result=0

test_file()
{
hash=`openssl sha1 $1`

if [ "$hash" = "$2" ] ; then
  echo File: $1 PASS
else
  echo File: $1 FAIL!
  result=1
fi
}

echo ''
echo Checking compiled example projects against known-good SHA1 values ...
echo ''

test_file ../examples/huc/acd/ac_test.iso \
 "SHA1(../examples/huc/acd/ac_test.iso)= 53294fcffdb77644d452b5ad089780cf4cc76e20"

test_file ../examples/huc/overlay/overlay.iso \
 "SHA1(../examples/huc/overlay/overlay.iso)= 40881bbfbe72b81e3fe80fe4e68de6b518ec2830"

test_file ../examples/huc/promotion/promotion.pce \
 "SHA1(../examples/huc/promotion/promotion.pce)= 218cefc1258f566ebd581e7d192797f667e487da"

test_file ../examples/huc/scroll/scroll.pce \
 "SHA1(../examples/huc/scroll/scroll.pce)= d343baa74c256fc0b1150b80735fc0fce7d1a893"

test_file ../examples/huc/sgx/sgx_test.iso \
 "SHA1(../examples/huc/sgx/sgx_test.iso)= b82e6968d128fb8afbfcf97e77c7ba74b85d498d"

test_file ../examples/huc/sgx/sgx_test.sgx \
 "SHA1(../examples/huc/sgx/sgx_test.sgx)= e0c867332b0595ea604ba6a30076a5e670aa245d"

test_file ../examples/huc/shmup/shmup.iso \
 "SHA1(../examples/huc/shmup/shmup.iso)= 4247e9e2b03d8007cbb35053a942affb1bf55d65"

test_file ../examples/huc/shmup/shmup.pce \
 "SHA1(../examples/huc/shmup/shmup.pce)= 00bf39ed207ba87c5c5bcaed8bb2fbe99cceb2f7"

exit $result
