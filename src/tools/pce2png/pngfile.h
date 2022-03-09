// **************************************************************************
// **************************************************************************
//
// pngfile.h
//
// Simple interface for reading files in PNG format.
//
// Limitations ...
//
// Lots.
//
// Only the following file contents are currently understood ...
//
//   4 bits-per-pixel, 1 plane, 256 colour palette.
//   8 bits-per-pixel, 1 plane, 256 colour palette.
//  24 bits-per-pixel, 1 plane, no colour palette.
//  32 bits-per-pixel, 1 plane, no colour palette.
//
// Copyright John Brandwood 2000-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __PNGFILE_h
#define __PNGFILE_h

#include "pxlmap.h"
#include "pnglibconf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_PNG_NOT_HANDLED   -256L
#define ERROR_PNG_TRUNCATED     -257L
#define ERROR_PNG_MALFORMED     -258L
#define ERROR_PNG_MISSING       -259L

#ifdef PNG_READ_SUPPORTED
extern PXLMAP_T * PngReadPxlmap (
  const char *pFileName );
#endif

#ifdef PNG_WRITE_SUPPORTED
extern ERRORCODE PngDumpPxlmap (
  PXLMAP_T *pPxlmap, const char *pFileName );
#endif

#ifdef __cplusplus
}
#endif
#endif // __PNGFILE_h
