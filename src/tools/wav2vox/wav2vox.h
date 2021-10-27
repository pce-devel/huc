// **************************************************************************
// **************************************************************************
//
// wav2vox.h
//
// ADPCM Compressor/Decompressor for the OKI MSM5205 and Hudson HuC6230,
// which are used in NEC's PC Engine CD-ROM and NEC's PC-FX consoles.
//
// Copyright John Brandwood 2016-2021.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __WAV2VOX_h
#define __WAV2VOX_h

#include "elmer.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Non-standard WAV type IDs, NOT registered with Microsoft.
//

#define WAV_TAG_MSM5205 0x3000
#define WAV_TAG_HUC6230 0x3001

//
// GLOBAL FUNCTION PROTOTYPES
//

extern int main ( int argc, char **argv );

//
// End of __WAV2VOX_h
//

#ifdef __cplusplus
}
#endif

#endif
