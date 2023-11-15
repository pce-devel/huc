// **************************************************************************
// **************************************************************************
//
// adpcmoki.c
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

#include "elmer.h"

#include "wav2vox.h"
#include "adpcmoki.h"

//
// DEFINITIONS
//

int iBiasValue;

//
// STATIC VARIABLES
//

static int OkiIndxTable [16] =
{
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8
};

static int16_t OkiStepTable [49] =
{   16,   17,   19,   21,   23,   25,   28,   31,   34,   37,
    41,   45,   50,   55,   60,   66,   73,   80,   88,   97,
   107,  118,  130,  143,  157,  173,  190,  209,  230,  253,
   279,  307,  337,  371,  408,  449,  494,  544,  598,  658,
   724,  796,  876,  963, 1060, 1166, 1282, 1411, 1552
};

// PreCalculated OkiStepTable for 12-bit decompression.

static int16_t OkiStepTable12 [49] [16] =
{
/*  0 */ {     2,     6,    10,    14,    18,    22,    26,    30,
              -2,    -6,   -10,   -14,   -18,   -22,   -26,   -30 },
/*  1 */ {     2,     6,    10,    14,    19,    23,    27,    31,
              -2,    -6,   -10,   -14,   -19,   -23,   -27,   -31 },
/*  2 */ {     2,     6,    11,    15,    21,    25,    30,    34,
              -2,    -6,   -11,   -15,   -21,   -25,   -30,   -34 },
/*  3 */ {     2,     7,    12,    17,    23,    28,    33,    38,
              -2,    -7,   -12,   -17,   -23,   -28,   -33,   -38 },
/*  4 */ {     2,     7,    13,    18,    25,    30,    36,    41,
              -2,    -7,   -13,   -18,   -25,   -30,   -36,   -41 },
/*  5 */ {     3,     9,    15,    21,    28,    34,    40,    46,
              -3,    -9,   -15,   -21,   -28,   -34,   -40,   -46 },
/*  6 */ {     3,    10,    17,    24,    31,    38,    45,    52,
              -3,   -10,   -17,   -24,   -31,   -38,   -45,   -52 },
/*  7 */ {     3,    10,    18,    25,    34,    41,    49,    56,
              -3,   -10,   -18,   -25,   -34,   -41,   -49,   -56 },
/*  8 */ {     4,    12,    21,    29,    38,    46,    55,    63,
              -4,   -12,   -21,   -29,   -38,   -46,   -55,   -63 },
/*  9 */ {     4,    13,    22,    31,    41,    50,    59,    68,
              -4,   -13,   -22,   -31,   -41,   -50,   -59,   -68 },
/* 10 */ {     5,    15,    25,    35,    46,    56,    66,    76,
              -5,   -15,   -25,   -35,   -46,   -56,   -66,   -76 },
/* 11 */ {     5,    16,    27,    38,    50,    61,    72,    83,
              -5,   -16,   -27,   -38,   -50,   -61,   -72,   -83 },
/* 12 */ {     6,    18,    31,    43,    56,    68,    81,    93,
              -6,   -18,   -31,   -43,   -56,   -68,   -81,   -93 },
/* 13 */ {     6,    19,    33,    46,    61,    74,    88,   101,
              -6,   -19,   -33,   -46,   -61,   -74,   -88,  -101 },
/* 14 */ {     7,    22,    37,    52,    67,    82,    97,   112,
              -7,   -22,   -37,   -52,   -67,   -82,   -97,  -112 },
/* 15 */ {     8,    24,    41,    57,    74,    90,   107,   123,
              -8,   -24,   -41,   -57,   -74,   -90,  -107,  -123 },
/* 16 */ {     9,    27,    45,    63,    82,   100,   118,   136,
              -9,   -27,   -45,   -63,   -82,  -100,  -118,  -136 },
/* 17 */ {    10,    30,    50,    70,    90,   110,   130,   150,
             -10,   -30,   -50,   -70,   -90,  -110,  -130,  -150 },
/* 18 */ {    11,    33,    55,    77,    99,   121,   143,   165,
             -11,   -33,   -55,   -77,   -99,  -121,  -143,  -165 },
/* 19 */ {    12,    36,    60,    84,   109,   133,   157,   181,
             -12,   -36,   -60,   -84,  -109,  -133,  -157,  -181 },
/* 20 */ {    13,    39,    66,    92,   120,   146,   173,   199,
             -13,   -39,   -66,   -92,  -120,  -146,  -173,  -199 },
/* 21 */ {    14,    43,    73,   102,   132,   161,   191,   220,
             -14,   -43,   -73,  -102,  -132,  -161,  -191,  -220 },
/* 22 */ {    16,    48,    81,   113,   146,   178,   211,   243,
             -16,   -48,   -81,  -113,  -146,  -178,  -211,  -243 },
/* 23 */ {    17,    52,    88,   123,   160,   195,   231,   266,
             -17,   -52,   -88,  -123,  -160,  -195,  -231,  -266 },
/* 24 */ {    19,    58,    97,   136,   176,   215,   254,   293,
             -19,   -58,   -97,  -136,  -176,  -215,  -254,  -293 },
/* 25 */ {    21,    64,   107,   150,   194,   237,   280,   323,
             -21,   -64,  -107,  -150,  -194,  -237,  -280,  -323 },
/* 26 */ {    23,    70,   118,   165,   213,   260,   308,   355,
             -23,   -70,  -118,  -165,  -213,  -260,  -308,  -355 },
/* 27 */ {    26,    78,   130,   182,   235,   287,   339,   391,
             -26,   -78,  -130,  -182,  -235,  -287,  -339,  -391 },
/* 28 */ {    28,    85,   143,   200,   258,   315,   373,   430,
             -28,   -85,  -143,  -200,  -258,  -315,  -373,  -430 },
/* 29 */ {    31,    94,   157,   220,   284,   347,   410,   473,
             -31,   -94,  -157,  -220,  -284,  -347,  -410,  -473 },
/* 30 */ {    34,   103,   173,   242,   313,   382,   452,   521,
             -34,  -103,  -173,  -242,  -313,  -382,  -452,  -521 },
/* 31 */ {    38,   114,   191,   267,   345,   421,   498,   574,
             -38,  -114,  -191,  -267,  -345,  -421,  -498,  -574 },
/* 32 */ {    42,   126,   210,   294,   379,   463,   547,   631,
             -42,  -126,  -210,  -294,  -379,  -463,  -547,  -631 },
/* 33 */ {    46,   138,   231,   323,   417,   509,   602,   694,
             -46,  -138,  -231,  -323,  -417,  -509,  -602,  -694 },
/* 34 */ {    51,   153,   255,   357,   459,   561,   663,   765,
             -51,  -153,  -255,  -357,  -459,  -561,  -663,  -765 },
/* 35 */ {    56,   168,   280,   392,   505,   617,   729,   841,
             -56,  -168,  -280,  -392,  -505,  -617,  -729,  -841 },
/* 36 */ {    61,   184,   308,   431,   555,   678,   802,   925,
             -61,  -184,  -308,  -431,  -555,  -678,  -802,  -925 },
/* 37 */ {    68,   204,   340,   476,   612,   748,   884,  1020,
             -68,  -204,  -340,  -476,  -612,  -748,  -884, -1020 },
/* 38 */ {    74,   223,   373,   522,   672,   821,   971,  1120,
             -74,  -223,  -373,  -522,  -672,  -821,  -971, -1120 },
/* 39 */ {    82,   246,   411,   575,   740,   904,  1069,  1233,
             -82,  -246,  -411,  -575,  -740,  -904, -1069, -1233 },
/* 40 */ {    90,   271,   452,   633,   814,   995,  1176,  1357,
             -90,  -271,  -452,  -633,  -814,  -995, -1176, -1357 },
/* 41 */ {    99,   298,   497,   696,   895,  1094,  1293,  1492,
             -99,  -298,  -497,  -696,  -895, -1094, -1293, -1492 },
/* 42 */ {   109,   328,   547,   766,   985,  1204,  1423,  1642,
            -109,  -328,  -547,  -766,  -985, -1204, -1423, -1642 },
/* 43 */ {   120,   360,   601,   841,  1083,  1323,  1564,  1804,
            -120,  -360,  -601,  -841, -1083, -1323, -1564, -1804 },
/* 44 */ {   132,   397,   662,   927,  1192,  1457,  1722,  1987,
            -132,  -397,  -662,  -927, -1192, -1457, -1722, -1987 },
/* 45 */ {   145,   436,   728,  1019,  1311,  1602,  1894,  2185,
            -145,  -436,  -728, -1019, -1311, -1602, -1894, -2185 },
/* 46 */ {   160,   480,   801,  1121,  1442,  1762,  2083,  2403,
            -160,  -480,  -801, -1121, -1442, -1762, -2083, -2403 },
/* 47 */ {   176,   528,   881,  1233,  1587,  1939,  2292,  2644,
            -176,  -528,  -881, -1233, -1587, -1939, -2292, -2644 },
/* 48 */ {   194,   582,   970,  1358,  1746,  2134,  2522,  2910,
            -194,  -582,  -970, -1358, -1746, -2134, -2522, -2910 },
};



