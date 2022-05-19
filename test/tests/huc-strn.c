#include <string.h>

const char *x = "Icks";
const char *y = "Uepsilon";

int main()
{
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
}
