// **************************************************************************
// **************************************************************************
//
// lexer.h
//
// Simple lexer wrapper around argc/argv that can be expanded to handle
// command files.
//
// Copyright John Brandwood 2004-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __LEXER_h
#define __LEXER_h

#include  "elmer.h"
#include  "sysid.h"
#include  "errorcode.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//
//

typedef struct
{
  // Public

  char *  m_pTokenStr;
  int     m_iTokenLen;
  sysID   m_uTokenCrc;

  // Private

  int     m_iArgC;
  char ** m_pArgV;
} LEX_INFO;

//
//
//

ERRORCODE InitLexer (
  LEX_INFO *pLexInfo, int argc, char **argv );

ERRORCODE GetLexToken (
  LEX_INFO *pLexInfo );

ERRORCODE GetLexValue (
  LEX_INFO *pLexInfo, long *pValue );

#ifdef __cplusplus
}
#endif
#endif // __LEXER_h
