/* PR tree-optimization/19828 */
//typedef __SIZE_TYPE__ size_t;

#include <string.h>

#define size_t unsigned int
extern void abort (void);

const char *a[16] = { "a", "bc", "de", "fgh" };

int
foo (char *x, const char *y, size_t n)
{
  size_t i, j; j = 0;
  for (i = 0; i < n; i++)
    {
      if (strncmp (x + j, a[i], strlen (a[i])) != 0)
        return 2;
      j += strlen (a[i]);
      if (y)
        j += strlen (y);
    }
  return 0;
}

int
main (void)
{
  if (foo ("abcde", /*(const char *)*/ 0, 3) != 0)
    abort ();
  return 0;
}
