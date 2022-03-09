// **************************************************************************
// **************************************************************************
//
// bmpfile.c
//
// Simple interface for reading files in Microsoft's BMP (aka DIB) format.
//
// Limitations ...
//
// Lots.
//
// Only the following file contents are currently understood ...
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

#include  "elmer.h"
#include  "errorcode.h"
#include  "pxlmap.h"
#include  "mappedfile.h"
#include  "bmpfile.h"

//
// DEFINITIONS
//

// Force byte alignment for the following structures.

#pragma pack(1)

// Pixels per meter for Windows DIB header.
//
// Printer has a resolution of 300 dpi.
// Monitor has a resolution of 100 dpi @ a .25mm dot pitch.

#define PRINTER_PPM         11811
#define MONITOR_PPM          3937

// ID for the DIB file.

#define ID2_BM  MakeID2('B', 'M')

// Windows structures.

typedef struct BMPHEAD_S {
  uint8_t       m_uType[2];             // Specifies the file type = "BM".
  uint32_t      m_uSize;                // Specifies the file size.
  uint16_t      m_uReserved1;           // Reserved (must be 0).
  uint16_t      m_uReserved2;           // Reserved (must be 0).
  uint32_t      m_uOffBits;             // Byte offset from BhType to actual data.
} BMPHEAD_T;

// BMP file header for Windows 3x and Windows NT 3x.

typedef struct BMPVER3_S {
  uint32_t      m_uSize;                // Size of this structure.
  uint32_t      m_uWidth;               // Width of bitmap in pixels.
  int32_t       m_iHeight;              // Height of bitmap in pixels.
  uint16_t      m_uPlanes;              // # of planes (must be 1).
  uint16_t      m_uBitCount;            // # of bits per pixel (1/4/8/24).
  uint32_t      m_uCompression;         // (BI_RGB/BI_RLE8/BI_RLE4).
  uint32_t      m_uSizeImage;           // Size of compressed image (bytes).
  uint32_t      m_uXPelsPerMeter;       // X resolution in pixels per meter.
  uint32_t      m_uYPelsPerMeter;       // Y resolution in pixels per meter.
  uint32_t      m_uClrUsed;             // 0=all (i.e. 2^bm_itCount).
  uint32_t      m_uClrImportant;        // 0=all.
} BMPVER3_T;

typedef struct BMPMASK_S {
  uint32_t      m_uMaskR;               // Bitfield mask for red.
  uint32_t      m_uMaskG;               // Bitfield mask for green.
  uint32_t      m_uMaskB;               // Bitfield mask for blue.
} BMPMASK_T;

// BMP file header for Windows 95 and Windows NT 4x.

typedef struct BMPVER4_S {
  uint32_t      m_uSize;                // Size of this structure.
  uint32_t      m_uWidth;               // Width of bitmap in pixels.
  int32_t       m_iHeight;              // Height of bitmap in pixels.
  uint16_t      m_uPlanes;              // # of planes (must be 1).
  uint16_t      m_uBitCount;            // # of bits per pixel (1/4/8/24).
  uint32_t      m_uCompression;         // (BI_RGB/BI_RLE8/BI_RLE4).
  uint32_t      m_uSizeImage;           // Size of compressed image (bytes).
  uint32_t      m_uXPelsPerMeter;       // X resolution in pixels per meter.
  uint32_t      m_uYPelsPerMeter;       // Y resolution in pixels per meter.
  uint32_t      m_uClrUsed;             // 0=all (i.e. 2^bm_itCount).
  uint32_t      m_uClrImportant;        // 0=all.

  uint32_t      m_uMaskR;               // Bitfield mask for red.
  uint32_t      m_uMaskG;               // Bitfield mask for green.
  uint32_t      m_uMaskB;               // Bitfield mask for blue.
  uint32_t      m_uMaskA;               // Bitfield mask for alpha.
  uint32_t      m_uColorSpace;          // Colour-space type.
  uint32_t      m_uRedX;                // X of red endpoint.
  uint32_t      m_uRedY;                // Y of red endpoint.
  uint32_t      m_uRedZ;                // Z of red endpoint.
  uint32_t      m_uGreenX;              // X of green endpoint.
  uint32_t      m_uGreenY;              // Y of green endpoint.
  uint32_t      m_uGreenZ;              // Z of green endpoint.
  uint32_t      m_uBlueX;               // X of blue endpoint.
  uint32_t      m_uBlueY;               // Y of blue endpoint.
  uint32_t      m_uBlueZ;               // Z of blue endpoint.
  uint32_t      m_uGammaR;              // Gamma scale for red.
  uint32_t      m_uGammaG;              // Gamma scale for green.
  uint32_t      m_uGammaB;              // Gamma scale for blue.
} BMPVER4_T;

