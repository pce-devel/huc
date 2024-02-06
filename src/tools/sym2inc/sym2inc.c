// **************************************************************************
// **************************************************************************
//
// SYM2INC.C
//
// This utility extracts a set of symbol names, listed in a ".S2I" text file
// from a PCEAS ".SYM" file, and it then uses them to create a PCEAS ".S"
// file suitable for including in another project.
//
// Copyright John Brandwood 2021.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#include "elmer.h"

#ifndef GIT_VERSION
  #define GIT_VERSION "unknown"
#endif

#ifndef GIT_DATE
  #define GIT_DATE __DATE__
#endif

#define VERSION_STR "sym2inc (" GIT_VERSION ", " GIT_DATE ")"

//
// Simple symbol table structure.
//

typedef struct SYMBOL_S {
  struct SYMBOL_S * pNextLink;
  struct SYMBOL_S * pNextHash;
  char *            pNameString;
  char *            pBankString;
  char *            pAddrString;
  uint32_t          uHash;
  bool              bOptional;
  } SYMBOL;

SYMBOL *g_pHeadSymbol;
SYMBOL *g_pTailSymbol;
SYMBOL *g_aSymbolHash [256];

//
//
//



// **************************************************************************
// **************************************************************************
//
// strupr ()
//
// For linux platforms.
//

#ifndef _WIN32

char * strupr ( char *pStr )
{
  char *pChr = pStr;

  while (*pChr != '\0') {
    *pChr = _toupper(*pChr);
    ++pChr;
  }

  return (pStr);
}

#endif



// **************************************************************************
// **************************************************************************
//
// s_aCRC32Table
//
// The following CRC lookup table was generated automagically by the
// Rocksoft(tm) Model CRC Algorithm Table Generation Program V1.0 using
// the following model parameters:
//
//    Width   : 4 bytes.
//    Poly    : 0x04C11DB7L
//    Reverse : true.
//
// For more information on the Rocksoft(tm) Model CRC Algorithm, see the
// document titled "A Painless Guide to CRC Error Detection Algorithms"
// by Ross Williams (ross@guest.adelaide.edu.au.). This document is
// likely to be in the FTP archive "ftp.adelaide.edu.au/pub/rocksoft".
//

