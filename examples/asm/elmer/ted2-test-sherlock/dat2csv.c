// **************************************************************************
// **************************************************************************
//
// dat2csv.c
//
// Convert the Sherlock logfile from binary to a CSV spreadsheet.
//
// Copyright John Brandwood 2024.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)y
//
// **************************************************************************
// **************************************************************************
//
// This is a simple PC/Linux/MacOS command-line program that takes the 192KB
// SHERLOCK.DAT data file and converts the event log into a .CSV spreadsheet
// so that the data is easier for a human to understand.
//
// Just run the "dat2csv" program in the same directory as the SHERLOCK.DAT
// file and it will output the human-readable SHERLOCK.CSV spreadsheet file.
//
// **************************************************************************
// **************************************************************************

//
// Standard C99 includes, with Visual Studio workarounds.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined (_MSC_VER)
  #include <io.h>
  #define SSIZE_MAX UINT_MAX
  #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
  #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
#else
  #include <unistd.h>
  #ifndef O_BINARY
    #define O_BINARY 0
    #define O_TEXT   0
  #endif
#endif

#if defined (_MSC_VER) && (_MSC_VER < 1600) // i.e. before VS2010
  #define __STDC_LIMIT_MACROS
  #include "msinttypes/stdint.h"
#else
  #include <stdint.h>
#endif

#if defined (_MSC_VER) && (_MSC_VER < 1800) // i.e. before VS2013
  #define static_assert( test, message )
  #include "msinttypes/inttypes.h"
  #include "msinttypes/stdbool.h"
#else
  #include <inttypes.h>
  #include <stdbool.h>
#endif



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

bool ReadBinaryFile (const char *pName, uint8_t **pBuffer, size_t *pLength)
{
  uint8_t *     pData = NULL;
  off_t         uSize;
  struct stat   cStat;

  int hFile = open(pName, O_BINARY | O_RDONLY);

  if (hFile == -1)
    goto errorExit;

  if ((fstat(hFile, &cStat) != 0) || (!S_ISREG(cStat.st_mode)))
    goto errorExit;

  if (cStat.st_size > SSIZE_MAX)
    goto errorExit;

  uSize = cStat.st_size;

  pData = (uint8_t *)malloc(uSize);

  if (pData == NULL)
    goto errorExit;

  if (read(hFile, pData, uSize) != uSize)
    goto errorExit;

  close(hFile);

  *pBuffer = pData;
  *pLength = uSize;

  return (true);

  // Handle errors.

errorExit:

  if (pData != NULL) free(pData);
  if (hFile >= 0) close(hFile);

  *pBuffer = NULL;
  *pLength = 0;

  return (false);
}



// **************************************************************************
// **************************************************************************
//
// main ()
//

// Data offsets for the information in each logged event.

#define EVENT_TYPE    0
#define EVENT_TIMER   1
#define EVENT_BLANK   2
#define EVENT_FRAME   3
#define EVENT_TICK    4
#define EVENT_LEFT    5
#define EVENT_WRONG   6
#define EVENT_DELAY   7

// Event types.

#define INFO_VBLANK     1

#define INFO_WAITVBL    2

#define INFO_SCSIRMV    3
#define INFO_SCSIIDLE   4
#define INFO_SCSISEND   5

#define INFO_DATAEND    6
#define INFO_FILLEND    7

#define INFO_SEEK_BEGIN 10
#define INFO_SEEK_ENDED 11

#define INFO_NOTBUSY    12

#define INFO_SCSI80     0x80u // PHASE_DATA_OUT
#define INFO_SCSI88     0x88u // PHASE_DATA_IN
#define INFO_SCSI98     0x98u // PHASE_STAT_IN

#define PHASE_COMMAND   0xD0u // (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + ........)
#define PHASE_DATA_OUT  0xC0u // (SCSI_BSY + SCSI_REQ + ........ + ........ + ........)
#define PHASE_DATA_IN   0xC8u // (SCSI_BSY + SCSI_REQ + ........ + ........ + SCSI_IXO)
#define PHASE_STAT_IN   0xD8u // (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + SCSI_IXO)
#define PHASE_MESG_OUT  0xF0u // (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + ........)
#define PHASE_MESG_IN   0xF8u // (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + SCSI_IXO)

