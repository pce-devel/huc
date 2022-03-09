// **************************************************************************
// **************************************************************************
//
// sysid.h
//
// Functions for converting a string into a 'unique' 32-bit ID.
//
// Copyright John Brandwood 2004-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************
//
// This is my solution to the classic "How to Identify Game Objects by Name".
//
// REFERENCES ...
//
//   http://cowboyprogramming.com/2007/01/04/practical-hash-ids/
//   http://msinilo.pl/blog/?p=82
//
//   Only available now at https://web.archive.org/  ...
//
//     http://entland.homelinux.com/blog/2009/04/28/compile-time-strings/
//
// **************************************************************************
// **************************************************************************

#ifndef __SYSID_h
#define __SYSID_h

#include "elmer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t sysID;

// Reserve some values for use by the system.

#define NUM_RESERVED_SYSID_VALUES 65536

// The first xxx sysIDs have predefined meanings to our game engine.

#define NUM_PREDEFINED_SYSID_VALUES 256

// If debugging, then make sure that the ID has not been edited.

#if defined (NDEBUG)
  #define MakeID(id, str) ((sysID)id)
#else
  #define MakeID(id, str) CheckCStringToID(str, id)
#endif

#define ConstID(id, str) ((sysID)id)

// A real ANSI X3.66/Ethernet/Zip/etc CRC-32.

extern uint32_t CalculateCRC32 (
  const void *pData, size_t iSize );

extern uint8_t g_aAsciiLowerCaseTable[256];

extern sysID CStringToID (
  const void *pString );

extern sysID PStringToID (
  size_t iStringLength, const void *pString );

extern sysID CheckCStringToID (
  const void *pString, const sysID iPreCalculatedID );

#ifdef __cplusplus
}
#endif
#endif // __SYSID_h