// **************************************************************************
// **************************************************************************
//
// EncodeAdpcmOki4 ()
//

void EncodeAdpcmOki4 (
  int16_t *pSrc, uint8_t *pDst, int iSrcLen, int iDstLen, OKI_ADPCM *pState )

{
  // Local Variables.

  int       sign;       // Current adpcm sign value
  int       code;       // Current adpcm output value
  int       diff;       // Difference between current and prev
  int       indx;       // Current step change index
  int       step;       // Stepsize
  int       samp;       // Predicted output value

  int       temp;       // Temporary workspace
  int       flip;       // Alternate nibbles

  int16_t * p1st = pSrc;

  int       dcbias = 0;

  // Initialize the state.

  samp = pState->value;
  indx = pState->index;

  flip = 0;

  // Are we supposed to add a DC bias to this track?

  iBiasValue = (iBiasValue + 1) & ~1;

  if (iBiasValue != 0)
  {
    if (iSrcLen < iBiasValue)
    {
      printf("Sample too short to add/remove DC bias (need at least 200 samples). DC bias reset to zero.\n");
     iBiasValue = 0;
    } else {
      // Clear the first and last samples to make room for the DC bias.

      for (temp = 0; temp < (iBiasValue / 2); ++temp)
      {
        *(pSrc + temp) = 0;
        *(pSrc + iSrcLen - 1 - temp) = 0;
      }
    }
  }

  // While there are samples to encode ...

  while (iDstLen--)
  {
    // Update the DC bias.

    if (iDstLen < (iBiasValue / 2))
    {
      if (dcbias > 0)
      {
        dcbias -= 2;
      }
    }
    else
    {
      if (dcbias < iBiasValue)
      {
        dcbias += 2;
      }
    }

    // Read the next signed 16-bit sample as unsigned 12-bits.

    temp = (32768 >> 4) + dcbias;

    if (iSrcLen)
    {
      --iSrcLen;
      temp = ((((int) *pSrc++) + 32768) >> 4) + dcbias;
    }

    if (temp > 4095)
    {
      temp = 4095;

      printf("DC bias causes sample to clip at sample index %ld.\n",
        (long) (pSrc - p1st - 1));
    }

    // Compute the delta from the previous sample.

    sign = code = 0;

    diff = (temp - samp);

    if (diff < 0) { diff = -diff; sign = 8; }

    // Find the closest ADPCM code less-than-or-equal-to the delta.

    step = OkiStepTable[indx];

    temp = diff - (step >> 3);

    if (temp >= step) { code |= 4; temp -= step; }

    step >>= 1;

    if (temp >= step) { code |= 2; temp -= step; }

    step >>= 1;

    if (temp >= step) { code |= 1; }

    // See if the next higher code is actually closer to the target.

    if (code != 7)
    {
      if ((diff - OkiStepTable12[indx][code+0]) >
        (OkiStepTable12[indx][code+1] - diff))
          ++code;
    }

    // Avoid wrapping in case of overflow by reducing the delta.

    if (pState->format == WAV_TAG_MSM5205)
    {
      int best = code;

      if (sign == 0)
      {
        while ((code >= 0) && ((samp + OkiStepTable12[indx][code]) > 0x0FFF))
          { --code; }
      }
      else
      {
        while ((code >= 0) && ((samp - OkiStepTable12[indx][code]) < 0x0000))
          { --code; }
      }

      if (code != best)
      {
        printf("Changed code from %d to %d to avoid wrap at sample index %ld.\n",
          best, code, (long) (pSrc - p1st - 1));

        if (code < 0) { code = 0; sign ^= 8; }
      }
    }

    code ^= sign;

    // Calculate the output sample value.

    samp += OkiStepTable12[indx][code];

    if (pState->minvalue > samp) pState->minvalue = samp;
    if (pState->maxvalue < samp) pState->maxvalue = samp;

    // Clamp the sample to 12 bits.

    temp = samp;

    if (pState->format == WAV_TAG_MSM5205)
    {
      samp &= 0x0FFF;

      if (samp != temp)
        printf("Waveform wrapped from %5d to %5d at sample index %ld.\n",
          temp, samp, (long) (pSrc - p1st - 1));
    }
    else
    {
      if (samp > 0x0FFF) samp = 0x0FFF;
      else
      if (samp < 0x0000) samp = 0x0000;

      if (samp != temp)
        printf("Waveform clamped from %5d to %5d at sample index %ld.\n",
          temp, samp, (long) (pSrc - p1st - 1));
    }

    // Update the adaptive step index.

    indx += OkiIndxTable[code];

    if (indx <  0) indx =  0;
    else
    if (indx > 48) indx = 48;

    // Output the compressed ADPCM code.

    if (flip) { *pDst |= code; pDst++; }
    else    { *pDst  = code << 4;    }

    flip ^= 1;
    }

  // Output last step, if needed.

  if (flip) pDst++;

  // Update compression state.

  pState->value = samp;
  pState->index = indx;

  // All done.

  return;
}



