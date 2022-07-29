// **************************************************************************
// **************************************************************************
//
// adpcmoki.h
//
// Compression code for OKI 12-bit ADPCM (a variant of IMA/DVI ADPCM).
//
// Based on the copyrighted freeware adpcm source V1.2, 18-Dec-92 by
// Stichting Mathematisch Centrum, Amsterdam, The Netherlands.
//
// The algorithm for this coder was taken from the IMA Compatability
// Project proceedings, Vol 2, Number 2; May 1992.
//
// Algorithm also checked against the one given in the document ...
//
// Microsoft Multimedia Standards Update - New Multimedia Data Types
// and Data Techniques (Apr 15 1994, Rev 3.0)
//
// **************************************************************************
// **************************************************************************
//
// Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
// Netherlands.
//
// All Rights Reserved
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose and without fee is hereby granted,
// provided that the above copyright notice appear in all copies and that
// both that copyright notice and this permission notice appear in
// supporting documentation, and that the names of Stichting Mathematisch
// Centrum or CWI not be used in advertising or publicity pertaining to
// distribution of the software without specific, written prior permission.
//
// STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
// THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
// FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
// OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// **************************************************************************
// **************************************************************************
//
// OKI MSM5205 and Hudson HuC6230 changes:
//
// Copyright John Brandwood 2016-2021.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __ADPCMOKI_h
#define __ADPCMOKI_h

#include "elmer.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// GLOBAL DATA STRUCTURES AND DEFINITIONS
//

typedef struct
{
  unsigned      format;   // WAVE_FORMAT_OKI_ADPCM or
                          // WAVE_FORMAT_ELMER_MSM5205 or
                          // WAVE_FORMAT_ELMER_HUC6230
  int           value;    // Previous output value (0..65535).
  int           index;    // Index into stepsize table.
  int           maxvalue; // Max value.
  int           minvalue; // Min value.
} OKI_ADPCM;

//
// GLOBAL VARIABLES
//

extern int iBiasValue;

//
// GLOBAL FUNCTION PROTOTYPES
//

extern void EncodeAdpcmOki4 (
  int16_t *pSrc, uint8_t *pDst, int iSrcLen, int iDstLen, OKI_ADPCM *pState );

extern void DecodeAdpcmOki4 (
  uint8_t *pSrc, int16_t *pDst, int iSrcLen, OKI_ADPCM *pState );

extern void EncodeAdpcmPcfx (
  int16_t *pSrc, uint8_t *pDst, int iSrcLen, int iDstLen, OKI_ADPCM *pState );

extern void DecodeAdpcmPcfx (
  uint8_t *pSrc, int16_t *pDst, int iSrcLen, OKI_ADPCM *pState );

//
// End of __ADPCMOKI_h
//

#ifdef __cplusplus
}
#endif

#endif
