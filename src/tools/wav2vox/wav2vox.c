// **************************************************************************
// **************************************************************************
//
// wav2vox.c
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

#include "elmer.h"

#include "wav2vox.h"
#include "adpcmoki.h"
#include "riffio.h"

//
// DEFINITIONS
//

#ifndef GIT_VERSION
  #define GIT_VERSION "unknown"
#endif

#ifndef GIT_DATE
  #define GIT_DATE __DATE__
#endif

#define VERSION_STR "wav2vox (v1.80-" GIT_VERSION ", " GIT_DATE ")"

#define ERROR_NONE         0
#define ERROR_DIAGNOSTIC  -1
#define ERROR_NO_MEMORY   -2
#define ERROR_NO_FILE     -3
#define ERROR_IO_EOF      -4
#define ERROR_IO_READ     -5
#define ERROR_IO_WRITE    -6
#define ERROR_IO_SEEK     -7
#define ERROR_PROGRAM     -8
#define ERROR_UNKNOWN     -9
#define ERROR_ILLEGAL    -10

typedef struct
{
  uint32_t      uManufacturer;
  uint32_t      uProduct;
  uint32_t      uSamplePeriod;
  uint32_t      uMidiUnityNote;
  uint32_t      uMidiPitchFraction;
  uint32_t      uSmpteFormat;
  uint32_t      uSmpteOffset;
  uint32_t      uCntSampleLoops;
  uint32_t      uCntSamplerData;
} SMPL_CHUNK;

typedef struct
{
  uint32_t      uIdentifier;
  uint32_t      uType;
  uint32_t      uStart;
  uint32_t      uEnd;
  uint32_t      uFraction;
  uint32_t      uPlayCount;
} SMPL_LOOP;

typedef struct
{
  RIFF_FILE *   pFile;

  uint32_t      uNumSamples;

  uint32_t      uLoopTyp;
  uint32_t      uLoop1st;
  uint32_t      uLoopEnd;

  FRMT_EXTEND * pFrmt;
  uint32_t *    pFact;
  uint8_t *     pData;
  SMPL_CHUNK *  pSmpl;
  char *        pInfo;

  CHUNK_INFO    cFrmt;
  CHUNK_INFO    cFact;
  CHUNK_INFO    cData;
  CHUNK_INFO    cSmpl;
  CHUNK_INFO    cInfo;
} WAV_FILE;

//
// GLOBAL VARIABLES
//

// Fatal error flag.
//
// Used to signal that the error returned by ErrorCode (see below) is fatal
// and that the program should terminate immediately.

int  iErrorCode = ERROR_NONE;
char aErrorMessage [256];

//
// STATIC VARIABLES
//

bool bUndent;

unsigned uFormat = WAV_TAG_MSM5205;

unsigned uSampleRate = 16000;

bool bEncode = false;
bool bDecode = false;
bool bOutput = false;
bool bBinary = false;

bool bShowFile = false;
bool bShowInfo = true;
bool bShowFrmt = true;

bool bIcmtLoop = true;

//
// STATIC FUNCTION PROTOTYPES
//

static int ProcessOption ( char *pOption );

static int ProcessFile ( const char *pFile );

static void ErrorQualify ( void );

static void InheritWaveChunks ( WAV_FILE *pSrc, WAV_FILE *pDst );

static int LoadWavFile ( WAV_FILE *pWave, const char *pName );

static int SaveWavFile ( WAV_FILE *pWave, const char *pName );

static int FreeWavFile ( WAV_FILE *pWave );

static int ScanRiffChunks ( WAV_FILE *pWave, CHUNK_INFO *parent, int level );

static void * LoadRiffChunk ( RIFF_FILE * pFile, CHUNK_INFO *pInfo );

static int XvertPcmToAdpcm ( WAV_FILE *pWave );

static int XvertAdpcmToPcm ( WAV_FILE *pWave );

static int LoadVoxFile ( WAV_FILE *pWave, const char *pName );

static int SaveVoxFile ( WAV_FILE *pWave, const char *pName );

static long LoadWholeFile ( const char *pName, void **pAddr, long *pSize );

static long SaveWholeFile ( const char *pName, void *pAddr, long iSize );

static long GetFileLength ( FILE *pFile );

//
//
//



// **************************************************************************
// **************************************************************************
//
// main ()
//

int main ( int argc, char **argv )

{
  // Local variables.

  int i;

  // Sign on.

  printf("\n%s by J.C.Brandwood\n\n", VERSION_STR);

  // Set up the default format and DC bias.

  uFormat = WAV_TAG_MSM5205;
  iBiasValue = 200;

  // Check the command line arguments.

  if (argc < 2)
  {
    ProcessOption("-?");
    goto exit;
  }

  // Read through and process the arguments.

  for (i = 1; i < argc; i++)
  {
    if ((*argv[i] == '-') || (*argv[i] == '/'))
    {
      if (ProcessOption(argv[i]) != ERROR_NONE) goto exit;
    }
    else
    {
      if (ProcessFile(argv[i]) != ERROR_NONE) goto exit;
    }
  }

  // Print success message.

  printf("wav2vox - Operation Completed OK !\n\n");

  // Program exit.
  //
  // This will either be dropped through to if everything is OK, or 'goto'ed
  // if there was an error.

exit:

  ErrorQualify();

  if (iErrorCode != ERROR_NONE)
  {
    puts(aErrorMessage);
  }

  // All done.

  return ((iErrorCode != ERROR_NONE));
}



// **************************************************************************
// **************************************************************************
//
// GetValue ()
//

static bool GetValue ( long *pVal, char *pStr )

{
  char * pEnd;

  pStr = strtok(pStr, " \t\n");

  if (pStr == NULL) return (false);

  *pVal = strtol(pStr, &pEnd, 0);

  if (*pEnd != '\0') return (false);

  return (true);
}



// **************************************************************************
// **************************************************************************
//
// ProcessOption ()
//

