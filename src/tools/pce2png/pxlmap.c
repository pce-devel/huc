// **************************************************************************
// **************************************************************************
//
// pxlmap.c
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

#include  "elmer.h"
#include  "errorcode.h"
#include  "pxlmap.h"

//
// DEFINITIONS
//

uint8_t RemappingTable[256];

bool g_bFlattenAlpha;

PXLMAP_T *pDifferencePxlmap = NULL;

uint8_t uDifferenceColorB = 255;
uint8_t uDifferenceColorG = 255;
uint8_t uDifferenceColorR = 255;
uint8_t uDifferenceColorA = 255;
uint8_t uDifferenceColorI = 255;

static int CheckPxlmapSize (
  int iWidth, int iHeight, int iBits );



// **************************************************************************
// **************************************************************************
//
// PxlmapAlloc ()
//
// Allocate a Pxlmap of the desired size
//
// Inputs  UD        Width
//     UD        Height
//     UW        Bits-per-pixel
//     FL        Clear the Pxlmap ?
//
// Output  PXLMAP_T * Ptr to data or NULL if failed
//

PXLMAP_T * PxlmapAlloc (
  int iWidth, int iHeight, int iBits, bool bClear )

{
  // Local variables.

  size_t l;
  size_t m;
  size_t s;
  PXLMAP_T *d;
  PXLMAP_T *b;

  // Check for illegal parameters.

  if (CheckPxlmapSize(iWidth, iHeight, iBits) != ERROR_NONE)
    goto errorExit;

  // Get size of line in bytes padded out to the next 4 byte boundary.
  // Given that ...

  l = ((((size_t)iWidth * iBits) + 31L) >> 3) & (~3L);

  // Now calculate the amount of memory needed for the whole block.

  m = l * iHeight;
  s = m + sizeof(PXLMAP_T);

  // Allocate the memory.

  d = malloc(s);

  if (d == NULL) {
    g_iErrorCode = ERROR_NO_MEMORY;
    goto errorExit;
  }

  // Initialize the data headers.

  b = (PXLMAP_T *)d;

  memset(b, 0, sizeof(PXLMAP_T));

  b->m_pPxmNext = NULL;
  b->m_pPxmPrev = NULL;
  b->m_uPxmSize = s;

  b->m_pPxmPixels = (uint8_t *)(b + 1);
  b->m_iPxmLineSize = l;
  b->m_iPxmXTopLeft = 0;
  b->m_iPxmYTopLeft = 0;
  b->m_iPxmW = iWidth;
  b->m_iPxmH = iHeight;
  b->m_iPxmB = iBits;

  b->m_iPxmPaletteHasAlpha = 0;

  if (b->m_iPxmB > 8)
    b->m_iPxmColorsInPalette = 0;
  else
    b->m_iPxmColorsInPalette = 1 << b->m_iPxmB;

  // 02Sep04 JCB - The calling code can change this later if needed.

  b->m_iPxmSignificantBits = b->m_iPxmB;

  // Clear the bitmap ?

  if (bClear && (m != 0))
    memset(b->m_pPxmPixels, 0, m);

  // Return with success.

  return (d);

  // Error handlers (reached via the dreaded goto).

errorExit:

  return ((PXLMAP_T *)NULL);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapClear ()
//
// Clear the bitmap
//
// Inputs  PXLMAP_T *  Ptr to bitmap
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapClear (
  PXLMAP_T *pPxm )

{
  // Set the bitmap data to all zeroes.

  if ((pPxm->m_pPxmPixels != NULL) && (pPxm->m_iPxmW != 0) && (pPxm->m_iPxmH != 0)) {
    //

    memset(pPxm->m_pPxmPixels, 0, pPxm->m_iPxmLineSize * pPxm->m_iPxmH);
  }

  // Return with success code.

  return (ERROR_NONE);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapCopy ()
//
// Inputs  PXLMAP_T *  Ptr to source bitmap
//     int        Source X coordinate of box to copy
//     int        Source Y coordinate of box to copy
//     int        Source W of box to copy
//     int        Source H of box to copy
//     PXLMAP_T *  Ptr to destination bitmap
//     int        Destination X coordinate of box to copy
//     int        Destination Y coordinate of box to copy
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapCopy (
  PXLMAP_T *pPxmSrc, int iBoxXSrc, int iBoxYSrc, int iBoxWSrc, int iBoxHSrc,
  PXLMAP_T *pPxmDst, int iBoxXDst, int iBoxYDst )

{
  // Local variables.

  uint8_t *pSrcLin;
  uint8_t *pDstLin;

  uint8_t *pSrcPxl;
  uint8_t *pDstPxl;

  int iLineSizeSrc;
  int iLineSizeDst;

  int iPxlSizeSrc;
  int iPxlSizeDst;

  int i;
  int j;

  // Is there a box to copy?

  if ((iBoxWSrc == 0) || (iBoxHSrc == 0)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Can't copy a zero size box.\n");
    goto errorExit;
  }

  // Get the remap box position and size (relative to the bitmap's physical
  // coordinates, rather than its logical coordinates).

  iBoxXSrc -= pPxmSrc->m_iPxmXTopLeft;
  iBoxYSrc -= pPxmSrc->m_iPxmYTopLeft;

  iBoxXDst -= pPxmDst->m_iPxmXTopLeft;
  iBoxYDst -= pPxmDst->m_iPxmYTopLeft;

  // Is the search rectangle within the bitmap ?

  if ((iBoxXSrc < 0) || ((iBoxXSrc + iBoxWSrc) > pPxmSrc->m_iPxmW) ||
      (iBoxYSrc < 0) || ((iBoxYSrc + iBoxHSrc) > pPxmSrc->m_iPxmH)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Copy rectangle overruns source bitmap.\n");
    goto errorExit;
  }

  if ((iBoxXDst < 0) || ((iBoxXDst + iBoxWSrc) > pPxmDst->m_iPxmW) ||
      (iBoxYDst < 0) || ((iBoxYDst + iBoxHSrc) > pPxmDst->m_iPxmH)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Copy rectangle overruns destination bitmap.\n");
    goto errorExit;
  }

  // Images nust be the same bpp unless I write some more code.

  if (pPxmSrc->m_iPxmB != pPxmDst->m_iPxmB) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Unable to copy rectangle between images with different bpp.\n");
    goto errorExit;
  }

  // Select the search routine depending upon the number of bits-per-pixel.

  iLineSizeSrc = pPxmSrc->m_iPxmLineSize;
  iLineSizeDst = pPxmDst->m_iPxmLineSize;

  iPxlSizeSrc = (pPxmSrc->m_iPxmB + 1) / 8;
  iPxlSizeDst = (pPxmDst->m_iPxmB + 1) / 8;

  pSrcLin = pPxmSrc->m_pPxmPixels + (iBoxYSrc * iLineSizeSrc) + (iBoxXSrc * iPxlSizeSrc);
  pDstLin = pPxmDst->m_pPxmPixels + (iBoxYDst * iLineSizeDst) + (iBoxXDst * iPxlSizeDst);

  //

  if (pPxmSrc->m_iPxmB == 8) {
    // Blit an 8 bits-per-pixel bitmap.

    for (i = iBoxHSrc; i != 0; i -= 1) {
      pSrcPxl = pSrcLin;
      pDstPxl = pDstLin;
      for (j = iBoxWSrc; j != 0; j -= 1)
        *pDstPxl++ = *pSrcPxl++;
      iBoxHSrc -= 1;
      iBoxYSrc += 1;
      iBoxYDst += 1;
      pSrcLin += iLineSizeSrc;
      pDstLin += iLineSizeDst;
    }
    // End of 8 bits-per-pixel blit.
  }
  else
  if (pPxmSrc->m_iPxmB == 24) {
    // Blit an 24 bits-per-pixel bitmap.

    for (i = iBoxHSrc; i != 0; i -= 1) {
      pSrcPxl = pSrcLin;
      pDstPxl = pDstLin;
      for (j = iBoxWSrc; j != 0; j -= 1) {
        pDstPxl[0] = pSrcPxl[0];
        pDstPxl[1] = pSrcPxl[1];
        pDstPxl[2] = pSrcPxl[2];
        pSrcPxl += 3;
        pDstPxl += 3;
      }
      iBoxHSrc -= 1;
      iBoxYSrc += 1;
      iBoxYDst += 1;
      pSrcLin += iLineSizeSrc;
      pDstLin += iLineSizeDst;
    }
    // End of 24 bits-per-pixel blit.
  }
  else
  if (pPxmSrc->m_iPxmB == 32) {
    // Blit an 32 bits-per-pixel bitmap.

    for (i = iBoxHSrc; i != 0; i -= 1) {
      pSrcPxl = pSrcLin;
      pDstPxl = pDstLin;
      for (j = iBoxWSrc; j != 0; j -= 1) {
        pDstPxl[0] = pSrcPxl[0];
        pDstPxl[1] = pSrcPxl[1];
        pDstPxl[2] = pSrcPxl[2];
        pDstPxl[3] = pSrcPxl[3];
        pSrcPxl += 4;
        pDstPxl += 4;
      }
      iBoxHSrc -= 1;
      iBoxYSrc += 1;
      iBoxYDst += 1;
      pSrcLin += iLineSizeSrc;
      pDstLin += iLineSizeDst;
    }
    // End of 32 bits-per-pixel blit.
  }
  else {
    // Can't handle this number of bits-per-pixel.

    sprintf(g_aErrorMessage,
      "Can't remap a bitmap that has %u bits-per-pixel.\n"
      "(DATA, PxlmapRemap)\n",
      (int)pPxmSrc->m_iPxmB);
    goto errorUnknown;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorUnknown:

  g_iErrorCode = ERROR_DATA_UNKNOWN;

errorExit:

  return (g_iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapQuarter ()
//
// Shrink the bitmap to 1/4 size
//
// Inputs  PXLMAP_T *  Ptr to bitmap
//     int        X coordinate of box to shrink
//     int        Y coordinate of box to shrink
//     int        W of box to shrink
//     int        H of box to shrink
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapQuarter (
  PXLMAP_T *pPxm, int iBoxX, int iBoxY, int iBoxW, int iBoxH )

{
  // Local variables.

  uint8_t *     pSrcLin;
  uint8_t *     pDstLin;
  uint8_t *     pSrc;
  uint8_t *     pDst;
  int           iHalfW;
  int           iHalfH;
  size_t        iLineSize;
  int           i;

  // Get the shrink box position and size (relative to the bitmap's physical
  // coordinates, rather than its logical coordinates).

  iBoxX -= pPxm->m_iPxmXTopLeft;
  iBoxY -= pPxm->m_iPxmYTopLeft;

  if ((iBoxW == 0) || (iBoxH == 0)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Can't shrink a zero size box.\n");
    goto errorExit;
  }

  // Is the shrink rectangle within the bitmap ?

  if ((iBoxX < 0) || ((iBoxX + iBoxW) > pPxm->m_iPxmW) ||
      (iBoxY < 0) || ((iBoxY + iBoxH) > pPxm->m_iPxmH)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Shrink rectangle overruns bitmap.\n");
    goto errorExit;
  }

  // Select the shrink routine depending upon the number of bits-per-pixel.

  iLineSize = pPxm->m_iPxmLineSize;

  if (pPxm->m_iPxmB == 8) {
    // Shrink an 8 bits-per-pixel bitmap.
    pSrcLin =
      pDstLin =
        pPxm->m_pPxmPixels + (iBoxY * iLineSize) + iBoxX;

    iHalfW = iBoxW / 2;
    iHalfH = iBoxH / 2;
    iBoxW -= iHalfW;
    iBoxH -= iHalfH;

    while (iHalfH--) {
      pSrc = pSrcLin;
      pDst = pDstLin;
      i = iHalfW;
      while (i--) {
        *pDst = *pSrc;
        pSrc += 2;
        pDst += 1;
      }
      i = iBoxW;
      while (i--)
        *pDst++ = 0;
      pDstLin += iLineSize;
      pSrcLin += iLineSize * 2;
    }

    iBoxW += iHalfW;

    while (iBoxH--) {
      memset(pDstLin, 0, iBoxW);
      pDstLin += iLineSize;
    }
    // End of 8 bits-per-pixel shrink.
  }

  else {
    // Can't handle this number of bits-per-pixel.
    sprintf(g_aErrorMessage,
      "(DATA) Can't shrink a %u bpp bitmap.\n",
      (int)pPxm->m_iPxmB);
    goto errorUnknown;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorUnknown:

  g_iErrorCode = ERROR_DATA_UNKNOWN;

errorExit:

  return (g_iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapRemap ()
//
// Remap the pixel data using a lookup table
//
// Inputs  PXLMAP_T *  Ptr to bitmap
//     int        X coordinate of box to remap
//     int        Y coordinate of box to remap
//     int        W of box to remap
//     int        H of box to remap
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapRemap (
  PXLMAP_T *pPxm, int iBoxX, int iBoxY, int iBoxW, int iBoxH )

{
  // Local variables.

  uint8_t *     pLin;
  uint8_t *     pPxl;
  int           iLineSize;
  int           i;
  int           j;

  // Get the remap box position and size (relative to the bitmap's physical
  // coordinates, rather than its logical coordinates).

  iBoxX -= pPxm->m_iPxmXTopLeft;
  iBoxY -= pPxm->m_iPxmYTopLeft;

  if ((iBoxW == 0) || (iBoxH == 0)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Can't remap a zero size box.\n");
    goto errorExit;
  }

  // Is the search rectangle within the bitmap ?

  if ((iBoxX < 0) || ((iBoxX + iBoxW) > pPxm->m_iPxmW) ||
      (iBoxY < 0) || ((iBoxY + iBoxH) > pPxm->m_iPxmH)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Remapping rectangle overruns bitmap.\n");
    goto errorExit;
  }

  // Select the search routine depending upon the number of bits-per-pixel.

  iLineSize = pPxm->m_iPxmLineSize;

  if (pPxm->m_iPxmB == 8) {
    // Remap an 8 bits-per-pixel bitmap.
    pLin = pPxm->m_pPxmPixels + (iBoxY * iLineSize) + iBoxX;

    for (i = iBoxH; i != 0; i -= 1) {
      pPxl = pLin;
      for (j = iBoxW; j != 0; j -= 1) {
        *pPxl = RemappingTable[*pPxl]; pPxl++;
      }
      iBoxY += 1;
      iBoxH -= 1;
      pLin += iLineSize;
    }
    // End of 8 bits-per-pixel filter.
  }
  else {
    // Can't handle this number of bits-per-pixel.

    sprintf(g_aErrorMessage,
      "Can't remap a bitmap that has %u bits-per-pixel.\n"
      "(DATA, PxlmapRemap)\n",
      (int)pPxm->m_iPxmB);
    goto errorUnknown;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorUnknown:

  g_iErrorCode = ERROR_DATA_UNKNOWN;

errorExit:

  return (g_iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapFilter ()
//
// Filter a bitmap of pixels that are above or below a specified range
//
// Inputs  PXLMAP_T *  Ptr to bitmap
//     int        X coordinate of box to filter
//     int        Y coordinate of box to filter
//     int        W of box to filter
//     int        H of box to filter
//     int        Lowest pixel value to keep
//     int        Highest pixel value to keep
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapFilter (
  PXLMAP_T *pPxm, int iBoxX, int iBoxY, int iBoxW, int iBoxH,
  int iRangeLo, int iRangeHi )

{
  // Local variables.

  uint8_t *     pLin;
  uint8_t *     pPxl;

  uint8_t       uLo;
  uint8_t       uHi;

  int           iLineSize;
  int           i;
  int           j;

  // Is the bitmap in a known format ?

  if (iRangeLo > iRangeHi) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Lo-filter limit is > Hi-filter limit.\n");
    goto errorExit;
  }

  // Get the filter box position and size (relative to the bitmap's physical
  // coordinates, rather than its logical coordinates).

  iBoxX -= pPxm->m_iPxmXTopLeft;
  iBoxY -= pPxm->m_iPxmYTopLeft;

  if ((iBoxW == 0) || (iBoxH == 0)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Can't filter a zero size box.\n");
    goto errorExit;
  }

  // Is the search rectangle within the bitmap ?

  if ((iBoxX < 0) || ((iBoxX + iBoxW) > pPxm->m_iPxmW) ||
      (iBoxY < 0) || ((iBoxY + iBoxH) > pPxm->m_iPxmH)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Filter rectangle overruns bitmap.\n");
    goto errorExit;
  }

  // Select the search routine depending upon the number of bits-per-pixel.

  iLineSize = pPxm->m_iPxmLineSize;

  if (pPxm->m_iPxmB == 8) {
    // Filter an 8 bits-per-pixel bitmap.

    if (iRangeLo > 255) {
      g_iErrorCode = ERROR_DATA_ILLEGAL;
      sprintf(g_aErrorMessage,
        "(DATA) Lo-filter limit is too large for an 8-bpp bitmap.\n");
      goto errorExit;
    }

    if (iRangeHi > 255) {
      g_iErrorCode = ERROR_DATA_ILLEGAL;
      sprintf(g_aErrorMessage,
        "(DATA) Hi-filter limit is too large for an 8-bpp bitmap.\n");
      goto errorExit;
    }

    uLo = iRangeLo;
    uHi = iRangeHi;

    pLin = pPxm->m_pPxmPixels + (iBoxY * iLineSize) + iBoxX;

    for (i = iBoxH; i != 0; i -= 1) {
      pPxl = pLin;
      for (j = iBoxW; j != 0; j -= 1) {
        if ((*pPxl < uLo) || (*pPxl > uHi))
          *pPxl = 0;
        pPxl += 1;
      }
      iBoxY += 1;
      iBoxH -= 1;
      pLin += iLineSize;
    }
    // End of 8 bits-per-pixel filter.
  }

  else {
    // Can't handle this number of bits-per-pixel.

    sprintf(g_aErrorMessage,
      "(DATA) Can't filter a %u bpp bitmap.\n",
      (int)pPxm->m_iPxmB);
    goto errorUnknown;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorUnknown:

  g_iErrorCode = ERROR_DATA_UNKNOWN;

errorExit:

  return (g_iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapBoundingBox ()
//
// Find the smallest box that contains all the non-zero pixel data
//
// Inputs  PXLMAP_T *  Ptr to bitmap
//     int *      Ptr to X coordinate of box
//     int *      Ptr to Y coordinate of box
//     int *      Ptr to W of box
//     int *      Ptr to H of box
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapBoundingBox (
  PXLMAP_T *pPxm, int *pBoxX, int *pBoxY, int *pBoxW, int *pBoxH )

{
  // Local variables.

  uint8_t *pLin;
  uint8_t *pPxl;

  int iPxmx;
  int iPxmy;
  int iPxmw;
  int iPxmh;

  int iLineSize;

  int i;
  int j;

  // Get the search box position and size (relative to the bitmap's physical
  // coordinates, rather than its logical coordinates).

  iPxmx = *pBoxX - pPxm->m_iPxmXTopLeft;
  iPxmy = *pBoxY - pPxm->m_iPxmYTopLeft;
  iPxmw = *pBoxW;
  iPxmh = *pBoxH;

  if ((iPxmw == 0) || (iPxmh == 0)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Can't find boundaries in a zero size box.\n");
    goto errorExit;
  }

  // Is the search rectangle within the bitmap ?

  if ((iPxmx < 0) || ((iPxmx + iPxmw) > pPxm->m_iPxmW) ||
      (iPxmy < 0) || ((iPxmy + iPxmh) > pPxm->m_iPxmH)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Boundary rectangle overruns bitmap.\n");
    goto errorExit;
  }

  // Select the search routine depending upon the number of bits-per-pixel.

  iLineSize = pPxm->m_iPxmLineSize;

  // Is it an 8-bpp bitmap ?

  if (pPxm->m_iPxmB == 8) {
    // Search an 8 bits-per-pixel bitmap.
    // Find the top edge.

    pLin = pPxm->m_pPxmPixels + (iPxmy * iLineSize) + iPxmx;

    for (i = iPxmh; i != 0; i -= 1) {
      pPxl = pLin;
      for (j = iPxmw; j != 0; j -= 1) {
        if (*pPxl != 0) goto foundTop;
        pPxl += 1;
      }
      iPxmy += 1;
      iPxmh -= 1;
      pLin += iLineSize;
    }

foundTop:

    // If i == 0 then the box was blank, else we have found the top edge
    // and can continue on with the search for the other edges.

    if (i == 0) {
      // Box is blank.
      //
      // Return with zero sized box.

      *pBoxW = 0;
      *pBoxH = 0;
    }
    else {
      // Box is not blank.
      //
      // Find the bottom edge.

      pLin = pPxm->m_pPxmPixels + ((iPxmy + iPxmh - 1) * iLineSize) + iPxmx;

      for (i = iPxmh; i != 0; i -= 1) {
        pPxl = pLin;
        for (j = iPxmw; j != 0; j -= 1) {
          if (*pPxl != 0) goto foundBottom;
          pPxl += 1;
        }
        iPxmh -= 1;
        pLin -= iLineSize;
      }

foundBottom:

      // Find the left edge.

      pLin = pPxm->m_pPxmPixels + (iPxmy * iLineSize) + iPxmx;

      for (i = iPxmw; i != 0; i -= 1) {
        pPxl = pLin;
        for (j = iPxmh; j != 0; j -= 1) {
          if (*pPxl != 0) goto foundLeft;
          pPxl += iLineSize;
        }
        iPxmx += 1;
        iPxmw -= 1;
        pLin += 1;
      }

foundLeft:

      // Find the right edge.

      pLin = pPxm->m_pPxmPixels + (iPxmy * iLineSize) + (iPxmx + iPxmw - 1);

      for (i = iPxmw; i != 0; i -= 1) {
        pPxl = pLin;
        for (j = iPxmh; j != 0; j -= 1) {
          if (*pPxl != 0) goto foundRight;
          pPxl += iLineSize;
        }
        iPxmw -= 1;
        pLin -= 1;
      }

foundRight:

      // Return logical coordinates relative to the origin.

      *pBoxX = iPxmx + pPxm->m_iPxmXTopLeft;
      *pBoxY = iPxmy + pPxm->m_iPxmYTopLeft;
      *pBoxW = iPxmw;
      *pBoxH = iPxmh;
    }

    // End of 8 bits-per-pixel search.
  }

  // Wierd size bitmap.

  else {
    // Can't handle this number of bits-per-pixel.
    sprintf(g_aErrorMessage,
      "(DATA) Can't find boundaries of a %u bpp bitmap.\n",
      (int)pPxm->m_iPxmB);
    goto errorUnknown;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorUnknown:

  g_iErrorCode = ERROR_DATA_UNKNOWN;

errorExit:

  return (g_iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapPalettize ()
//
// Flag characters that use more than one palette
//
// Inputs  PXLMAP_T *  Ptr to bitmap
//     int        X coordinate of box to remap
//     int        Y coordinate of box to remap
//     int        W of box to remap
//     int        H of box to remap
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapPalettize (
  PXLMAP_T *pPxm, int iBoxX, int iBoxY, int iBoxW, int iBoxH )

{
  // Local variables.

  uint8_t *pLin;

  int iLineSize;

  int i;
  int j;

  int ix;
  int iy;

  uint8_t uPal;

  uint8_t aPalette[16];

  // Round width and height to character multiples.

  iBoxW = (iBoxW + 7) & ~7;
  iBoxH = (iBoxH + 7) & ~7;

  // Get the remap box position and size (relative to the bitmap's physical
  // coordinates, rather than its logical coordinates).

  iBoxX -= pPxm->m_iPxmXTopLeft;
  iBoxY -= pPxm->m_iPxmYTopLeft;

  if ((iBoxW == 0) || (iBoxH == 0)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Can't filter characters in a zero size box.\n");
    goto errorExit;
  }

  // Is the search rectangle within the bitmap ?

  if ((iBoxX < 0) || ((iBoxX + iBoxW) > pPxm->m_iPxmW) ||
      (iBoxY < 0) || ((iBoxY + iBoxH) > pPxm->m_iPxmH)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Filter characters rectangle overruns bitmap.\n");
    goto errorExit;
  }

  // Select the search routine depending upon the number of bits-per-pixel.

  iLineSize = pPxm->m_iPxmLineSize;

  if (pPxm->m_iPxmB == 8) {
    // Remap an 8 bits-per-pixel bitmap.

    pLin = pPxm->m_pPxmPixels + (iBoxY * iLineSize) + iBoxX;

    for (ix = 0; ix != (int)iBoxW; ix += 8) {
      for (iy = 0; iy != (int)iBoxH; iy += 8) {
        // Clear palette used flags.

        for (i = 0; i < 16; i += 1) aPalette[i] = 0;

        // Find out which palettes are used within this character.

        pLin = pPxm->m_pPxmPixels +
               ((iBoxY + iy) * iLineSize) + (iBoxX + ix);

        for (i = 0; i != 8; i += 1) {
          for (j = 0; j != 8; j += 1) {
            uPal = 0x0Fu & (*pLin >> 4);

            aPalette[uPal] += 1;

            pLin += 1;
          }
          pLin += iLineSize - 8;
        }

        // Select the palette to use.

        uPal = 0x00;

        for (i = 0; i < 16; i += 1) {
          if ((aPalette[i] != 0) && (aPalette[i] != 64)) {
//            uPal = (UB) (i << 4);
            uPal = 0x80u;
            break;
          }
        }

        // Remap the character to use the selected palette.

        if (uPal != 0) {
          pLin = pPxm->m_pPxmPixels +
                 ((iBoxY + iy) * iLineSize) + (iBoxX + ix);

          for (i = 0; i != 8; i += 1) {
            for (j = 0; j != 8; j += 1) {
              *pLin = (0x0Fu & (*pLin)) | uPal;

              pLin += 1;
            }
            pLin += iLineSize - 8;
          }
        }
      }
    }

    // End of 8 bits-per-pixel filter.
  }
  else {
    // Can't handle this number of bits-per-pixel.

    sprintf(g_aErrorMessage,
      "Can't filter characters in a bitmap that has %u bits-per-pixel.\n"
      "(DATA, PxlmapPalettize)\n",
      (int)pPxm->m_iPxmB);
    goto errorUnknown;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorUnknown:

  g_iErrorCode = ERROR_DATA_UNKNOWN;

errorExit:

  return (g_iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapDuplicate ()
//
// Makes a copy of a data block
//
// Inputs  PXLMAP_T *   Ptr to block to copy
//
// Output  PXLMAP_T *   Ptr to new block, or NULL if an error
//

PXLMAP_T * PxlmapDuplicate (
  PXLMAP_T *pdbold )

{
  // Local variables.

  PXLMAP_T *pdbnew;
  PXLMAP_T *pbmh;

  // Does it exist ?

  if (pdbold == NULL) {
    g_iErrorCode = ERROR_DATA_UNKNOWN;
    sprintf(g_aErrorMessage,
      "(DATA) Unable to duplicate NULL data object.\n");
    goto errorExit;
  }

  // Is it a DATA_PXLMAP ?

  pdbnew = malloc(pdbold->m_uPxmSize);
  if (pdbnew == NULL) {
    g_iErrorCode = ERROR_NO_MEMORY;
    goto errorExit;
  }
  memcpy(pdbnew, pdbold, pdbold->m_uPxmSize);
  pbmh = (PXLMAP_T *)pdbnew;
  pbmh->m_pPxmPixels = (uint8_t *)(pbmh + 1);

  // Return with success.

  return (pdbnew);

  // Error handlers (reached via the dreaded goto).

errorExit:

  return ((PXLMAP_T *)NULL);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapFree ()
//
// Frees up the memory belonging to the data block
//
// Inputs  PXLMAP_T *   Ptr to block to free
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapFree (
  PXLMAP_T *pPxm )

{
  if (pPxm != NULL) {
    free(pPxm);
  }

  return (ERROR_NONE);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapToIA4 ()
//
// Convert a 32-bpp RGBA bitmap to an 8-bpp IA4 bitmap
//
// Inputs  PXLMAP_T *  Ptr to bitmap to convert
//
// Output  PXLMAP_T *  Ptr to new bitmap, or NULL if an error
//
// N.B.  The original bitmap is always free'd even if an error occurs.
//

PXLMAP_T *PxlmapToIA4 (
  PXLMAP_T *pOldPxm )

{
  // Local variables.

  PXLMAP_T *pNewPxm;

  RGBQUAD_T *pPalette;

  int i;
  int iA;

  int iBGr;
  int iBGg;
  int iBGb;

  int iFGr;
  int iFGg;
  int iFGb;

  int iFr;
  int iFg;
  int iFb;

  int iBr;
  int iBg;
  int iBb;

  uint8_t *pSrcPtr;
  uint8_t *pDstPtr;

  uint8_t *pSrcCol;
  uint8_t *pDstCol;

  RGBQUAD_T *pSrcPxl;

  int iSrcH;
  int iSrcW;

  // Does it exist ?

  if (pOldPxm == NULL) {
    g_iErrorCode = ERROR_DATA_UNKNOWN;
    sprintf(g_aErrorMessage,
      "(DATA) Unable to convert NULL data object to IA4.\n");
    goto errorExit;
  }

  // Is it a DATA_PXLMAP ?

  if (pOldPxm->m_iPxmB != 32) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Only able to convert a 32-bpp bitmap to IA4.\n");
    goto errorExit;
  }

  // Create a new 8-bpp bitmap of the same size as the source.

//  pNewPxm = PxlmapAlloc(pOldPxm->m_iPxmW, pOldPxm->m_iPxmH, 8, false);
  pNewPxm = PxlmapAlloc(pOldPxm->m_iPxmW, pOldPxm->m_iPxmH, 8, true);

  if (pNewPxm == NULL)
    goto errorExit;

  // Inherit a few settings from the old bitmap.

  pNewPxm->m_iPxmXTopLeft = pOldPxm->m_iPxmXTopLeft;
  pNewPxm->m_iPxmYTopLeft = pOldPxm->m_iPxmYTopLeft;

  // Read the background pixel colour from the image data.

  pSrcPxl = (RGBQUAD_T *)pOldPxm->m_pPxmPixels;

  iBGr = pSrcPxl->m_uRgbR;
  iBGg = pSrcPxl->m_uRgbG;
  iBGb = pSrcPxl->m_uRgbB;

  // Create a palette for the new image that simulates IA4.

  pPalette = pNewPxm->m_aPxmC;

  iFGr = 0xFFu;
  iFGg = 0xFFu;
  iFGb = 0xFFu;

//  iBGr = 0x00u;
//  iBGg = 0x80u;
//  iBGb = 0x00u;

  // Construct palette for A in hi 4-bits and I in lo 4-bits.

  for (iA = 0; iA < 16; iA += 1) {
    iFr = ((iFGr * (iA)) / 15);
    iFg = ((iFGg * (iA)) / 15);
    iFb = ((iFGb * (iA)) / 15);

    iBr = ((iBGr * (15 - iA)) / 15);
    iBg = ((iBGg * (15 - iA)) / 15);
    iBb = ((iBGb * (15 - iA)) / 15);

    for (i = 0; i < 16; i += 1) {
      pPalette->m_uRgbA = iA << 4;  // 08May04 JCB - Use real alpha for the palette.

      pPalette->m_uRgbR = (((iFr * i) / 15) + iBr);
      pPalette->m_uRgbG = (((iFg * i) / 15) + iBg);
      pPalette->m_uRgbB = (((iFb * i) / 15) + iBb);

      pPalette += 1;
    }
  }

  // Now read in the uncompressed frame.

  pSrcPtr = pOldPxm->m_pPxmPixels;
  pDstPtr = pNewPxm->m_pPxmPixels;

  iSrcH = pOldPxm->m_iPxmH;

  do {
    pSrcCol = pSrcPtr;
    pSrcPtr += pOldPxm->m_iPxmLineSize;

    pDstCol = pDstPtr;
    pDstPtr += pNewPxm->m_iPxmLineSize;

    iSrcW = pOldPxm->m_iPxmW;

    do {
      // Locate the truecolor pixel.

      pSrcPxl = (RGBQUAD_T *)pSrcCol;

      // Remove the background color components.

      iA = pSrcPxl->m_uRgbA;

      iFGr = pSrcPxl->m_uRgbR - ((iBGr * (255 - iA)) / 255);
      iFGg = pSrcPxl->m_uRgbG - ((iBGg * (255 - iA)) / 255);
      iFGb = pSrcPxl->m_uRgbB - ((iBGb * (255 - iA)) / 255);

      if (iFGr < 0) iFGr = 0;
      if (iFGg < 0) iFGg = 0;
      if (iFGb < 0) iFGb = 0;

      // Convert from RGB to grayscale.

      iFGr = (iFGr * 77);
      iFGg = (iFGg * 150);
      iFGb = (iFGb * 28);

      i = (iFGr + iFGg + iFGb) / 255;

      // Clamp the components to 4bpp.

      i = (i + 8) >> 4;

      if (i > 15) i = 15;

      iA = (iA + 8) >> 4;

      if (iA > 15) iA = 15;

      //

      if (iA == 0) {
        if (i < 8) {
          // Catch stray pixels that are should be the background colour.

          i = 0;
        }
        else {
          // Catch stray index Lines that don't have the alpha channel set correctly.

          i = 15;
          iA = 15;
        }
      }

      // Save the converted pixel.

      *pDstCol = ((iA << 4) + (i));

      // Move onto the next pixel.

      pSrcCol += sizeof(RGBQUAD_T);
      pDstCol += 1;
    } while (--iSrcW > 0);
  } while (--iSrcH != 0);

  // Free up the original bitmap.

  if (pOldPxm != NULL)
    PxlmapFree(pOldPxm);

  // Return with success.

  return (pNewPxm);

  // Error handlers (reached via the dreaded goto).

errorExit:

  if (pOldPxm != NULL)
    PxlmapFree(pOldPxm);

  return (NULL);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapFontTo4bpp ()
//
// Convert a 32-bpp RGBA bitmap to an 4-bpp bitmap
//
// Inputs  PXLMAP_T *  Ptr to bitmap to convert
//
// Output  PXLMAP_T *  Ptr to new bitmap, or NULL if an error
//
// N.B.  The original bitmap is always free'd even if an error occurs.
//

RGBQUAD_T g_aFontPalette [] =
{
  {0x00u, 0x00u, 0x00u, 0x00u}, // 0

  {0x00u, 0x00u, 0x00u, 0x33u}, // 1
  {0x00u, 0x00u, 0x00u, 0x66u}, // 2
  {0x00u, 0x00u, 0x00u, 0x99u}, // 3
  {0x00u, 0x00u, 0x00u, 0xCCu}, // 4

  {0x66u, 0x66u, 0x66u, 0x33u}, // 5
  {0x99u, 0x99u, 0x99u, 0x66u}, // 6
  {0xCCu, 0xCCu, 0xCCu, 0x99u}, // 7
  {0xFFu, 0xFFu, 0xFFu, 0xCCu}, // 8

  {0x00u, 0x00u, 0x00u, 0xFFu}, // 9
  {0x2Au, 0x2Au, 0x2Au, 0xFFu}, // 10
  {0x54u, 0x54u, 0x54u, 0xFFu}, // 11
  {0x7Eu, 0x7Eu, 0x7Eu, 0xFFu}, // 12
  {0xA8u, 0xA8u, 0xA8u, 0xFFu}, // 13
  {0xD2u, 0xD2u, 0xD2u, 0xFFu}, // 14
  {0xFFu, 0xFFu, 0xFFu, 0xFFu}  // 15
};

PXLMAP_T *PxlmapFontTo4bpp (
  PXLMAP_T *pOldPxm )

{
  // Local variables.

  PXLMAP_T *pNewPxm;

  int uAlpha;
  int uColor;
//  int uError;
  int iPixel;
  int i;

  uint8_t *pSrcPtr;
  uint8_t *pDstPtr;

  uint8_t *pSrcCol;
  uint8_t *pDstCol;

  int iSrcH;
  int iSrcW;

  // Does it exist ?

  if (pOldPxm == NULL) {
    g_iErrorCode = ERROR_DATA_UNKNOWN;
    sprintf(g_aErrorMessage,
      "(DATA) Unable to convert NULL data object to IA4.\n");
    goto errorExit;
  }

  // Is it a DATA_PXLMAP ?

  if (pOldPxm->m_iPxmB < 8) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Only able to convert an 8, 24 or 32-bpp bitmap to font.\n");
    goto errorExit;
  }

  // Create a new 8-bpp bitmap of the same size as the source.

  pNewPxm = PxlmapAlloc(pOldPxm->m_iPxmW, pOldPxm->m_iPxmH, 8, true);

  if (pNewPxm == NULL)
    goto errorExit;

  // Inherit a few settings from the old bitmap.

  pNewPxm->m_iPxmXTopLeft = pOldPxm->m_iPxmXTopLeft;
  pNewPxm->m_iPxmYTopLeft = pOldPxm->m_iPxmYTopLeft;

  // Create a palette for the new font image that simulates IA4.

  pNewPxm->m_iPxmPaletteHasAlpha = 1;

  for (i = 0; i < 16; i++) {
    // Save the real palette that the game will use.

    pNewPxm->m_aPxmC[i] = g_aFontPalette[i];

    // Save the test palette that debug dump will use.

    uAlpha = g_aFontPalette[i].m_uRgbA;

    pNewPxm->m_aPxmC[i + 16].m_uRgbB =
//      ((g_aFontPalette[i].m_uRgbB * uAlpha) + (0x80u * (255 - uAlpha))) >> 8;
      ((g_aFontPalette[i].m_uRgbB * uAlpha)) >> 8;

    pNewPxm->m_aPxmC[i + 16].m_uRgbG =
//      ((g_aFontPalette[i].m_uRgbG * uAlpha)) >> 8;
      ((g_aFontPalette[i].m_uRgbG * uAlpha) + (0x80u * (255 - uAlpha))) >> 8;

    pNewPxm->m_aPxmC[i + 16].m_uRgbR =
      ((g_aFontPalette[i].m_uRgbR * uAlpha)) >> 8;

    pNewPxm->m_aPxmC[i + 16].m_uRgbA = 255;
  }

  for (i = 32; i < 256; i++) {
    pNewPxm->m_aPxmC[i].m_uRgbA = 0;
    pNewPxm->m_aPxmC[i].m_uRgbR = 0;
    pNewPxm->m_aPxmC[i].m_uRgbG = 0;
    pNewPxm->m_aPxmC[i].m_uRgbB = 0;
  }

  // Now read in the uncompressed frame.

  pSrcPtr = pOldPxm->m_pPxmPixels;
  pDstPtr = pNewPxm->m_pPxmPixels;

  iSrcH = pOldPxm->m_iPxmH;

  do {
    pSrcCol = pSrcPtr;
    pSrcPtr += pOldPxm->m_iPxmLineSize;

    pDstCol = pDstPtr;
    pDstPtr += pNewPxm->m_iPxmLineSize;

    iSrcW = pOldPxm->m_iPxmW;

    do {
      // Locate the truecolor pixel.
      //
      // The font is created in Photoshop as bright green font on
      // a bright red background.
      //
      // The red and green channels are then used to determine the
      // alpha and greyscale values.

      uColor = 0;

      if (pOldPxm->m_iPxmB == 8) {
        RGBQUAD_T *pQuad = &(pOldPxm->m_aPxmC[(*pSrcCol)]);
        uAlpha = pQuad->m_uRgbR;
        uColor = pQuad->m_uRgbG;
//        uError = pQuad->m_uRgbB;
      }
      else
      if (pOldPxm->m_iPxmB == 24) {
        RGBTRPL_T *pTrpl = (RGBTRPL_T *)pSrcCol;
        uAlpha = pTrpl->m_uRgbR;
        uColor = pTrpl->m_uRgbG;
//        uError = pTrpl->m_uRgbB;
      }
      else
      if (pOldPxm->m_iPxmB == 32) {
        RGBQUAD_T *pQuad = (RGBQUAD_T *)pSrcCol;
        uAlpha = pQuad->m_uRgbR;
        uColor = pQuad->m_uRgbG;
//        uError = pQuad->m_uRgbB;
      }

      //

      if (uAlpha < uColor) {
//        if (uError > 0)
//          uColor = uColor - (uAlpha);
        uAlpha = 255;
      }
      else {
        uAlpha = 255 - (uAlpha - uColor);
        uColor = 0;
      }

      // Convert the color and alpha into a palette index.

      if (uAlpha < 0x1Au)
        iPixel = 0;
      else
      if (uAlpha < 0xEAu) {
        if (uColor < 0x33u) {
          iPixel = ((uAlpha + 0x1A) / 0x33u) + 0;
          if (iPixel > 4) iPixel = 4;
        }
        else {
          iPixel = ((uAlpha + 0x1A) / 0x33u) + 4;
          if (iPixel > 8) iPixel = 8;
        }
      }
      else {
        iPixel = ((uColor + 0x15) / 0x2Au) + 9;
        if (iPixel > 15) iPixel = 15;
      }

      // Write the pixel into the 8bpp bitmap +16 so that it uses the test palette.

//      if (iPixel != 0) iPixel += 16;
      iPixel += 16;

      // Save the converted pixel.

      *pDstCol = iPixel;

      // Move onto the next pixel.

      if (pOldPxm->m_iPxmB == 8)
        pSrcCol += 1;
      else
      if (pOldPxm->m_iPxmB == 24)
        pSrcCol += sizeof(RGBTRPL_T);
      else
      if (pOldPxm->m_iPxmB == 32)
        pSrcCol += sizeof(RGBQUAD_T);

      pDstCol += 1;
    } while (--iSrcW > 0);
  } while (--iSrcH != 0);

  // Free up the original bitmap.

  if (pOldPxm != NULL)
    PxlmapFree( pOldPxm );

  // Return with success.

  return (pNewPxm);

  // Error handlers (reached via the dreaded goto).

errorExit:

  if (pOldPxm != NULL)
    PxlmapFree( pOldPxm );

  return (NULL);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapFixTransparency ()
//
// Make sure that all transparent pixels are black
//
// Inputs  PXLMAP_T *  Ptr to bitmap to fix
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE PxlmapFixTransparency (
  PXLMAP_T *pPxlmap )

{
  // Local variables.

  uint8_t *     pRow;
  RGBQUAD_T *   pCol;
  int           uCol;
  int           uRow;
  uint8_t       uThreshold;
  int           iNumSolid;
  int           iNumTranslucent;
  int           iNumTransparent;
  int           iNumNonZero;

  //

  if ((pPxlmap->m_iPxmB == 32) ||
      (pPxlmap->m_iPxmB <= 8 && pPxlmap->m_iPxmPaletteHasAlpha != 0)) {
    // This bitmap has transparency!
  }
  else {
    // This bitmap has no transparency, return immediately.
    return (ERROR_NONE);
  }

  //

  uThreshold = 16; // Was 32.

  iNumSolid = 0;
  iNumTranslucent = 0;
  iNumTransparent = 0;
  iNumNonZero = 0;

  if (pPxlmap->m_iPxmB == 32) {
    // Scan and fix the entire source image.

    for (uRow = pPxlmap->m_iPxmH, pRow = pPxlmap->m_pPxmPixels; uRow != 0; uRow -= 1, pRow += pPxlmap->m_iPxmLineSize) {
      for (uCol = pPxlmap->m_iPxmW, pCol = (RGBQUAD_T *)pRow; uCol != 0; pCol += 1, uCol -= 1) {
        if (g_bFlattenAlpha) {
          if (pCol->m_uRgbA < 48) {
            ++iNumTransparent;

            pCol->m_uRgbB = 0;
            pCol->m_uRgbG = 255;
            pCol->m_uRgbR = 0;
            pCol->m_uRgbA = 0;
          }
          else
          if (pCol->m_uRgbA < 240) {
            ++iNumTranslucent;

            pCol->m_uRgbB = (pCol->m_uRgbB * pCol->m_uRgbA) / 255;
            pCol->m_uRgbG = (pCol->m_uRgbG * pCol->m_uRgbA) / 255;
            pCol->m_uRgbR = (pCol->m_uRgbR * pCol->m_uRgbA) / 255;
            pCol->m_uRgbA = 255;
          }
          else
            ++iNumSolid;
        }
        else {
          if (pCol->m_uRgbA < uThreshold) {
            ++iNumTransparent;

            if (pCol->m_uRgbB || pCol->m_uRgbG || pCol->m_uRgbR) ++iNumNonZero;

            pCol->m_uRgbB =
              pCol->m_uRgbG =
                pCol->m_uRgbR = 0;
          }
          else
          if (pCol->m_uRgbA < 240)
            ++iNumTranslucent;
          else
            ++iNumSolid;
        }
      }
    }

    // Report the results.

    printf("Found %d solid pixels.\n", iNumSolid);
    printf("Found %d translucent pixels.\n", iNumTranslucent);
    printf("Found %d transparent pixels.\n", iNumTransparent);
    printf("Fixed %d transparent pixels that were not black.\n", iNumNonZero);
  }
  else {
    // Scan and fix the palette.

    for (uCol = 256, pCol = pPxlmap->m_aPxmC; uCol != 0; uCol -= 1, pCol += 1) {
      if (pCol->m_uRgbA < uThreshold) {
        ++iNumTransparent;

        if (pCol->m_uRgbB || pCol->m_uRgbG || pCol->m_uRgbR) ++iNumNonZero;

        pCol->m_uRgbB =
          pCol->m_uRgbG =
            pCol->m_uRgbR = 0;
      }
      else
      if (pCol->m_uRgbA < 240)
        ++iNumTranslucent;
      else
        ++iNumSolid;
    }

    // Report the results.

    printf("Found %d solid palette entries.\n", iNumSolid);
    printf("Found %d translucent palette entries.\n", iNumTranslucent);
    printf("Found %d transparent palette entries.\n", iNumTransparent);
    printf("Fixed %d transparent palette entries that were not black.\n", iNumNonZero);
  }

  // Return with success code.

  return (ERROR_NONE);
}



// **************************************************************************
// **************************************************************************
//
// MakeAlphaRemapTable ()
//
// Create a palette remapping table by alpha
//
// Inputs  PXLMAP_T *  Ptr to bitmap to fix
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE MakeAlphaRemapTable (
  PXLMAP_T *pPxlmap )

{
  // Local variables.

  int i;

  // Is it a DATA_PXLMAP ?

  if (pPxlmap->m_iPxmB != 8) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Only able to create an alpha remap table for an 8-bpp bitmap.\n");
    goto errorExit;
  }

  // Initialize the remapping table.

  for (i = 0; i < 256; i++) RemappingTable[i] = i;

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorExit:

  return (g_iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// PxlmapDifference ()
//
// Inputs  PXLMAP_T *  Ptr to destination bitmap
//         PXLMAP_T *  Ptr to source bitmap
//
// Output  int     ERROR_NONE if OK
//

ERRORCODE     PxlmapDifference (
  PXLMAP_T *    pPxmDst,
  PXLMAP_T *    pPxmSrc)

{
  // Local variables.

  uint8_t *     pSrcLin;
  uint8_t *     pDstLin;
  uint8_t *     pSrcPxl;
  uint8_t *     pDstPxl;
  int           iLineSizeSrc;
  int           iLineSizeDst;
  int           i;
  int           j;

  // Images nust be the same bpp unless I write some more code.

  if (pPxmDst->m_iPxmB != pPxmSrc->m_iPxmB) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Unable to remove identical pixels between images with different bpp.\n");
    goto errorExit;
  }

  // Images nust be the same size unless I write some more code.

  if ((pPxmDst->m_iPxmW != pPxmSrc->m_iPxmW) ||
      (pPxmDst->m_iPxmH != pPxmSrc->m_iPxmH)) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Unable to remove identical pixels between images with different sizes.\n");
    goto errorExit;
  }

  // Select the search routine depending upon the number of bits-per-pixel.

  iLineSizeSrc = pPxmSrc->m_iPxmLineSize;
  iLineSizeDst = pPxmDst->m_iPxmLineSize;

  pSrcLin = pPxmSrc->m_pPxmPixels;
  pDstLin = pPxmDst->m_pPxmPixels;

  //

  if (pPxmSrc->m_iPxmB == 8) {
    // Remove identical pixels between two 8 bits-per-pixel bitmaps.
    for (i = pPxmDst->m_iPxmH; i != 0; i -= 1) {
      pSrcPxl = pSrcLin;
      pDstPxl = pDstLin;

      for (j = pPxmDst->m_iPxmW; j != 0; j -= 1) {
        if (pDstPxl[0] == pSrcPxl[0])
          pDstPxl[0] = uDifferenceColorI;
        pDstPxl += 1;
        pSrcPxl += 1;
      }
      pSrcLin += iLineSizeSrc;
      pDstLin += iLineSizeDst;
    }
    // End of 8 bits-per-pixel differencing.
  }
  else
  if (pPxmSrc->m_iPxmB == 24) {
    // Remove identical pixels between two 24 bits-per-pixel bitmaps.
    for (i = pPxmDst->m_iPxmH; i != 0; i -= 1) {
      pSrcPxl = pSrcLin;
      pDstPxl = pDstLin;

      for (j = pPxmDst->m_iPxmW; j != 0; j -= 1) {
        if ((pDstPxl[0] == pSrcPxl[0]) &&
            (pDstPxl[1] == pSrcPxl[1]) &&
            (pDstPxl[2] == pSrcPxl[2])) {
          pDstPxl[0] = uDifferenceColorB;
          pDstPxl[1] = uDifferenceColorG;
          pDstPxl[2] = uDifferenceColorR;
        }
        pDstPxl += 3;
        pSrcPxl += 3;
      }
      pSrcLin += iLineSizeSrc;
      pDstLin += iLineSizeDst;
    }
    // End of 24 bits-per-pixel differencing.
  }
  else
  if (pPxmSrc->m_iPxmB == 32) {
    // Remove identical pixels between two 32 bits-per-pixel bitmaps.
    for (i = pPxmDst->m_iPxmH; i != 0; i -= 1) {
      pSrcPxl = pSrcLin;
      pDstPxl = pDstLin;

      for (j = pPxmDst->m_iPxmW; j != 0; j -= 1) {
        if ((pDstPxl[0] == pSrcPxl[0]) &&
            (pDstPxl[1] == pSrcPxl[1]) &&
            (pDstPxl[2] == pSrcPxl[2]) &&
            (pDstPxl[3] == pSrcPxl[3])) {
          pDstPxl[0] = uDifferenceColorB;
          pDstPxl[1] = uDifferenceColorG;
          pDstPxl[2] = uDifferenceColorR;
          pDstPxl[3] = uDifferenceColorA;
        }
        pDstPxl += 4;
        pSrcPxl += 4;
      }
      pSrcLin += iLineSizeSrc;
      pDstLin += iLineSizeDst;
    }
    // End of 32 bits-per-pixel differencing.
  }
  else {
    // Can't handle this number of bits-per-pixel.

    sprintf(g_aErrorMessage,
      "Can't process \"difference\" of a bitmap that has %u bits-per-pixel.\n"
      "(DATA, PxlmapDifference)\n",
      (int)pPxmSrc->m_iPxmB);
    goto errorUnknown;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorUnknown:

  g_iErrorCode = ERROR_DATA_UNKNOWN;

errorExit:

  return (g_iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// CheckPxlmapSize ()
//
// Check whether the bitmap parameters are legal and understood
//
// Inputs  int        Width
//     int        Height
//     int        Bits-per-pixel
//
// Output  int     ERROR_NONE if OK
//
// N.B.  The restrictions are :
//
//     width   <= 65536
//     height  <= 65536
//     bits  == 1/4/8/24/32
//     size  <= 8MB (this is arbitrary and can easily be increased)
//

static ERRORCODE CheckPxlmapSize (
  int iWidth, int iHeight, int iBits )

{
  // Local variables.

  int s;

  // Start the checks ...

  if ((iBits != 1) && (iBits != 4) && (iBits != 8) && (iBits != 24) && (iBits != 32)) {
    sprintf(g_aErrorMessage,
      "(DATA) Illegal bitmap depth (number of bits must be 1/4/8/24/32).\n");
    goto errorExit;
  }

  if ((iBits != 8) && (iBits != 24) && (iBits != 32)) {
    sprintf(g_aErrorMessage,
      "(DATA) Illegal bitmap depth (8/24/32 bpp are currently implemented).\n");
    goto errorExit;
  }

  if (iWidth > 32768L) {
    sprintf(g_aErrorMessage,
      "(DATA) Illegal bitmap size (max width is 32768).\n");
    goto errorExit;
  }

  if (iHeight > 32768L) {
    sprintf(g_aErrorMessage,
      "(DATA) Illegal bitmap size (max height is 32768).\n");
    goto errorExit;
  }

  s = iWidth * iHeight;

  switch (iBits) {
    case  1: s = s / 8; break;
    case  4: s = s / 2; break;
    case  8: break;
    case 16: s = s * 2; break;
    case 24: s = s * 3; break;
    case 32: s = s * 4; break;
  }

  if (s > (8192 * 1024)) {
    sprintf(g_aErrorMessage,
      "(DATA) Illegal bitmap size (bitmap > 8MB is far too big).\n");
    goto errorExit;
  }

  // Somewhat arbitrary check for width without height or vice versa.

  if (((iWidth == 0) && (iHeight != 0)) || ((iWidth != 0) && (iHeight == 0))) {
    g_iErrorCode = ERROR_DATA_ILLEGAL;
    sprintf(g_aErrorMessage,
      "(DATA) Illegal bitmap size (width without height or vice versa).\n");
    goto errorExit;
  }

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

errorExit:

  return (g_iErrorCode = ERROR_DATA_ILLEGAL);
}
