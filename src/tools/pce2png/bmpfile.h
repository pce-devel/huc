// **************************************************************************
// **************************************************************************
//
// bmpfile.h
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

#ifndef __BMPFILE_h
#define __BMPFILE_h

#include "pxlmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_BMP_NOT_HANDLED   -256L
#define ERROR_BMP_TRUNCATED     -257L
#define ERROR_BMP_MALFORMED     -258L
#define ERROR_BMP_MISSING       -259L

extern PXLMAP_T * BmpReadPxlmap (
  const char *pFileName );

extern ERRORCODE BmpDumpPxlmap (
  PXLMAP_T *pPxlmap, const char *pFileName );

#ifdef __cplusplus
}
#endif
#endif // __BMPFILE_h
