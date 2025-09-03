// **************************************************************************
// **************************************************************************
//
// riffio.c
//
// Simple memory-mapped replacements for Win32's mmioXXX RIFF libraries.
//
// Copyright John Brandwood 1997-2021.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#include "elmer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "riffio.h"

//
//
//

static uint8_t bPad = 0; // Byte to write for 16-bit padding.



// **************************************************************************
// **************************************************************************
//
// riffOpen ()
//
// Similar to WIN32 function mmioOpen()
//

RIFF_FILE * riffOpen (
  const char * pFileName, unsigned uFlags )

{
  // Local Variables.

  RIFF_FILE *   pRIFF = NULL;
  FILE *        pFile = NULL;
  size_t        uSize;

  // Can't handle simultaneous read and write.

  if ((uFlags & OPEN_ACCMODE) == OPEN_RDWR)
  {
    goto errorExit;
  }

  // Allocate a RIFF file handle.

  if ((pRIFF = calloc(sizeof(RIFF_FILE), 1)) == NULL)
  {
    goto errorExit;
  }

  pRIFF->uFlg = uFlags;

  strcpy(pRIFF->aNam, pFileName);

  // Open for reading ?

  if ((uFlags & OPEN_ACCMODE) == OPEN_RDONLY)
  {
    // Read the whole file into memory.

    if ((pFile = fopen(pRIFF->aNam, "rb")) == NULL)
    {
      goto errorExit;
    }

    fseek(pFile, 0, SEEK_END);

    uSize = ftell(pFile);

    fseek(pFile, 0, SEEK_SET);

    if ((pRIFF->pBuf = (uint8_t *) malloc(uSize)) == NULL)
    {
      goto errorExit;
    }

    if (fread(pRIFF->pBuf, 1, uSize, pFile) != (size_t) uSize)
    {
      goto errorExit;
    }

    fclose(pFile);

    pRIFF->pCur = pRIFF->pBuf;
    pRIFF->pMrk = pRIFF->pBuf + uSize;
    pRIFF->pEnd = pRIFF->pBuf + uSize;

    return ((RIFF_FILE *) pRIFF);
  }

  // Open for writing ?

  else
  if ((uFlags & OPEN_ACCMODE) == OPEN_WRONLY)
  {
    // Allocate a memory buffer to write file to.
    //
    // Oh Dear ... this may have made some sense in 1997, but YUK!!!

    uSize = 64 * 1024 * 1024;

    if ((pRIFF->pBuf = (uint8_t *) malloc(uSize)) == NULL)
    {
      goto errorExit;
    }

    pRIFF->pCur = pRIFF->pBuf;
    pRIFF->pMrk = pRIFF->pBuf;
    pRIFF->pEnd = pRIFF->pBuf + uSize;

    return ((RIFF_FILE *) pRIFF);
  }

  // Error condition.

errorExit:

  if (pFile != NULL)
  {
    fclose(pFile);
  }

  if (pRIFF != NULL)
  {
    if (pRIFF->pBuf != NULL)
    {
      free(pRIFF->pBuf);
    }
    free(pRIFF);
  }

  return (NULL);
}



// **************************************************************************
// **************************************************************************
//
// riffClose ()
//
// Similar to WIN32 function mmioClose()
//

int riffClose (
  RIFF_FILE * pRIFF )

{
  // Local Variables.

  FILE *        pFile = NULL;
  size_t        uSize;

  int errorcode = ERROR_CANNOTWRITE;

  // Opened for reading ?

  if ((pRIFF->uFlg & OPEN_ACCMODE) == OPEN_RDONLY)
  {
    // Nothing to do.

    errorcode = 0;

    goto errorExit;
  }

  // Opened for writing ?

  else
  if ((pRIFF->uFlg & OPEN_ACCMODE) == OPEN_WRONLY)
  {
    // Write the whole file out from memory.

    if ((pFile = fopen(pRIFF->aNam, "wb")) == NULL)
    {
      goto errorExit;
    }

    if ((uSize = pRIFF->pMrk - pRIFF->pBuf) > 0)
    {
      if (fwrite(pRIFF->pBuf, 1, uSize, pFile) != uSize)
      {
        goto errorExit;
      }
    }

    fclose(pFile);

    errorcode = 0;

    goto errorExit;
  }

  // Error condition.

  errorExit:

  if (pFile != NULL)
  {
    fclose(pFile);
  }

  if (pRIFF != NULL)
  {
    if (pRIFF->pBuf != NULL)
    {
      free(pRIFF->pBuf);
    }
    free(pRIFF);
  }

  return (errorcode);
}