// **************************************************************************
// **************************************************************************
//
// DecodeAdpcmOki4 ()
//

void DecodeAdpcmOki4 (
  uint8_t *pSrc, int16_t *pDst, int iSrcLen, OKI_ADPCM *pState )

{
  // Local Variables.

  int       code;       // Current adpcm output value
  int       step;       // Stepsize
  int       valdiff;    // Current change to valpred
  int       valpred;    // Predicted output value
  int       indx;       // Current step change index
  int       sample;     // Current output sample

  int       flip;       // Alternate nibbles

  int16_t * p1st = pDst;

  int       dcbias = 0;

  // Initialize the state.

  valpred = pState->value;
  indx  = pState->index;

  step  = OkiStepTable[indx];
  flip  = 0;

  // Are we supposed to add a DC bias to this track?

  if ((iBiasValue != 0) && (iSrcLen < 200))
  {
    iBiasValue = 0;

    printf("Sample too short to add/remove DC bias (need at least 200 samples). DC bias reset to zero.\n");
  }

  iBiasValue = (iBiasValue + 1) & ~1;

  // While there are samples to decode ...

  while (iSrcLen--)
  {
    // Update the DC bias.

    if (iSrcLen < (iBiasValue / 2))
    {
      if (dcbias > 0)
      {
        dcbias -= 2;
      }
    }
    else
    {
      if (dcbias < iBiasValue)
      {
        dcbias += 2;
      }
    }

    // Step 1 - Get the delta value.

    if (flip) { code = *pSrc & 15; pSrc += 1; }
    else    { code = *pSrc >> 4; }

    flip ^= 1;

    // Step 2 - Compute difference and new predicted value.

    valdiff = step >> 3;

    if (code & 4) valdiff += step;
    if (code & 2) valdiff += step >> 1;
    if (code & 1) valdiff += step >> 2;

    valdiff <<= 4; // 12-bit to 16-bit

    if (code & 8)
      valpred -= valdiff;
    else
      valpred += valdiff;

    // Step 4 - clamp output value.

    if (pState->format == WAV_TAG_MSM5205)
    {
      sample = valpred;

      if (sample > 0x0FFF0) sample = 0x0FFF0;
      else
      if (sample < 0x00000) sample = 0x00000;

      if (sample != valpred)
        printf("Waveform saturated from %5d to %5d at sample %ld.\n", valpred, sample, (long) (pDst - p1st));

      valpred &= 0xFFF0u;
    }
    else
    {
      if (valpred > 65535) valpred = 65535;
      else
      if (valpred < 0) valpred = 0;

      sample = valpred;
    }

    if (pState->minvalue > sample) pState->minvalue = sample;
    if (pState->maxvalue < sample) pState->maxvalue = sample;

    // Step 2 - Find new index value (for later).

    indx += OkiIndxTable[code];

    if (indx <  0) indx =  0;
    else
    if (indx > 48) indx = 48;

    // Step 5 - Update step value.

    step = OkiStepTable[indx];

    // Step 6 - sample value (converting from unsigned into signed).

    sample -= (dcbias << 4);

    if (sample < 0)
    {
      sample = 0;

      printf("DC bias causes sample to clip at sample index %ld.\n",
        (long) (pDst - p1st - 1));
    }

    *pDst++ = (sample - 32768);
  }

  // Update compression state.

  pState->value = valpred;
  pState->index = indx;

  // All done.

  return;
}



