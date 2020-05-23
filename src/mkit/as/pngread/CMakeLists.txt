set(pngread_SRCS
    adler32.c  inflate.c   pngerror.c  pngread.c   pngrutil.c  zutil.c
    crc32.c    inftrees.c  pngget.c    pngrio.c    pngset.c
    inffast.c  png.c       pngmem.c    pngrtran.c  pngtrans.c
)
add_library(pngread STATIC ${pngread_SRCS})
target_compile_definitions(pngread PUBLIC PNG_USER_CONFIG NO_GZCOMPRESS Z_SOLO NO_GZIP)

