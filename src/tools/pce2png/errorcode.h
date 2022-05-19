// **************************************************************************
// **************************************************************************
//
// errorcode.h
//
// Global error handling for a program.
//
// Copyright John Brandwood 1994-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __ERRORCODE_h
#define __ERRORCODE_h

#include "elmer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Fatal error flag.
//
// Used to signal that the error returned by ErrorCode (see below) is fatal
// and that the program should terminate immediately.

#define NO                  (0)
#define YES                 (~0)

extern bool FatalError;

// Global error reporting variables.
//
// Used by all functions to indicate what error has occurred if they signal
// that something has gone wrong.
//
// The string ErrorMessage will be printed out to tell the user what error
// has occurred.
//
// Values of -1 to -255 are reserved to indicate standard OS errors.
// Values <= -256 should be used to indicate code specific errors.

typedef int ERRORCODE;

extern ERRORCODE g_iErrorCode;
extern char g_aErrorMessage [256];

#define ERROR_FAIL           1L
#define ERROR_NONE           0L
#define ERROR_DIAGNOSTIC    -1L
#define ERROR_NO_MEMORY     -2L
#define ERROR_NO_FILE       -3L
#define ERROR_IO_EOF        -4L
#define ERROR_IO_READ       -5L
#define ERROR_IO_WRITE      -6L
#define ERROR_IO_SEEK       -7L
#define ERROR_PROGRAM       -8L
#define ERROR_UNKNOWN       -9L
#define ERROR_UNKNOWN_FILE  -9L
#define ERROR_ILLEGAL      -10L

// When a routine returns an error code to indicate that something has
// gone wrong, it will usually set up the string ErrorMessage to indicate
// what the error was.  Some errors are so generic that the routine will
// just return an error code and not set up ErrorMessage.
//
// If you are going to print out the ErrorMessage string to tell the user
// that something has gone wrong, then you should call ErrorQualify() first
// so that if the routine itself did not supply a message then it can set
// up a default one.

extern void ErrorQualify (void);

// Once you have acknowledged that an error has occurred, then you should
// call ErrorReset() to reset the error code and the error message before
// using any of my library functions again.

extern void ErrorReset (void);

#ifdef __cplusplus
}
#endif
#endif // __ERRORCODE_h