static  uint32_t s_aCRC32Table [256] =
{
  0x00000000u, 0x77073096u, 0xEE0E612Cu, 0x990951BAu,
  0x076DC419u, 0x706AF48Fu, 0xE963A535u, 0x9E6495A3u,
  0x0EDB8832u, 0x79DCB8A4u, 0xE0D5E91Eu, 0x97D2D988u,
  0x09B64C2Bu, 0x7EB17CBDu, 0xE7B82D07u, 0x90BF1D91u,
  0x1DB71064u, 0x6AB020F2u, 0xF3B97148u, 0x84BE41DEu,
  0x1ADAD47Du, 0x6DDDE4EBu, 0xF4D4B551u, 0x83D385C7u,
  0x136C9856u, 0x646BA8C0u, 0xFD62F97Au, 0x8A65C9ECu,
  0x14015C4Fu, 0x63066CD9u, 0xFA0F3D63u, 0x8D080DF5u,
  0x3B6E20C8u, 0x4C69105Eu, 0xD56041E4u, 0xA2677172u,
  0x3C03E4D1u, 0x4B04D447u, 0xD20D85FDu, 0xA50AB56Bu,
  0x35B5A8FAu, 0x42B2986Cu, 0xDBBBC9D6u, 0xACBCF940u,
  0x32D86CE3u, 0x45DF5C75u, 0xDCD60DCFu, 0xABD13D59u,
  0x26D930ACu, 0x51DE003Au, 0xC8D75180u, 0xBFD06116u,
  0x21B4F4B5u, 0x56B3C423u, 0xCFBA9599u, 0xB8BDA50Fu,
  0x2802B89Eu, 0x5F058808u, 0xC60CD9B2u, 0xB10BE924u,
  0x2F6F7C87u, 0x58684C11u, 0xC1611DABu, 0xB6662D3Du,
  0x76DC4190u, 0x01DB7106u, 0x98D220BCu, 0xEFD5102Au,
  0x71B18589u, 0x06B6B51Fu, 0x9FBFE4A5u, 0xE8B8D433u,
  0x7807C9A2u, 0x0F00F934u, 0x9609A88Eu, 0xE10E9818u,
  0x7F6A0DBBu, 0x086D3D2Du, 0x91646C97u, 0xE6635C01u,
  0x6B6B51F4u, 0x1C6C6162u, 0x856530D8u, 0xF262004Eu,
  0x6C0695EDu, 0x1B01A57Bu, 0x8208F4C1u, 0xF50FC457u,
  0x65B0D9C6u, 0x12B7E950u, 0x8BBEB8EAu, 0xFCB9887Cu,
  0x62DD1DDFu, 0x15DA2D49u, 0x8CD37CF3u, 0xFBD44C65u,
  0x4DB26158u, 0x3AB551CEu, 0xA3BC0074u, 0xD4BB30E2u,
  0x4ADFA541u, 0x3DD895D7u, 0xA4D1C46Du, 0xD3D6F4FBu,
  0x4369E96Au, 0x346ED9FCu, 0xAD678846u, 0xDA60B8D0u,
  0x44042D73u, 0x33031DE5u, 0xAA0A4C5Fu, 0xDD0D7CC9u,
  0x5005713Cu, 0x270241AAu, 0xBE0B1010u, 0xC90C2086u,
  0x5768B525u, 0x206F85B3u, 0xB966D409u, 0xCE61E49Fu,
  0x5EDEF90Eu, 0x29D9C998u, 0xB0D09822u, 0xC7D7A8B4u,
  0x59B33D17u, 0x2EB40D81u, 0xB7BD5C3Bu, 0xC0BA6CADu,
  0xEDB88320u, 0x9ABFB3B6u, 0x03B6E20Cu, 0x74B1D29Au,
  0xEAD54739u, 0x9DD277AFu, 0x04DB2615u, 0x73DC1683u,
  0xE3630B12u, 0x94643B84u, 0x0D6D6A3Eu, 0x7A6A5AA8u,
  0xE40ECF0Bu, 0x9309FF9Du, 0x0A00AE27u, 0x7D079EB1u,
  0xF00F9344u, 0x8708A3D2u, 0x1E01F268u, 0x6906C2FEu,
  0xF762575Du, 0x806567CBu, 0x196C3671u, 0x6E6B06E7u,
  0xFED41B76u, 0x89D32BE0u, 0x10DA7A5Au, 0x67DD4ACCu,
  0xF9B9DF6Fu, 0x8EBEEFF9u, 0x17B7BE43u, 0x60B08ED5u,
  0xD6D6A3E8u, 0xA1D1937Eu, 0x38D8C2C4u, 0x4FDFF252u,
  0xD1BB67F1u, 0xA6BC5767u, 0x3FB506DDu, 0x48B2364Bu,
  0xD80D2BDAu, 0xAF0A1B4Cu, 0x36034AF6u, 0x41047A60u,
  0xDF60EFC3u, 0xA867DF55u, 0x316E8EEFu, 0x4669BE79u,
  0xCB61B38Cu, 0xBC66831Au, 0x256FD2A0u, 0x5268E236u,
  0xCC0C7795u, 0xBB0B4703u, 0x220216B9u, 0x5505262Fu,
  0xC5BA3BBEu, 0xB2BD0B28u, 0x2BB45A92u, 0x5CB36A04u,
  0xC2D7FFA7u, 0xB5D0CF31u, 0x2CD99E8Bu, 0x5BDEAE1Du,
  0x9B64C2B0u, 0xEC63F226u, 0x756AA39Cu, 0x026D930Au,
  0x9C0906A9u, 0xEB0E363Fu, 0x72076785u, 0x05005713u,
  0x95BF4A82u, 0xE2B87A14u, 0x7BB12BAEu, 0x0CB61B38u,
  0x92D28E9Bu, 0xE5D5BE0Du, 0x7CDCEFB7u, 0x0BDBDF21u,
  0x86D3D2D4u, 0xF1D4E242u, 0x68DDB3F8u, 0x1FDA836Eu,
  0x81BE16CDu, 0xF6B9265Bu, 0x6FB077E1u, 0x18B74777u,
  0x88085AE6u, 0xFF0F6A70u, 0x66063BCAu, 0x11010B5Cu,
  0x8F659EFFu, 0xF862AE69u, 0x616BFFD3u, 0x166CCF45u,
  0xA00AE278u, 0xD70DD2EEu, 0x4E048354u, 0x3903B3C2u,
  0xA7672661u, 0xD06016F7u, 0x4969474Du, 0x3E6E77DBu,
  0xAED16A4Au, 0xD9D65ADCu, 0x40DF0B66u, 0x37D83BF0u,
  0xA9BCAE53u, 0xDEBB9EC5u, 0x47B2CF7Fu, 0x30B5FFE9u,
  0xBDBDF21Cu, 0xCABAC28Au, 0x53B39330u, 0x24B4A3A6u,
  0xBAD03605u, 0xCDD70693u, 0x54DE5729u, 0x23D967BFu,
  0xB3667A2Eu, 0xC4614AB8u, 0x5D681B02u, 0x2A6F2B94u,
  0xB40BBE37u, 0xC30C8EA1u, 0x5A05DF1Bu, 0x2D02EF8Du
};



