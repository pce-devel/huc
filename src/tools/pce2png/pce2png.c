// **************************************************************************
// **************************************************************************
//
// pce2png.c
//
// Convert PCE memory dumps from Mednafen into BMP/PCX/PNG bitmaps.
//
// Copyright John Brandwood 2016-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#include  "elmer.h"

#include  "sysid.h"
#include  "lexer.h"

#include  "pxlmap.h"
#include  "bmpfile.h"
#include  "pcxfile.h"
#include  "pngfile.h"

#include  "pce2png.h"

//
// DEFINITIONS
//

#ifndef GIT_VERSION
  #define GIT_VERSION "unknown"
#endif

#ifndef GIT_DATE
  #define GIT_DATE __DATE__
#endif

#define VERSION_STR "pce2png (v0.30-" GIT_VERSION ", " GIT_DATE ")"

//
// GLOBAL VARIABLES
//

char        g_aVdcFilename [256];
char        g_aVceFilename [256];
char        g_aSatFilename [256];
char        g_aOutFilename [256];

int         g_iVdcBeginOffset = 0;
int         g_iVdcDeltaOffset = 0;

uint8_t *   g_pVdcBuffer = NULL;
size_t      g_uVdcLength = 0;

uint8_t *   g_pVceBuffer = NULL;
size_t      g_uVceLength = 0;

uint8_t *   g_pSatBuffer = NULL;
size_t      g_uSatLength = 0;

int         g_iBatW = -1;
int         g_iBatH = -1;
int         g_iSprW = -1;
int         g_iSprH = -1;
int         g_iOutW = 256;
int         g_iOutH = 256;
int         g_iRgbN = -1;

int         g_iPalN = 0;

sysID       g_uRgbType = ConstID( 0x357211C6u, "hce" );

//
// STATIC VARIABLES
//

//
// STATIC FUNCTION PROTOTYPES
//

int         main (int argc, char * argv[]);

//
// STATIC FUNCTION PROTOTYPES
//

ERRORCODE ReadBinaryFile ( const char * pName, uint8_t ** pBuffer, size_t * pLength );

static ERRORCODE ProcessArguments ( int argc, char ** argv );

void PceSprToPxlmap ( PXLMAP_T * pPxlmap );
void PceChrToPxlmap ( PXLMAP_T * pPxlmap );



// **************************************************************************
// **************************************************************************
//
// main ()
//