//

typedef struct BMPCOLOUR_S {
  RGBQUAD_T     m_aRGB[256];
} BMPCOLOUR_T;

//

typedef struct BMPFILE31_S {
  uint16_t      m_uPadding;
  BMPHEAD_T     m_cHead;
  BMPVER3_T     m_cInfo;
} BMPFILE31_T;

typedef struct BMPFILENT_S {
  uint16_t      m_uPadding;
  BMPHEAD_T     m_cHead;
  BMPVER3_T     m_cInfo;
  BMPMASK_T     m_cMask;
} BMPFILENT_T;

typedef struct BMPFILE95_S {
  uint16_t      m_uPadding;
  BMPHEAD_T     m_cHead;
  BMPVER4_T     m_cInfo;
} BMPFILE95_T;

// Compression types.

#ifndef BI_RGB
#define BI_RGB              0
#define BI_RLE8             1
#define BI_RLE4             2
#define BI_MASK             3
#endif

// Restore default alignment.

#pragma pack()



// **************************************************************************
// **************************************************************************
//
// ByteSwapVer3Info ()
//
// Inputs  BMPVER3_T *     Ptr to bitmap info
//
// Output  -
//

#if BYTE_ORDER_HILO

static void ByteSwapVer3Info (
  BMPVER3_T *pInfo )

{
  // Convert the version 3 file info into the native format.

  pInfo->m_uSize          = SwapD32(pInfo->m_uSize);
  pInfo->m_uWidth         = SwapD32(pInfo->m_uWidth);
  pInfo->m_iHeight        = SwapD32(pInfo->m_iHeight);
  pInfo->m_uPlanes        = SwapD16(pInfo->m_uPlanes);
  pInfo->m_uBitCount      = SwapD16(pInfo->m_uBitCount);
  pInfo->m_uCompression   = SwapD32(pInfo->m_uCompression);
  pInfo->m_uSizeImage     = SwapD32(pInfo->m_uSizeImage);
  pInfo->m_uXPelsPerMeter = SwapD32(pInfo->m_uXPelsPerMeter);
  pInfo->m_uYPelsPerMeter = SwapD32(pInfo->m_uYPelsPerMeter);
  pInfo->m_uClrUsed       = SwapD32(pInfo->m_uClrUsed);
  pInfo->m_uClrImportant  = SwapD32(pInfo->m_uClrImportant);
}

#endif



// **************************************************************************
// **************************************************************************
//
// ByteSwapVer4Info ()
//
// Inputs  BMPVER4_T *     Ptr to bitmap info
//
// Output  -
//

#if BYTE_ORDER_HILO