// **************************************************************************
// **************************************************************************
//
// EncodeAdpcmPcfx ()
//

void EncodeAdpcmPcfx (
  int16_t *pSrc, uint8_t *pDst, int iSrcLen, int iDstLen, OKI_ADPCM *pState )

{
  // Local Variables.

  int       sign;   // Current adpcm sign value
  int       code;   // Current adpcm output value
  int       diff;   // Difference between current and prev
  int       indx;   // Current step change index
  int       step;   // Stepsize
  int       samp;   // Predicted output value

  int       temp;   // Temporary workspace
  int       flip;   // Alternate nibbles

  int16_t * p1st = pSrc;

  // Initialize the state.

  samp = pState->value;
  indx = pState->index;

  flip = 0;

  // While there are samples to encode ...

  while (iDstLen--)
  {
    // Read the next signed 16-bit sample as unsigned 12.3-bits.

    temp = 32768 >> 1;

    if (iSrcLen)
    {
      --iSrcLen;
      temp = ((((int) *pSrc++) + 32768) >> 1);
    }

    // Compute the delta from the previous sample.

    sign = code = 0;

    diff = (temp - samp);

    if (diff < 0) { diff = -diff; sign = 8; }

    // Find the closest ADPCM code rounded to nearest 1/2.

    step = OkiStepTable[indx];

    if (1)
      // Rounded to nearest 1/2
      code = ((((diff << 1) / step) + 1) >> 1) - 1;
    else
      // Not rounded
      code = (diff / step) - 1;

    if (code > 7) code = 7;
    else
    if (code < 0) code = 0;

    // Calculate the output sample value.

    diff = step * (code + 1);

    if (sign)
      samp -= diff;
    else
      samp += diff;

    // Clamp the sample to 12.3 bits.

    temp = samp;

    if (samp > 0x7FFF) samp = 0x7FFF;
    else
    if (samp < 0x0000) samp = 0x0000;

    if (samp != temp)
      printf("Waveform clamped from %5d to %5d at sample index %ld.\n",
        temp, samp, (long) (pSrc - p1st - 1));

    if (pState->minvalue > samp) pState->minvalue = samp;
    if (pState->maxvalue < samp) pState->maxvalue = samp;

    // Update the adaptive step index.

    indx += OkiIndxTable[code];

    if (indx <  0) indx =  0;
    else
    if (indx > 48) indx = 48;

    // Output the compressed ADPCM code.

    code ^= sign;

    if (flip) { *pDst |= code; pDst++; }
    else    { *pDst  = code << 4;    }

    flip ^= 1;
  }

  // Output last step, if needed.

  if (flip) pDst++;

  // Update compression state.

  pState->value = samp;
  pState->index = indx;

  // All done.

  return;
}



