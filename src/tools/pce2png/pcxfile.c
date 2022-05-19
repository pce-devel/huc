// **************************************************************************
// **************************************************************************
//
// pcxfile.c
//
// Simple interface for reading files in Zsoft's PCX format.
//
// Limitations ...
//
// Lots.
//
// Only the following file contents are currently understood ...
//
//   1 bits-per-pixel, 4 plane,  16 colour palette.
//   8 bits-per-pixel, 1 plane, 256 colour palette.
//
// Copyright John Brandwood 1994-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#include  "elmer.h"
#include  "errorcode.h"
#include  "pxlmap.h"
#include  "mappedfile.h"
#include  "pcxfile.h"

//
// DEFINITIONS
//

// Force byte alignment for the following structures.

#pragma pack(1)

// PCX file header (128 bytes).

typedef struct {
  uint8_t       m_uFlag;
  uint8_t       m_uVersion;
  uint8_t       m_uPacked;
  uint8_t       m_uBPP;
  uint16_t      m_uXMin;
  uint16_t      m_uYMin;
  uint16_t      m_uXMax;
  uint16_t      m_uYMax;
  uint16_t      m_uXDPI;
  uint16_t      m_uYDPI;
  uint8_t       m_uPalette[48];
  uint8_t       m_uReserved;
  uint8_t       m_uPlanes;
  uint16_t      m_uBytesPerLine;
  uint16_t      m_uInterp;
  uint16_t      m_uVideoX;
  uint16_t      m_uVideoY;
  uint8_t       m_uBlank[54];
} PCX_HEADER_T;

typedef struct {
  uint8_t       m_uR;
  uint8_t       m_uG;
  uint8_t       m_uB;
} PCX_RGB_T;

typedef struct PCX_PALETTE_S {
  PCX_RGB_T     m_aRgb [256];
} PCX_PALETTE_T;

// Restore default alignment.

#pragma pack()



// **************************************************************************
// **************************************************************************
//
// ConvertFromPCX4RLE ()
//
// Convert the PCX RLE data into DIB data
//
// Inputs  PCX_HEADER_T *  Ptr to PCX header
//         uint8_t *       Ptr to start of RLE src
//         uint8_t *       Ptr to end   of RLE src
//         uint8_t *       Destination buffer
//         int             Destination buffer width
//         RGBQUAD_T *     Ptr to colour palette
//
// Output  ERRORCODE       ERROR_NONE if OK
//
// N.B.    g_iErrorCode and g_aErrorMessage are NOT set on failure.
//

static ERRORCODE ConvertFromPCX4RLE (
  PCX_HEADER_T *pHeader,
  uint8_t *     pSrcPtr,
  uint8_t *     pSrcEnd,
  uint8_t *     pDstPtr,
  int           iDstOff,
  RGBQUAD_T *   pPalette )

