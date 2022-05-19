// **************************************************************************
// **************************************************************************
//
// riffio.h
//
// Simple memory-mapped replacements for Win32's mmioXXX RIFF libraries.
//
// Copyright John Brandwood 1997-2021.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __RIFFIO_h
#define __RIFFIO_h

#include "elmer.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// GLOBAL DATA STRUCTURES AND DEFINITIONS
//

#ifndef SEEK_SET
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#endif

#define OPEN_RDONLY     0
#define OPEN_WRONLY     1
#define OPEN_RDWR       2
#define OPEN_ACCMODE    (OPEN_RDONLY | OPEN_WRONLY | OPEN_RDWR)

#define CREATE_RIFF     1
#define CREATE_LIST     2

#define FIND_RIFF       1
#define FIND_LIST       2
#define FIND_CHUNK      4

#define ERROR_CHUNKNOTFOUND -256
#define ERROR_CANNOTSEEK    -257
#define ERROR_CANNOTWRITE   -258

typedef uint32_t RIFF_ID; // 4 ASCII characters

#define makeID(ch0, ch1, ch2, ch3) \
  ((uint32_t)(uint8_t)(ch0)        | \
  ((uint32_t)(uint8_t)(ch1) <<  8) | \
  ((uint32_t)(uint8_t)(ch2) << 16) | \
  ((uint32_t)(uint8_t)(ch3) << 24 ))

#define ID_RIFF makeID('R', 'I', 'F', 'F')
#define ID_LIST makeID('L', 'I', 'S', 'T')

//
// Important structures within WAV files.
//

#define WAV_TAG_PCM       0x0001
#define WAV_TAG_OKI_ADPCM 0x0010

typedef struct {
  uint16_t      uFormatTag;
  uint16_t      uNumChannels;
  uint32_t      uSamplesPerSec;
  uint32_t      uAvgBytesPerSec;
  uint16_t      uBlockAlign;
  uint16_t      uBitsPerSample;
} FRMT_PCM;

typedef struct {
  uint16_t      uFormatTag;
  uint16_t      uNumChannels;
  uint32_t      uSamplesPerSec;
  uint32_t      uAvgBytesPerSec;
  uint16_t      uBlockAlign;
  uint16_t      uBitsPerSample;
  uint16_t      uExtraInfo;
} FRMT_EXTEND;

typedef struct {
  FRMT_EXTEND   cExtended;
  uint16_t      uSamplesPerBlock;
  uint16_t      uNumCoef;
} FRMT_ADPCM;

//
// Structures for memory-mapped RIFF files.
//

typedef struct {
  uint8_t *     pBuf;
  uint8_t *     pCur;
  uint8_t *     pMrk;
  uint8_t *     pEnd;
  uint32_t      uFlg;
  char          aNam [256 + 8];
} RIFF_FILE;

typedef struct {
  uint32_t      uChunkID;       // chunk ID
  uint32_t      uChunkSize;     // chunk size
  uint32_t      uChunkType;     // form type or list type
  uint32_t      uDataOffset;    // offset of data portion of chunk
  bool          bDirty;         // has this been written to?
} CHUNK_INFO;

//
// GLOBAL FUNCTION PROTOTYPES
//

extern RIFF_FILE * riffOpen (
  const char * pFileName, unsigned uFlags );

extern int riffClose (
  RIFF_FILE * pRIFF );

extern long riffSeek (
  RIFF_FILE * pRIFF, long lOffset, int iOrigin );

extern long riffRead (
  RIFF_FILE * pRIFF, void *pBuffer, long iLength );

extern long riffWrite (
  RIFF_FILE * pRIFF, const void *pBuffer, long iLength );

extern int riffDescend (
  RIFF_FILE * pRIFF, CHUNK_INFO *pChunk, const CHUNK_INFO *pParent, unsigned uFlags );

extern int riffAscend (
  RIFF_FILE * pRIFF, CHUNK_INFO *pChunk );

extern int riffCreateChunk (
  RIFF_FILE * pRIFF, CHUNK_INFO *pChunk, unsigned uFlags );

//
// End of __RIFFIO_h
//

#ifdef __cplusplus
}
#endif

#endif