// **************************************************************************
// **************************************************************************
//
// riffSeek ()
//
// Similar to WIN32 function mmioSeek()
//

long riffSeek (
  RIFF_FILE * pRIFF, long iOffset, int iOrigin )

{
  // Move the current file position.

  if (iOrigin == SEEK_CUR)
  {
    pRIFF->pCur += iOffset;
  }
  else
  if (iOrigin == SEEK_SET)
  {
    pRIFF->pCur = pRIFF->pBuf + iOffset;
  }
  else
  if (iOrigin == SEEK_END)
  {
    pRIFF->pCur = pRIFF->pMrk + iOffset;
  }

  // All done, return current position.

  return (long) (pRIFF->pCur - pRIFF->pBuf);
}



// **************************************************************************
// **************************************************************************
//
// riffRead ()
//
// Similar to WIN32 function mmioRead()
//

long riffRead (
  RIFF_FILE * pRIFF, void *pBuffer, long iLength )

{
  // Opened for reading ?

  if ((pRIFF->uFlg & OPEN_ACCMODE) == OPEN_WRONLY)
  {
    return (-1);
  }

  // Beyond the start of the file ?

  if (pRIFF->pCur < pRIFF->pBuf)
  {
    return (-1);
  }

  // Beyond the end of the file ?

  if (pRIFF->pCur >= pRIFF->pMrk)
  {
    return (0);
  }

  // Make sure that we don't go beyond the end of the file.

  if (iLength > (pRIFF->pMrk - pRIFF->pCur))
  {
    iLength = (long) (pRIFF->pMrk - pRIFF->pCur);
  }

  // Copy the data from memory and update the current position.

  memcpy(pBuffer, pRIFF->pCur, iLength);

  pRIFF->pCur += iLength;

  // Return the number of bytes read.

  return (iLength);
}



// **************************************************************************
// **************************************************************************
//
// riffWrite ()
//
// Similar to WIN32 function mmioWrite()
//

long riffWrite (
  RIFF_FILE * pRIFF, const void *pBuffer, long iLength )

{
  // Opened for writing ?

  if ((pRIFF->uFlg & OPEN_ACCMODE) == OPEN_RDONLY)
  {
    return (-1);
  }

  // Beyond the start of the file ?

  if (pRIFF->pCur < pRIFF->pBuf)
  {
    return (-1);
  }

  // Beyond the end of the file ?

  if (pRIFF->pCur >= pRIFF->pEnd)
  {
    return (0);
  }

  // Make sure that we don't go beyond the end of the buffer.

  if (iLength > (pRIFF->pEnd - pRIFF->pCur))
  {
    iLength = (long) (pRIFF->pEnd - pRIFF->pCur);
  }

  // Copy the data from memory and update the current position.

  memcpy(pRIFF->pCur, pBuffer, iLength);

  pRIFF->pCur += iLength;

  if (pRIFF->pMrk < pRIFF->pCur)
  {
    pRIFF->pMrk = pRIFF->pCur;
  }

  // Return the number of bytes read.

  return (iLength);
}



// **************************************************************************
// **************************************************************************
//
// riffDescend ()
//
// Similar to WIN32 function mmioDescend()
//

int riffDescend (
  RIFF_FILE * pRIFF, CHUNK_INFO *pChunk, const CHUNK_INFO *pParent, unsigned uFlags )