int ProcessOption ( char * pOption )

   {
   // Local variables.

  int i;
  long l;

  // Process option string.

  switch(pOption[1])
  {
    // Display help.

    case '?':
    case 'h':
    {
      printf
        (
          "Purpose    : Microsoft WAV to VOX (OKI/Dialogic ADPCM) Converter\n"
                "\n"
                "Usage      : wav2vox [<option>] <filename>\n"
                "\n"
                "<option>   : Option........Description.......................................\n"
                "\n"
                "             -b<bias>      Add/Remove DC bias (default 200 if MSM5205)\n"
                "             -f[<type>]    Select compression format (default MSM5205)\n"
                "             -r<rate>      Sample rate of VOX input files (default 16000)\n"
                "             -vi           Display WAV file information\n"
                "             -vf           Display WAV file structure\n"
                "\n"
                "<type>     : Type..........Win95...Author..................Rate..............\n"
                "\n"
                "             msm5205       no      OKI MSM5205 (PCE-CD)    (4bits-per-sample)\n"
                "             huc6230       no      Hudson 6230 (PC-FX)     (4bits-per-sample)\n"
                "             oki           yes     Dialogic / OKI MSM6585  (4bits-per-sample)\n"
                "\n"
                "<filename> : Path to .WAV or .VOX file\n"
                "\n"
        );

      iErrorCode = ERROR_NONE;

      return (ERROR_DIAGNOSTIC);
    }

    // Select DC Bias to add/remove during compression/decompression .
    // N.B. This only applies to the OKI and MSM5205 formats.

    case 'b':
    {
      if ((GetValue(&l, &pOption[2]) == false) || (l < 0) || (l > 200))
      {
        sprintf(aErrorMessage,
            "wav2vox - Illegal DC bias value (must be 0 <= n <= 200) !\n");
        return (iErrorCode = ERROR_ILLEGAL);
      }

      iBiasValue = l;

      break;
    }

    // Select compression format (auto-detected if decompressing a WAV).

    case 'f':
    {
      if (strcasecmp(&pOption[2], "msm5205") == 0) {
        uFormat = WAV_TAG_MSM5205;
      }
      else
      if (strcasecmp(&pOption[2], "huc6230") == 0) {
        uFormat = WAV_TAG_HUC6230;
        iBiasValue = 0;
      }
      else
      if (strcasecmp(&pOption[2], "oki") == 0 || pOption[2] == 0) {
        uFormat = WAV_TAG_OKI_ADPCM; // or WAV_TAG_DIALOGIC_OKI_ADPCM
        iBiasValue = 0;
      }
      else {
        sprintf(aErrorMessage, "wav2vox - Unknown compression option !\n");
        return (iErrorCode = ERROR_ILLEGAL);
      }

      break;
    }

    // Select output rate info for WAV file (overidden by source file).

    case 'r':
    {
      if ((GetValue(&l, &pOption[2]) == false) || (l < 1) || (l > 48000))
      {
        sprintf(aErrorMessage,
            "wav2vox - Illegal sample rate value (must be 1 <= n <= 48000) !\n");
        return (iErrorCode = ERROR_ILLEGAL);
      }

      uSampleRate = l;

      break;
    }

    // Switch on view option.

    case 'v':
    {
      i = 2;

      while (pOption[i] != 0)
      {
        if (pOption[i] == 'f') bShowFile = true;
        if (pOption[i] == 'i') bShowInfo = true;
        i += 1;
      }

      break;
    }

    // Unknown option.

    default:
    {
      sprintf(aErrorMessage, "wav2vox - Unknown option !\n");
      return (iErrorCode = ERROR_ILLEGAL);
    }
  }

  // All done.

  return (ERROR_NONE);
}



// **************************************************************************
// **************************************************************************
//
// ProcessFile ()
//

static int ProcessFile ( const char *pName )

{
  // Local variables.

  FILE *        pFile;
  uint32_t      aData[3];
  WAV_FILE      cWave;
  char          aName[256 + 8];
  char *        pExtn;

  // Initialize the WAV file info block.

  iErrorCode = 0;

  memset(&cWave, 0, sizeof(WAV_FILE));

  // File name too long ?

  if (strlen(pName) >= 256)
  {
    sprintf(aErrorMessage, "wav2vox - File name too long !\n");
    return (iErrorCode = ERROR_ILLEGAL);
  }

  // Create input filename, and locate its extension.

  strcpy(aName, pName);

  pExtn = strrchr(aName, '.');
  if (pExtn == NULL) pExtn = aName + strlen(aName);

  // Read in the first part of the file to help us determine the file format.

  if ((pFile = fopen(aName, "rb")) == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Unable to open input file \"%s\" !\n",
      aName);
    return (iErrorCode = ERROR_NO_FILE);
  }

  if (fread(&aData[0], sizeof(uint32_t), 3, pFile) != 3)
  {
    sprintf(aErrorMessage, "wav2vox - Unable to identify input file \"%s\" !\n",
      aName);
    return (iErrorCode = ERROR_NO_FILE);
  }

  fclose(pFile);

  // Is it a WAV file ?

  if ((aData[0] == makeID('R', 'I', 'F', 'F')) &&
      (aData[2] == makeID('W', 'A', 'V', 'E')))
  {
    // Load up the WAV file.

    if (LoadWavFile(&cWave, aName) < 0) {
      return (-1);
    }

    bEncode = true;
    bDecode = false;
    bOutput = true;
    bBinary = true;

//  // Read the decoding format from the WAV file header.
//
//  if (bDecode)
//    uFormat = cWave.pFrmt->uFormatTag;
  }

  // If not, it must be a VOX file!

  else
  {
    if (LoadVoxFile(&cWave, aName) < 0) {
      return (-1);
    }

    bEncode = false;
    bDecode = true;
    bOutput = true;
    bBinary = false;
  }

  // Encode the file ?

  if (bEncode && (cWave.pFrmt->uFormatTag == WAV_TAG_PCM))
  {
    if ((uFormat == WAV_TAG_MSM5205) ||
        (uFormat == WAV_TAG_HUC6230) ||
        (uFormat == WAV_TAG_OKI_ADPCM))
    {
      if (XvertPcmToAdpcm(&cWave) < 0) {
        return (-1);
      }
    }
  }

  // Decode the file ?

  if (bDecode && (cWave.pFrmt->uFormatTag != WAV_TAG_PCM))
  {
    if ((cWave.pFrmt->uFormatTag == WAV_TAG_MSM5205) ||
        (cWave.pFrmt->uFormatTag == WAV_TAG_HUC6230) ||
        (cWave.pFrmt->uFormatTag == WAV_TAG_OKI_ADPCM))
    {
      if (XvertAdpcmToPcm(&cWave) < 0) {
        return (-1);
      }
    }
  }

  // Write out the result ?

  if (bOutput)
  {
    // Save as a VOX file, or a WAV file ?

    if (bBinary)
    {
      strcpy(pExtn, ".vox");

      if (SaveVoxFile(&cWave, aName) < 0) {
        goto errorExit;
      }
    }
    else
    {
      strcpy(pExtn, ".vox.wav");

      if (SaveWavFile(&cWave, aName) < 0) {
        return (-1);
      }
    }
  }

  // Free up resources and return error code.

  iErrorCode = 0;

