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
 "SHA1(../examples/huc/acd/ac_test.iso)= 593634d1374824d37d6d8466a44e4e1e644702b4"
 
test_file ../examples/huc/overlay/overlay.iso \
 "SHA1(../examples/huc/overlay/overlay.iso)= aad3c0d139aafe4846db65ab95ea8cf3b747a649"
 
test_file ../examples/huc/promotion/promotion.pce \
 "SHA1(../examples/huc/promotion/promotion.pce)= a686f00b9d30acfc980ff29dd23703e5286db435"
 
test_file ../examples/huc/scroll/scroll.pce \
 "SHA1(../examples/huc/scroll/scroll.pce)= f07b6a176453540bbbfc7c39fc8140427ca678b5"
 
test_file ../examples/huc/sgx/sgx_test.iso \
 "SHA1(../examples/huc/sgx/sgx_test.iso)= 0a60830eda7ab6bfaf211e857e0e10c1a3f6613c"
 
test_file ../examples/huc/sgx/sgx_test.sgx \
 "SHA1(../examples/huc/sgx/sgx_test.sgx)= a39e295675c077c5cbc1ae479775f4ce674fdd9e"
 
test_file ../examples/huc/shmup/shmup.iso \
 "SHA1(../examples/huc/shmup/shmup.iso)= ce4cb4252098ff684422a14fe7f54598af49bfcc"
 
test_file ../examples/huc/shmup/shmup.pce \
 "SHA1(../examples/huc/shmup/shmup.pce)= 66dcba4e063f8076608fdc7340eab5de4c65494a"
 
exit $result