{
  // Local variables.

  uint8_t *pDstCol;
  int iSrcOff;
  int iDstRhs;
  int iSrcW;
  int iSrcH;
  uint8_t uByte;
  uint8_t uRept;

  int i;

  // Calculate source line width and destination RHS edge size.

  iSrcOff = pHeader->m_uBytesPerLine;
  iDstRhs = iDstOff - iSrcOff;

  // Now read in the frame.

  iSrcH = 1 + pHeader->m_uYMax - pHeader->m_uYMin;

  do {
    pDstCol = pDstPtr;
    pDstPtr += iDstOff;
    iSrcW = iSrcOff;

    do {
      uByte = *pSrcPtr++;
      if ((uByte & 0xC0u) != 0xC0u) {
        *pDstCol++ = uByte;
        iSrcW -= 1;
      }
      else {
        uRept = (uByte & 0x3Fu);
        uByte = *pSrcPtr++;
        if ((iSrcW -= uRept) < 0) goto errorExit;
        do
          *pDstCol++ = uByte;
        while (--uRept);
      }
    } while (iSrcW >= 0);

    if (iDstRhs != 0) {
      memset(pDstCol, 0, iDstRhs);
    }
    iSrcH -= 1;
  } while (iSrcH != 0);

  // Colour palette present ?

  if (pHeader->m_uVersion != 3) {
    pSrcPtr = pHeader->m_uPalette;
    for (i = 16; i != 0; i -= 1) {
      pPalette->m_uRgbA = 0;
      pPalette->m_uRgbR = pSrcPtr[0];
      pPalette->m_uRgbG = pSrcPtr[1];
      pPalette->m_uRgbB = pSrcPtr[2];
      pSrcPtr += 3;
      pPalette += 1;
    }
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorExit:

  return (~ERROR_NONE);
}



// **************************************************************************
// **************************************************************************
//
// ConvertFromPCX8RLE ()
//
// Convert the PCX RLE data into DIB data
//
// Inputs  PCX_HEADER_T *  Ptr to PCX header
//         uint8_t *       Ptr to start of RLE src
//         uint8_t *       Ptr to end   of RLE src
//         uint8_t *       Destination buffer
//         int             Destination buffer width
//         RGBQUAD_T *     Ptr to colour palette
//
// Output  ERRORCODE       ERROR_NONE if OK
//
// N.B.    g_iErrorCode and g_aErrorMessage are NOT set on failure.
//

static ERRORCODE ConvertFromPCX8RLE (
  PCX_HEADER_T *pHeader,
  uint8_t *     pSrcPtr,
  uint8_t *     pSrcEnd,
  uint8_t *     pDstPtr,
  int           iDstOff,
  RGBQUAD_T *   pPalette )

{
  // Local variables.

  uint8_t *     pDstCol;
  int           iSrcOff;
  int           iDstRhs;
  int           iSrcW;
  int           iSrcH;
  uint8_t       uByte;
  uint8_t       uRept;
  int           i;

  // Calculate source line width and destination RHS edge size.

  iSrcOff = pHeader->m_uBytesPerLine;
  iDstRhs = iDstOff - iSrcOff;

  // Now read in the frame.

  iSrcH = 1 + pHeader->m_uYMax - pHeader->m_uYMin;

  do {
    pDstCol = pDstPtr;
    pDstPtr += iDstOff;
    iSrcW = iSrcOff;

    do {
      uByte = *pSrcPtr++;
      if ((uByte & 0xC0u) != 0xC0u) {
        *pDstCol++ = uByte;
        iSrcW -= 1;
      }
      else {
        uRept = (uByte & 0x3Fu);
        uByte = *pSrcPtr++;
        if ((iSrcW -= uRept) < 0)
          goto errorExit;
        do {
          *pDstCol++ = uByte;
        } while (--uRept);
      }
    } while (iSrcW > 0);

    if (iDstRhs != 0)
      memset(pDstCol, 0, iDstRhs);
    iSrcH -= 1;
  } while (iSrcH != 0);

  // Colour palette present ?

  if (pSrcPtr == (pSrcEnd - 769)) {
    if (*pSrcPtr++ == 0x0Cu) {
      for (i = 256; i != 0; i -= 1) {
        pPalette->m_uRgbA = 0;
        pPalette->m_uRgbR = pSrcPtr[0];
        pPalette->m_uRgbG = pSrcPtr[1];
        pPalette->m_uRgbB = pSrcPtr[2];
        pSrcPtr += 3;
        pPalette += 1;
      }
    }
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorExit:

  return (~ERROR_NONE);
}



// **************************************************************************
// **************************************************************************
//
// EncodePCX8 ()
//
// RLE compress a bitplane
//
// Inputs  uint8_t *       Ptr to src
//         uint8_t *       Ptr to dst
//         int             Width
//
// Output  uint8_t *       Updated ptr to dst, or NULL if an error
//

static uint8_t *EncodePCX8 (
  uint8_t *pSrc, uint8_t *pDst, int iWidth )

{
  uint8_t *pTmp;
  uint8_t uVal;
  int iCount;
  int iLimit;

  // Now encode and write out the line.

  while (iWidth) {
    // Find out how many repeats there are of the current pixel.

    pTmp = pSrc;
    uVal = *pTmp++;
    iCount = 1;
    iLimit = iWidth;

    if (iLimit > 63) iLimit = 63;

    while (iCount < iLimit) {
      if (*pTmp++ != uVal) break;
      iCount += 1;
    }

    // Write out the value byte or repeat/value pair.

    if ((iCount > 1) || (uVal > 191))
      *pDst++ = iCount | 0xC0u;

    *pDst++ = uVal;

    // Advance the source pointers.

    pSrc += iCount;
    iWidth -= iCount;
  }

  // Return with success.

  return (pDst);
}



// **************************************************************************
// **************************************************************************
//
// PcxReadPxlmap ()
//
// Read the next data item from the file
//
// Inputs  void **       Ptr to address of input context
//
// Output  PXLMAP_T * Ptr to data item, or NULL if an error or no data
//
// N.B.    The end of the file is signalled by a return of NULL with an
//         g_iErrorCode of ERROR_NONE
//

PXLMAP_T *PcxReadPxlmap (
  const char *pFileName )

{
  // Local variables.

  ERRORCODE (*pfread)(PCX_HEADER_T *, uint8_t *, uint8_t *, uint8_t *, int, RGBQUAD_T *) = NULL;

  MAPPEDFILE_T  cMapping;
  PCX_HEADER_T  cHeader;
  uint8_t *     pFileBuffer;
  PXLMAP_T *    pPxlmap;
  int           iSrcW;
  int           iSrcH;
  int           iSrcB;
  int           i;

  memset(&cMapping, 0, sizeof(cMapping));

  if (!OpenMemoryMappedFile(&cMapping, pFileName, true))
    goto errorExit;

  if (cMapping.m_uViewSize < sizeof(cMapping)) {
    g_iErrorCode = ERROR_UNKNOWN_FILE;
    goto errorExit;
  }

  // Convert the header into the native format.

  pFileBuffer = cMapping.m_pViewAddr;

  memcpy(&cHeader, cMapping.m_pViewAddr, sizeof(cHeader));

  pFileBuffer += sizeof(cHeader);

#if BYTE_ORDER_HILO
  cHeader.m_uXMin = SwapD16(cHeader.m_uXMin);
  cHeader.m_uYMin = SwapD16(cHeader.m_uYMin);
  cHeader.m_uXMax = SwapD16(cHeader.m_uXMax);
  cHeader.m_uYMax = SwapD16(cHeader.m_uYMax);
  cHeader.m_uBytesPerLine = SwapD16(cHeader.m_uBytesPerLine);
#endif

  // Check for a version 5 PCX file with RLE compression.

  if ((cHeader.m_uFlag != 0x0A) || (cHeader.m_uPacked != 0x01)) {
    g_iErrorCode = ERROR_PCX_NOT_HANDLED;
    sprintf(g_aErrorMessage,
      "(PCX) Unknown variant : not a type 5 RLE-compressed PCX file.\n");
    goto errorExit;
  }

  // Check that we can handle this bitmap.

  i = (cHeader.m_uPlanes << 8) + cHeader.m_uBPP;

  if ((i != 0x0108) && (i != 0x0401)) {
    g_iErrorCode = ERROR_PCX_NOT_HANDLED;
    sprintf(g_aErrorMessage,
      "(PCX) Unknown variant : cannot read %d planes with %d bpp.\n",
      (int)cHeader.m_uPlanes,
      (int)cHeader.m_uBPP);
    goto errorExit;
  }

  // Get the bitmap's important parameters.

  iSrcW = 1 + cHeader.m_uXMax - cHeader.m_uXMin;
  iSrcH = 1 + cHeader.m_uYMax - cHeader.m_uYMin;
  iSrcB = cHeader.m_uBPP * cHeader.m_uPlanes;

  // Inform user.

//  printf("Reading %d bpp, %dx%d PCX image.\n", iSrcB, iSrcW, iSrcH);

  // Create a native bitmap of the required size.

  pPxlmap = PxlmapAlloc(iSrcW, iSrcH, iSrcB, NO);

  if (pPxlmap == NULL)
    goto errorExit;

  // 04Jul08 JCB - Is this a 16-color image?

  if (i == 0x0401 || i == 0x0104)
    pPxlmap->m_iPxmSignificantBits = 4;

  // If we have both width and height, then lets try to actually convert
  // the bitmap data.

  if ((iSrcW != 0) && (iSrcH != 0)) {
    // Call ConvertFromPCXRLE() to do the actual conversion, this routine
    // could do with being written in assembler for greater speed.

    if (i == 0x0108) pfread = ConvertFromPCX8RLE;
    else if (i == 0x0104) pfread = ConvertFromPCX4RLE;
    else if (i == 0x0401) pfread = ConvertFromPCX4RLE;

    if ((*pfread)(
          &cHeader,
          cMapping.m_pViewAddr + 128,
          cMapping.m_pViewAddr + cMapping.m_uViewSize,
          pPxlmap->m_pPxmPixels,
          pPxlmap->m_iPxmLineSize,
          pPxlmap->m_aPxmC) != ERROR_NONE) {
      PxlmapFree(pPxlmap);
      g_iErrorCode = ERROR_PCX_MALFORMED;
      sprintf(g_aErrorMessage,
        "(PCX) File malformed : error in RLE bitmap data.\n");
      goto errorExit;
    }
  }

  // Return the bitmap.

  CloseMemoryMappedFile(&cMapping);

  return (pPxlmap);

  // Error handlers (reached via the dreaded goto).

errorExit:

  CloseMemoryMappedFile(&cMapping);

  return (NULL);
}



// **************************************************************************
// **************************************************************************
//
// PcxDumpPxlmap ()
//
// Writes out the bitmap (in PCX format) with the given filename
//
// Inputs  PXLMAP_T *   Ptr to bitmap datablock
//         char *          Ptr to filename
//
// Output  ERRORCODE       ERROR_NONE if OK
//

ERRORCODE PcxDumpPxlmap (
  PXLMAP_T *pPxlmap, const char *pFileName )

{
  // Local variables.

  FILE *f;

  int i;

  int iW;
  int iH;
  int iL;

  uint8_t *pPxlData;
  uint8_t *pPlaneBuf;
  uint8_t *pPlanePtr;

  PCX_HEADER_T cHeader;

  uint8_t *pPalette;
  uint8_t aPalette[769];

  char aString[256];

  // Make sure that its an 8 BPP bitmap.

  if (pPxlmap->m_iPxmB != 8) {
    g_iErrorCode = ERROR_PCX_NOT_HANDLED;
    sprintf(g_aErrorMessage,
      "(PCX) Variant not handled : cannot dump %d bpp data.\n",
      pPxlmap->m_iPxmB);
    goto errorExit;
  }

  // Fill in the PCX file header.

  iW = pPxlmap->m_iPxmW - 1;
  iH = pPxlmap->m_iPxmH - 1;
  iL = (pPxlmap->m_iPxmW + 1) & ~1;

  memset(&cHeader, 0, sizeof(PCX_HEADER_T));

  cHeader.m_uFlag = 0x0A;
  cHeader.m_uVersion = 5;
  cHeader.m_uPacked = 1;
  cHeader.m_uBPP = 8;
  cHeader.m_uXMin = 0;
  cHeader.m_uYMin = 0;
#if BYTE_ORDER_HILO
  cHeader.m_uXMax = SwapD16(iW);
  cHeader.m_uYMax = SwapD16(iH);
  cHeader.m_uXDPI = SwapD16(72);
  cHeader.m_uYDPI = SwapD16(72);
  cHeader.m_uBytesPerLine = SwapD16(iL);
#else
  cHeader.m_uXMax = iW;
  cHeader.m_uYMax = iH;
  cHeader.m_uXDPI = 72;
  cHeader.m_uYDPI = 72;
  cHeader.m_uBytesPerLine = iL;
#endif
  cHeader.m_uPlanes = 1;
  cHeader.m_uInterp = 0;
  cHeader.m_uVideoX = 0;
  cHeader.m_uVideoY = 0;

  iW += 1;
  iH += 1;

  // Fill in the palette data.

  pPalette = &aPalette[0];

  *pPalette++ = 0x0Cu;

  for (i = 0; i != 256; i += 1) {
    pPalette[0] = pPxlmap->m_aPxmC[i].m_uRgbR;
    pPalette[1] = pPxlmap->m_aPxmC[i].m_uRgbG;
    pPalette[2] = pPxlmap->m_aPxmC[i].m_uRgbB;
    pPalette += 3;
  }

  // Allocate a buffer for encoding a plane.

  pPlaneBuf = malloc(iW * 2);

  if (pPlaneBuf == NULL) {
    g_iErrorCode = ERROR_NO_MEMORY;
    goto errorExit;
  }

  // Write it out.

  strcpy(aString, pFileName);
//strcat(aString, ".pcx");

  f = fopen(aString, "wb");

  if (f == NULL)
    goto errorWrite;

  if (fwrite(&cHeader, 1, sizeof(PCX_HEADER_T), f) != sizeof(PCX_HEADER_T)) {
    g_iErrorCode = ERROR_IO_WRITE;
    goto errorClose;
  }

  pPxlData = pPxlmap->m_pPxmPixels;

  while (iH) {
    pPlanePtr = EncodePCX8(pPxlData, pPlaneBuf, iL);

    i = pPlanePtr - pPlaneBuf;

    if (fwrite(pPlaneBuf, 1, i, f) != i) {
      g_iErrorCode = ERROR_IO_WRITE;
      goto errorClose;
    }

    pPxlData += pPxlmap->m_iPxmLineSize;
    iH -= 1;
  }

  if (fwrite(aPalette, 1, 769, f) != 769) {
    g_iErrorCode = ERROR_IO_WRITE;
    goto errorClose;
  }

  if (fclose(f) != 0)
    goto errorWrite;

  // Free up the plane buffer.

  free(pPlaneBuf);

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorClose:

  fclose(f);

errorWrite:

  g_iErrorCode = ERROR_IO_WRITE;

  free(pPlaneBuf);

errorExit:

  return (g_iErrorCode);
}