// **************************************************************************
// **************************************************************************
//
// CalculateCRC32 ()
//
// Basically, it is the CRC32 calculation as used in Ethernet, PKZIP, etc.
//

uint32_t CalculateCRC32 ( const void * pData, size_t iSize )
{
  // Calculate the CRC.

  const uint8_t * pByte = (const uint8_t *) pData;

  uint32_t iCRC = 0xFFFFFFFFu;

  while (iSize--)
  {
    iCRC = s_aCRC32Table[((iCRC ^ *pByte++) & 0xFFu)] ^ (iCRC >> 8);
  }

  // All Done, return result.

  return(iCRC ^ 0xFFFFFFFFu);
}



// **************************************************************************
// **************************************************************************
//
// ReadBinaryFile ()
//
// Uses POSIX file functions rather than C file functions.
//
// Google "FIO19-C" for the reason why.
//
// N.B. Will return an error for files larger than 2GB on a 32-bit system.
//

bool ReadBinaryFile ( const char * pName, void ** pBuffer, size_t * pLength )
{
  uint8_t *   pData = NULL;
  off_t       uSize;
  struct stat cStat;

  int hFile = open( pName, O_BINARY | O_RDONLY );

  if (hFile == -1)
    goto errorExit;

  if ((fstat( hFile, &cStat ) != 0) || (!S_ISREG( cStat.st_mode )))
    goto errorExit;

  if (cStat.st_size > 0x7FFFFFFF)
    goto errorExit;

  uSize = cStat.st_size;

  pData = (void *) malloc( uSize );

  if (pData == NULL)
    goto errorExit;

  if (read( hFile, pData, uSize ) != uSize)
    goto errorExit;

  close( hFile );

  *pBuffer = pData;
  *pLength = uSize;

  return true;

  // Handle errors.

  errorExit:

    if (pData != NULL) free( pData );
    if (hFile >= 0) close( hFile );

    *pBuffer = NULL;
    *pLength = 0;

    return false;
}


// **************************************************************************
// **************************************************************************
//
// WriteBinaryFile ()
//
// Uses POSIX file functions rather than C file functions, just because
// it might save some space since ReadBinaryFile() uses POSIX functions.
//

bool WriteBinaryFile ( const char * pName, void * pBuffer, size_t iLength )
{
  int hFile = open( pName, O_BINARY | O_WRONLY );

  if (hFile == -1)
    goto errorExit;

  if (write( hFile, pBuffer, iLength ) != iLength)
    goto errorExit;

  close( hFile );

  return true;

  // Handle errors.

  errorExit:

    if (hFile >= 0) close( hFile );

    return false;
}



// **************************************************************************
// **************************************************************************
//
// CreateSymbolList ()
//
// Scan the .S2I file to create the list of symbols that we're interested in.
//

void CreateSymbolList ( char * pTxtBuffer )

{
  char *        pLine;
  char *        pBOL;
  char *        pEOL;
  SYMBOL *      pSymbol;
  bool          bOptional;

  for ( pLine = pTxtBuffer; *pLine != '\0'; pLine = pEOL )
  {
    char *pToken;

    // Find the beginning and end of the currrent line.

    pBOL = pLine;
    pEOL = pLine;
    while ((*pEOL != '\n') && (*pEOL != '\r') && (*pEOL != '\0')) { ++pEOL; }
    while ((*pEOL == '\n') || (*pEOL == '\r')) { *pEOL++ = '\0'; }

    // Read the "name" tokens from the line.

    while ((pToken = strtok( pBOL, " \t" )) != NULL)
    {
      // Have we found the start of a comment?
      if ( *pToken == ';' ) break;

      // Allocate space for a new symbol.

      if ((pSymbol = malloc( sizeof(SYMBOL) )) == NULL)
      {
        printf( "Out of memory!\n" );
        exit(EXIT_FAILURE);
      }

      // Is this symbol optional?

      bOptional = false;

      if ( *pToken == '?' ) {
        bOptional = true;
        ++pToken;
      }

      // Initialize the symbol.

      pSymbol->pNextLink    = NULL;
      pSymbol->pNameString  = pToken;
      pSymbol->pBankString  = NULL;
      pSymbol->pAddrString  = NULL;
      pSymbol->uHash        = CalculateCRC32( pToken, strlen(pToken) );
      pSymbol->bOptional    = bOptional;

      // Add this symbol to the hash chain for searching.

      pSymbol->pNextHash  = g_aSymbolHash[ pSymbol->uHash >> 24 ];
      g_aSymbolHash[ pSymbol->uHash >> 24 ] = pSymbol;

      // Maintain an in-order list of symbols for printing.

      if (g_pHeadSymbol == NULL) { g_pHeadSymbol = pSymbol; }
      if (g_pTailSymbol != NULL) { g_pTailSymbol->pNextLink = pSymbol; }
      g_pTailSymbol = pSymbol;

      // Continue on to the next token on the line.

      pBOL = NULL;
    }
  }

  return;
}