static void ByteSwapVer4Info (
  BMPVER4_T *pInfo )
{
  // Convert the version 4 file info extensions into the native format.

  pInfo->m_uMaskR         = SwapD32(pInfo->m_uMaskR);
  pInfo->m_uMaskG         = SwapD32(pInfo->m_uMaskG);
  pInfo->m_uMaskB         = SwapD32(pInfo->m_uMaskB);
  pInfo->m_uMaskA         = SwapD32(pInfo->m_uMaskA);
  pInfo->m_uColorSpace    = SwapD32(pInfo->m_uColorSpace);
  pInfo->m_uRedX          = SwapD32(pInfo->m_uRedX);
  pInfo->m_uRedY          = SwapD32(pInfo->m_uRedY);
  pInfo->m_uRedZ          = SwapD32(pInfo->m_uRedZ);
  pInfo->m_uGreenX        = SwapD32(pInfo->m_uGreenX);
  pInfo->m_uGreenY        = SwapD32(pInfo->m_uGreenY);
  pInfo->m_uGreenZ        = SwapD32(pInfo->m_uGreenZ);
  pInfo->m_uBlueX         = SwapD32(pInfo->m_uBlueX);
  pInfo->m_uBlueY         = SwapD32(pInfo->m_uBlueY);
  pInfo->m_uBlueZ         = SwapD32(pInfo->m_uBlueZ);
  pInfo->m_uGammaR        = SwapD32(pInfo->m_uGammaR);
  pInfo->m_uGammaG        = SwapD32(pInfo->m_uGammaG);
  pInfo->m_uGammaB        = SwapD32(pInfo->m_uGammaB);
}

#endif



// **************************************************************************
// **************************************************************************
//
// ConvertFromRGB ()
//
// Convert the uncompressed BMP data into DIB data
//
// Inputs  BMPFILE95_T *   Ptr to BMP file
//         uint8_t *       Ptr to start of BMP file
//         uint8_t *       Ptr to end   of BMP file
//         uint8_t *       Destination buffer
//         int             Destination buffer width
//         RGBQUAD_T *     Ptr to colour palette
//
// Output  ERRORCODE       ERROR_NONE if OK
//
// N.B.    g_iErrorCode and g_aErrorMessage are NOT set on failure.
//

static ERRORCODE ConvertFromRGB (
  BMPFILE95_T *pBmpFile, uint8_t *pSrcPtr, uint8_t *pSrcEnd,
  uint8_t *pDstPtr, int iDstOff, RGBQUAD_T *pPalette )

