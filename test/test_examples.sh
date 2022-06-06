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
 "SHA1(../examples/huc/acd/ac_test.iso)= 8e69b870dca009afdd5a0b7e1bc1517d08377510"

test_file ../examples/huc/overlay/overlay.iso \
 "SHA1(../examples/huc/overlay/overlay.iso)= a91dd4e7ddc8362995f77a6d6ad6e34b81628a75"

test_file ../examples/huc/promotion/promotion.pce \
 "SHA1(../examples/huc/promotion/promotion.pce)= 965bf4dcc434c2c145f3a82e33f5abeda93420dc"

test_file ../examples/huc/scroll/scroll.pce \
 "SHA1(../examples/huc/scroll/scroll.pce)= 8a4e7c910e7f4901415e06654f1f4cbfb2e52ce3"

test_file ../examples/huc/sgx/sgx_test.iso \
 "SHA1(../examples/huc/sgx/sgx_test.iso)= 30d45b601cf0e4bfb0c1af81801b3ca8b29454c2"

test_file ../examples/huc/sgx/sgx_test.sgx \
 "SHA1(../examples/huc/sgx/sgx_test.sgx)= 132833d582fa72fc36ef1b1ef295678ba1fba05f"

test_file ../examples/huc/shmup/shmup.iso \
 "SHA1(../examples/huc/shmup/shmup.iso)= 6ca2cf2cb57decc7f230ba1e995ac279a332d47a"

test_file ../examples/huc/shmup/shmup.pce \
 "SHA1(../examples/huc/shmup/shmup.pce)= 0871470fd364a80d4aa78076bd7dc8db1fbd1007"

exit $result
