// **************************************************************************
// **************************************************************************
//
// elmer.h
//
// General type definitions.
//
// Copyright John Brandwood 1994-2021.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __ELMER_h
#define __ELMER_h

#ifdef _WIN32
  #include "targetver.h"
  #define  WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

//
// Standard C99 includes, with Visual Studio workarounds.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

// DETERMINE TARGET MACHINE BYTE ORDERING ...
//
// either BYTE_ORDER_LO_HI for 80x86 ordering,
// or     BYTE_ORDER_HI_LO for 680x0 ordering.

#ifdef PC
  #define BYTE_ORDER_LO_HI  1
  #define BYTE_ORDER_HI_LO  0
#endif

#endif // !__ELMER_h
