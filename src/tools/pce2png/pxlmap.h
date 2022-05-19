// **************************************************************************
// **************************************************************************
//
// pxlmap.h
//
// Simple generic 8/24/32-bit pixelmap type and handling functions.
//
// Only the following pixel types are currently handled ...
//
//   8 bits-per-pixel, 1 plane, 256 colour palette.
//  24 bits-per-pixel, 1 plane,  no colour palette.
//  32 bits-per-pixel, 1 plane,  no colour palette.
//
// Copyright John Brandwood 1994-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __PIXELMAP_h
#define __PIXELMAP_h

#include "elmer.h"
#include "errorcode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_DATA_UNKNOWN    -0x0100L
#define ERROR_DATA_ILLEGAL    -0x0101L

// Force byte alignment.

#if _MSC_VER
#pragma pack(1)
#endif

// Definition of the PIXELMAP_T structure for a generic pixelmap type.
//
// This structure is based on the MicroSoft Windows 3.x device-independant
// bitmap file structure, and whilst the header itself is different, the
// actual data is stored in a Windows-compatible format.

typedef struct RGBQUAD_S {
  uint8_t       m_uRgbB;
  uint8_t       m_uRgbG;
  uint8_t       m_uRgbR;
  uint8_t       m_uRgbA;                // 0=solid 255=transparent
} RGBQUAD_T;

typedef struct RGBTRPL_S {
  uint8_t       m_uRgbB;
  uint8_t       m_uRgbG;
  uint8_t       m_uRgbR;
} RGBTRPL_T;

typedef struct PXLMAP_S PXLMAP_T;

struct PXLMAP_S {
  PXLMAP_T *    m_pPxmNext;
  PXLMAP_T *    m_pPxmPrev;
  size_t        m_uPxmSize;
  uint8_t *     m_pPxmPixels;           // Pointer to pixel data.
  int           m_iPxmLineSize;         // Size of a (padded) line in bytes.
  int           m_iPxmXTopLeft;         // X coord of top left corner.
  int           m_iPxmYTopLeft;         // Y coord of top left corner.
  int           m_iPxmW;                // Width  of bitmap in pixels.
  int           m_iPxmH;                // Height of bitmap in pixels.
  int           m_iPxmB;                // Bits-per-pixel (1/4/8/24/32).
  int           m_iPxmSignificantBits;  // Bits-per-pixel (1/4/8/24/32).
  int           m_iPxmPaletteHasAlpha;  // 0 = NO, 1 = YES.
  int           m_iPxmColorsInPalette;  // Number of significant colors (for a 16-color or quantized bitmap).
  RGBQUAD_T     m_aPxmC[256];           // Windows DIB color palette.
};

// Restore default alignment.

#if _MSC_VER
#pragma pack()
#endif

//
//
//

typedef void (*FuncPxmToChr) (uint8_t *pChrBufData, uint8_t *pPxmPxlmap, int uPxmLineSize, int *pPalette, int *pPriority);

//
//
//

//typedef struct CHRSET_S {

typedef struct {
  uint32_t *    m_pChrBufKeys;
  uint8_t *     m_pChrBuffer;
  uint8_t *     m_pChrBufEnd;
  int           m_iChrSizeof;
  int           m_iChrCount;
  int           m_iChrMaximum;
  int           m_iChrXPxlSize;
  int           m_iChrXPxlShift;
  int           m_iChrYPxlSize;
  int           m_iChrYPxlShift;
  int           m_iChrPxlBits;
//int           m_iChrU32Size;
//int           m_iChrU32Shift;
//int           m_iChrBytSize;
//int           m_iChrBytShift;
  FuncPxmToChr  m_pXvertPxmToChr;
} CHRSET_T;

//
//
//

typedef struct BLKSET_S {
  uint32_t *    m_pBlkBufKeys;
  uint8_t *     m_pBlkBufData;
  uint8_t *     m_pBlkBufEnd;
  int           m_iBlkCount;
  int           m_iBlkMaximum;
  int           m_iBlkXChrSize;
  int           m_iBlkYChrSize;
  int           m_iBlkChrSize;
  int           m_iBlkBytSize;
} BLKSET_T;

//
//
//

typedef struct MAPIDX_S {
  uint32_t *    m_pMapIdxBufPtr;  // NULL if blank sprite.
  long          m_iMapIdxBufLen;   // ==0 if sprite is blank/repeated.
  int           m_iMapIdxXOffset;
  int           m_iMapIdxYOffset;
  int           m_iMapIdxNumber;
  uint32_t      m_iMapIdxKeyVal;
} MAPIDX_T;

