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

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xgetopt.h"

// Instance of extern global variables
int optind;
int optopt;
int opterr = 1;
int optreset;
char *optarg;

// A local-scope variable.
// Used for tracking parsing point within the same argument element.
static char *optcur;

static int getopt_impl(int argc, char * const argv[],
	const char *optstring, const struct option *longopts,
	int *longindex, char only);

// Permute an argv array. So that all non-option element will be
// moved to the end of argv with respect to their original presenting order.
static void getopt_permute(int argc, char * const argv[],
	const char *optstring, const struct option *longopts,
	char only)
{
	// Tweak the optstring:
	//   1. Remove any heading GNU flags ('+','-') at front of it (if any).
	//   2. Insert a single '+' at the front.
	char *pstr = 0;
	if (optstring == 0)
	{
#ifdef _MSC_VER
		pstr = _strdup("+");
#else
		pstr = strdup("+");
#endif
	}
	else
	{
		int len = (int)strlen(optstring);
		pstr = (char*)malloc(len+2);
		memset(pstr, 0, len+2);
		while (((*optstring) == '+') || ((*optstring) == '-'))
			optstring++;
		pstr[0] = '+';
		strcpy(&pstr[1], optstring);
	}

	// Clone longopts and tweak on flag and val.
	struct option *x = 0;
	if (longopts)
	{
		int cnt;
		for (cnt=0; (longopts[cnt].name != 0) || (longopts[cnt].flag != 0) || (longopts[cnt].has_arg != 0) || (longopts[cnt].val != 0); ++cnt);
		x = (struct option*) malloc(sizeof(struct option) * (cnt+1));
		memset(x, 0, sizeof(struct option) * (cnt+1));
		for (int i=0; i<cnt; ++i)
		{
			x[i].name = longopts[i].name;
			x[i].has_arg = longopts[i].has_arg;
			x[i].flag = 0;
			x[i].val = 0x5299;
		}
	}

	// A char* buffer at most can hold all entries in argv.
	char **nonopts = (char**) malloc( sizeof(char*) * argc );
	int nonoptind = 0;

	int old_opterr = opterr;
	opterr = 0;
	optreset = 1;

	char stop = 0;
	while (!stop)
	{
		int code = getopt_impl(argc, argv, pstr, x, 0, only);
		switch (code)
		{
		case -1:
			if (optind < argc)
			{
				// Detach current element to nonopts buffer.
				// Rotate all remaining elements forward.
				nonopts[nonoptind++] = argv[optind];
				char **pargs = (char**)argv;
				for (int i=optind+1; i < argc; ++i)
					pargs[i-1] = pargs[i];
				argc--;

				// Continue iteration on optind again.
				optcur = 0;
				continue;
			}
			else
			{
				stop = 1;

				// Append all element in nonopts at
				// the end of argv.
				char **pargs = (char**)argv;
				for (int i=0; i<nonoptind; ++i)
				{
					pargs[argc++] = nonopts[i];
				}
			}
			break;
		default:
			break;
		}
	}

	if (x)
		free(x);
	free(pstr);
	free(nonopts);

	opterr = old_opterr;
}

