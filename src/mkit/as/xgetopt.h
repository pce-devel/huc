/*******************************************************************************
 * Copyright (c) 2016 hklo.tw@gmail.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 ********************************************************************************
 * Original source repository ... https://github.com/matthklo/xgetopt
 *
 * All original xopt* and xget* labels have changed to opt* and get* so that this
 * is a drop-in replacement for the GNU/BSD functions on Windows.
 *
 * Error messages have been changed to make them more consistent with MSYS2.
 *
 * In getopt_long_only(), ambiguous single letter options now try short options.
 ********************************************************************************/


#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern char  *optarg;
extern int  optind;
extern int  opterr;
extern int  optopt;
extern int  optreset;


/*****
 *  This is a cross-platform implementation of legacy getopt related functions.
 *  Here provides 4 functions: getopt(), getopt_long(), getopt_long_only(),
 *  and getsubopt() which mimic almost the same functionality of both GNU
 *  and BSD's version of getopt(), getopt_long(), getopt_long_only, and
 *  getsubopt().
 *
 *  Since there were some differences between GNU and BSD's getopt impl, getopt
 *  have to choose a side on those differences. Here list the behaviours:
 *
 *    Argv permuting:
 *       GNU: By default would permute the argv so that all non-option argv eventually
 *            will be moved to the end of argv. User can disable permuting by specifying
 *            a '+' char in the optstring.
 *       BSD: No permuting.
 *   xgetopt: Follow GNU. Both '+' and '-' flags in the front of optstring can be recognized
 *            and should work properly.
 *
 *    value of optopt:
 *       GNU: Set optopt only when either meet an undefined option, or detect on a missing
 *            argument.
 *       BSD: Always set it to be the option code which is currently examining at.
 *   xgetopt: Follow GNU.
 *
 *    value of optarg:
 *       GNU: Reset to 0 at the start of every getopt() call.
 *       BSD: No reseting. It remains the same value until next write by getopt().
 *   xgetopt: Follow GNU.
 *
 *    Reset state for parsing a new argc/argv set:
 *       GNU: Simply reset optind to be 1. However, it is impossible to reset parsing
 *            state when an internal short option parsing is taking place in argv[1].
 *       BSD: Use another global boolean 'optreset' for this purpose.
 *   xgetopt: Both behaviours are supported.
 *
 *    -W special case:
 *       GNU: If "W;" is present in optstring, the "-W foo" in commandline will be
 *            treated as the long option "--foo".
 *       BSD: No mentioned in the man page but seems to support such usage.
 *   xgetopt: Do NOT support -W special case.
 */

int getopt(
	int argc,
	char * const argv[],
	const char *optstr);

struct option
{
  const char *name;
  int       has_arg;
  int      *flag;
  int       val;
};

#define no_argument (0)
#define required_argument (1)
#define optional_argument (2)

int getopt_long(
	int argc,
	char * const argv[],
	const char *optstring,
	const struct option *longopts,
	int *longindex);

int getopt_long_only(
	int argc,
	char * const argv[],
	const char *optstring,
	const struct option *longopts,
	int *longindex);


int getsubopt(
	char **optionp,
	char * const *tokens,
	char **valuep);

#ifdef __cplusplus
}
#endif