typedef struct MAPSET_S {
  PXLMAP_T      m_cHead;
  int **        m_pMapBufIndx;    // ***
  uint32_t *    m_pMapBufKeys;  // ***
  uint16_t *    m_pMapBufData;  // ***
  MAPIDX_T *    m_aMapSetBufIndx;
  uint16_t *    m_pMapSetBuf1st;
  uint16_t *    m_pMapSetBufCur;
  uint16_t *    m_pMapSetBufEnd;
  int           m_iMapSetCount;
  int           m_iMapSetMaximum;
} MAPSET_T;

//
//
//

typedef struct SPRIDX_S {
  uint8_t *     m_pSprIdxBufPtr; // NULL if blank sprite.
  long          m_iSprIdxBufLen;   // ==0 if sprite is blank/repeated.
  int           m_iSprIdxBPP;
  int           m_iSprIdxXOffset;
  int           m_iSprIdxYOffset;
  int           m_iSprIdxWidth;
  int           m_iSprIdxHeight;
  int           m_iSprIdxDisplayW;
  int           m_iSprIdxDisplayH;
  int           m_iSprIdxPalette;
  int           m_iSprIdxNumber;
  int           m_iSprIdxTextureN;
  int           m_iSprIdxTextureX;
  int           m_iSprIdxTextureY;
  uint32_t      m_iSprIdxKeyVal;
//  char        m_aSprIdxTagName [(MAX_TAG_NAME_LENGTH+1)];
} SPRIDX_T;

typedef struct SPRSET_S {
  PXLMAP_T      m_cHead;
  SPRIDX_T *    m_aSprSetBufIndx;
  uint8_t *     m_pSprSetBuf1st;
  uint8_t *     m_pSprSetBufCur;
  uint8_t *     m_pSprSetBufEnd;
  int           m_iSprSetCount;
  int           m_iSprSetMaximum;
} SPRSET_T;

//
//
//

typedef struct FNTIDX_S {
  uint8_t *     m_pFntIdxBufPtr; // NULL if blank sprite.
  long          m_iFntIdxBufLen;   // ==0 if sprite is blank/repeated.
  int           m_iFntIdxXOffset;
  int           m_iFntIdxYOffset;
  int           m_iFntIdxWidth;
  int           m_iFntIdxHeight;
  int           m_iFntIdxDeltaW;
  int           m_iFntIdxTextureN;
  int           m_iFntIdxTextureX;
  int           m_iFntIdxTextureY;
  int           m_iFntIdxUnicodeN;
} FNTIDX_T;

typedef struct KERNPAIR_S {
  uint16_t      m_iChr0;
  uint16_t      m_iChr1;
  int16_t       m_iDeltaX;
  int16_t       m_iPadding;
} KERNPAIR_T;

typedef struct FNTSET_S {
  PXLMAP_T      head;
  FNTIDX_T *    m_aFntSetBufIndx;
  uint8_t *     m_pFntSetBuf1st;
  uint8_t *     m_pFntSetBufCur;
  uint8_t *     m_pFntSetBufEnd;
  int           m_iFntSetCount;
  int           m_iFntSetMaximum;
  KERNPAIR_T *  m_aFntSetKrnTbl;
  int           m_iFntSetKrnCnt;
  int           m_iFntSetKrnMax;

//int           m_iFntSetChr0;
  int           m_iFntSetSpcW;
  int           m_iFntSetXSpc;
  int           m_iFntSetYSpc;
  int           m_iFntSetXLft;
  int           m_iFntSetXRgt;
  int           m_iFntSetYTop;
  int           m_iFntSetYBtm;
  int           m_iFntSetYCap;
  int           m_iFntSetYOvr;
} FNTSET_T;

//
//
//

typedef struct PALIDX_S {
  uint8_t *     m_pPalIdxBufPtr; // NULL if blank sprite.
  long          m_iPalIdxBufLen;   // ==0 if sprite is blank/repeated.
  int           m_iPalIdxMaxVal;
  int           m_iPalIdxNumber;
  uint32_t      m_iPalIdxKeyVal;
} PALIDX_T;

typedef struct PALSET_S {
  PALIDX_T *    m_aPalSetBufIndx;
  uint8_t *     m_pPalSetBuf1st;
  uint8_t *     m_pPalSetBufCur;
  uint8_t *     m_pPalSetBufEnd;
  int           m_iPalSetCount;
  int           m_iPalSetMaximum;
} PALSET_T;

//
//
//

#define ID4_GenS MakeID4('G', 'e', 'n', 'S')
#define ID4_SfxS MakeID4('S', 'f', 'x', 'S')
#define ID4_PceS MakeID4('P', 'c', 'e', 'S')
#define ID4_3doS MakeID4('3', 'd', 'o', 'S')
#define ID4_SatS MakeID4('S', 'a', 't', 'S')
#define ID4_PsxS MakeID4('P', 's', 'x', 'S')
#define ID4_IbmS MakeID4('I', 'b', 'm', 'S')
#define ID4_N64S MakeID4('N', '6', '4', 'S')
#define ID4_GcnS MakeID4('G', 'c', 'n', 'S')