// A general getopt parser. Used for both short options and long options.
static int getopt_impl(int argc, char * const argv[],
	const char *optstring, const struct option *longopts,
	int *longindex, char only)
{
	// Determine the executable name. This is used when outputing
	// stderr messages.
	const char *progname = strrchr(argv[0], '/');
#ifdef _WIN32
	if (!progname) progname = strrchr(argv[0], '\\');
#endif
	if (progname)
		progname++;
	else
		progname = argv[0];

	if (optstring == 0)
		optstring = "";

	// BSD implementation provides a user controllable optreset flag.
	// Set to be a non-zero value to ask getopt to clean up
	// all internal state and thus perform a whole new parsing
	// iteration.
	if (optreset != 0)
	{
		optreset = 0;
		optcur = 0;
		optind = 0;
		optarg = 0;
		optopt = 0;
	}

	if (optind < 1)
		optind = 1;

	char missingarg = 0; // Whether a missing argument should return as ':' (true) or '?' (false).
	char permute = 1;    // Whether to perform content permuting: Permute the contents of the argument vector (argv)
	                   // as it scans, so that eventually all the non-option arguments are at the end.
	char argofone = 0;   // Whether to handle each nonoption argv-element as if it were the argument of an option
	                   // with character code 1.
	//bool dashw = (0 != strstr(optstring, "W;")); // TODO: If 'W;' exists in optstring indicates '-W foo' will be treated as '--foo'.

	// Parse the head of optstring for flags: "+-:"
	for (const char *p = optstring; *p != '\0'; ++p)
	{
		if ((*p == '+') || (*p == '-'))
		{
			permute = 0;
			argofone = (*p == '-');
		}
		else if (*p == ':')
			missingarg = 0;
		else
			break;

		optstring = p + 1;
	}

	optarg = 0;

	if ((optind >= argc) || (0 == argv))
	{
		optind = argc;
		if (permute)
			getopt_permute(argc, argv, optstring, longopts, only);
		return -1;
	}

	// Check whether optind has been changed since the last valid getopt_impl call.
	// Reset optcur to NULL if does.
	if (optcur)
	{
		char inrange = 0;
		for (char * p = argv[optind]; *p != '\0'; ++p)
		{
			if (p != optcur) continue;
			inrange = 1;
			break;
		}

		if (!inrange)
			optcur = 0;
	}

	// Check the content of argv[optind], to see if it is:
	//   1. Non-option argument (not start with '-')
	//   2. A solely '-' or '--'
	//   3. A option which starts with '-' or '--'
	while (optind < argc)
	{
		// case 1
		if (!optcur && (argv[optind][0] != '-'))
		{
			if (argofone)
			{
				optarg = argv[optind++];
				return 1;
			}
			else if (permute)
			{
				optind++;
				continue;
			}
			return -1;
		}

		// case 2
		if (!optcur &&
			((0 == strcmp("-", argv[optind])) ||
			 (0 == strcmp("--", argv[optind]))))
		{
			optind++;
			break;
		}

		// case 3
		const char *p = (argv[optind][1] == '-')? &argv[optind][2] : &argv[optind][1];

		// matching long opts
		do
		{
			if (!longopts || optcur || ((argv[optind][1] != '-') && !only))
				break; // continue to match short opts ...

			const char *r;
			r = strchr(p, '=');
			int plen = (r)? (int)(r-p) : (int)strlen(p);

			const struct option *xo = 0;
			int hitcnt = 0;
			for (const struct option *x = longopts; ; ++x)
			{
				if (!x->name && !x->flag && !x->has_arg && !x->val) // terminating case: option == {0,0,0,0}
					break;

				if (!x->name)
					continue;

				// Long option names may be abbreviated if the abbreviation
				// is unique or is an exact match for some defined option.
				if (0 == strncmp(p, x->name, plen))
				{
					xo = x;
					hitcnt++;
					// is this an exact match?
					if (plen == strlen(x->name))
					{
						hitcnt = 1;
						break;
					}
				}
			}

			if (!xo) // unrecognized option
			{
				if (only)
					break; // continue to match short opts..

				if (opterr)
					fprintf(stderr, "%s: unknown option \"%s\"\n\n", progname, argv[optind]);
				optind++;
				return '?';
			}
			else if (hitcnt > 1) // ambiguous
			{
				if ((only) && (plen == 1))
					break; // continue to match short opts..

				if (opterr)
					fprintf(stderr, "%s: option \"%s\" is ambiguous\n\n", progname, argv[optind]);
				optind++;
				return '?';
			}

			if (longindex)
				*longindex = (int)(xo - longopts);

			switch (xo->has_arg)
			{
			case required_argument: // argument required
			case optional_argument: // optional argument
				{
					char valid = 1;
					int curind = optind++;

					// Check whether the argument is provided as inline. i.e.: '--opt=argument'
					const char *q = strchr(p, '=');
					if (q) // Has inline argument.
					{
						optarg = (char*)(q+1);
					}
					else if (xo->has_arg == optional_argument) // No inline argument implies no argument for 'optional_argument'
					{
						;
					}
					else if (optind < argc) // For required_argument, use the next arg value if no inline argument present.
					{
						optarg = argv[optind++];
					}
					else // Missing argument.
					{
						valid = 0;
					}

					if (valid)
					{
						if (xo->flag == 0)
							return xo->val;
						*xo->flag = xo->val;
						return 0;
					}

					if (opterr)
						fprintf(stderr, "%s: option \"%s\" requires an argument\n\n", progname, argv[curind]);
					if (missingarg)
						return ':';
				}
				return '?';

			default: // no argument
				optind++;
				if (xo->flag == 0)
					return xo->val;
				*xo->flag = xo->val;
				return 0;
			}
		} while (0);

		// matching short opts ...
		if (optcur)
			p = optcur;
		optarg = 0;
		const char *d = strchr(optstring, *p);
		if (d && ((0 == strncmp(d+1, "::", 2)) || (*(d+1) == ':')))
		{
			optind++;
			optarg = 0;
			optcur = 0;
			const char required = (*(d+2) != ':') ? 1 : 0;
			if (*(p+1) != '\0')
			{
				optarg = (char*)(p+1);
			}
			else if ((optind < argc) && (required || (argv[optind][0] != '-')))
			{
				optarg = argv[optind];
				optind++;
			}
			else if (required)
			{
				if (opterr)
					fprintf(stderr, "%s: option \"-%c\" requires an argument\n\n", progname, *p);
				optopt = *p;
				return (missingarg)? ':': '?';
			}
			return *p;
		}

		if (*(p+1) != '\0')
		{
			optcur = (char*)(p+1);
		}
		else
		{
			optind++;
			optcur = 0;
		}

		if (!d)
		{
			optopt = *p;
			if (!missingarg && opterr)
				fprintf(stderr, "%s: unknown option -- %c\n\n", progname, *p);
			return '?';
		}

		return *p;
	} // end of while (optind < argc)

	if (permute)
		getopt_permute(argc, argv, optstring, longopts, only);

	return -1;
}

