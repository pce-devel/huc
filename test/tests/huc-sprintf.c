#ifdef __HUCC__

#include <huc.h>
#include <stdio.h>

int which = 0;

unsigned char test[16];

// These are the results that GCC gives, reduced to 16-bit int.
//
// HuCC's code follows GCC's basic rules, except for "%u" with a '+'.
//
// See https://man7.org/linux/man-pages/man3/fprintf.3.html
//
// If both '+' and ' ' flags are used, then '+' overrides the ' ' flag.
//
// "%u" is unsigned and treats '+' as ' ' (GCC just ignores the '+').
//
// "%x", "%X", "%c" and "%s" all ignore the '+' and ' ' flags.

const unsigned char *result[] = {
	// Strings.

	"=hello=\n",		// "=hello=\n"	"=%s=\n"
	"==\n",			// "==\n"	"=%.s=\n"
	"=h=\n",		// "=h=\n"	"=%.*s=\n"

	// Simple decimal with no <width> or <precision>.

	"=-33=\n",		// "=-33=\n"	"=%d=\n"
	"=33=\n",		// "=33=\n"	"=%d=\n"
	"=-33=\n",		// "=-33=\n"	"=%+d=\n"
	"=+33=\n",		// "=+33=\n"	"=%+d=\n"
	"=-33=\n",		// "=-33=\n"	"=% d=\n"
	"= 33=\n",		// "= 33=\n"	"=% d=\n"
	"=-33=\n",		// "=-33=\n"	"=%+ d=\n"
	"=+33=\n",		// "=+33=\n"	"=% +d=\n"

	// Signed decimal.

	"= -03=\n",		// "= -03=\n"	"=%4.2d=\n"
	"= +03=\n",		// "= +03=\n"	"=%+4.2d=\n"
	"= +03=\n",		// "= +03=\n"	"=%+04.2d=\n"
	"= -03=\n",		// "= -03=\n"	"=% 04.2d=\n"
	"=  -3=\n",		// "=  -3=\n"	"=%+4d=\n"
	"=  +3=\n",		// "=  +3=\n"	"=%+4d=\n"
	"=-003=\n",		// "=-003=\n"	"=%+04d=\n"
	"=+003=\n",		// "=+003=\n"	"=%+04d=\n"
	"= 003=\n",		// "= 003=\n"	"=% 04d=\n"
	"=+003=\n",		// "=+003=\n"	"=%+ 04d=\n"
	"=+003=\n",		// "=+003=\n"	"=% +04d=\n"
	"=0003=\n",		// "=0003=\n"	"=%04d=\n"

	// Unsigned decimal.

	"=65533=\n",		// "=65533=\n"	"=%4.2u=\n"
	"=  03=\n",		// "=  03=\n"	"=%+4.2u=\n"
	"=  03=\n",		// "=  03=\n"	"=%+04.2u=\n"
	"=  03=\n",		// "=  03=\n"	"=% 04.2u=\n"
	"= 65533=\n",		// "=65533=\n"	"=%+4u=\n"	HuCC is different to GCC!
	"=   3=\n",		// "=   3=\n"	"=%+4u=\n"
	"= 65533=\n",		// "=65533=\n"	"=%+04u=\n"	HuCC is different to GCC!
	"= 003=\n",		// "=0003=\n"	"=%+04u=\n"	HuCC is different to GCC!
	"= 003=\n",		// "= 003=\n"	"=% 04u=\n"
	"= 003=\n",		// "= 003=\n"	"=%+ 04u=\n"
	"= 003=\n",		// "= 003=\n"	"=% +04u=\n"
	"=0003=\n",		// "=0003=\n"	"=%04u=\n"

	// Hexadecimal.

	"=fffd=\n",		// "=fffd=\n"	"=%4.2x=\n"
	"=  03=\n",		// "=  03=\n"	"=%+4.2x=\n"
	"=  03=\n",		// "=  03=\n"	"=%+04.2x=\n"
	"=  03=\n",		// "=  03=\n"	"=% 04.2x=\n"
	"=fffd=\n",		// "=fffd=\n"	"=%+4x=\n"
	"=   3=\n",		// "=   3=\n"	"=%+4x=\n"
	"=FFFD=\n",		// "=FFFD=\n"	"=%+04X=\n"
	"=0003=\n",		// "=0003=\n"	"=%+04X=\n"
	"=0003=\n",		// "=0003=\n"	"=% 04X=\n"
	"=0003=\n",		// "=0003=\n"	"=%+ 04X=\n"
	"=0003=\n",		// "=0003=\n"	"=% +04X=\n"
	"=0003=\n",		// "=0003=\n"	"=%04X=\n"

	// Characters.

	"=   A=\n",		// "=   A=\n"	"=%4c=\n"
	"=   A=\n",		// "=   A=\n"	"=%04c=\n"
	"=   A=\n",		// "=   A=\n"	"=%4.4c=\n"

	"=   A=\n",		// "=   A=\n"	"=%+4c=\n"
	"=   A=\n",		// "=   A=\n"	"=% 04c=\n"
	"=   A=\n",		// "=   A=\n"	"=%+ 4.4c=\n"

	// Signed decimal left-justified.

	"=-03 =\n",		// "=-03 =\n"	"=%-*.2d=\n"
	"=+03 =\n",		// "=+03 =\n"	"=%-+*.2d=\n"
	"=+03 =\n",		// "=+03 =\n"	"=%-+0*.2d=\n"
	"=-03 =\n",		// "=-03 =\n"	"=%- 0*.2d=\n"
	"=-3  =\n",		// "=-3  =\n"	"=%-+*d=\n"
	"=+3  =\n",		// "=+3  =\n"	"=%-+*d=\n"
	"=-3  =\n",		// "=-3  =\n"	"=%-+0*d=\n"
	"=+3  =\n",		// "=+3  =\n"	"=%-+0*d=\n"
	"= 3  =\n",		// "= 3  =\n"	"=%- 0*d=\n"
	"=+3  =\n",		// "=+3  =\n"	"=%-+ 0*d=\n"
	"=+3  =\n",		// "=+3  =\n"	"=%- +0*d=\n"
	"=3   =\n",		// "=3   =\n"	"=%-0*d=\n"

	// Unsigned decimal left-justified.

	"=65533=\n",		// "=65533=\n"	"=%-4.*u=\n"
	"= 03 =\n",		// "=03  =\n"	"=%-+4.*u=\n"	HuCC is different to GCC!
	"= 03 =\n",		// "=03  =\n"	"=%-+04.*u=\n"	HuCC is different to GCC!
	"= 03 =\n",		// "= 03 =\n"	"=%- 04.*u=\n"
	"= 65533=\n",		// "=65533=\n"	"=%-+4u=\n"	HuCC is different to GCC!
	"= 3  =\n",		// "=3   =\n"	"=%-+4u=\n"	HuCC is different to GCC!
	"= 65533=\n",		// "=65533=\n"	"=%-+04u=\n"
	"= 3  =\n",		// "=3   =\n"	"=%-+04u=\n"	HuCC is different to GCC!
	"= 3  =\n",		// "= 3  =\n"	"=%- 04u=\n"
	"= 3  =\n",		// "= 3  =\n"	"=%-+ 04u=\n"
	"= 3  =\n",		// "= 3  =\n"	"=%- +04u=\n"
	"=3   =\n",		// "=3   =\n"	"=%-04u=\n"

	// Hexadecimal left-justified.

	"=fffd=\n",		// "=fffd=\n"	"=%-*.*x=\n"
	"=03  =\n",		// "=03  =\n"	"=%-+*.*x=\n"
	"=03  =\n",		// "=03  =\n"	"=%-+0*.*x=\n"
	"=03  =\n",		// "=03  =\n"	"=%- 0*.*x=\n"
	"=fffd=\n",		// "=fffd=\n"	"=%-+4x=\n"
	"=3   =\n",		// "=3   =\n"	"=%-+4x=\n"
	"=FFFD=\n",		// "=FFFD=\n"	"=%-+04X=\n"
	"=3   =\n",		// "=3   =\n"	"=%-+04X=\n"
	"=3   =\n",		// "=3   =\n"	"=%- 04X=\n"
	"=3   =\n",		// "=3   =\n"	"=%-+ 04X=\n"
	"=3   =\n",		// "=3   =\n"	"=%- +04X=\n"
	"=3   =\n",		// "=3   =\n"	"=%-04X=\n"

	// Characters left-justifie

	"=A   =\n",		// "=A   =\n"	"=%-4c=\n"
	"=A   =\n",		// "=A   =\n"	"=%-04c=\n"
	"=A   =\n",		// "=A   =\n"	"=%-4.4c=\n"

	"=A   =\n",		// "=A   =\n"	"=%-+4c=\n"
	"=A   =\n",		// "=A   =\n"	"=%- 04c=\n"
	"=A   =\n",		// "=A   =\n"	"=%-+ 4.4c=\n"

	// Stress test printing +/- 123 which is <width>-1 characters.

	"=-123=\n",		// "=-123=\n"	"=%4.2d=\n"
	"=+123=\n",		// "=+123=\n"	"=%+4.2d=\n"
	"=+123=\n",		// "=+123=\n"	"=%+04.2d=\n"
	"=-123=\n",		// "=-123=\n"	"=% 04.2d=\n"
	"=-123=\n",		// "=-123=\n"	"=%+4d=\n"
	"=+123=\n",		// "=+123=\n"	"=%+4d=\n"
	"=-123=\n",		// "=-123=\n"	"=%+04d=\n"
	"=+123=\n",		// "=+123=\n"	"=%+04d=\n"
	"= 123=\n",		// "= 123=\n"	"=% 04d=\n"
	"=+123=\n",		// "=+123=\n"	"=%+ 04d=\n"
	"=+123=\n",		// "=+123=\n"	"=% +04d=\n"
	"=0123=\n",		// "=0123=\n"	"=%04d=\n"

	// Stress test printing +/- 1234 which is <width> characters.

	"=-1234=\n",		// "=-1234=\n"	"=%4.2d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%+4.2d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%+04.2d=\n"
	"=-1234=\n",		// "=-1234=\n"	"=% 04.2d=\n"
	"=-1234=\n",		// "=-1234=\n"	"=%+4d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%+4d=\n"
	"=-1234=\n",		// "=-1234=\n"	"=%+04d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%+04d=\n"
	"= 1234=\n",		// "= 1234=\n"	"=% 04d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%+ 04d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=% +04d=\n"
	"=1234=\n",		// "=1234=\n"	"=%04d=\n"

	// Stress test printing +/- 12345 which is <width>+1 characters.

	"=-12345=\n",		// "=-12345=\n"	"=%4.2d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%+4.2d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%+04.2d=\n"
	"=-12345=\n",		// "=-12345=\n"	"=% 04.2d=\n"
	"=-12345=\n",		// "=-12345=\n"	"=%+4d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%+4d=\n"
	"=-12345=\n",		// "=-12345=\n"	"=%+04d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%+04d=\n"
	"= 12345=\n",		// "= 12345=\n"	"=% 04d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%+ 04d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=% +04d=\n"
	"=12345=\n",		// "=12345=\n"	"=%04d=\n"

	// Stress test left-justified printing +/- 123 which is <width>-1 characters.

	"=-123=\n",		// "=-123=\n"	"=%-4.2d=\n"
	"=+123=\n",		// "=+123=\n"	"=%-+4.2d=\n"
	"=+123=\n",		// "=+123=\n"	"=%-+04.2d=\n"
	"=-123=\n",		// "=-123=\n"	"=%- 04.2d=\n"
	"=-123=\n",		// "=-123=\n"	"=%-+4d=\n"
	"=+123=\n",		// "=+123=\n"	"=%-+4d=\n"
	"=-123=\n",		// "=-123=\n"	"=%-+04d=\n"
	"=+123=\n",		// "=+123=\n"	"=%-+04d=\n"
	"= 123=\n",		// "= 123=\n"	"=%- 04d=\n"
	"=+123=\n",		// "=+123=\n"	"=%-+ 04d=\n"
	"=+123=\n",		// "=+123=\n"	"=%- +04d=\n"
	"=123 =\n",		// "=123 =\n"	"=%-04d=\n"

	// Stress test left-justified printing +/- 1234 which is <width> characters.

	"=-1234=\n",		// "=-1234=\n"	"=%-4.2d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%-+4.2d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%-+04.2d=\n"
	"=-1234=\n",		// "=-1234=\n"	"=%- 04.2d=\n"
	"=-1234=\n",		// "=-1234=\n"	"=%-+4d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%-+4d=\n"
	"=-1234=\n",		// "=-1234=\n"	"=%-+04d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%-+04d=\n"
	"= 1234=\n",		// "= 1234=\n"	"=%- 04d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%-+ 04d=\n"
	"=+1234=\n",		// "=+1234=\n"	"=%- +04d=\n"
	"=1234=\n",		// "=1234=\n"	"=%-04d=\n"

	// Stress test left-justified printing +/- 12345 which is <width>+1 characters.

	"=-12345=\n",		// "=-12345=\n"	"=%-4.2d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%-+4.2d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%-+04.2d=\n"
	"=-12345=\n",		// "=-12345=\n"	"=%- 04.2d=\n"
	"=-12345=\n",		// "=-12345=\n"	"=%-+4d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%-+4d=\n"
	"=-12345=\n",		// "=-12345=\n"	"=%-+04d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%-+04d=\n"
	"= 12345=\n",		// "= 12345=\n"	"=%- 04d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%-+ 04d=\n"
	"=+12345=\n",		// "=+12345=\n"	"=%- +04d=\n"
	"=12345=\n"		// "=12345=\n"	"=%-04d=\n"
	};