int main ( int argc, char **argv )

  {

  // Local variables.

  PXLMAP_T * pPxlmap = NULL;
  char * pOutExtn = NULL;

  // Find out what we're supposed to do.

  if (ProcessArguments( argc, argv ) != ERROR_NONE)
  {
    goto errorExit;
  }

  // Sanity check!

  if (g_aOutFilename[0] == 0)
  {
    goto errorExit;
  }

  // Load the VDC data.

  if (g_aVdcFilename[0] != 0)
  {
    if (ERROR_NONE != ReadBinaryFile( g_aVdcFilename, &g_pVdcBuffer, &g_uVdcLength ))
    {
      printf( "Unable to load VDC data file \"%s\". Aborting!\n", g_aVdcFilename );
      goto errorExit;
    }
  }

  // Load the VCE data.

  if (g_aVceFilename[0] != 0)
  {
    if (ERROR_NONE != ReadBinaryFile( g_aVceFilename, &g_pVceBuffer, &g_uVceLength ))
    {
      printf( "Unable to load VCE data file \"%s\". Aborting!\n", g_aVceFilename );
      goto errorExit;
    }
  }

  // Load the SAT data.

  if (g_aSatFilename[0] != 0)
  {
    if (ERROR_NONE != ReadBinaryFile( g_aSatFilename, &g_pSatBuffer, &g_uSatLength ))
    {
      printf( "Unable to load SAT data file \"%s\". Aborting!\n", g_aSatFilename );
      goto errorExit;
    }
  }

  // Allocate the output bitmap.

  pPxlmap = (PXLMAP_T *) PxlmapAlloc( g_iOutW, g_iOutH, 8, true );

  // Copy the VCE palette data.

  if (g_pVceBuffer != NULL)
  {
    uint8_t * pVceBin = g_pVceBuffer;
    unsigned  uVceCnt = g_uVceLength / 32;
    unsigned  uRGB;

    if ((g_iPalN > 15) && (uVceCnt > 16))
    {
      pVceBin += 0x0200;
      uVceCnt -= 16;
    }

    if (uVceCnt > 16) uVceCnt = 16;

    uVceCnt *= 16;

    for (uRGB = 0; uRGB < uVceCnt; uRGB++)
    {
      static uint8_t aPC98[8] = { 0x00,0x22,0x44,0x66,0x88,0xAA,0xBB,0xCC };

      RGBQUAD_T * pRGB = pPxlmap->m_aPxmC + uRGB;

      #if BYTE_ORDER_LO_HI
        unsigned uVceRGB = *((uint16_t *) pVceBin);
      #else
        unsigned uVceRGB = ((unsigned) pVceBin[0x01]) * 256 + ((unsigned) pVceBin[0x00]);
      #endif

      if (g_uRgbType == ConstID( 0x5F468818u, "lin" )) {
        // Linear step-by-36
        pRGB->m_uRgbB = ((uVceRGB >> 0) & 7) * 36;
        pRGB->m_uRgbR = ((uVceRGB >> 3) & 7) * 36;
        pRGB->m_uRgbG = ((uVceRGB >> 6) & 7) * 36;
      } else {
        // Hudson's CE.EXE Editor (original)
        pRGB->m_uRgbB = aPC98[ ((uVceRGB >> 0) & 7) ];
        pRGB->m_uRgbR = aPC98[ ((uVceRGB >> 3) & 7) ];
        pRGB->m_uRgbG = aPC98[ ((uVceRGB >> 6) & 7) ];
      }

      pRGB->m_uRgbA = 0;

      pVceBin += 2;
    }
  }

  // Write PCE data into the bitmap.

  if (g_iSprW != -1)
  {
    PceSprToPxlmap( pPxlmap );
  }
  else
  {
    PceChrToPxlmap( pPxlmap );
  }

  // Output the result.

  pOutExtn = strrchr(g_aOutFilename, '.');
  if (pOutExtn == NULL) pOutExtn = g_aOutFilename + strlen(g_aOutFilename);

  if (strcasecmp(pOutExtn, ".pcx") == 0)
  {
    if (PcxDumpPxlmap( pPxlmap, g_aOutFilename ) != ERROR_NONE)
    {
      goto errorExit;
    }
  }
  else
  if (strcasecmp(pOutExtn, ".bmp") == 0)
  {
    if (BmpDumpPxlmap( pPxlmap, g_aOutFilename ) != ERROR_NONE)
    {
      goto errorExit;
    }
  }
  else
  if (strcasecmp(pOutExtn, ".png") == 0)
  {
    if (PngDumpPxlmap( pPxlmap, g_aOutFilename ) != ERROR_NONE)
    {
      goto errorExit;
    }
  }
  else
  {
    sprintf( g_aErrorMessage, "Unknown output file extension, it must be \".pcx\", \".bmp\" or \".png\"!\n" );
    g_iErrorCode = ERROR_ILLEGAL;
    goto errorExit;
  }

  // Print success message.

//printf("PCE2PNG finshed OK!\n");

  //
  // Program exit.
  //
  // This will either be dropped through to if everything is OK, or 'goto'ed
  // if there was an error.

  errorExit:

  ErrorQualify();

  if (g_iErrorCode != ERROR_NONE) {
    puts(g_aErrorMessage);
    }

  PxlmapFree(pPxlmap);

  return ((g_iErrorCode != ERROR_NONE));

  }



// **************************************************************************
// **************************************************************************
//
// GetPow2Value () -
//

static int GetPow2Value (
  long iInput )

{
  unsigned long uMask = iInput;
  int           iPow2 = 0;

  while ((uMask & 1) == 0) { uMask >>= 1; iPow2 += 1; }

  if ((uMask ^ 1) != 0)
  {
    return (g_iErrorCode = ERROR_ILLEGAL);
  }

  return iPow2;
}



// **************************************************************************
// **************************************************************************
//
// ProcessArguments () -
//

static ERRORCODE ProcessArguments (
  int argc, char **argv )