#define REQ_DATA_OUT    (0xC0u+1) // (SCSI_BSY + SCSI_REQ + ........ + ........ + ........)
#define REQ_DATA_IN     (0xC8u+1) // (SCSI_BSY + SCSI_REQ + ........ + ........ + SCSI_IXO)
#define REQ_STAT_IN     (0xD8u+1) // (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + SCSI_IXO)

#define RMV_COMMAND     (0xD0u+2) // (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + ........)
#define RMV_DATA_OUT    (0xC0u+2) // (SCSI_BSY + SCSI_REQ + ........ + ........ + ........)
#define RMV_DATA_IN     (0xC8u+2) // (SCSI_BSY + SCSI_REQ + ........ + ........ + SCSI_IXO)
#define RMV_STAT_IN     (0xD8u+2) // (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + SCSI_IXO)
#define RMV_MESG_OUT    (0xF0u+2) // (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + ........)
#define RMV_MESG_IN     (0xF8u+2) // (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + SCSI_IXO)

#define INFO_EXIT       0x7Fu

//

int main ( int argc, char **argv )

{
  //

  uint8_t * pDatFile;
  uint8_t * pDatStop;
  size_t uDatSize;

  FILE * pCsvFile;

  uint8_t * pLog;

  unsigned uLastData;
  unsigned uThisData;

  unsigned uTimerAdd;
  unsigned uBlankAdd;
  unsigned uFrameAdd;

  uint8_t uLastTimer;
  uint8_t uLastBlank;
  uint8_t uLastFrame;

  // Sign on.

  printf("dat2csv - Converting SHERLOCK.DAT to SHERLOCK.CSV\n\n");

  // Read the Sherlock data file.

  pDatFile = NULL;
  uDatSize = 0;

  if (!ReadBinaryFile( "SHERLOCK.DAT", &pDatFile, &uDatSize))
  {
    printf("Cannot load SHERLOCK.DAT!\n");
    exit(1);
  }

  pDatStop = pDatFile + uDatSize;

  //

  pCsvFile = NULL;

  if ((pCsvFile = fopen( "SHERLOCK.CSV", "w" )) == NULL)
  {
    printf( "Cannot open SHERLOCK.CSV!\n" );
    exit( 1 );
  }

  fprintf( pCsvFile, "Timer,Vblank,Frame,Tick,Sectors,Wrong,Delay,Reason\n" );

  //

  uTimerAdd = 0;
  uBlankAdd = 0;
  uFrameAdd = 0;

  uLastTimer = 0;
  uLastBlank = 0;
  uLastFrame = 0;

  uLastData = 0;
  uThisData = 0;

  pLog = pDatFile;

  for (pLog = pDatFile; pLog <= (pDatStop - 8); pLog += 8)
  {
    // Put a blank line between each frame of video.

    if (uLastFrame != pLog[EVENT_FRAME])
    {
      fprintf( pCsvFile, "\n" );
    }

    // Did any of the byte values just wrap?

    if (uLastTimer > pLog[EVENT_TIMER]) uTimerAdd += 256;
    if (uLastBlank > pLog[EVENT_BLANK]) uBlankAdd += 256;
    if (uLastFrame > pLog[EVENT_FRAME]) uFrameAdd += 256;

    uLastTimer = pLog[EVENT_TIMER];
    uLastBlank = pLog[EVENT_BLANK];
    uLastFrame = pLog[EVENT_FRAME];

    // Filter out irrelevent VBLANK events.

    if ((pLog[EVENT_TYPE] == INFO_VBLANK) && (pLog[EVENT_DELAY] == 0))
    {
      continue;
    }

    // Output the data values.

    fprintf( pCsvFile, "%d," , uTimerAdd + pLog[EVENT_TIMER] );
    fprintf( pCsvFile, "%d," , uBlankAdd + pLog[EVENT_BLANK] );
    fprintf( pCsvFile, "%d," , uFrameAdd + pLog[EVENT_FRAME] );
    fprintf( pCsvFile, "%d," , (unsigned) pLog[EVENT_TICK] );
    fprintf( pCsvFile, "%d," , (unsigned) pLog[EVENT_LEFT] );
    fprintf( pCsvFile, "%d," , (int) *((int8_t *) (pLog + EVENT_WRONG)) );
    fprintf( pCsvFile, "%d," , (unsigned) pLog[EVENT_DELAY] );

    // Output the event type.

    switch (pLog[EVENT_TYPE])
    {
      case INFO_VBLANK:
        fprintf( pCsvFile, "VBLANK IRQ\n" );
        break;

      case INFO_WAITVBL:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Wait for VBLANK\n" );
        break;

      case INFO_SCSIRMV:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Flush byte from queue\n" );
        break;

      case INFO_SCSIIDLE:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Idle: Awaiting command\n" );
        break;

      case INFO_SCSISEND:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Sending command\n" );
        break;

      case INFO_DATAEND:
        fprintf( pCsvFile, "Finished reading Sector\n" );
        break;

      case INFO_FILLEND:
        fprintf( pCsvFile, "ADPCM Prefill Completed\n\n" );
        break;

      case INFO_SEEK_BEGIN:
        fprintf( pCsvFile, "Movie finished; System Card cd_seek\n" );
        break;

      case INFO_SEEK_ENDED:
        fprintf( pCsvFile, "Movie finished; System Card cd_seek completed\n" );
        break;

      case INFO_NOTBUSY:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Idle: Queue flushed\n" );
        break;

      case INFO_SCSI80:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Got SCSI $80 (PHASE_DATA_OUT without REQ)\n" );
        break;

      case INFO_SCSI88:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Got SCSI $88 (PHASE_DATA_IN without REQ)\n" );
        break;

      case INFO_SCSI98:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Got SCSI $98 (PHASE_STAT_IN without REQ)\n" );
        break;

      case REQ_DATA_OUT:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: REQ asserted beginning PHASE_DATA_OUT\n" );
        break;

      case REQ_DATA_IN:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: REQ asserted beginning PHASE_DATA_IN\n" );
        break;

      case REQ_STAT_IN:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: REQ asserted beginning PHASE_STAT_IN\n" );
        break;

      case RMV_COMMAND:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Flush byte from queue during PHASE_COMMAND\n" );
        break;

      case RMV_DATA_OUT:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Flush byte from queue during PHASE_DATA_OUT\n" );
        break;

      case RMV_DATA_IN:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Flush byte from queue during PHASE_DATA_IN\n" );
        break;

      case RMV_STAT_IN:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Flush byte from queue during PHASE_STAT_IN\n" );
        break;

      case RMV_MESG_OUT:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Flush byte from queue during PHASE_MESG_OUT\n" );
        break;

      case RMV_MESG_IN:
        fprintf( pCsvFile, "Send SCSI command; CD-ROM Busy: Flush byte from queue during PHASE_MESG_IN\n" );
        break;

      case PHASE_DATA_IN:
        uThisData = uTimerAdd + pLog[EVENT_TIMER];
        if ((uThisData - uLastData) > 18)
        {
          fprintf( pCsvFile, "Wait for next Sector; Got SCSI PHASE_DATA_IN; %dms since last sector!\n", uThisData - uLastData );
        }
        else
        {
          fprintf( pCsvFile, "Wait for next Sector; Got SCSI PHASE_DATA_IN\n" );
        }
        uLastData = uThisData;
        break;

      case PHASE_STAT_IN:
        fprintf( pCsvFile, "Wait for next Sector; Got SCSI PHASE_STAT_IN\n" );
        break;

      case INFO_EXIT:
        fprintf( pCsvFile, "Movie finished!\n" );
        break;

      default:
        printf( "Unknown event type 0x%2X in data!\n", pLog[EVENT_TYPE] );
        fprintf( pCsvFile, "Unknown event type 0x%2X in data!\n", pLog[EVENT_TYPE] );
        fclose( pCsvFile );
        exit( 1 );
    }

    if (pLog[EVENT_TYPE] == INFO_EXIT) break;
  }

  fclose( pCsvFile );

  //

  printf( "Finished writing SHERLOCK.CSV\n" );

  return( 0 );
}
