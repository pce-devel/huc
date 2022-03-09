*****************************************************************************
ELMER'S NOTES FOR CONFIGURING LIBPNG & ZLIB
*****************************************************************************
2019-04-14 LIBPNG version 1.6.37
2017-01-15 ZLIB   version 1.2.11
*****************************************************************************



*****************************************************************************
To configure and prepare the source for a minimal libpng reader.
*****************************************************************************

Extract the libpng and zlib distributions into the same root directory.

Open up MSYS2 or BASH (it's got to be a Unix-like environment).

 cd libpng/contrib/pngminim/decoder
 cp pngusr.dfa ../../../
 echo option BENIGN_ERRORS on >> ../../../pngusr.dfa 
 echo option READ_PACK on >> ../../../pngusr.dfa
 echo option READ_BGR on >> ../../../pngusr.dfa
 echo option READ_tRNS on >> ../../../pngusr.dfa
 make
 cp pnglibconf.h ../../../



*****************************************************************************
To configure and prepare the source for a minimal libpng writer.
*****************************************************************************

Extract the libpng and zlib distributions into the same root directory.

Open up MSYS2 or BASH (it's got to be a Unix-like environment).

 cd libpng/contrib/pngminim/encoder
 cp pngusr.dfa ../../../
 echo option WRITE_BGR on >> ../../../pngusr.dfa
 echo option WRITE_tRNS on >> ../../../pngusr.dfa
 make
 cp pnglibconf.h ../../../



*****************************************************************************
To compile minimal libpng writer. 
*****************************************************************************

Compiler flags for both reader and writer ...

 -DPNG_USER_CONFIG -DNO_GZCOMPRESS -DZ_SOLO -DNO_GZIP



*****************************************************************************
Files needed for the minimal libpng reader. 
*****************************************************************************

zlib files needed:
  adler32.c
  crc32.c
  crc32.h
  inffast.c
  inffast.h
  inffixed.h
  inflate.c
  inflate.h
  inftrees.c
  inftrees.h
  zconf.h
  zlib.h
  zutil.c
  zutil.h

libpng files needed:
  png.c
  png.h
  pngconf.h
  pngdebug.h
  pngerror.c
  pngget.c
  pnginfo.h
* pnglibconf.h (this is the only file that is changed from the original)
  pngmem.c
  pngpriv.h
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngstruct.h
  pngtrans.c
  pngusr.h



*****************************************************************************
Files needed for the minimal libpng writer.
*****************************************************************************

zlib files needed:
  adler32.c
  compress.c
  crc32.c
  crc32.h
  deflate.c
  deflate.h
  trees.c
  trees.h
  zconf.h
  zlib.h
  zutil.c
  zutil.h

libpng files needed:
  png.c
  png.h
  pngconf.h
  pngdebug.h
  pngerror.c
  pngget.c
  pnginfo.h
* pnglibconf.h (this is the only file that is changed from the original)
  pngmem.c
  pngpriv.h
  pngset.c
  pngstruct.h
  pngtrans.c
  pngusr.h
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