{
  // Sign on.

  LEX_INFO cLexInfo;

  long     iValue;
  bool     bShowHelp = false;

  // Process the command-line options.

  InitLexer( &cLexInfo, argc - 1, argv + 1 );

  while (GetLexToken( &cLexInfo ) == ERROR_NONE)
  {
    if (cLexInfo.m_pTokenStr[0] == '-')
    {
      switch (cLexInfo.m_uTokenCrc)
      {
        //

        case ConstID( 0x85E8AC4Eu, "vdc" ) :
        {
          if (GetLexToken( &cLexInfo )) goto errorExit;

          if (cLexInfo.m_iTokenLen >= sizeof( g_aVdcFilename ))
          {
            sprintf( g_aErrorMessage, "VCE binary data filename too long!\n" );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          memcpy( g_aVdcFilename, cLexInfo.m_pTokenStr, cLexInfo.m_iTokenLen );
          g_aVdcFilename[cLexInfo.m_iTokenLen] = 0;

          break;
        }

        //

        case ConstID( 0x23CA9FBCu, "vce" ):
        {
          if (GetLexToken( &cLexInfo )) goto errorExit;

          if (cLexInfo.m_iTokenLen >= sizeof( g_aVceFilename ))
          {
            sprintf( g_aErrorMessage, "VCE binary data filename too long!\n" );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          memcpy( g_aVceFilename, cLexInfo.m_pTokenStr, cLexInfo.m_iTokenLen );
          g_aVceFilename[cLexInfo.m_iTokenLen] = 0;

          break;
        }

        //

        case ConstID( 0x7D871F27u, "sat" ):
        {
          if (GetLexToken( &cLexInfo )) goto errorExit;

          if (cLexInfo.m_iTokenLen >= sizeof( g_aSatFilename ))
          {
            sprintf( g_aErrorMessage, "SAT binary data filename too long!\n" );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          memcpy( g_aSatFilename, cLexInfo.m_pTokenStr, cLexInfo.m_iTokenLen );
          g_aSatFilename[cLexInfo.m_iTokenLen] = 0;

          break;
        }

        //

        case ConstID( 0x46159266u, "out" ):
        {
          if (GetLexToken( &cLexInfo )) goto errorExit;

          if (cLexInfo.m_iTokenLen >= sizeof( g_aOutFilename ))
          {
            sprintf( g_aErrorMessage, "OUT bitmap filename too long!\n" );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          memcpy( g_aOutFilename, cLexInfo.m_pTokenStr, cLexInfo.m_iTokenLen );
          g_aOutFilename[cLexInfo.m_iTokenLen] = 0;

          break;
        }

        //

        case ConstID( 0x6063D660u, "bat" ):
        {
          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          if (((g_iBatW = GetPow2Value( iValue )) < 0) || (g_iBatW < 5) || (g_iBatW > 7))
          {
            printf( "Illegal BAT width \"%ld\", it must be 32, 64 or 128!\n", iValue );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          if (((g_iBatH = GetPow2Value( iValue )) < 0) || (g_iBatH < 5) || (g_iBatH > 6))
          {
            printf( "Illegal BAT height \"%ld\", it must be 32 or 64!\n", iValue );
            g_iErrorCode = ERROR_ILLEGAL;
            break;
          }

          g_iBatW = 1 << g_iBatW;
          g_iBatH = 1 << g_iBatH;
          g_iOutW = g_iBatW << 3;
          g_iOutH = g_iBatH << 3;

          break;
        }

        //

        case ConstID( 0xC73D9902u, "spr" ):
        {
          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          if (((g_iSprW = GetPow2Value( iValue )) < 0) || (g_iSprW < 4) || (g_iSprW > 5))
          {
            printf( "Illegal SPR width \"%ld\", it must be 16 or 32!\n", iValue );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          if (((g_iSprH = GetPow2Value( iValue )) < 0) || (g_iSprH < 4) || (g_iSprH > 6))
          {
            printf( "Illegal SPR height \"%ld\", it must be 16, 32 or 64!\n", iValue );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          g_iSprW = 1 << g_iSprW;
          g_iSprH = 1 << g_iSprH;

          // We don't handle interleaved sprite memory, yet!

          if (g_iSprW == 16)
              g_iSprW = 32;

          break;
        }

        //

        case ConstID( 0xE39CF4EDu, "w" ):
        {
          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          if (((g_iOutW = GetPow2Value( iValue )) < 0) || (g_iOutW < 3) || (g_iOutW > 10))
          {
            printf( "Illegal output width \"%ld\", it must be a power-of-2 (8..1024)!\n", iValue );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          g_iOutW = 1 << g_iOutW;

          break;
        }

        //

        case ConstID( 0x6E94F918u, "h" ):
        {
          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          if (((g_iOutH = GetPow2Value( iValue )) < 0) || (g_iOutH < 3) || (g_iOutH > 12))
          {
            printf( "Illegal output width \"%ld\", it must be a power-of-2 (8..4096)!\n", iValue );
            g_iErrorCode = ERROR_ILLEGAL;
            goto errorExit;
          }

          g_iOutH = 1 << g_iOutH;

          break;
        }

        //

        case ConstID( 0x6CAD3928u, "pal" ):
        {
          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          g_iPalN = (int) iValue & 31;

          break;
        }

        //

        case ConstID( 0xDECB67C7u, "rgb" ):
        {
          if (GetLexToken( &cLexInfo )) goto errorExit;

          g_uRgbType = PStringToID( cLexInfo.m_iTokenLen, cLexInfo.m_pTokenStr );

          switch (g_uRgbType)
          {
            case ConstID( 0x357211C6u, "hce" ):
            case ConstID( 0x5F468818u, "lin" ):
              break;
            default:
              sprintf( g_aErrorMessage, "Unknown RGB conversion type \"%s\"!\n", cLexInfo.m_pTokenStr );
              g_iErrorCode = ERROR_ILLEGAL;
              goto errorExit;
          }

          break;
        }

        //

        case ConstID( 0xBDDCCF8Bu, "vdcbegin" ):
        {
          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          g_iVdcBeginOffset = (int) iValue;

          break;
        }

        //

        case ConstID( 0x511AA447u, "vdcdelta" ):
        {
          if (GetLexValue( &cLexInfo, &iValue )) goto errorExit;

          g_iVdcDeltaOffset = (int) iValue;

          break;
        }

        //

        default:
        {
          sprintf( g_aErrorMessage, "Unknown option \"%s\"!\n", cLexInfo.m_pTokenStr );
          g_iErrorCode = ERROR_UNKNOWN;
          goto errorExit;
        }

      } // switch (cLexInfo.m_uTokenCrc)

    }
    else
    {
      // (cLexInfo.m_pTokenStr[0] != '-')

      sprintf( g_aErrorMessage, "Unknown option \"%s\"!\n", cLexInfo.m_pTokenStr );
      g_iErrorCode = ERROR_UNKNOWN;
      goto errorExit;
    }

  } // while (GetLexToken( &cLexInfo ) == ERROR_NONE)

  // Did we get to the end of the input commands?

  if (g_iErrorCode == ERROR_IO_EOF)
  {
    g_iErrorCode = ERROR_NONE;
  }

  //

  errorExit:

    if (bShowHelp || ((g_iErrorCode == ERROR_NONE) && (g_aOutFilename[0] == 0)))
    {
      // Sign on.

      printf("\n%s by J.C.Brandwood\n", VERSION_STR);

      printf
        (
        "\n"
        "Purpose    : Convert PCE VDC memory dump into a BMP/PCX/PNG bitmap.\n"
        "\n"
        "Usage      : pce2png [<inopt>] [<outopt>]\n"
        "\n"
        "<inopt>    : Option........Description....................................\n"
        "\n"
        "             -vdc <file>   Binary file of VDC data\n"
        "             -vde <file>   Binary file of VCE data\n"
//      "             -sat <file>   Binary file of SAT data\n"
        "             -pal <n>      Write bitmap in palette N from VCE data\n"
        "             -bat <w> <h>  Set BAT size in tiles and dump screen image\n"
        "             -spr <w> <h>  Set SPR size and dump VDC data as sprites\n"
        "             -vdcbegin <n> Starting offset into VDC data (in bytes)\n"
        "             -vdcdelta <n> Amount to add to get to next CHR or SPR\n"
        "\n"
        "<outopt>   : OutOpt........Description....................................\n"
        "\n"
        "             -w <n>        Pixel width of output bitmap (a power of 2)\n"
        "             -h <n>        Pixel height of output bitmap (a power of 2)\n"
        "             -out <file>   Filename to save output bitmap\n"
        "             -rgb <type>   VCE palette conversion method ...\n"
        "                  hce        Hudson's Character Editor tool (default)\n"
        "                  lin        Traditional-but-inaccurate linear palette\n"
        "\n"
        );

      g_iErrorCode = ERROR_NONE;

      return (ERROR_DIAGNOSTIC);
    }

  return g_iErrorCode;
}



// **************************************************************************
// **************************************************************************
//
// ReadBinaryFile ()
//
// Uses POSIX file functions rather than C file functions.
//
// Google FIO19-C for the reason why.
//
// N.B. Will return an error for files larger than 2GB on a 32-bit system.
//

ERRORCODE ReadBinaryFile (
  const char *pName, uint8_t **pBuffer, size_t *pLength )

{
  int         hFile = 0;
  uint8_t *   pData = NULL;
  off_t       uSize;
  struct stat cStat;

  hFile = open( pName, O_BINARY | O_RDONLY );

  if (hFile == -1)
    goto errorExit;

  if ((fstat( hFile, &cStat ) != 0) || (!S_ISREG( cStat.st_mode )))
    goto errorExit;

  if (cStat.st_size > 0x7FFFFFFF)
    goto errorExit;

  uSize = cStat.st_size;

  pData = (uint8_t *) malloc( uSize );

  if (pData == NULL)
    goto errorExit;

  if (read( hFile, pData, uSize ) != uSize)
    goto errorExit;

  close( hFile );

  *pBuffer = pData;
  *pLength = uSize;

  return ERROR_NONE;

  // Handle errors.

  errorExit:

    if (pData != NULL) free( pData );
    if (hFile >= 0) close( hFile );

    *pBuffer = NULL;
    *pLength = 0;

    return (g_iErrorCode = ERROR_IO_READ );
}



// **************************************************************************
// **************************************************************************
//
// PceSprToPxlmap ()
//
// Convert PC Engine sprite data into a 256 color bitmap.
//

void PceSprToPxlmap (
  PXLMAP_T *pPxlmap )

{

  // Local variables.

  uint8_t * pSprBin;

  unsigned  uCGXCnt = g_iSprW >> 4; // 1 or 2
  unsigned  uCGYCnt = g_iSprH >> 4; // 1 or 2 or 4

  unsigned  uCGXMsk = (uCGXCnt - 1) << 7; // 1=0x0000 2=0x0080
  unsigned  uCGYMsk = (uCGYCnt - 1) << 8; // 1=0x0000 2=0x0100 4=0x0300

  unsigned  uCGXOff = 0;
  unsigned  uCGYOff = 0;

  unsigned  uCols = g_iOutW / g_iSprW;
  unsigned  uRows = g_iOutH / g_iSprH;

  unsigned  uPalN = (g_iPalN & 15) << 4;

  unsigned  uSprOff;
  uint8_t * pDstRow;
  unsigned  r;
  unsigned  c;
  unsigned  h;
  unsigned  w;
  unsigned  l;

  // Sanity Check.

  g_iVdcBeginOffset &= 0x1ff80;
  g_iVdcDeltaOffset &= 0x1ff80;

  // Calc initial offset and mask off the bits used for the NxN sprites.

  uSprOff = g_iVdcBeginOffset;
  uSprOff = (uSprOff | uCGXMsk) ^ uCGXMsk;
  uSprOff = (uSprOff | uCGYMsk) ^ uCGYMsk;

  // Draw uRows of sprites high.

  pDstRow = pPxlmap->m_pPxmPixels;

  for (r = uRows; r != 0; --r)
  {
    // Draw uCols of sprites wide.

    uint8_t * pDstCol = pDstRow;

    for (c = uCols; c != 0; --c)
    {
      // Draw uCGY 16x16 sprites high.

      uint8_t * pDstCGY = pDstCol;

      uCGYOff &= uCGYMsk;

      for (h = uCGYCnt; h != 0; --h)
      {
        // Draw uCGX 16x16 sprites wide.

        uint8_t * pDstCGX = pDstCGY;

        uCGXOff &= uCGXMsk;

        for (w = uCGXCnt; w != 0; --w)
        {
          // Draw a 16x16 sprite.

          uint8_t * pDstPxl = pDstCGX;

          pSprBin = g_pVdcBuffer + uSprOff + uCGYOff + uCGXOff;

          for (l = 16; l != 0; l--)
          {
            #if BYTE_ORDER_LO_HI
              unsigned uBit0 = *((uint16_t *) (pSprBin + 0x00));
              unsigned uBit1 = *((uint16_t *) (pSprBin + 0x20));
              unsigned uBit2 = *((uint16_t *) (pSprBin + 0x40));
              unsigned uBit3 = *((uint16_t *) (pSprBin + 0x60));
            #else
              unsigned uBit0 = ((unsigned) pSprBin[0x01]) * 256 + ((unsigned) pSprBin[0x00]);
              unsigned uBit1 = ((unsigned) pSprBin[0x21]) * 256 + ((unsigned) pSprBin[0x20]);
              unsigned uBit2 = ((unsigned) pSprBin[0x41]) * 256 + ((unsigned) pSprBin[0x40]);
              unsigned uBit3 = ((unsigned) pSprBin[0x61]) * 256 + ((unsigned) pSprBin[0x60]);
            #endif

            unsigned uMask = 0x8000u;

            do
            {
              unsigned uPxl = 0;
              if (uBit0 & uMask) uPxl += 1;
              if (uBit1 & uMask) uPxl += 2;
              if (uBit2 & uMask) uPxl += 4;
              if (uBit3 & uMask) uPxl += 8;
              *pDstPxl++ = uPxl + uPalN;
            } while ((uMask >>= 1) != 0);

            pSprBin += 2;
            pDstPxl += pPxlmap->m_iPxmLineSize - 16;
          }

          // Next col of CGX within a sprite.

          uCGXOff += 0x0080;
          pDstCGX += 16;
        }

        // Next row of CGY within a sprite.

        uCGYOff += 0x0100;
        pDstCGY += pPxlmap->m_iPxmLineSize * 16;
      }

      // Next col of sprites in the output bitmap.

      uSprOff += (0x0080 * 2 * uCGYCnt) + (g_iVdcDeltaOffset & 0x1ff80);
      pDstCol += 16 * uCGXCnt;
    }

    // Next row of sprites in the output bitmap.

    pDstRow += pPxlmap->m_iPxmLineSize * 16 * uCGYCnt;
  }

  // All Done!

}



// **************************************************************************
// **************************************************************************
//
//  PceChrToPxlmap ()
//
//  Convert PC Engine tile data into a 256 color bitmap.
//

void PceChrToPxlmap (
  PXLMAP_T *pPxlmap )

{
  // Local variables.

  uint8_t * pBatBin = g_pVdcBuffer;
  uint8_t * pChrBin = g_pVdcBuffer;

  unsigned  uCols = g_iOutW / 8;
  unsigned  uRows = g_iOutH / 8;

  unsigned  uPalN = (g_iPalN & 15) << 4;
  uint8_t * pDstRow;
  unsigned  r;
  unsigned  c;
  unsigned  l;

  // Just dump raw character data?

  if (g_iBatW < 1 || g_iBatH < 1)
  {
    pBatBin  = NULL;
    pChrBin += g_iVdcBeginOffset;
  }

  // Draw uRows tiles high.

  pDstRow = pPxlmap->m_pPxmPixels;

  for (r = uRows; r != 0; --r)
  {
    // Draw uCols tiles wide.

    uint8_t * pDstCol = pDstRow;

    for (c = uCols; c != 0; --c)
    {
      // Draw a 8x8 tile.

      uint8_t * pDstPxl = pDstCol;

      if (pBatBin)
      {
        #if BYTE_ORDER_LO_HI
          unsigned uTile = *((uint16_t *) pBatBin);
        #else
          unsigned uTile = ((unsigned) pBatBin[0x01]) * 256 + ((unsigned) pBatBin[0x00]);
        #endif

        pBatBin += 2;

        pChrBin = g_pVdcBuffer + ((uTile & 0x0fff) * 0x20);

        uPalN = (uTile >> 12) << 4;
      }

      for (l = 8; l != 0; l--)
      {
        unsigned uBit0 = pChrBin[0x00];
        unsigned uBit1 = pChrBin[0x01];
        unsigned uBit2 = pChrBin[0x10];
        unsigned uBit3 = pChrBin[0x11];

        unsigned uMask = 0x80u;

        do
        {
          unsigned uPxl = 0;
          if (uBit0 & uMask) uPxl += 1;
          if (uBit1 & uMask) uPxl += 2;
          if (uBit2 & uMask) uPxl += 4;
          if (uBit3 & uMask) uPxl += 8;
          *pDstPxl++ = uPxl + uPalN;
        } while ((uMask >>= 1) != 0);

        pChrBin += 2;
        pDstPxl += pPxlmap->m_iPxmLineSize - 8;
      }

      // Next col of tiles.

      pChrBin += 0x20 - 0x10;
      pDstCol += 8;
    }

    // Next row of tiles.

    pDstRow += pPxlmap->m_iPxmLineSize * 8;
  }

  // All Done!

}
