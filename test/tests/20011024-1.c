/* Test whether store motion recognizes pure functions as potentially reading
   any memory.  */

#include <string.h>

//typedef __SIZE_TYPE__ size_t;
#define size_t unsigned int

char buf[50];

static void foo (void)
{
  int l;
#ifdef __HUCC__
  // HuCC has a standard return value from mempcpy().
  if (mempcpy (buf, "abc", 4) != buf + 4) abort ();
#else
  // HuC has a non-standard return value from memcpy().
  if (memcpy (buf, "abc", 4) != buf + 4) abort (); /* standard wants : buf */
#endif
  if (strcmp (buf, "abc")) abort ();
  l = strlen("abcdefgh") + 1;
  memcpy (buf, "abcdefgh", /* strlen ("abcdefgh") + 1 */ l);
  if (strcmp (buf, "abcdefgh")) abort ();
}

int main (void)
{
  foo ();
  return 0;
}