//

int test1()
{
	// Signed decimal.

	if (sprintf(test, "=%4.2d=\n", -3) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4.2d=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04.2d=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04.2d=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4d=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4d=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04d=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04d=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04d=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+ 04d=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% +04d=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%04d=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();

	// Unsigned decimal.

	if (sprintf(test, "=%4.2u=\n", -3) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4.2u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04.2u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04.2u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4u=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04u=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+ 04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% +04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();

	// Hexadecimal.

	if (sprintf(test, "=%4.2x=\n", -3) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4.2x=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04.2x=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04.2x=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4x=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4x=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04X=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+ 04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% +04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();

	// Characters.

	if (sprintf(test, "=%4c=\n", 'A') != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%04c=\n", 'A');
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%4.4c=\n", 'A');
	if (strcmp(test, result[which++]) != 0) abort();

	if (sprintf(test, "=%+4c=\n", 'A') != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04c=\n", 'A');
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+ 4.4c=\n", 'A');
	if (strcmp(test, result[which++]) != 0) abort();

	// Do more tests.

	return 0;
}

//

int test2()
{
	// Signed decimal left-justified.

	if (sprintf(test, "=%-*.2d=\n", 4, -3) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+*.2d=\n", 4, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+0*.2d=\n", 4, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 0*.2d=\n", 4, -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+*d=\n", 4, -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+*d=\n", 4, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+0*d=\n", 4, -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+0*d=\n", 4, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 0*d=\n", 4, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+ 0*d=\n", 4, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- +0*d=\n", 4, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-0*d=\n", 4, 3);
	if (strcmp(test, result[which++]) != 0) abort();

	// Unsigned decimal left-justified.

	if (sprintf(test, "=%-4.*u=\n", 2, -3) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4.*u=\n", 2, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04.*u=\n", 2, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04.*u=\n", 2, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4u=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04u=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+ 04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- +04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-04u=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();

	// Hexadecimal left-justified.

	if (sprintf(test, "=%-*.*x=\n", 4, 2, -3) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+*.*x=\n", 4, 2, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+0*.*x=\n", 4, 2, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 0*.*x=\n", 4, 2, 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4x=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4x=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04X=\n", -3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+ 04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- +04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-04X=\n", 3);
	if (strcmp(test, result[which++]) != 0) abort();

	// Characters left-justified.

	if (sprintf(test, "=%-4c=\n", 'A') != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-04c=\n", 'A');
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-4.4c=\n", 'A');
	if (strcmp(test, result[which++]) != 0) abort();

	if (sprintf(test, "=%-+4c=\n", 'A') != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04c=\n", 'A');
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+ 4.4c=\n", 'A');
	if (strcmp(test, result[which++]) != 0) abort();

	// Do more tests.

	return 0;
}

//

int stress_width1()
{
	// Stress test printing +/- 123 which is <width>-1 characters.

	if (sprintf(test, "=%4.2d=\n", -123) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4.2d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04.2d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04.2d=\n", -123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4d=\n", -123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04d=\n", -123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+ 04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% +04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();

	// Stress test printing +/- 1234 which is <width> characters.

	if (sprintf(test, "=%4.2d=\n", -1234) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4.2d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04.2d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04.2d=\n", -1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4d=\n", -1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04d=\n", -1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+ 04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% +04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();

	// Stress test printing +/- 12345 which is <width>+1 characters.

	if (sprintf(test, "=%4.2d=\n", -12345) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4.2d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04.2d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04.2d=\n", -12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4d=\n", -12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+4d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04d=\n", -12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% 04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+ 04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% +04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();

	return 0;
}

//

int stress_width2()
{
	// Stress test left-justified printing +/- 123 which is <width>-1 characters.

	if (sprintf(test, "=%-4.2d=\n", -123) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4.2d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04.2d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04.2d=\n", -123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4d=\n", -123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04d=\n", -123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+ 04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- +04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-04d=\n", 123);
	if (strcmp(test, result[which++]) != 0) abort();

	// Stress test left-justified printing +/- 1234 which is <width> characters.

	if (sprintf(test, "=%-4.2d=\n", -1234) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4.2d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04.2d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04.2d=\n", -1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4d=\n", -1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04d=\n", -1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+ 04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- +04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-04d=\n", 1234);
	if (strcmp(test, result[which++]) != 0) abort();

	// Stress test left-justified printing +/- 12345 which is <width>+1 characters.

	if (sprintf(test, "=%-4.2d=\n", -12345) != strlen(result[which])) abort();
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4.2d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04.2d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04.2d=\n", -12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4d=\n", -12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+4d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04d=\n", -12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- 04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-+ 04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%- +04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%-04d=\n", 12345);
	if (strcmp(test, result[which++]) != 0) abort();

	return 0;
}

//

int main()
{
	// Strings.

	sprintf(test, "=%s=\n", "hello");
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%.s=\n", "hello");
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%.*s=\n", 1, "hello");
	if (strcmp(test, result[which++]) != 0) abort();

	// Simple decimal with no <width> or <precision>.

	sprintf(test, "=%d=\n", -33);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%d=\n",  33);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+d=\n", -33);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+d=\n",  33);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% d=\n", -33);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% d=\n",  33);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=%+ d=\n", -33);
	if (strcmp(test, result[which++]) != 0) abort();
	sprintf(test, "=% +d=\n",  33);
	if (strcmp(test, result[which++]) != 0) abort();

	// Numbers with <width> and/or <precision>.

	test1();

	// Left-justified numbers with <width> and/or <precision>.

	test2();

	// Stress handling numbers that approach <width>.

	stress_width1();

	// Stress handling left-justified numbers that approach <width>.

	stress_width2();

	// Do more tests.

	return 0;
}

#else

/* HuC does not support sprintf() */
int main()
{
	return 0;
}

#endif
