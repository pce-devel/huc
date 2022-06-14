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
 "SHA1(../examples/huc/acd/ac_test.iso)= d2941fd5ace435a74fb83f7d170f0a95dff35033"
 
test_file ../examples/huc/overlay/overlay.iso \
 "SHA1(../examples/huc/overlay/overlay.iso)= 8c05afb5c50ad835a371bfe9bb2f0cbfd32951ae"
 
test_file ../examples/huc/promotion/promotion.pce \
 "SHA1(../examples/huc/promotion/promotion.pce)= c90902e93cfc9d15a9d0b3991f6152a6fc085abb"
 
test_file ../examples/huc/scroll/scroll.pce \
 "SHA1(../examples/huc/scroll/scroll.pce)= 14d16f5e1d3bab981b659f930b95857cbb3f85c8"
 
test_file ../examples/huc/sgx/sgx_test.iso \
 "SHA1(../examples/huc/sgx/sgx_test.iso)= b1a54443cad932b7ff4ac0a6112cbe68ffb9a79e"
 
test_file ../examples/huc/sgx/sgx_test.sgx \
 "SHA1(../examples/huc/sgx/sgx_test.sgx)= 4a4d861be0489f3c58e464accbed69e59036355b"
 
test_file ../examples/huc/shmup/shmup.iso \
 "SHA1(../examples/huc/shmup/shmup.iso)= 33451cbb41b6896aec4ecc17c2a13c1500b4c548"
 
test_file ../examples/huc/shmup/shmup.pce \
 "SHA1(../examples/huc/shmup/shmup.pce)= bd897f4e1c9deba94c9d9c9171b37dbd28736bb1"
 
exit $result
