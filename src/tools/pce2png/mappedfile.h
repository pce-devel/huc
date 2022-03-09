// **************************************************************************
// **************************************************************************
//
// mappedfile.h
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

#ifndef __MAPPEDFILE_h
#define __MAPPEDFILE_h

#ifdef __cplusplus
extern "C" {
#endif

#include "elmer.h"

#if defined (_WIN32)
  typedef struct {
    uint8_t *    m_pViewAddr;
    size_t       m_uViewSize;
    const char * m_pFileName;
    HANDLE       m_hFileHnd;
    HANDLE       m_hFileMap;
  } MAPPEDFILE_T;
#else
  #include <sys/mman.h>

  typedef struct {
    uint8_t *    m_pViewAddr;
    size_t       m_uViewSize;
    const char * m_pFileName;
    int          m_hFile;
  } MAPPEDFILE_T;
#endif

extern bool OpenMemoryMappedFile (
  MAPPEDFILE_T *pMapping, const char *pName, bool bReadOnly );

extern bool CloseMemoryMappedFile (
  MAPPEDFILE_T *pMapping );

#ifdef __cplusplus
}
#endif
#endif // __MAPPEDFILE_h
