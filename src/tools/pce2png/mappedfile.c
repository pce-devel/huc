// **************************************************************************
// **************************************************************************
//
// mappedfile.c
//
// Simple platform-independant wrapper around a memory-mapped file.
//
// Copyright John Brandwood 2019-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#include "elmer.h"
#include "errorcode.h"
#include "mappedfile.h"


#if defined (_WIN32)


// **************************************************************************
// **************************************************************************
//
// CloseMemoryMappedFile() - Close a memory-mapped file.
//
// WINDOWS version.
//

bool CloseMemoryMappedFile (
  MAPPEDFILE_T *pMapping )

{
  if (pMapping->m_pViewAddr) {
    FlushViewOfFile(pMapping->m_pViewAddr, 0);
  }
  if (pMapping->m_pViewAddr) {
    UnmapViewOfFile(pMapping->m_pViewAddr);
  }
  if (pMapping->m_hFileMap) {
    CloseHandle(pMapping->m_hFileMap);
  }
  if (pMapping->m_hFileHnd) {
    CloseHandle(pMapping->m_hFileHnd);
  }

  pMapping->m_pFileName = NULL;
  pMapping->m_pViewAddr = NULL;
  pMapping->m_uViewSize = 0;
  pMapping->m_hFileMap = NULL;
  pMapping->m_hFileHnd = NULL;

  return (true);
}



// **************************************************************************
// **************************************************************************
//
// OpenMemoryMappedFile() - Open a memory-mapped file.
//
// WINDOWS version.
//

bool OpenMemoryMappedFile (
  MAPPEDFILE_T *pMapping, const char *pName, bool bReadOnly )

{
  int           iErrorCode = -1;
  DWORD         uOpnFileMode;
  DWORD         uMapFileMode;
  DWORD         uMapViewMode;
  LARGE_INTEGER cFileSize;

  // Make sure that any existing file is closed.

  CloseMemoryMappedFile(pMapping);

  //

  uOpnFileMode = (bReadOnly) ? (GENERIC_READ) : (GENERIC_READ | GENERIC_WRITE);
  uMapFileMode = (bReadOnly) ? (PAGE_READONLY) : (PAGE_READWRITE);
  uMapViewMode = (bReadOnly) ? (FILE_MAP_READ) : (FILE_MAP_WRITE);

  pMapping->m_pFileName = pName;

  printf("\nProcessing File \"%s\".\n", pMapping->m_pFileName);

  pMapping->m_hFileHnd = CreateFile(pMapping->m_pFileName,
            uOpnFileMode, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (pMapping->m_hFileHnd == INVALID_HANDLE_VALUE) {
    iErrorCode = GetLastError();
    printf("Unable to open file for reading!\nAborting!");
    goto errorExit;
  }

  if (GetFileSizeEx(pMapping->m_hFileHnd, &cFileSize) == 0) {
    iErrorCode = GetLastError();
    printf("Unable to find size of file!\nAborting!");
    goto errorExit;
  }

  if (cFileSize.HighPart != 0 || cFileSize.LowPart == 0) {
    printf("File has a bad file size! (Must be > 0 and < 4GB.)\nAborting!");
    goto errorExit;
  }

  pMapping->m_hFileMap = CreateFileMapping(pMapping->m_hFileHnd, NULL, uMapFileMode, 0, 0, NULL);

  if (pMapping->m_hFileMap == NULL) {
    iErrorCode = GetLastError();
    printf("Windows error %d!\nAborting!", iErrorCode);
    goto errorExit;
  }

  pMapping->m_pViewAddr = (uint8_t *)MapViewOfFile(pMapping->m_hFileMap, uMapViewMode, 0, 0, 0);

  if (pMapping->m_pViewAddr == NULL) {
    iErrorCode = GetLastError();
    goto errorExit;
  }

  pMapping->m_uViewSize = cFileSize.LowPart;

  return (true);

  //

errorExit:

  CloseMemoryMappedFile(pMapping);

  g_iErrorCode = iErrorCode;

  return (false);
}

#else



#include <errno.h>

// **************************************************************************
// **************************************************************************
//
// CloseMemoryMappedFile() - Close a memory-mapped file.
//
// UNIX version.
//

bool CloseMemoryMappedFile (
  MAPPEDFILE_T *pMapping )

{
  if (pMapping->m_pViewAddr) {
    msync(pMapping->m_pViewAddr, pMapping->m_uViewSize, MS_ASYNC);
  }
  if (pMapping->m_pViewAddr) {
    munmap(pMapping->m_pViewAddr, pMapping->m_uViewSize);
  }
  if (pMapping->m_hFile > 0) {
    close(pMapping->m_hFile);
  }

  pMapping->m_pFileName = NULL;
  pMapping->m_pViewAddr = NULL;
  pMapping->m_uViewSize = 0;
  pMapping->m_hFile = -1;

  return (true);
}



// **************************************************************************
// **************************************************************************
//
// OpenMemoryMappedFile() - Open a memory-mapped file.
//
// UNIX version.
//

bool OpenMemoryMappedFile (
  MAPPEDFILE_T *pMapping, const char *pName, bool bReadOnly )

{
  struct stat   cStat;
  int           iErrorCode = -1;
  int           iMode;

  // Make sure that we any existing file is closed.

  CloseMemoryMappedFile(pMapping);

  //

  pMapping->m_pFileName = pName;

  iMode = (bReadOnly) ? (O_RDONLY) : (O_RDWR);

  pMapping->m_hFile = open(pName, O_BINARY | iMode);

  if (pMapping->m_hFile == -1) {
    iErrorCode = errno;
    printf("Unable to open file for reading!\nAborting!");
    goto errorExit;
  }

  if ((fstat(pMapping->m_hFile, &cStat) != 0) || (!S_ISREG(cStat.st_mode))) {
    iErrorCode = errno;
    printf("Unable to find size of file!\nAborting!");
    goto errorExit;
  }

  if (cStat.st_size > SSIZE_MAX) {
    printf("File has a bad file size! (Must be > 0 and < 4GB.)\nAborting!");
    goto errorExit;
  }

  pMapping->m_uViewSize = cStat.st_size;

  iMode = (bReadOnly) ? (PROT_READ) : (PROT_READ | PROT_WRITE);

  pMapping->m_pViewAddr = mmap(NULL, pMapping->m_uViewSize, iMode, MAP_SHARED, pMapping->m_hFile, 0);

  if (pMapping->m_pViewAddr == MAP_FAILED) {
    pMapping->m_pViewAddr = NULL;
    iErrorCode = errno;
    goto errorExit;
  }

  return (true);

  //

errorExit:

  CloseMemoryMappedFile(pMapping);

  g_iErrorCode = iErrorCode;

  return (false);
}

#endif