{
  // Local Variables.

  RIFF_ID uSearchID;
  RIFF_ID uSearchType;

  int i;

  // Figure out what chunk id and form/list type to search for.

  if (uFlags & FIND_RIFF) {
    uSearchID = ID_RIFF; uSearchType = pChunk->uChunkType;
  }
  else
  if (uFlags & FIND_LIST) {
    uSearchID = ID_LIST; uSearchType = pChunk->uChunkType;
  }
  else
  if (uFlags & FIND_CHUNK) {
    uSearchID = pChunk->uChunkID; uSearchType = 0;
  }
  else {
    uSearchID = uSearchType = 0;
  }

  pChunk->bDirty = false;

  while (true)
  {
    // Read the chunk header.

    if (riffRead(pRIFF, pChunk, 2 * sizeof(uint32_t)) != 2 * sizeof(uint32_t))
    {
      return (ERROR_CHUNKNOTFOUND);
    }

    // Store the offset of the data part of the chunk.

    if ((pChunk->uDataOffset = riffSeek(pRIFF, 0L, SEEK_CUR)) == -1)
    {
      return (ERROR_CANNOTSEEK);
    }

    // See if the chunk is within the parent chunk (if given).

    if ((pParent != NULL) &&
        (pChunk->uDataOffset - 8L >= pParent->uDataOffset + pParent->uChunkSize))
    {
      return (ERROR_CHUNKNOTFOUND);
    }

    // If the chunk is a 'RIFF' or 'LIST' chunk, read the form type or list type.

    if ((pChunk->uChunkID == ID_RIFF) || (pChunk->uChunkID == ID_LIST))
    {
      if (riffRead(pRIFF, &pChunk->uChunkType, sizeof(uint32_t)) != sizeof(uint32_t))
      {
        return (ERROR_CHUNKNOTFOUND);
      }
    }
    else
    {
      pChunk->uChunkType = 0;
    }

    // If this is the chunk we're looking for, stop looking.

    if (((uSearchID == 0) || (uSearchID  == pChunk->uChunkID)) &&
        ((uSearchType == 0) || (uSearchType == pChunk->uChunkType)))
    {
      break;
    }

    // Ascend out of the chunk and try again.

    if ((i = riffAscend(pRIFF, pChunk)) != 0)
    {
      return (i);
    }
  }

  return (0);
}



// **************************************************************************
// **************************************************************************
//
// riffAscend ()
//
// Similar to WIN32 function mmioAscend()
//

int riffAscend (
  RIFF_FILE * pRIFF, CHUNK_INFO *pChunk )

{
  // Local Variables.

  long iOffset;     // current offset in file
  long lActualSize; // actual size of chunk data

  //

  if (pChunk->bDirty)
  {
    // <pChunk> refers to a chunk created by mmioCreateChunk();
    //
    // Check that the chunk size that was written when
    // mmioCreateChunk() was called is the real chunk size;
    // if not, fix it.

    if ((iOffset = riffSeek(pRIFF, 0L, SEEK_CUR)) == -1)
    {
      return (ERROR_CANNOTSEEK);
    }

    if ((lActualSize = iOffset - pChunk->uDataOffset) < 0)
    {
      return (ERROR_CANNOTWRITE);
    }

    if (lActualSize & 1)
    {
      // Chunk size is odd -- write a null pad byte.

      if (riffWrite(pRIFF, &bPad, 1) != 1)
      {
        return (ERROR_CANNOTWRITE);
      }
    }

    if (pChunk->uChunkSize != (uint32_t) lActualSize)
    {
      // Fix the chunk header.

      pChunk->uChunkSize = lActualSize;

      if (riffSeek(pRIFF, pChunk->uDataOffset - sizeof(uint32_t), SEEK_SET) == -1)
      {
        return (ERROR_CANNOTSEEK);
      }

      if (riffWrite(pRIFF, &pChunk->uChunkSize, sizeof(uint32_t)) != sizeof(uint32_t))
      {
        return (ERROR_CANNOTWRITE);
      }
    }
  }

  // Seek to the end of the chunk, passed the null pad byte if present.

  if (riffSeek(pRIFF, pChunk->uDataOffset + pChunk->uChunkSize + (pChunk->uChunkSize & 1L), SEEK_SET) == -1)
  {
    return (ERROR_CANNOTSEEK);
  }

  return (0);
}



// **************************************************************************
// **************************************************************************
//
// riffCreateChunk ()
//
// Similar to WIN32 function mmioCreateChunk()
//

int riffCreateChunk (
  RIFF_FILE * pRIFF, CHUNK_INFO *pChunk, unsigned uFlags )

{
  // Local Variables.

  long iBytes;  // bytes to write
  long iOffset; // current offset in file

  // Store the offset of the data part of the chunk

  if ((iOffset = riffSeek(pRIFF, 0L, SEEK_CUR)) == -1)
  {
    return (ERROR_CANNOTSEEK);
  }

  pChunk->uDataOffset = iOffset + 2 * sizeof(uint32_t);

  // Figure out if a form/list type needs to be written.

  if (uFlags & CREATE_RIFF)
  {
    pChunk->uChunkID = ID_RIFF; iBytes = 3 * sizeof(uint32_t);
  }
  else
  if (uFlags & CREATE_LIST)
  {
    pChunk->uChunkID = ID_LIST; iBytes = 3 * sizeof(uint32_t);
  }
  else
  {
    iBytes = 2 * sizeof(uint32_t);
  }

  // Write the chunk header.

  if (riffWrite(pRIFF, pChunk, iBytes) != iBytes)
  {
    return (ERROR_CANNOTWRITE);
  }

  pChunk->bDirty = true;

  return (0);
}
