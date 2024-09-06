#include <string.h>

const char *x = "Icks";
const char *y = "Uepsilon";

int main()
{
#ifdef __HUCC__
  /* HuCC replaces strncpy()/strncat() with strlcpy()/strlcat() */

  char a[20];
  a[0] = 0;
  strlcpy(a, y, 20);
  if (strcmp(a, y))
    exit(1);
  if (strlcat(a, y, 20) != 16)
    exit(2);
  if (strcmp(a, "UepsilonUepsilon"))
    exit(3);
  if (strlcat(a, y, 20) < 20)
    exit(4);
  if (strcmp(a, "UepsilonUepsilonUep"))
    exit(5);
  if (strlcat(a, x, 8) != 8)
    exit(6);
  if (strcmp(a, "UepsilonUepsilonUep"))
    exit(7);
  if (strlcpy(a, x, 4) < 4)
    exit(8);
  if (strcmp(a, "Ick"))
    exit(9);
  exit(0);

#else

  // N.B. HuC has non-standard behavior & return values.
  char a[20];
  a[0] = 0;
  strncat(a, x, 6);
  if (strcmp(a, x))
    exit(1);
  strncat(a, x, 6);
  if (strcmp(a, "IcksIcks"))
    exit(2);
  a[0] = 0;
  if (strncat(a, x,   2) != a + 2) /* standard wants : a */
    exit(3);
  if (strcmp(a, "Ic"))
    exit(4);
  if (strncpy(a, y, 250) != a + 8) /* standard wants : a */
    exit(5);
  if (strcmp(a, y))
    exit(6);
  strncpy(a, x, 2);
  if (strcmp(a, "Ic")) /* standard wants : "Icpsilon" */
    exit(7);
  exit(0);

#endif
}
