// **************************************************************************
// **************************************************************************
//
// lexer.c
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

#include  "elmer.h"
#include  "sysid.h"
#include  "lexer.h"



// **************************************************************************
// **************************************************************************
//
// InitLexer () - Simple wrapper around argc/argv.
//

ERRORCODE InitLexer (
  LEX_INFO *pLexInfo, int argc, char **argv )

{
  memset( pLexInfo, 0, sizeof( LEX_INFO ));

  pLexInfo->m_iArgC = argc;
  pLexInfo->m_pArgV = argv;

  return ERROR_NONE;
}



// **************************************************************************
// **************************************************************************
//
// GetLexToken () - Simple wrapper around argc/argv.
//

ERRORCODE GetLexToken (
  LEX_INFO *pLexInfo )

{
  if (pLexInfo->m_iArgC == 0)
  {
    pLexInfo->m_pTokenStr = NULL;
    pLexInfo->m_iTokenLen = 0;
    return (g_iErrorCode = ERROR_IO_EOF);
  }

  pLexInfo->m_iArgC--;

  pLexInfo->m_pTokenStr = *( pLexInfo->m_pArgV++ );
  pLexInfo->m_iTokenLen = strlen( pLexInfo->m_pTokenStr );
  pLexInfo->m_uTokenCrc = 0;

  if (pLexInfo->m_pTokenStr[0] == '-')
  {
    pLexInfo->m_uTokenCrc =
      PStringToID( pLexInfo->m_iTokenLen - 1, pLexInfo->m_pTokenStr + 1 );
  }

  return ERROR_NONE;
}



// **************************************************************************
// **************************************************************************
//
//  GetLexValue () - Simple wrapper around argc/argv.
//

ERRORCODE GetLexValue (
  LEX_INFO *pLexInfo, long *pValue )

{
  char * pToken;

  if (GetLexToken( pLexInfo )) return (g_iErrorCode = ERROR_IO_EOF);

  pToken = pLexInfo->m_pTokenStr;

  *pValue = strtol( pToken, &pToken, 0 );

  if (*pToken != 0)
  {
    printf( "Expected a number, found \"%s\"!\n", pLexInfo->m_pTokenStr );
    return (g_iErrorCode = ERROR_ILLEGAL);
  }

  return ERROR_NONE;
}