// **************************************************************************
// **************************************************************************
//
// ScanSymbolDefinitions ()
//
// Scan the .SYM file to locate the symbol definitions that we want to know.
//

void ScanSymbolDefinitions ( char * pSymBuffer )

{
  int           iLine = 0;
  char *        pLine;
  char *        pBOL;
  char *        pEOL;

  for ( pLine = pSymBuffer; *pLine != '\0'; pLine = pEOL )
  {
    int         iToken = 0;
    char *      aToken[3];
    SYMBOL *    pSymbol;
    uint32_t    uHash;

    // Find the beginning and end of the currrent line.

    pBOL = pLine;
    pEOL = pLine;
    while ((*pEOL != '\n') && (*pEOL != '\r') && (*pEOL != '\0')) { ++pEOL; }
    while ((*pEOL == '\n') || (*pEOL == '\r')) { *pEOL++ = '\0'; }

    // Skip the first 2 lines.

    if ( ++iLine < 3 ) continue;

    // Read the "bank", "addr" and "name" tokens from the line.

    while (((aToken[iToken] = strtok( pBOL, " \t" )) != NULL) && ( ++iToken < 3 ))
    {
      // Continue on to the next token on the line.

      pBOL = NULL;
    }

    if ( iToken == 0 ) continue;
    if ( iToken != 3 ) {
      printf( "Malformed line in .SYM file, \"%s\", aborting!\n", pLine );
      exit(EXIT_FAILURE);
    }

    // Search for the symbol name in our list of symbols.

    uHash = CalculateCRC32( aToken[2], strlen(aToken[2]) );

    for (pSymbol = g_aSymbolHash[ uHash >> 24 ]; pSymbol != NULL; pSymbol =  pSymbol->pNextHash)
    {
      if (pSymbol->uHash == uHash) {
        if (strcmp( pSymbol->pNameString, aToken[2] ) == 0) {
          break;
        }
      }
    }

    // Remember the symbol's "bank" and "addr".

    if (pSymbol != NULL) {
      pSymbol->pBankString = aToken[0];
      pSymbol->pAddrString = aToken[1];
    }
  }

  return;
}



// **************************************************************************
// **************************************************************************
//
// OutputSymbols ()
//
// Scan the .S2I file to create the list of symbols that we're interested in.
//

void OutputSymbols ( char * pFileName )

{
  SYMBOL *      pSymbol;
  bool          bUndefined = false;
  FILE *        pFile;
  int           iTabNeeded;

  // Check for undefined symbols.

  for ( pSymbol = g_pHeadSymbol; pSymbol != NULL; pSymbol = pSymbol->pNextLink )
  {
    if ((pSymbol->pAddrString == NULL) && !pSymbol->bOptional)
    {
      printf( "Required symbol \"%s\" not defined!\n", pSymbol->pNameString );
      bUndefined = true;
    }
  }

  if (bUndefined)
  {
    printf( "Note: You can put a \"?\" in front of a symbol to make it optional.\n" );
    exit(EXIT_FAILURE);
  }

  // Write out the .S file.

  if ((pFile = fopen( pFileName, "w" )) == NULL)
  {
    printf( "Cannot create .S file \"%s\"!\n", pFileName );
    exit(EXIT_FAILURE);
  }

  fputs( "; This file is autogenerated by SYM2INC, do NOT add it to git!\n", pFile);
  fputs( "\n", pFile);

  for ( pSymbol = g_pHeadSymbol; pSymbol != NULL; pSymbol = pSymbol->pNextLink )
  {
    if (pSymbol->pAddrString != NULL)
    {
      fputs( pSymbol->pNameString, pFile );

      iTabNeeded = (15 - strlen( pSymbol->pNameString )) / 8;

      while (iTabNeeded-- >= 0) {
        fputs( "\t", pFile );
      }

      fputs( "=\t$", pFile );

      strupr( pSymbol->pBankString );

      if (( pSymbol->pBankString[0] != 'F' ) || ( pSymbol->pBankString[1] != '0' )) {
        fputs( pSymbol->pBankString, pFile );
        fputs( ":", pFile );
      }

      fputs( strupr( pSymbol->pAddrString ), pFile );

      fputs( "\n", pFile );
    }
  }

  fclose( pFile );
}