{
  // Local variables.

  RGBQUAD_T *   pBmpPalette;
  BMPHEAD_T *   pBmpHead;
  BMPVER4_T *   pBmpInfo;
  uint8_t *     pSrcLin;
  uint8_t *     pDstLin;
  uint8_t *     pSrcPxl;
  uint8_t *     pDstPxl;
  int           iSrcW;
  int           iSrcH;
  int           iSrcLen;
  int           iDstLen;
  int           iSrcOff;
  int           iDstRhs;
  int           iNumPaletteEntries;
  int           i;

  //

  pBmpHead = &pBmpFile->m_cHead;
  pBmpInfo = &pBmpFile->m_cInfo;

  // Clear out the colour palette.

  for (i = 0; i < 256; i += 1) {
    pPalette[i].m_uRgbB = 0;
    pPalette[i].m_uRgbG = 0;
    pPalette[i].m_uRgbR = 0;
    pPalette[i].m_uRgbA = 0;
  }

  // Copy the colour palette from the BMP file.

  if ((pBmpInfo->m_uBitCount == 4) ||
      (pBmpInfo->m_uBitCount == 8)) {
    // How many colours are stored.

    if (pBmpInfo->m_uClrUsed == 0)
      iNumPaletteEntries = 1 << pBmpInfo->m_uBitCount;
    else
      iNumPaletteEntries = pBmpInfo->m_uClrUsed;

    if (iNumPaletteEntries > 256)
      iNumPaletteEntries = 256;

    // Copy the colour palette.

    pBmpPalette = (RGBQUAD_T *)
            (pSrcPtr + sizeof(BMPHEAD_T) + pBmpInfo->m_uSize);

    for (i = 0; i < iNumPaletteEntries; i += 1) {
      pPalette->m_uRgbB = pBmpPalette->m_uRgbB;
      pPalette->m_uRgbG = pBmpPalette->m_uRgbG;
      pPalette->m_uRgbR = pBmpPalette->m_uRgbR;
      pPalette->m_uRgbA = pBmpPalette->m_uRgbA;
      pPalette += 1;
      pPalette += 1;
    }
  }

  // Read the size of the image.

  iSrcW = pBmpInfo->m_uWidth;
  iSrcH = pBmpInfo->m_iHeight;

  // Calculate the src data and line sizes.

  if (pBmpInfo->m_uBitCount == 4)
    iSrcLen = (iSrcW + 1) >> 1;
  else
    iSrcLen = iSrcW * ((pBmpInfo->m_uBitCount + 1) >> 3);

  iSrcOff = (iSrcLen + 3) & ~3;

  pSrcLin = pSrcPtr + pBmpHead->m_uOffBits;

  // Calculate the dst line padding size.

  iDstRhs = iDstOff - iSrcLen;

  // What direction should we put the file data into the destination image.

  if (iSrcH < 0) {
    // Image is stored top-to-bottom.

    pDstLin = pDstPtr;

    iSrcH = -iSrcH;
  }
  else {
    // Image is stored bottom-to-top.

    pDstLin = pDstPtr + (iDstOff * (iSrcH - 1));

    iDstOff = -iDstOff;
  }

  // Copy the data into the destination image.

  while (iSrcH--) {
    pSrcPxl = pSrcLin;
    pDstPxl = pDstLin;

    iDstLen = iSrcLen;

    while (iDstLen--) {
      *pDstPxl++ = *pSrcPxl++;
    }

    iDstLen = iDstRhs;

    while (iDstLen--) {
      *pDstPxl++ = 0;
    }

    pSrcLin += iSrcOff;
    pDstLin += iDstOff;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

//  errorExit:
//
//    return (~ERROR_NONE);
}



// **************************************************************************
// **************************************************************************
//
// BmpReadPxlmap ()
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

PXLMAP_T * BmpReadPxlmap (
  const char *pFileName )

{
  // Local variables.

  ERRORCODE (*pfread)(BMPFILE95_T *, uint8_t *, uint8_t *, uint8_t *, int, RGBQUAD_T *) = NULL;

  MAPPEDFILE_T  cMapping;
  BMPFILE95_T   cFile;
  PXLMAP_T *    pPxlmap;
  int           iSrcW;
  int           iSrcH;
  int           iSrcB;

  memset(&cMapping, 0, sizeof(cMapping));

  if (!OpenMemoryMappedFile(&cMapping, pFileName, true)) {
    goto errorExit;
  }

  if (cMapping.m_uViewSize < (sizeof(BMPHEAD_T) + sizeof(BMPVER3_T))) {
    g_iErrorCode = ERROR_UNKNOWN_FILE;
    goto errorExit;
  }

  // Convert the header into the native format.

  memset(&cFile, 0, sizeof(cFile));

  memcpy(&cFile.m_cHead, cMapping.m_pViewAddr, sizeof(BMPHEAD_T) + sizeof(BMPVER3_T));

#if BYTE_ORDER_HILO
  cFile.m_cHead.m_uSize = SwapD32(cFile.m_cHead.m_uSize);
  cFile.m_cHead.m_uOffBits = SwapD32(cFile.m_cHead.m_uOffBits);

  ByteSwapVer3Info((BMPVER3_T *)&cFile.m_cInfo);
#endif

  if (cFile.m_cInfo.m_uSize == sizeof(BMPVER4_T)) {
    memcpy(&cFile.m_cInfo.m_uMaskR, cMapping.m_pViewAddr + sizeof(BMPHEAD_T) + sizeof(BMPVER3_T), sizeof(BMPVER4_T) - sizeof(BMPVER3_T));

#if BYTE_ORDER_HILO
    ByteSwapVer4Info((BMPVER4_T *)&cFile.m_cInfo);
#endif
  }

  // Convert the file info into the native format.

  if ((cFile.m_cHead.m_uType[0] != 'B') ||
      (cFile.m_cHead.m_uType[1] != 'M') ||
      (cFile.m_cHead.m_uReserved1 != 0) ||
      (cFile.m_cHead.m_uReserved2 != 0)) {
    g_iErrorCode = ERROR_BMP_NOT_HANDLED;
    sprintf(g_aErrorMessage,
      "(BMP) Unknown variant : is this a BMP file?\n");
    goto errorExit;
  }

  // Check that we can handle this bitmap.

  if ((cFile.m_cInfo.m_uBitCount != 8) &&
      (cFile.m_cInfo.m_uBitCount != 24) &&
      (cFile.m_cInfo.m_uBitCount != 32)) {
    g_iErrorCode = ERROR_BMP_NOT_HANDLED;
    sprintf(g_aErrorMessage,
      "(BMP) Unknown variant : cannot read image with %d bpp.\n",
      (int)cFile.m_cInfo.m_uBitCount);
    goto errorExit;
  }

  if (cFile.m_cInfo.m_uPlanes != 1) {
    g_iErrorCode = ERROR_BMP_NOT_HANDLED;
    sprintf(g_aErrorMessage,
      "(BMP) Unknown variant : cannot read image with %d planes.\n",
      (int)cFile.m_cInfo.m_uPlanes);
    goto errorExit;
  }

  if (cFile.m_cInfo.m_uCompression != BI_RGB) {
    g_iErrorCode = ERROR_BMP_NOT_HANDLED;
    sprintf(g_aErrorMessage,
      "(BMP) Unknown variant : cannot read compressed image.\n");
    goto errorExit;
  }

  // Get the bitmap's important parameters.

  iSrcW = cFile.m_cInfo.m_uWidth;
  iSrcH = cFile.m_cInfo.m_iHeight;
  iSrcB = cFile.m_cInfo.m_uBitCount;

  if (iSrcH < 0) {
    // Image is stored top-to-bottom in the file.
    iSrcH = -iSrcH;
  }

  // Inform user.

//  printf("Reading %d bpp, %dx%d BMP image.\n", cFile.m_cInfo.m_uBitCount, uSrcW, abs(sSrcH));

  // Create a native bitmap of the required size.

  pPxlmap = PxlmapAlloc(iSrcW, iSrcH, iSrcB, NO);

  if (pPxlmap == NULL) {
    goto errorExit;
  }

  // If we have both width and height, then lets try to actually convert
  // the bitmap data.

  if ((iSrcW != 0) && (iSrcH != 0)) {
    // Call ConvertFromRGB() to do the actual conversion.

    if (cFile.m_cInfo.m_uCompression == BI_RGB)
      pfread = ConvertFromRGB;

    if (pfread == NULL) {
      PxlmapFree(pPxlmap);
      g_iErrorCode = ERROR_BMP_MALFORMED;
      sprintf(g_aErrorMessage,
        "(BMP) File malformed : Cannot handle compressed BMP data.\n");
      goto errorExit;
    }

    if ((*pfread)(
          &cFile,
          cMapping.m_pViewAddr,
          cMapping.m_pViewAddr + cMapping.m_uViewSize,
          pPxlmap->m_pPxmPixels,
          pPxlmap->m_iPxmLineSize,
          pPxlmap->m_aPxmC) != ERROR_NONE) {
      PxlmapFree(pPxlmap);
      g_iErrorCode = ERROR_BMP_MALFORMED;
      sprintf(g_aErrorMessage,
        "(BMP) File malformed : error in bitmap data.\n");
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
// BmpDumpPxlmap ()
//
// Writes out the bitmap (in Windows' BMP format) with the given filename
//
// Inputs  PXLMAP_T *  Ptr to bitmap
//         char *          Ptr to filename
//
// Output  ERRORCODE       ERROR_NONE if OK
//

ERRORCODE BmpDumpPxlmap (
  PXLMAP_T *pPxlmap, const char *pFileName )

{
  // Local variables.

  size_t        h;
  size_t        i;
  size_t        c;
  size_t        m;
  FILE *        f;
  BMPFILE95_T   x;
  BMPCOLOUR_T   p;
  char          aString[256];

  // Fill in the BMP colour palette.

  if (pPxlmap->m_iPxmB == 8) {
    c = sizeof(BMPCOLOUR_T);

    for (i = 0; i < 256; i += 1) {
      p.m_aRGB[i].m_uRgbB = pPxlmap->m_aPxmC[i].m_uRgbB;
      p.m_aRGB[i].m_uRgbG = pPxlmap->m_aPxmC[i].m_uRgbG;
      p.m_aRGB[i].m_uRgbR = pPxlmap->m_aPxmC[i].m_uRgbR;
      p.m_aRGB[i].m_uRgbA = pPxlmap->m_aPxmC[i].m_uRgbA;
    }
  }
  else
  if ((pPxlmap->m_iPxmB == 24) || (pPxlmap->m_iPxmB == 32)) {
    c = 0;
  }
  else {
    sprintf(g_aErrorMessage,
      "BMP varient not handled : cannot write %d BPP image.\n",
      pPxlmap->m_iPxmB);
    goto errorExit;
  }

  // Fill in the BMP file head.

  h = sizeof(BMPHEAD_T);
  i = sizeof(BMPVER3_T);

  m = pPxlmap->m_iPxmH * pPxlmap->m_iPxmLineSize;

  x.m_cHead.m_uType[0] = 'B';
  x.m_cHead.m_uType[1] = 'M';
  x.m_cHead.m_uSize = h + i + c + m;
  x.m_cHead.m_uReserved1 = 0;
  x.m_cHead.m_uReserved2 = 0;
  x.m_cHead.m_uOffBits = h + i + c;

  // Fill in the version 3 BMP file info.

  x.m_cInfo.m_uSize = i;
  x.m_cInfo.m_uWidth = pPxlmap->m_iPxmW;
  x.m_cInfo.m_iHeight = -pPxlmap->m_iPxmH;
  x.m_cInfo.m_uPlanes = 1;
  x.m_cInfo.m_uBitCount = pPxlmap->m_iPxmB;
  x.m_cInfo.m_uCompression = BI_RGB;
  x.m_cInfo.m_uSizeImage = m;
  x.m_cInfo.m_uXPelsPerMeter = MONITOR_PPM;
  x.m_cInfo.m_uYPelsPerMeter = MONITOR_PPM;
  x.m_cInfo.m_uClrUsed = 0;
  x.m_cInfo.m_uClrImportant = 0;

  // Convert the file info from the native format.

#if BYTE_ORDER_HILO
  ByteSwapVer3Info((BMPVER3_T *)&x.m_cInfo);
#endif

  // Open the file for output.

  strcpy(aString, pFileName);
//strcat(aString, ".bmp");

  f = fopen(aString, "wb");

  if (f == NULL)
    goto errorWrite;

  // Write out the file header.

  if (fwrite(&x.m_cHead, 1, h, f) != h) {
    g_iErrorCode = ERROR_IO_WRITE;
    goto errorClose;
  }

  if (fwrite(&x.m_cInfo, 1, i, f) != i) {
    g_iErrorCode = ERROR_IO_WRITE;
    goto errorClose;
  }

  // Write out the file palette.

  if (c != 0) {
    if (fwrite(&p, 1, c, f) != c) {
      g_iErrorCode = ERROR_IO_WRITE;
      goto errorClose;
    }
  }

  // Write out the file bitmap data.

  if (fwrite(pPxlmap->m_pPxmPixels, 1, m, f) != m) {
    g_iErrorCode = ERROR_IO_WRITE;
    goto errorClose;
  }

  // Close the file.

  if (fclose(f) != 0)
    goto errorWrite;

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorClose:

  fclose(f);

errorWrite:

  g_iErrorCode = ERROR_IO_WRITE;

errorExit:

  return (g_iErrorCode);
}
