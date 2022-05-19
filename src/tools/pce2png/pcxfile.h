// **************************************************************************
// **************************************************************************
//
// pcxfile.h
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

#ifndef __PCXFILE_h
#define __PCXFILE_h

#include "pxlmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_PCX_NOT_HANDLED   -256L
#define ERROR_PCX_TRUNCATED     -257L
#define ERROR_PCX_MALFORMED     -258L
#define ERROR_PCX_MISSING       -259L

extern PXLMAP_T * PcxReadPxlmap (
  const char *pFileName );

extern ERRORCODE PcxDumpPxlmap (
  PXLMAP_T *pPxlmap, const char *pFileName );

#ifdef __cplusplus
}
#endif
#endif // __PCXFILE_h