#define ID4_Mxxx MakeID4('M', 'x', 'x', 'x')
#define ID4_Mgen MakeID4('M', 'g', 'e', 'n')
#define ID4_Msfx MakeID4('M', 's', 'f', 'x')
#define ID4_Mpce MakeID4('M', 'p', 'c', 'e')
#define ID4_M3do MakeID4('M', '3', 'd', 'o')
#define ID4_Msat MakeID4('M', 's', 'a', 't')
#define ID4_Mpsx MakeID4('M', 'p', 's', 'x')
#define ID4_Mibm MakeID4('M', 'i', 'b', 'm')
#define ID4_Mn64 MakeID4('M', 'n', '6', '4')

#define ID4_palI MakeID4('p', 'a', 'l', 'I')
#define ID4_palD MakeID4('p', 'a', 'l', 'D')
#define ID4_chrI MakeID4('c', 'h', 'r', 'I')
#define ID4_chrD MakeID4('c', 'h', 'r', 'D')
#define ID4_blkI MakeID4('b', 'l', 'k', 'I')
#define ID4_blkD MakeID4('b', 'l', 'k', 'D')
#define ID4_mapI MakeID4('m', 'a', 'p', 'I')
#define ID4_mapD MakeID4('m', 'a', 'p', 'D')
#define ID4_sprI MakeID4('s', 'p', 'r', 'I')
#define ID4_sprD MakeID4('s', 'p', 'r', 'D')
#define ID4_fntI MakeID4('f', 'n', 't', 'I')
#define ID4_fntD MakeID4('f', 'n', 't', 'D')

#define ID4_head MakeID4('h', 'e', 'a', 'd')
#define ID4_sect MakeID4('s', 'e', 'c', 't')

/*
typedef struct CHUNK_S {
  uint32_t      m_ckSize;
  ID4           m_ckType;
  ID4           m_ckMach;
  int32_t       m_ckFlag;
  } CHUNK_T;
*/

//
// GLOBAL VARIABLES
//

extern bool g_bFlattenAlpha;

extern uint8_t g_aRemappingTable[256];

extern PXLMAP_T *pDifferencePxlmap;
extern uint8_t uDifferenceColorB;
extern uint8_t uDifferenceColorG;
extern uint8_t uDifferenceColorR;
extern uint8_t uDifferenceColorA;
extern uint8_t uDifferenceColorI;

//
// GLOBAL FUNCTION PROTOTYPES
//

extern PXLMAP_T * PxlmapAlloc (
  int iWidth, int iHeight, int iBits, bool bClear);

extern ERRORCODE PxlmapClear (
  PXLMAP_T *pPxm );

extern ERRORCODE PxlmapCopy (
  PXLMAP_T *pPxmSrc, int iSrcBoxX, int iSrcBoxY, int iSrcBoxW, int iSrcBoxH,
  PXLMAP_T *pPxmDst, int iDstBoxX, int iDstBoxY );

extern ERRORCODE PxlmapQuarter (
  PXLMAP_T *pPxm, int iBoxX, int iBoxY, int iBoxW, int iBoxH );

extern ERRORCODE PxlmapRemap (
  PXLMAP_T *pPxm, int iBoxX, int iBoxY, int iBoxW, int iBoxH );

extern ERRORCODE PxlmapFilter (
  PXLMAP_T *pPxm, int iBoxX, int iBoxY, int iBoxW, int iBoxH, int iRangeLo, int iRangeHi );

extern ERRORCODE PxlmapBoundingBox (
  PXLMAP_T *pPxm, int *pBoxX, int *pBoxY, int *pBoxW, int *pBoxH );

extern ERRORCODE PxlmapPalettize (
  PXLMAP_T *pPxm, int iBoxX, int iBoxY, int iBoxW, int iBoxH );

extern PXLMAP_T * PxlmapDuplicate (
  PXLMAP_T *pPxm );

extern ERRORCODE PxlmapFree (
  PXLMAP_T *pPxm );

extern PXLMAP_T * PxlmapToIA4 (
  PXLMAP_T *pPxm );

extern PXLMAP_T * PxlmapFontTo4bpp (
  PXLMAP_T *pPxm );

extern ERRORCODE PxlmapFixTransparency (
  PXLMAP_T *pPxm );

extern ERRORCODE MakeAlphaRemapTable (
  PXLMAP_T *pPxm );

extern ERRORCODE PxlmapFlattenAlpha (
  PXLMAP_T *pPxm );

extern ERRORCODE PxlmapDifference (
  PXLMAP_T *pPxmDst, PXLMAP_T *pPxmSrc );

//
// End of PXLMAP_h
//

#ifdef __cplusplus
}
#endif
#endif // __PIXELMAP_h