// **************************************************************************
// **************************************************************************
//
// main ()
//

int main ( int argc, char* argv[] )
{
  // Local Variables.

  char *        pFilePath;
  char *        pFileName;
  char *        pFileExtn;

  char *        pTxtBuffer = NULL;
  size_t        uTxtBufLen = 0;
  char *        pSymBuffer = NULL;
  size_t        uSymBufLen = 0;

  bool          bShowHelp = false;
  int           iArg = 1;

  // Read through any options on the command line.

  while (iArg < argc)
  {
    if (argv[iArg][0] != '-') break;

    switch (tolower(argv[iArg][1]))
    {
      case 'h':
        bShowHelp = true;
        break;
      default:
        printf("Unknown option \"%s\", aborting!\n", argv[iArg]);
        exit(EXIT_FAILURE);
    }

    iArg += 1;
  }

  // Sign-On

  printf("\n%s\n\n", VERSION_STR);

  // Show the help information if called incorrectly (too many or too few arguments).

  if (bShowHelp || (argc < (iArg+1)) || (argc > (iArg+1)))
  {
    puts(

      "This utility extracts a set of symbol names, listed in a \".S2I\" text file\n"
      "from a PCEAS \".SYM\" file, and it then uses them to create a PCEAS \".S\"\n"
      "file suitable for including in another project.\n"
      "\n"
      "Usage   : sym2inc [<options>] <filename.s2i>\n"
      "\n"
      "Options :\n"
      "  -h      Print this help message\n"
      );
    exit(EXIT_FAILURE);
  }

  // Create input filename, and locate its extension.

  pFilePath = malloc( strlen( argv[iArg] ) + 4 + 1 );
  strcpy( pFilePath, argv[iArg] );

  pFileName = strrchr( pFilePath, '/' );
  if (pFileName == NULL)
    { pFileName = strrchr( pFilePath, '\\' ); }
  if (pFileName == NULL)
    { pFileName = pFilePath; }
  else
    { ++pFileName; }

  pFileExtn = strrchr( pFileName, '.' );
  if ( pFileExtn == NULL ) pFileExtn = pFileName + strlen( pFileName );

  //
  // Read in the text file with the list of symbols that we're interested in.
  //

  if (!ReadBinaryFile( pFilePath, (void **) &pTxtBuffer, &uTxtBufLen ))
  {
    printf( "Failed to load .S2I file \"%s\" into memory!\n", pFileName );
    exit(EXIT_FAILURE);
  }

  // Terminate the .TXT buffer!

  if ((pTxtBuffer[uTxtBufLen - 1] != '\n') && (pTxtBuffer[uTxtBufLen - 1] != '\r'))
  {
    printf( "The list of symbols in \"%s\" must end with a CR (newline)!\n", pFileName );
    exit(EXIT_FAILURE);
  }

  pTxtBuffer[uTxtBufLen - 1] = '\0';

  // Scan the .S2I file to create the list of symbols that we're interested in.

  CreateSymbolList( pTxtBuffer );

  //
  // Read in the .SYM file with the list of symbols definitions.
  //

  strcpy( pFileExtn, ".sym" );

  if (!ReadBinaryFile( pFileName, (void **) &pSymBuffer, &uSymBufLen ))
  {
    printf( "Failed to load .SYM file \"%s\" into memory!\n", pFileName );
    exit(EXIT_FAILURE);
  }

  // Terminate the .SYM buffer!

  if ((pSymBuffer[uSymBufLen - 1] != '\n') && (pSymBuffer[uSymBufLen - 1] != '\r'))
  {
    printf( "The .SYM file must end with a CR (newline)!\n" );
    exit(EXIT_FAILURE);
  }

  pSymBuffer[uSymBufLen - 1] = '\0';

  // Scan the .SYM file to locate the symbol definitions that we want to know.

  ScanSymbolDefinitions( pSymBuffer );

  //
  // Output the results.
  //

  strcpy( pFileExtn, ".s" );

  OutputSymbols( pFileName );

  // All Done!

  return (0);
}