int getopt(int argc, char * const argv[], const char *optstring)
{
	return getopt_impl(argc, argv, optstring, 0, 0, 0);
}

int getopt_long(int argc, char * const argv[], const char *optstring,
	const struct option *longopts, int *longindex)
{
	return getopt_impl(argc, argv, optstring, longopts, longindex, 0);
}

int getopt_long_only(int argc, char * const argv[], const char *optstring,
	const struct option *longopts, int *longindex)
{
	return getopt_impl(argc, argv, optstring, longopts, longindex, 1);
}

int getsubopt(char **optionp, char * const *tokens, char **valuep)
{
	char *p = *optionp;
	char *n = 0;
	char *a = 0;
	int ret = -1;
	int nlen = 0;
	char stop = 0;

	for (n=p; !stop; ++n)
	{
		switch(*n)
		{
		case '=':
			if (!a) a = n + 1;
			break;
		case ',':
		case '\0':
			*optionp = (*n == ',')? n+1 : n;
			*n = '\0';
			stop = 1;
			break;
		default:
			if (!a) nlen++;
			break;
		}
	}

	char found = 0;
	for (int i=0; (nlen > 0) && (tokens[i] != NULL); ++i)
	{
		if (0 == strncmp(tokens[i], p, nlen))
		{
			found = 1;
			ret = i;
			break;
		}
	}

	*valuep = (found)? a: p;
	return ret;
}