errorExit:

    FreeWavFile(&cWave);

    return (iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// XvertPcmToAdpcm ()
//
// WAV_TAG_OKI_ADPCM or
// WAV_TAG_MSM5205 or
// WAV_TAG_HUC6230
//

static int XvertPcmToAdpcm ( WAV_FILE *pWave )

{
  // Local variables.

  uint8_t *     pSrc;
  uint8_t *     pDst;

  long          l;
  long          m;

  WAV_FILE      cNext;
  OKI_ADPCM     cState;

  // Initialize new header.

  memset(&cNext, 0, sizeof(WAV_FILE));

  // Only handle mono.

  iErrorCode = 0;

  if (pWave->pFrmt->uNumChannels != 1)
  {
    sprintf(aErrorMessage,
        "wav2vox - Only mono samples can be compressed to OKI Adpcm !\n");
    iErrorCode = ERROR_ILLEGAL;
    goto errorExit;
  }

  // Only handle 16-bit PCM WAV files.

  if ((pWave->pFrmt->uFormatTag  != WAV_TAG_PCM) ||
    (pWave->pFrmt->uBitsPerSample != 16))
  {
    sprintf(aErrorMessage,
        "wav2vox - Only 16-bit PCM samples can be compressed to OKI Adpcm !\n");
    iErrorCode = ERROR_ILLEGAL;
    goto errorExit;
  }

  // Get total number of samples per channel.

  l = pWave->cData.uChunkSize / pWave->pFrmt->uBlockAlign;

  pWave->uNumSamples = l;

  // Create the 'frmt' chunk.

  if ((cNext.pFrmt = (FRMT_EXTEND *) malloc(sizeof(FRMT_EXTEND))) == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Not enough memory !\n");
    iErrorCode = ERROR_NO_MEMORY;
    goto errorExit;
  }

  // WAV_TAG_OKI_ADPCM or WAV_TAG_MSM5205 or WAV_TAG_HUC6230

  cNext.pFrmt->uFormatTag       = uFormat;
  cNext.pFrmt->uNumChannels     = pWave->pFrmt->uNumChannels;
  cNext.pFrmt->uBlockAlign      = cNext.pFrmt->uNumChannels;
  cNext.pFrmt->uSamplesPerSec   = pWave->pFrmt->uSamplesPerSec;
  cNext.pFrmt->uAvgBytesPerSec  =
    (cNext.pFrmt->uSamplesPerSec * cNext.pFrmt->uBlockAlign) / 2;
  cNext.pFrmt->uBitsPerSample   = 4;
  cNext.pFrmt->uExtraInfo       = 0;

  cNext.cFrmt.uChunkID    = makeID('f', 'm', 't', ' ');
  cNext.cFrmt.uChunkSize  = sizeof(FRMT_EXTEND);
  cNext.cFrmt.uDataOffset = 0;
  cNext.cFrmt.bDirty      = false;

  // Create the 'fact' chunk.

  if ((cNext.pFact = (uint32_t *) malloc(4)) == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Not enough memory !\n");
    iErrorCode = ERROR_NO_MEMORY;
    goto errorExit;
  }

  *cNext.pFact = (pWave->uNumSamples + 1) & ~1;

  cNext.cFact.uChunkID    = makeID('f', 'a', 'c', 't');
  cNext.cFact.uChunkSize  = 4;
  cNext.cFact.uDataOffset = 0;
  cNext.cFact.bDirty      = false;

  // Create the 'data' chunk.

  m = (l + 1) >> 1;

  if ((cNext.pData = (uint8_t *) malloc(m)) == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Not enough memory !\n");
    iErrorCode = ERROR_NO_MEMORY;
    goto errorExit;
  }

  cNext.cData.uChunkID    = makeID('d', 'a', 't', 'a');
  cNext.cData.uChunkSize  = m;
  cNext.cData.uDataOffset = 0;
  cNext.cData.bDirty      = false;

  // Now loop around until we've compressed that many samples.

  cState.format   = uFormat;
  cState.index    = 0;
  cState.maxvalue =
  cState.minvalue =
  cState.value    = // Unsigned 0..4095
    (uFormat == WAV_TAG_HUC6230) ? 2048 << 3 : 2048;

  pSrc = pWave->pData;
  pDst = cNext.pData;

  if (uFormat == WAV_TAG_HUC6230)
  {
    EncodeAdpcmPcfx((int16_t *) pSrc, pDst, l, &cState);

    printf( "Compressed %ld samples, min=%d, max=%d.\n",
      l, (cState.minvalue << 1) - 32768, (cState.maxvalue << 1) - 32768);
  }
  else
  {
    EncodeAdpcmOki4((int16_t *) pSrc, pDst, l, &cState);

    printf( "Compressed %ld samples, min=%d, max=%d.\n",
      l, (cState.minvalue << 4) - 32768, (cState.maxvalue << 4) - 32768);
  }

  // Inherit the general chunks.

  InheritWaveChunks(pWave, &cNext);

  // Move the new header to the old header.

  FreeWavFile(pWave);

  memcpy(pWave, &cNext, sizeof(WAV_FILE));
  memset(&cNext, 0, sizeof(WAV_FILE));

  // All done.

errorExit:

  FreeWavFile(&cNext);

  return (iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// XvertAdpcmToPcm ()
//
// WAV_TAG_OKI_ADPCM or
// WAV_TAG_MSM5205 or
// WAV_TAG_HUC6230
//

static int XvertAdpcmToPcm ( WAV_FILE *pWave )

{
  // Local variables.

  uint8_t *   pSrc;
  uint8_t *   pDst;

  long        l;
  long        m;

  WAV_FILE    cNext;
  OKI_ADPCM   cState;

  unsigned    uAdpcmBlock;
  unsigned    uAdpcmCount;

  int         i;

  // Initialize new header.

  memset(&cNext, 0, sizeof(WAV_FILE));

  // Only handle mono.

  iErrorCode = 0;

  if (pWave->pFrmt->uNumChannels != 1)
  {
    sprintf(aErrorMessage,
      "wav2vox - Only mono samples can be decompressed from OKI Adpcm !\n");
    iErrorCode = ERROR_ILLEGAL;
    goto errorExit;
  }

  // Only handle our own WAV files.

  if (pWave->pFrmt->uBitsPerSample != 4)
  {
    sprintf(aErrorMessage,
      "wav2vox - Only 4-bit adpcm samples can be decompressed from OKI Adpcm !\n");
    iErrorCode = ERROR_ILLEGAL;
    goto errorExit;
  }

  if ((pWave->pFrmt->uFormatTag != WAV_TAG_OKI_ADPCM) &&
      (pWave->pFrmt->uFormatTag != WAV_TAG_MSM5205) &&
      (pWave->pFrmt->uFormatTag != WAV_TAG_HUC6230))
  {
    sprintf(aErrorMessage, "wav2vox - Format is not a variant of OKI Adpcm !\n");
    iErrorCode = ERROR_ILLEGAL;
    goto errorExit;
  }

  // Read samples-in-file.

  uAdpcmBlock = pWave->pFrmt->uBlockAlign;     // size of block
  uAdpcmCount = pWave->pFrmt->uBlockAlign * 2; // samples per block

  // Get total number of samples (and cope with Sound Recorder bug).

  l = pWave->pFact[0];

  i = uAdpcmCount;

  i = i * (pWave->cData.uChunkSize / uAdpcmBlock);

  if (l > (long) i)
  {
    l = (long) i;
  }

  pWave->pFact[0]    = l;
  pWave->uNumSamples = l;

  // Create the 'frmt' chunk.

  if ((cNext.pFrmt = (FRMT_EXTEND *) malloc(sizeof(FRMT_PCM))) == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Not enough memory !\n");
    iErrorCode = ERROR_NO_MEMORY;
    goto errorExit;
  }

  cNext.pFrmt->uFormatTag       = WAV_TAG_PCM;
  cNext.pFrmt->uNumChannels     = pWave->pFrmt->uNumChannels;
  cNext.pFrmt->uBlockAlign      = 2 * cNext.pFrmt->uNumChannels;
  cNext.pFrmt->uSamplesPerSec   = pWave->pFrmt->uSamplesPerSec;
  cNext.pFrmt->uAvgBytesPerSec  = (cNext.pFrmt->uSamplesPerSec * cNext.pFrmt->uBlockAlign);
  cNext.pFrmt->uBitsPerSample   = 16;

  cNext.cFrmt.uChunkID    = makeID('f', 'm', 't', ' ');
  cNext.cFrmt.uChunkSize  = sizeof(FRMT_PCM);
  cNext.cFrmt.uDataOffset = 0;
  cNext.cFrmt.bDirty      = false;

  // Create the 'data' chunk.

  m = l * 2;

  if ((cNext.pData = (uint8_t *) malloc(m)) == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Not enough memory !\n");
    iErrorCode = ERROR_NO_MEMORY;
    goto errorExit;
  }

  cNext.cData.uChunkID    = makeID('d', 'a', 't', 'a');
  cNext.cData.uChunkSize  = m;
  cNext.cData.uDataOffset = 0;
  cNext.cData.bDirty      = false;

  // Now decompress the data itself.

  cState.format   = uFormat;
  cState.index    = 0;
  cState.maxvalue =
  cState.minvalue =
  cState.value    = // Unsigned 0..65535
    (uFormat == WAV_TAG_HUC6230) ? 2048 << 3 : 2048 << 4;

  pSrc = pWave->pData;
  pDst = cNext.pData;

  if (uFormat == WAV_TAG_HUC6230)
  {
    DecodeAdpcmPcfx(pSrc, (int16_t *) pDst, l, &cState);

    printf( "Decompressed %ld samples, min=%d, max=%d.\n",
      l, (cState.minvalue << 1) - 32768, (cState.maxvalue << 1) - 32768);
  }
  else
  {
    DecodeAdpcmOki4(pSrc, (int16_t *) pDst, l, &cState);

    printf( "Decompressed %ld samples, min=%d, max=%d.\n",
      l, (cState.minvalue << 0) - 32768, (cState.maxvalue << 0) - 32768);
  }

  // Inherit the general chunks.

  InheritWaveChunks(pWave, &cNext);

  // Move the new header to the old header.

  FreeWavFile(pWave);

  memcpy(pWave, &cNext, sizeof(WAV_FILE));
  memset(&cNext, 0, sizeof(WAV_FILE));

  // All done.

errorExit:

  FreeWavFile(&cNext);

  return (iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// InheritWaveChunks ()
//

static void InheritWaveChunks ( WAV_FILE *pSrc, WAV_FILE *pDst )

{
  // Create the 'smpl' chunk.

  memcpy(&pDst->cSmpl, &pSrc->cSmpl, sizeof(CHUNK_INFO));

  pDst->pSmpl = pSrc->pSmpl;
  pSrc->pSmpl = NULL;

  // Create the 'INFO' chunk.

  memcpy(&pDst->cInfo, &pSrc->cInfo, sizeof(CHUNK_INFO));

  pDst->pInfo = pSrc->pInfo;
  pSrc->pInfo = NULL;

  // Inherit the loop points.

  pDst->uLoopTyp = pSrc->uLoopTyp;
  pDst->uLoop1st = pSrc->uLoop1st;
  pDst->uLoopEnd = pSrc->uLoopEnd;
}



// **************************************************************************
// **************************************************************************
//
// LoadWavFile ()
//

static int LoadWavFile ( WAV_FILE *pWave, const char *pName )

{
  // Local variables.

  int         i;
  int         iLoadError = -1;

  // Initialize the data.

  memset(pWave, 0, sizeof(WAV_FILE));

  // Open the file.

  pWave->pFile = riffOpen(pName, OPEN_RDONLY);

  if (pWave->pFile == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Unable to open file \"%s\" !\n", pName);
    return (iErrorCode = ERROR_IO_READ);
  }

  // Scan through the file identifying all the chunks.

  if ((i = ScanRiffChunks(pWave, NULL, 0)) != 0) {
    goto errorExit;
  }

  // Is there a "fmt " chunk ?

  if (pWave->cFrmt.uChunkID == 0)
  {
    sprintf(aErrorMessage, "wav2vox - Missing \"fmt \" chunk !\n");
    iErrorCode = ERROR_IO_READ;
    goto errorExit;
  }

  // Load up the "fmt " chunk.

  pWave->pFrmt = (FRMT_EXTEND *) LoadRiffChunk(pWave->pFile, &pWave->cFrmt);

  if (pWave->pFrmt == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Unable to read \"fmt \" chunk !\n");
    iErrorCode = ERROR_IO_READ;
    goto errorExit;
  }

  // Correct for Sound Forge sometimes saving an invalid uBlockAlign value.

  if (pWave->pFrmt->uFormatTag == WAV_TAG_PCM)
  {
    pWave->pFrmt->uBlockAlign =
      pWave->pFrmt->uNumChannels * ((pWave->pFrmt->uBitsPerSample + 7) >> 3);
  }

  // Should there be a "fact" chunk (or is there one anyway) ?

  if ((pWave->cFact.uChunkID != 0) || (pWave->pFrmt->uFormatTag != WAV_TAG_PCM))
  {
    // Is there a "fact" chunk ?

    if (pWave->cFact.uChunkID == 0)
    {
      sprintf(aErrorMessage, "wav2vox - Missing \"fact\" chunk !\n");
      iErrorCode = ERROR_IO_READ;
      goto errorExit;
    }

    // Load up the "fact" chunk.

    pWave->pFact = (uint32_t *) LoadRiffChunk(pWave->pFile, &pWave->cFact);

    if (pWave->pFact == NULL)
    {
      sprintf(aErrorMessage, "wav2vox - Unable to read \"fact\" chunk !\n");
      iErrorCode = ERROR_IO_READ;
      goto errorExit;
    }
  }

  // Is there a "data" chunk ?

  if (pWave->cData.uChunkID == 0)
  {
    sprintf(aErrorMessage, "wav2vox - Missing \"data\" chunk !\n");
    iErrorCode = ERROR_IO_READ;
    goto errorExit;
  }

  // Load up the "data" chunk.

  pWave->pData = (uint8_t *) LoadRiffChunk(pWave->pFile, &pWave->cData);

  if (pWave->pData == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Unable to read \"data\" chunk !\n");
    iErrorCode = ERROR_IO_READ;
    goto errorExit;
  }

  // Is there a "smpl" chunk ?

  if (pWave->cSmpl.uChunkID != 0)
  {
    // Load up the "smpl" chunk.

    pWave->pSmpl = (SMPL_CHUNK *) LoadRiffChunk(pWave->pFile, &pWave->cSmpl);

    if (pWave->pSmpl == NULL)
    {
      sprintf(aErrorMessage, "wav2vox - Unable to read \"smpl\" chunk !\n");
      iErrorCode = ERROR_IO_READ;
      goto errorExit;
    }

    // Read the loop points from SMPL structure.

    if (pWave->pSmpl->uCntSampleLoops > 0)
    {
      pWave->uLoopTyp = ((SMPL_LOOP *) (pWave->pSmpl + 1))->uType;
      pWave->uLoop1st = ((SMPL_LOOP *) (pWave->pSmpl + 1))->uStart;
      pWave->uLoopEnd = ((SMPL_LOOP *) (pWave->pSmpl + 1))->uEnd + 1;
    }
  }

  // Is there a "INFO" chunk ?

  if (pWave->cInfo.uChunkID != 0)
  {
    // Load up the "INFO" chunk.

    pWave->pInfo = (char *) LoadRiffChunk(pWave->pFile, &pWave->cInfo);

    if (pWave->pInfo == NULL)
    {
      sprintf(aErrorMessage, "wav2vox - Unable to read \"INFO\" chunk !\n");
      iErrorCode = ERROR_IO_READ;
      goto errorExit;
    }

    // Read the loop points from the ICMT text field.

    if (bIcmtLoop)
    {
      char * pStr = pWave->pInfo;
      char * pEnd = pStr + pWave->cInfo.uChunkSize;
      char   aStr[256];

      // Skip INFO field.

      pStr += 4;

      while (pStr < pEnd)
      {
        // Comment chunk ?

        if (pStr[0] == 'I' && pStr[1] == 'C' && pStr[2] == 'M' && pStr[3] == 'T')
        {
          int var1st;
          int varend;

          pStr += 8;

          if (strlen(pStr) > 255) break;

          strcpy(aStr, pStr);

          if ((pStr = strtok(aStr,     " \012\015\t\n")) == NULL) break;

          var1st = strtol(pStr, &pStr, 0);

          if (*pStr != '\0') break;

          if ((pStr = strtok(pStr + 1, " \012\015\t\n")) == NULL) break;

          varend = strtol(pStr, &pStr, 0);

          if (*pStr != '\0') break;

          pWave->uLoopTyp = 0;
          pWave->uLoop1st = var1st;
          pWave->uLoopEnd = varend + 1;

          break;
        }

        // Skip the chunk.

        i = ((pStr[4] + (pStr[5] << 8) + (pStr[6] << 16) + (pStr[7] << 24)) + 1 + 8) & ~1;

        pStr = pStr + i;
      }
    }
  }

  // Inform the user what we've discovered.

  if (bShowInfo)
  {
    switch (pWave->pFrmt->uFormatTag)
    {
      case WAV_TAG_PCM:
          pName = "Uncompressed PCM";
        break;
      case WAV_TAG_OKI_ADPCM:
          pName = "OKI MSM6285 ADPCM (aka Dialogic)";
        break;
      case WAV_TAG_MSM5205:
          pName = "OKI MSM5205 ADPCM (for NEC PCE-CD)";
        break;
      case WAV_TAG_HUC6230:
          pName = "Hudson 6230 ADPCM (for NEC PC-FX)";
        break;
      default:
          pName = "Unknown";
    }

    printf("\n");

    printf("Format            = %s\n", pName);
    printf("Channels          = %d\n", pWave->pFrmt->uNumChannels);
    printf("Samples-per-sec   = %d\n", pWave->pFrmt->uSamplesPerSec);
    printf("AvgBytes-per-sec  = %d\n", pWave->pFrmt->uAvgBytesPerSec);
    printf("Block Align       = %d\n", pWave->pFrmt->uBlockAlign);
    printf("Bits-per-sample   = %d\n", pWave->pFrmt->uBitsPerSample);

    if (pWave->pFrmt->uFormatTag != WAV_TAG_PCM)
    {
        printf("Total Samples     = %d\n", pWave->pFact[0]);
    }
    else
    {
        printf("Total Samples     = %d\n",
        (pWave->cData.uChunkSize * 8) / pWave->pFrmt->uBitsPerSample);
    }

    printf("\n");
  }

  // All done OK.

  iLoadError = 0;

  // Close the file and return the errorcode.

errorExit:

  if (pWave->pFile) {
    riffClose(pWave->pFile);
  }

  pWave->pFile = NULL;

  if (iLoadError) {
    FreeWavFile(pWave);
  }

  return (iLoadError);
}



// **************************************************************************
// **************************************************************************
//
// SaveWavFile ()
//

static int SaveWavFile ( WAV_FILE *pWave, const char *pName )

{
  // Local variables.

  int iSaveError = -1;

  CHUNK_INFO      cRiff;

  // Is there both a "fmt " chunk and a "data" chunk ?

  if ((pWave->cFrmt.uChunkID == 0) || (pWave->cData.uChunkID == 0))
  {
    sprintf(aErrorMessage, "wav2vox - Output file is missing vital RIFF chunk !\n");
    return (iErrorCode = ERROR_IO_WRITE);
  }

  // Open the file.

  pWave->pFile = riffOpen(pName, OPEN_WRONLY);

  if (pWave->pFile == NULL)
  {
    sprintf(aErrorMessage, "wav2vox - Unable to open file \"%s\" !\n", pName);
    return (iErrorCode = ERROR_IO_WRITE);
  }

  // Write the 'RIFF' header.

  cRiff.uChunkID = makeID('R', 'I', 'F', 'F');
  cRiff.uChunkSize = 0;
  cRiff.uChunkType = makeID('W', 'A', 'V', 'E');

  if (riffCreateChunk(pWave->pFile, &cRiff, CREATE_RIFF) != 0) {
    goto errorExit;
  }

  // Write the 'fmt ' chunk.

  if (pWave->cFrmt.uChunkID != 0)
  {
    if (riffCreateChunk(pWave->pFile, &pWave->cFrmt, 0) != 0) {
      goto errorExit;
    }

    if (riffWrite(pWave->pFile, (char *) pWave->pFrmt, pWave->cFrmt.uChunkSize)
      != (long) pWave->cFrmt.uChunkSize) {
      goto errorExit;
    }

    if (riffAscend(pWave->pFile, &pWave->cFrmt) != 0) {
      goto errorExit;
    }
  }

  // Write the 'fact' chunk.

  if (pWave->cFact.uChunkID != 0)
  {
    if (riffCreateChunk(pWave->pFile, &pWave->cFact, 0) != 0) {
      goto errorExit;
    }

    if (riffWrite(pWave->pFile, (char *) pWave->pFact, pWave->cFact.uChunkSize)
      != (long) pWave->cFact.uChunkSize) {
      goto errorExit;
    }

    if (riffAscend(pWave->pFile, &pWave->cFact) != 0) {
      goto errorExit;
    }
  }

  // Write the 'data' chunk.

  if (pWave->cData.uChunkID != 0)
  {
    if (riffCreateChunk(pWave->pFile, &pWave->cData, 0) != 0) {
      goto errorExit;
    }

    if (riffWrite(pWave->pFile, (char *) pWave->pData, pWave->cData.uChunkSize)
      != (long) pWave->cData.uChunkSize) {
      goto errorExit;
    }

    if (riffAscend(pWave->pFile, &pWave->cData) != 0) {
      goto errorExit;
    }
  }

  // Write the 'smpl' chunk.

  if (pWave->cSmpl.uChunkID != 0)
  {
    if (riffCreateChunk(pWave->pFile, &pWave->cSmpl, 0) != 0) {
      goto errorExit;
    }

    if (riffWrite(pWave->pFile, (char *) pWave->pSmpl, pWave->cSmpl.uChunkSize)
      != (long) pWave->cSmpl.uChunkSize) {
      goto errorExit;
    }

    if (riffAscend(pWave->pFile, &pWave->cSmpl) != 0) {
      goto errorExit;
    }
  }

  // Write the 'INFO' chunk.

  if (pWave->cInfo.uChunkID != 0)
  {
    if (riffCreateChunk(pWave->pFile, &pWave->cInfo, 0) != 0) {
      goto errorExit;
    }

    if (riffWrite(pWave->pFile, (char *) pWave->pInfo, pWave->cInfo.uChunkSize) != (long) pWave->cInfo.uChunkSize) {
      goto errorExit;
    }

    if (riffAscend(pWave->pFile, &pWave->cInfo) != 0) {
      goto errorExit;
    }
  }

  // Close the file.

  if (riffAscend(pWave->pFile, &cRiff) != 0) {
    goto errorExit;
  }

  if (riffClose(pWave->pFile) != 0) {
    goto errorExit;
  }

  pWave->pFile = NULL;

  // All done OK.

  iSaveError = 0;

  // Close the file and return the errorcode.

errorExit:

  if (pWave->pFile) {
    riffClose(pWave->pFile);
  }

  pWave->pFile = NULL;

  if (iSaveError) {
    sprintf(aErrorMessage, "wav2vox - Unable the write chunk !\n");
    iErrorCode = ERROR_IO_WRITE;
  }

  return (iSaveError);
}



// **************************************************************************
// **************************************************************************
//
// FreeWavFile ()
//

static int FreeWavFile ( WAV_FILE * pWave )

{
  if (pWave->pFrmt) { free(pWave->pFrmt); pWave->pFrmt = NULL; }
  if (pWave->pFact) { free(pWave->pFact); pWave->pFact = NULL; }
  if (pWave->pData) { free(pWave->pData); pWave->pData = NULL; }
  if (pWave->pSmpl) { free(pWave->pSmpl); pWave->pSmpl = NULL; }

  return (0);
}



// **************************************************************************
// **************************************************************************
//
// ScanRiffChunks ()
//

static int ScanRiffChunks ( WAV_FILE *pWave, CHUNK_INFO *pParent, int level )

{
  // Local variables.

  int           i;

  CHUNK_INFO    cInfo;

  char          aName[16];
  char          aStep[16];

  // Create the indent string.

  for (i = 0; i != level; i++) {
      aStep[i] = ' ';
  }

  aStep[i] = '\0';

  // Read through all the chunks.

  while (riffDescend(pWave->pFile, &cInfo, pParent, 0) == 0)
  {
    // Is it a chunk that we're interested in ?

    if (pParent != NULL)
    {
      if (pParent->uChunkType == makeID('W', 'A', 'V', 'E'))
      {
        if (cInfo.uChunkID == makeID('f', 'm', 't', ' '))
          memcpy(&pWave->cFrmt, &cInfo, sizeof(CHUNK_INFO));
        else
        if (cInfo.uChunkID == makeID('f', 'a', 'c', 't'))
          memcpy(&pWave->cFact, &cInfo, sizeof(CHUNK_INFO));
        else
        if (cInfo.uChunkID == makeID('d', 'a', 't', 'a'))
          memcpy(&pWave->cData, &cInfo, sizeof(CHUNK_INFO));
        else
        if (cInfo.uChunkID == makeID('s', 'm', 'p', 'l'))
          memcpy(&pWave->cSmpl, &cInfo, sizeof(CHUNK_INFO));
        else
        if (cInfo.uChunkID   == makeID('L', 'I', 'S', 'T') &&
            cInfo.uChunkType == makeID('I', 'N', 'F', 'O'))
          memcpy(&pWave->cInfo, &cInfo, sizeof(CHUNK_INFO));
      }

      /*
      else
        if (pParent->uChunkType == makeID('I', 'N', 'F', 'O'))
      {
          if (cInfo.uChunkID == makeID('I', 'C', 'M', 'T'))
          memcpy(&pWave->cIcmt, &cInfo, sizeof(CHUNK_INFO));
      }
      */
    }

    // Inform the user what we found.

    if (bShowFile)
    {
      aName[ 4] = '\0';
      aName[12] = '\0';

      *((uint32_t *) &aName[0]) = cInfo.uChunkID;
      *((uint32_t *) &aName[8]) = cInfo.uChunkType;

      if ((cInfo.uChunkID == ID_RIFF) || (cInfo.uChunkID == ID_LIST))
      {
        printf("\n");
        printf("%s%s\n", &aStep[0], &aName[0]);
        printf("%s%s 0x%08X\n", &aStep[0], &aName[8], cInfo.uChunkSize);
      }
      else
      {
        printf("%s%s 0x%08X\n", &aStep[0], &aName[0], cInfo.uChunkSize);
      }

      bUndent = true;
    }

    // Skip passed this chunk.

    if ((cInfo.uChunkID == ID_RIFF) || (cInfo.uChunkID == ID_LIST))
    {
      if ((i = ScanRiffChunks(pWave, &cInfo, level+1)) != 0) {
        return (i);
      }
    }
    else
    {
      if ((i = riffAscend(pWave->pFile, &cInfo)) != 0)
      {
        sprintf(aErrorMessage, "wav2vox - Unable to ascend from chunk !\n");
        return (iErrorCode = ERROR_IO_READ);
      }
    }
  }

  // Print undent line break ?

  if (bShowFile)
  {
    if (bUndent)
    {
        bUndent = false;

        printf("\n");
    }
  }

  // Skip to the end of the RIFF/LIST wrapper chunk.

  if (pParent != NULL) {
    riffSeek(pWave->pFile, pParent->uDataOffset + pParent->uChunkSize,  SEEK_SET);
  } else {
    riffSeek(pWave->pFile, 0, SEEK_END);
  }

  // All done.

  return (0);
}



// **************************************************************************
// **************************************************************************
//
// LoadRiffChunk ()
//

static void * LoadRiffChunk ( RIFF_FILE * pFile, CHUNK_INFO * pInfo )

{
  // Local Variables.

  void *pBuffer;

  // Valid info ?

  if (pInfo->uChunkID == 0) {
    return (NULL);
  }

  // Seek to the correct position.

  if (riffSeek(pFile, pInfo->uDataOffset, SEEK_SET) < 0) {
    return (NULL);
  }

  // Allocate space for the data.

  if ((pBuffer = malloc(pInfo->uChunkSize)) == NULL) {
    return (NULL);
  }

  // Read the data into memory.

  if (riffRead(pFile, pBuffer, pInfo->uChunkSize) != (long) pInfo->uChunkSize) {
    free(pBuffer);
    return (NULL);
  }

  // Return the structure.

  return ((void *) pBuffer);
}



// **************************************************************************
// **************************************************************************
//
// LoadVoxFile ()
//

static int LoadVoxFile ( WAV_FILE *pWave, const char *pName )

{
  // Local variables.

  int           errorcode = -1;

  void *        pAddr = NULL;
  long          iSize = 0;

  // Initialize the data.

  memset(pWave, 0, sizeof(WAV_FILE));

  // Load up the entire binary file.

  if (LoadWholeFile( pName, &pAddr, &iSize ) < 0) {
    goto errorExit;
  }

  // Create a fake "fmt " chunk.

  pWave->pFrmt = malloc(sizeof(FRMT_EXTEND));

  pWave->pFrmt->uFormatTag      = uFormat;
  pWave->pFrmt->uNumChannels    = 1;
  pWave->pFrmt->uSamplesPerSec  = uSampleRate;
  pWave->pFrmt->uAvgBytesPerSec = uSampleRate / 2;
  pWave->pFrmt->uBlockAlign     = 1;
  pWave->pFrmt->uBitsPerSample  = 4;
  pWave->pFrmt->uExtraInfo      = 0;

  // Create a fake "fact" chunk.

  pWave->pFact = malloc(4);

  pWave->pFact[0] = pWave->uNumSamples = (2 * iSize) / pWave->pFrmt->uNumChannels;

  // Create a fake "data" chunk.

  pWave->pData = (uint8_t *) pAddr;
  pWave->cData.uChunkSize = iSize;

  // All done OK.

  errorcode = 0;

  // Close the file and return the errorcode.

errorExit:

  return (errorcode);
}



// **************************************************************************
// **************************************************************************
//
// SaveVoxFile ()
//

static int SaveVoxFile ( WAV_FILE *pWave, const char *pName )

{
  // Only handle mono.

  iErrorCode = 0;

  if (pWave->pFrmt->uNumChannels != 1)
  {
    sprintf(aErrorMessage,
      "wav2vox - Only mono samples can be output to VOX format !\n");
    iErrorCode = ERROR_ILLEGAL;
    goto errorExit;
  }

  // Only handle certain types of WAV data.

  switch (pWave->pFrmt->uFormatTag)
  {
    case WAV_TAG_MSM5205:
    case WAV_TAG_HUC6230:
    case WAV_TAG_OKI_ADPCM:
      break;
    default:
      sprintf(aErrorMessage,
        "wav2vox - You cannot write PCM data to a VOX file!\n");
      iErrorCode = ERROR_ILLEGAL;
      goto errorExit;
  }

  // Write the file.

  if (SaveWholeFile(pName, pWave->pData, pWave->cData.uChunkSize) != pWave->cData.uChunkSize)
  {
    sprintf(aErrorMessage,
      "wav2vox - Can't save output file \"%s\" !\n", pName);
    iErrorCode = ERROR_IO_WRITE;
    goto errorExit;
  }

  // All done.

errorExit:

  return (iErrorCode);
}



// **************************************************************************
// **************************************************************************
//
// ErrorQualify ()
//

static void ErrorQualify ( void )

{
  if (*aErrorMessage == '\0')

  {
    if (iErrorCode == ERROR_NONE) {
    }
    else if (iErrorCode == ERROR_DIAGNOSTIC) {
      sprintf(aErrorMessage, "Error : Error during diagnostic printout.\n");
    }
    else if (iErrorCode == ERROR_NO_MEMORY) {
      sprintf(aErrorMessage, "Error : Not enough memory to complete this operation.\n");
    }
    else if (iErrorCode == ERROR_NO_FILE) {
      sprintf(aErrorMessage, "Error : File not found.\n");
    }
    else if (iErrorCode == ERROR_IO_EOF) {
      sprintf(aErrorMessage, "Error : Unexpected end-of-file.\n");
    }
    else if (iErrorCode == ERROR_IO_READ) {
      sprintf(aErrorMessage, "Error : I/O read failure (file corrupted ?).\n");
    }
    else if (iErrorCode == ERROR_IO_WRITE) {
      sprintf(aErrorMessage, "Error : I/O write failure (disk full ?).\n");
    }
    else if (iErrorCode == ERROR_IO_SEEK) {
      sprintf(aErrorMessage, "Error : I/O seek failure (file corrupted ?).\n");
    }
    else if (iErrorCode == ERROR_PROGRAM) {
      sprintf(aErrorMessage, "Error : A program error has occurred.\n");
    }
    else {
      sprintf(aErrorMessage, "Error : Unknown error number.\n");
    }
  }
}



// **************************************************************************
// **************************************************************************
//
// GetFileLength ()
//

long GetFileLength ( FILE *file )

{
  // Local Variables.

  long        CurrentPos;
  long        FileLength;

  //

  CurrentPos = ftell(file);

  fseek(file, 0, SEEK_END);

  FileLength = ftell(file);

  fseek(file, CurrentPos, SEEK_SET);

  return (FileLength);
}



// **************************************************************************
// **************************************************************************
//
// LoadWholeFile ()
//

static long LoadWholeFile ( const char *pName, void **pAddr, long * pSize)

{
  // Local Variables.

  FILE *        pFile;
  void *        pBuffer;
  long          iSize;

  //

  if ((pName == NULL) || (pAddr == NULL) || (pSize == NULL))
  {
    return (-1);
  }

  if ((pFile = fopen(pName, "rb")) == NULL)
  {
    return (-1);
  }

  iSize = GetFileLength(pFile);

  if ((*pSize != 0) && (*pSize < iSize))
  {
    iSize = *pSize;
  }

  pBuffer = *pAddr;

  if (pBuffer == NULL)
  {
    pBuffer = malloc(iSize);
  }

  if (pBuffer == NULL)
  {
    iSize = -1;
  }
  else
  {
    if (fread(pBuffer, 1, iSize, pFile) != (size_t) iSize)
    {
      iSize = -1;
    }
  }

  fclose(pFile);

  *pSize = iSize;

  if (*pAddr == NULL)
  {
    if (iSize < 0)
    {
      free(pBuffer);
    }
    else
    {
      *pAddr = pBuffer;
    }
  }

  // All done, return size or -ve if error.

  return (iSize);
}



// **************************************************************************
// **************************************************************************
//
// SaveWholeFile ()
//

static long SaveWholeFile ( const char *pName, void *pAddr, long iSize )

{
  // Local Variables.

  FILE *pFile;

  //

  if ((pName == NULL) || (pAddr == NULL) || (iSize < 0))
  {
    return (-1);
  }

  if ((pFile = fopen(pName, "wb")) == NULL)
  {
    return (-1);
  }

  if (iSize != 0)
  {
    if (fwrite(pAddr, 1, iSize, pFile) != (size_t) iSize)
    {
      iSize = -1;
    }
  }

  fclose(pFile);

  // All done, return size or -ve if error.

  return (iSize);
}
