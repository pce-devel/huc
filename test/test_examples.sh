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
 "SHA1(../examples/huc/acd/ac_test.iso)= 7a9c268dcb8d2ef982d5b7918ff66df9acddd282"

test_file ../examples/huc/overlay/overlay.iso \
 "SHA1(../examples/huc/overlay/overlay.iso)= 5c5ae8951e7d48a2aba70712a159bf26fda47d4b"

test_file ../examples/huc/promotion/promotion.pce \
 "SHA1(../examples/huc/promotion/promotion.pce)= 965bf4dcc434c2c145f3a82e33f5abeda93420dc"

test_file ../examples/huc/scroll/scroll.pce \
 "SHA1(../examples/huc/scroll/scroll.pce)= 8a4e7c910e7f4901415e06654f1f4cbfb2e52ce3"

test_file ../examples/huc/sgx/sgx_test.iso \
 "SHA1(../examples/huc/sgx/sgx_test.iso)= 8be72e6d43025f207a07ea9cb18bc189026d0fc9"

test_file ../examples/huc/sgx/sgx_test.sgx \
 "SHA1(../examples/huc/sgx/sgx_test.sgx)= 49d3804cbfd252cdd98065233ac37a2d38a005c3"

test_file ../examples/huc/shmup/shmup.iso \
 "SHA1(../examples/huc/shmup/shmup.iso)= 8738e745b3fb7f3941f653d69c4a6d2dbb3a8e64"

test_file ../examples/huc/shmup/shmup.pce \
 "SHA1(../examples/huc/shmup/shmup.pce)= ecea56f2dd564de06d7f1fc9f514bde0aff71990"

exit $result