// **************************************************************************
// **************************************************************************
//
// DecodeAdpcmPcfx ()
//

void DecodeAdpcmPcfx (
  uint8_t *pSrc, int16_t *pDst, int iSrcLen, OKI_ADPCM *pState )

{
  // Local Variables.

  int       code; // Current adpcm output value
  int       diff; // Current change to valpred
  int       indx; // Current step change index
  int       samp; // Current output sample

  int       flip; // Alternate nibbles

  // Initialize the state.

  samp = pState->value;
  indx = pState->index;

  flip = 0;

  // While there are samples to decode ...

  while (iSrcLen--)
  {
    // Step 1 - Get the delta value.

    if (flip) { code = *pSrc & 15; pSrc += 1; }
    else    { code = *pSrc >> 4; }

    flip ^= 1;

    // Step 2 - Compute difference and new predicted value.

    diff = ((code & 7) + 1) * OkiStepTable[indx];

    if (code & 8)
      samp -= diff;
    else
      samp += diff;

    // Step 3 - clamp output value to 12.3 bit precision.

    if (samp > 0x7fff) samp = 0x7fff;
    else
    if (samp < 0x0000) samp = 0x0000;

    if (pState->minvalue > samp) pState->minvalue = samp;
    if (pState->maxvalue < samp) pState->maxvalue = samp;

    // Step 4 - Find new index value (for later).

    indx += OkiIndxTable[code];

    if (indx <  0) indx =  0;
    else
    if (indx > 48) indx = 48;

    // Step 5 - sample value (converting from unsigned 12.3 bits into signed 16).

    *pDst++ = (samp << 1) - 32768;
  }

  // Update compression state.

  pState->value = samp;
  pState->index = indx;

  // All done.

  return;
}
